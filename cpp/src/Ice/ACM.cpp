//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ACM.h>
#include <Ice/ConnectionI.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/Instance.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::ACMConfig::ACMConfig(bool server) :
    timeout(chrono::seconds(60)),
    heartbeat(ACMHeartbeat::HeartbeatOnDispatch),
    close(server ? ACMClose::CloseOnInvocation : ACMClose::CloseOnInvocationAndIdle)
{
}

IceInternal::ACMConfig::ACMConfig(const Ice::PropertiesPtr& p,
                                  const Ice::LoggerPtr& l,
                                  const string& prefix,
                                  const ACMConfig& dflt)
{
    string timeoutProperty;
    if((prefix == "Ice.ACM.Client" || prefix == "Ice.ACM.Server") && p->getProperty(prefix + ".Timeout").empty())
    {
        timeoutProperty = prefix; // Deprecated property.
    }
    else
    {
        timeoutProperty = prefix + ".Timeout";
    }

    int timeoutVal = p->getPropertyAsIntWithDefault(timeoutProperty, static_cast<int>(dflt.timeout.count()));
    if(timeoutVal >= 0)
    {
        timeout = chrono::seconds(timeoutVal);
    }
    else
    {
        l->warning("invalid value for property `" + timeoutProperty + "', default value will be used instead");
        timeout = dflt.timeout;
    }

    int hb = p->getPropertyAsIntWithDefault(prefix + ".Heartbeat", static_cast<int>(dflt.heartbeat));
    if(hb >= static_cast<int>(ACMHeartbeat::HeartbeatOff) &&
       hb <= static_cast<int>(ACMHeartbeat::HeartbeatAlways))
    {
        heartbeat = static_cast<Ice::ACMHeartbeat>(hb);
    }
    else
    {
        l->warning("invalid value for property `" + prefix + ".Heartbeat" + "', default value will be used instead");
        heartbeat = dflt.heartbeat;
    }

    int cl = p->getPropertyAsIntWithDefault(prefix + ".Close", static_cast<int>(dflt.close));
    if(cl >= static_cast<int>(ACMClose::CloseOff) &&
       cl <= static_cast<int>(ACMClose::CloseOnIdleForceful))
    {
        close = static_cast<Ice::ACMClose>(cl);
    }
    else
    {
        l->warning("invalid value for property `" + prefix + ".Close" + "', default value will be used instead");
        close = dflt.close;
    }
}

IceInternal::FactoryACMMonitor::FactoryACMMonitor(const InstancePtr& instance, const ACMConfig& config) :
    _instance(instance), _config(config)
{
}

IceInternal::FactoryACMMonitor::~FactoryACMMonitor()
{
    assert(!_instance);
    assert(_connections.empty());
    assert(_changes.empty());
    assert(_reapedConnections.empty());
}

void
IceInternal::FactoryACMMonitor::destroy()
{
    unique_lock lock(_mutex);
    if(!_instance)
    {
        //
        // Ensure all the connections have been cleared, it's important to wait here
        // to prevent the timer destruction in IceInternal::Instance::destroy.
        //
        _conditionVariable.wait(lock, [this] { return _connections.empty(); });
        return;
    }

    //
    // Cancel the scheduled timer task and schedule it again now to clear the
    // connection set from the timer thread.
    //
    if(!_connections.empty())
    {
        _instance->timer()->cancel(shared_from_this());
        _instance->timer()->schedule(shared_from_this(), chrono::seconds::zero());
    }

    _instance = 0;
    _changes.clear();

    //
    // Wait for the connection set to be cleared by the timer thread.
    //
    _conditionVariable.wait(lock, [this] { return _connections.empty(); });
}

void
IceInternal::FactoryACMMonitor::add(const ConnectionIPtr& connection)
{
    if(_config.timeout == chrono::seconds::zero())
    {
        return;
    }

    lock_guard lock(_mutex);
    if(_connections.empty())
    {
        _connections.insert(connection);
        _instance->timer()->scheduleRepeated(
            shared_from_this(),
            chrono::duration_cast<chrono::nanoseconds>(_config.timeout) / 2);
    }
    else
    {
        _changes.push_back(make_pair(connection, true));
    }
}

void
IceInternal::FactoryACMMonitor::remove(const ConnectionIPtr& connection)
{
    if(_config.timeout == chrono::seconds::zero())
    {
        return;
    }

    lock_guard lock(_mutex);
    assert(_instance);
    _changes.push_back(make_pair(connection, false));
}

void
IceInternal::FactoryACMMonitor::reap(const ConnectionIPtr& connection)
{
    lock_guard lock(_mutex);
    _reapedConnections.push_back(connection);
}

ACMMonitorPtr
IceInternal::FactoryACMMonitor::acm(const optional<int>& timeout,
                                    const optional<Ice::ACMClose>& close,
                                    const optional<Ice::ACMHeartbeat>& heartbeat)
{
    lock_guard lock(_mutex);
    assert(_instance);

    ACMConfig config(_config);
    if(timeout)
    {
        config.timeout = chrono::seconds(*timeout);
    }
    if(close)
    {
        config.close = *close;
    }
    if(heartbeat)
    {
        config.heartbeat = *heartbeat;
    }
    return make_shared<ConnectionACMMonitor>(shared_from_this(), _instance->timer(), config);
}

Ice::ACM
IceInternal::FactoryACMMonitor::getACM()
{
    Ice::ACM acm;
    acm.timeout = static_cast<int>(_config.timeout.count());
    acm.close = _config.close;
    acm.heartbeat = _config.heartbeat;
    return acm;
}

void
IceInternal::FactoryACMMonitor::swapReapedConnections(vector<ConnectionIPtr>& connections)
{
    lock_guard lock(_mutex);
    _reapedConnections.swap(connections);
}

void
IceInternal::FactoryACMMonitor::runTimerTask()
{
    {
        lock_guard lock(_mutex);
        if(!_instance)
        {
            _connections.clear();
            _conditionVariable.notify_all();
            return;
        }

        for(vector<pair<ConnectionIPtr, bool> >::const_iterator p = _changes.begin(); p != _changes.end(); ++p)
        {
            if(p->second)
            {
                _connections.insert(p->first);
            }
            else
            {
                _connections.erase(p->first);
            }
        }
        _changes.clear();

        if(_connections.empty())
        {
            _instance->timer()->cancel(shared_from_this());
            return;
        }
    }

    //
    // Monitor connections outside the thread synchronization, so
    // that connections can be added or removed during monitoring.
    //
    auto now = chrono::steady_clock::now();
    for(set<ConnectionIPtr>::const_iterator p = _connections.begin(); p != _connections.end(); ++p)
    {
        try
        {
            (*p)->monitor(now, _config);
        }
        catch(const exception& ex)
        {
            handleException(ex);
        }
        catch(...)
        {
            handleException();
        }
    }
}

void
FactoryACMMonitor::handleException(const exception& ex)
{
    lock_guard lock(_mutex);
    if(!_instance)
    {
        return;
    }

    Error out(_instance->initializationData().logger);
    out << "exception in connection monitor:\n" << ex.what();
}

void
FactoryACMMonitor::handleException()
{
    lock_guard lock(_mutex);
    if(!_instance)
    {
        return;
    }

    Error out(_instance->initializationData().logger);
    out << "unknown exception in connection monitor";
}

IceInternal::ConnectionACMMonitor::ConnectionACMMonitor(const FactoryACMMonitorPtr& parent,
                                                        const IceUtil::TimerPtr& timer,
                                                        const ACMConfig& config) :
    _parent(parent), _timer(timer), _config(config)
{
}

IceInternal::ConnectionACMMonitor::~ConnectionACMMonitor()
{
    assert(!_connection);
}

void
IceInternal::ConnectionACMMonitor::add(const ConnectionIPtr& connection)
{
    lock_guard lock(_mutex);
    assert(!_connection && connection);
    _connection = connection;
    if(_config.timeout != chrono::seconds::zero())
    {
        _timer->scheduleRepeated(
            shared_from_this(),
            chrono::duration_cast<chrono::nanoseconds>(_config.timeout) / 2);
    }
}

void
IceInternal::ConnectionACMMonitor::remove(ICE_MAYBE_UNUSED const ConnectionIPtr& connection)
{
#ifdef _MSC_VER
    UNREFERENCED_PARAMETER(connection);
#endif
    lock_guard lock(_mutex);
    assert(_connection == connection);
    if(_config.timeout != chrono::seconds::zero())
    {
        _timer->cancel(shared_from_this());
    }
    _connection = 0;
}

void
IceInternal::ConnectionACMMonitor::reap(const ConnectionIPtr& connection)
{
    _parent->reap(connection);
}

ACMMonitorPtr
IceInternal::ConnectionACMMonitor::acm(const optional<int>& timeout,
                                       const optional<Ice::ACMClose>& close,
                                       const optional<Ice::ACMHeartbeat>& heartbeat)
{
    return _parent->acm(timeout, close, heartbeat);
}

Ice::ACM
IceInternal::ConnectionACMMonitor::getACM()
{
    Ice::ACM acm;
    acm.timeout = static_cast<int>(_config.timeout.count());
    acm.close = _config.close;
    acm.heartbeat = _config.heartbeat;
    return acm;
}

void
IceInternal::ConnectionACMMonitor::runTimerTask()
{
    Ice::ConnectionIPtr connection;
    {
        lock_guard lock(_mutex);
        if(!_connection)
        {
            return;
        }
        connection = _connection;
    }

    try
    {
        connection->monitor(chrono::steady_clock::now(), _config);
    }
    catch(const exception& ex)
    {
        _parent->handleException(ex);
    }
    catch(...)
    {
        _parent->handleException();
    }
}
