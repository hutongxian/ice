//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Ice/UUID.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/Database.h>

#include <IceSSL/Plugin.h>

#include "SynchronizationException.h"

using namespace std;
using namespace IceGrid;

namespace
{

class SubscriberForwarderI : public Ice::BlobjectArrayAsync
{
public:

    SubscriberForwarderI(const Ice::ObjectPrxPtr& proxy) :
        _proxy(proxy)
    {
    }

    void
    ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                    function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                    function<void(exception_ptr)> exception, const Ice::Current& current) override
    {
        _proxy->ice_invokeAsync(current.operation, current.mode, inParams,
                                std::move(response),
                                [exception = std::move(exception)] (exception_ptr)
                                {
                                    // Throw ObjectNotExistException, the subscriber is unreachable
                                    exception(make_exception_ptr(Ice::ObjectNotExistException(__FILE__, __LINE__)));
                                },
                                nullptr, current.ctx);
    }

private:

    const Ice::ObjectPrxPtr _proxy;
};

}

FileIteratorI::FileIteratorI(const shared_ptr<AdminSessionI>& session,
                             const FileReaderPrxPtr& reader,
                             const string& filename,
                             int64_t offset,
                             int messageSizeMax) :
    _session(session),
    _reader(reader),
    _filename(filename),
    _offset(offset),
    _messageSizeMax(messageSizeMax - 256) // Room for the header
{
}

bool
FileIteratorI::read(int size, Ice::StringSeq& lines, const Ice::Current&)
{
    try
    {
        return _reader->read(_filename, _offset, size > _messageSizeMax ? _messageSizeMax : size, _offset, lines);
    }
    catch(const std::exception& ex)
    {
        throw FileNotAvailableException(ex.what());
    }
}

void
FileIteratorI::destroy(const Ice::Current& current)
{
    _session->removeFileIterator(current.id, current);
}

AdminSessionI::AdminSessionI(const string& id, const shared_ptr<Database>& db, chrono::seconds timeout,
                             const shared_ptr<RegistryI>& registry) :
    BaseSessionI(id, "admin", db),
    _timeout(timeout),
    _replicaName(registry->getName()),
    _registry(registry)
{
}

Ice::ObjectPrxPtr
AdminSessionI::_register(const shared_ptr<SessionServantManager>& servantManager,
                         const shared_ptr<Ice::Connection>& con)
{
    //
    // This is supposed to be called after creation only, no need to synchronize.
    //
    _servantManager = servantManager;

    string category;

    if(con)
    {
        category = _database->getInstanceName() + "-" + Ice::generateUUID();

        Ice::Identity templateId;
        templateId.name = "pick-a-name";
        templateId.category = category;

        _adminCallbackTemplate = _registry->createAdminCallbackProxy(templateId);
    }

    auto self = static_pointer_cast<AdminSessionI>(shared_from_this());

    auto session = _servantManager->addSession(self, con, category);

    _admin = Ice::uncheckedCast<AdminPrx>(_servantManager->add(make_shared<AdminI>(_database, _registry, self), self));

    return session;
}

AdminPrxPtr
AdminSessionI::getAdmin(const Ice::Current&) const
{
    return _admin;
}

Ice::ObjectPrxPtr
AdminSessionI::getAdminCallbackTemplate(const Ice::Current&) const
{
    return _adminCallbackTemplate;
}

void
AdminSessionI::setObservers(RegistryObserverPrxPtr registryObserver,
                            NodeObserverPrxPtr nodeObserver,
                            ApplicationObserverPrxPtr appObserver,
                            AdapterObserverPrxPtr adapterObserver,
                            ObjectObserverPrxPtr objectObserver,
                            const Ice::Current& current)
{
    lock_guard lock(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    const auto t = secondsToInt(_timeout);
    assert(t != 0);
    const auto l = _registry->getLocator();
    if(registryObserver)
    {
        setupObserverSubscription(TopicName::RegistryObserver,
                                  addForwarder(registryObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(TopicName::RegistryObserver, nullopt);
    }

    if(nodeObserver)
    {
        setupObserverSubscription(TopicName::NodeObserver,
                                  addForwarder(nodeObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(TopicName::NodeObserver, nullopt);
    }

    if(appObserver)
    {
        setupObserverSubscription(TopicName::ApplicationObserver,
                                  addForwarder(appObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(TopicName::ApplicationObserver, nullopt);
    }

    if(adapterObserver)
    {
        setupObserverSubscription(TopicName::AdapterObserver,
                                  addForwarder(adapterObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(TopicName::AdapterObserver, nullopt);
    }

    if(objectObserver)
    {
        setupObserverSubscription(TopicName::ObjectObserver,
                                  addForwarder(objectObserver->ice_timeout(t)->ice_locator(l)));
    }
    else
    {
        setupObserverSubscription(TopicName::ObjectObserver, nullopt);
    }
}

void
AdminSessionI::setObserversByIdentity(Ice::Identity registryObserver,
                                      Ice::Identity nodeObserver,
                                      Ice::Identity appObserver,
                                      Ice::Identity adapterObserver,
                                      Ice::Identity objectObserver,
                                      const Ice::Current& current)
{
    lock_guard lock(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    setupObserverSubscription(TopicName::RegistryObserver, addForwarder(registryObserver, current), true);
    setupObserverSubscription(TopicName::NodeObserver, addForwarder(nodeObserver, current), true);
    setupObserverSubscription(TopicName::ApplicationObserver, addForwarder(appObserver, current), true);
    setupObserverSubscription(TopicName::AdapterObserver, addForwarder(adapterObserver, current), true);
    setupObserverSubscription(TopicName::ObjectObserver, addForwarder(objectObserver, current), true);
}

int
AdminSessionI::startUpdate(const Ice::Current& current)
{
    lock_guard lock(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    int serial = _database->lock(this, _id);
    return serial;
}

void
AdminSessionI::finishUpdate(const Ice::Current& current)
{
    lock_guard lock(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    _database->unlock(this);
}

string
AdminSessionI::getReplicaName(const Ice::Current&) const
{
    return _replicaName;
}

FileIteratorPrxPtr
AdminSessionI::openServerLog(string id, string path, int nLines, const Ice::Current& current)
{
    try
    {
        return addFileIterator(_database->getServer(std::move(id))->getProxy(false, 5s), "#" + std::move(path), nLines, current);
    }
    catch(const SynchronizationException&)
    {
        throw DeploymentException("server is being updated");
    }
}

FileIteratorPrxPtr
AdminSessionI::openServerStdOut(string id, int nLines, const Ice::Current& current)
{
    try
    {
        return addFileIterator(_database->getServer(std::move(id))->getProxy(false, 5s), "stdout", nLines, current);
    }
    catch(const SynchronizationException&)
    {
        throw DeploymentException("server is being updated");
    }
}

FileIteratorPrxPtr
AdminSessionI::openServerStdErr(string id, int nLines, const Ice::Current& current)
{
    try
    {
        return addFileIterator(_database->getServer(std::move(id))->getProxy(false, 5s), "stderr", nLines, current);
    }
    catch(const SynchronizationException&)
    {
        throw DeploymentException("server is being updated");
    }
}

FileIteratorPrxPtr
AdminSessionI::openNodeStdOut(string name, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getNode(std::move(name))->getProxy(), "stdout", nLines, current);
}

FileIteratorPrxPtr
AdminSessionI::openNodeStdErr(string name, int nLines, const Ice::Current& current)
{
    return addFileIterator(_database->getNode(std::move(name))->getProxy(), "stderr", nLines, current);
}

FileIteratorPrxPtr
AdminSessionI::openRegistryStdOut(string name, int nLines, const Ice::Current& current)
{
    FileReaderPrxPtr reader;
    if(name == _replicaName)
    {
        reader = _database->getReplicaCache().getInternalRegistry();
    }
    else
    {
        reader = _database->getReplica(std::move(name))->getProxy();
    }
    return addFileIterator(reader, "stdout", nLines, current);
}

FileIteratorPrxPtr
AdminSessionI::openRegistryStdErr(string name, int nLines, const Ice::Current& current)
{
    FileReaderPrxPtr reader;
    if(name == _replicaName)
    {
        reader = _database->getReplicaCache().getInternalRegistry();
    }
    else
    {
        reader = _database->getReplica(std::move(name))->getProxy();
    }
    return addFileIterator(reader, "stderr", nLines, current);
}

void
AdminSessionI::destroy(const Ice::Current&)
{
    destroyImpl(false);
}

void
AdminSessionI::setupObserverSubscription(TopicName name, const Ice::ObjectPrxPtr& observer, bool forwarder)
{
    if(_observers.find(name) != _observers.end() && _observers[name].first != observer)
    {
        _database->getObserverTopic(name)->unsubscribe(_observers[name].first);
        if(_observers[name].second)
        {
            try
            {
                // Unregister forwarder object
                _registry->getRegistryAdapter()->remove(_observers[name].first->ice_getIdentity());
            }
            catch(const Ice::ObjectAdapterDeactivatedException&)
            {
            }
        }
        _observers.erase(name);
    }

    if(observer)
    {
        _observers[name].first = observer;
        _observers[name].second = forwarder;
        _database->getObserverTopic(name)->subscribe(observer);
    }
}

Ice::ObjectPrxPtr
AdminSessionI::addForwarder(const Ice::Identity& id, const Ice::Current& current)
{
    if(id.name.empty())
    {
        return nullopt;
    }
    return addForwarder(current.con->createProxy(id)->ice_encodingVersion(current.encoding));
}

Ice::ObjectPrxPtr
AdminSessionI::addForwarder(const Ice::ObjectPrxPtr& prx)
{
    return _registry->getRegistryAdapter()->addWithUUID(make_shared<SubscriberForwarderI>(prx));
}

FileIteratorPrxPtr
AdminSessionI::addFileIterator(const FileReaderPrxPtr& reader, const string& filename, int nLines,
                               const Ice::Current& current)
{
    lock_guard lock(_mutex);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, "", "");
    }

    //
    // Always call getOffsetFromEnd even if nLines < 0. This allows to
    // throw right away if the file doesn't exit.
    //
    int64_t offset;
    try
    {
        offset = reader->getOffsetFromEnd(filename, nLines);
    }
    catch(const std::exception& ex)
    {
        throw FileNotAvailableException(ex.what());
    }

    auto properties = reader->ice_getCommunicator()->getProperties();
    int messageSizeMax = properties->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024) * 1024;

    auto self = static_pointer_cast<AdminSessionI>(shared_from_this());
    auto obj = _servantManager->add(make_shared<FileIteratorI>(self, reader, filename, offset, messageSizeMax), self);
    return Ice::uncheckedCast<FileIteratorPrx>(obj);
}

void
AdminSessionI::removeFileIterator(const Ice::Identity& id, const Ice::Current&)
{
    lock_guard lock(_mutex);

    _servantManager->remove(id);
}

void
AdminSessionI::destroyImpl(bool shutdown)
{
    BaseSessionI::destroyImpl(shutdown);

    _servantManager->removeSession(shared_from_this());

    try
    {
        _database->unlock(this);
    }
    catch(const AccessDeniedException&)
    {
    }

    if(!shutdown)
    {
        //
        // Unsubscribe from the topics.
        //
        setupObserverSubscription(TopicName::RegistryObserver, nullopt);
        setupObserverSubscription(TopicName::NodeObserver, nullopt);
        setupObserverSubscription(TopicName::ApplicationObserver, nullopt);
        setupObserverSubscription(TopicName::AdapterObserver, nullopt);
        setupObserverSubscription(TopicName::ObjectObserver, nullopt);
    }
}

AdminSessionFactory::AdminSessionFactory(const shared_ptr<SessionServantManager>& servantManager,
                                         const shared_ptr<Database>& database,
                                         const shared_ptr<ReapThread>& reaper,
                                         const shared_ptr<RegistryI>& registry) :
    _servantManager(servantManager),
    _database(database),
    _timeout(registry->getSessionTimeout(Ice::emptyCurrent)),
    _reaper(reaper),
    _registry(registry),
    _filters(false)
{
    if(_servantManager) // Not set if Glacier2 session manager adapter not enabled
    {
        auto props = database->getCommunicator()->getProperties();
        const_cast<bool&>(_filters) = props->getPropertyAsIntWithDefault("IceGrid.Registry.AdminSessionFilters", 0) > 0;
    }
}

Glacier2::SessionPrxPtr
AdminSessionFactory::createGlacier2Session(const string& sessionId, const Glacier2::SessionControlPrxPtr& ctl)
{
    assert(_servantManager);

    auto session = createSessionServant(sessionId);
    auto proxy = session->_register(_servantManager, nullptr);

    chrono::seconds timeout = 0s;
    if(ctl)
    {
        try
        {
            if(_filters)
            {
                _servantManager->setSessionControl(session, ctl, { {"Query", _database->getInstanceName() } });
            }
            timeout = chrono::seconds(ctl->getSessionTimeout());
        }
        catch(const Ice::LocalException& e)
        {
            session->destroy(Ice::Current());

            Ice::Warning out(_database->getTraceLevels()->logger);
            out << "Failed to callback Glacier2 session control object:\n" << e;

            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    _reaper->add(make_shared<SessionReapable<AdminSessionI>>(_database->getTraceLevels()->logger, session), timeout);
    return Ice::uncheckedCast<Glacier2::SessionPrx>(proxy);
}

shared_ptr<AdminSessionI>
AdminSessionFactory::createSessionServant(const string& id)
{
    return make_shared<AdminSessionI>(id, _database, _timeout, _registry);
}

const shared_ptr<TraceLevels>&
AdminSessionFactory::getTraceLevels() const
{
    return _database->getTraceLevels();
}

AdminSessionManagerI::AdminSessionManagerI(const shared_ptr<AdminSessionFactory>& factory) : _factory(factory)
{
}

Glacier2::SessionPrxPtr
AdminSessionManagerI::create(string userId, Glacier2::SessionControlPrxPtr ctl,
                             const Ice::Current&)
{
    return _factory->createGlacier2Session(std::move(userId), std::move(ctl));
}

AdminSSLSessionManagerI::AdminSSLSessionManagerI(const shared_ptr<AdminSessionFactory>& factory) : _factory(factory)
{
}

Glacier2::SessionPrxPtr
AdminSSLSessionManagerI::create(Glacier2::SSLInfo info,
                                Glacier2::SessionControlPrxPtr ctl,
                                const Ice::Current&)
{
    string userDN;
    if(!info.certs.empty()) // TODO: Require userDN?
    {
        try
        {
            auto cert = IceSSL::Certificate::decode(info.certs[0]);
            userDN = cert->getSubjectDN();
        }
        catch(const Ice::Exception& ex)
        {
            // This shouldn't happen, the SSLInfo is supposed to be encoded by Glacier2.
            Ice::Error out(_factory->getTraceLevels()->logger);
            out << "SSL session manager couldn't decode SSL certificates:\n" << ex;
            throw Glacier2::CannotCreateSessionException("internal server error");
        }
    }

    return _factory->createGlacier2Session(std::move(userDN), std::move(ctl));
}
