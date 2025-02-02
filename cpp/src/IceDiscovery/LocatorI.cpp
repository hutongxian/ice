//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceDiscovery/LocatorI.h>
#include <IceDiscovery/LookupI.h>

#include <Ice/LocalException.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>

#include <IceUtil/Random.h>

#include <iterator>

using namespace std;
using namespace Ice;
using namespace IceDiscovery;

LocatorRegistryI::LocatorRegistryI(const Ice::CommunicatorPtr& com) :
    _wellKnownProxy(com->stringToProxy("p")->ice_locator(nullopt)->ice_router(nullopt)->ice_collocationOptimized(true))
{
}

void
LocatorRegistryI::setAdapterDirectProxyAsync(string adapterId,
                                              ObjectPrxPtr proxy,
                                              function<void()> response,
                                              function<void(exception_ptr)>,
                                              const Ice::Current&)
{
    lock_guard lock(_mutex);
    if(proxy)
    {
        _adapters[adapterId] = proxy;
    }
    else
    {
        _adapters.erase(adapterId);
    }
    response();
}

void
LocatorRegistryI::setReplicatedAdapterDirectProxyAsync(string adapterId,
                                                        string replicaGroupId,
                                                        ObjectPrxPtr proxy,
                                                        function<void()> response,
                                                        function<void(exception_ptr)>,
                                                        const Ice::Current&)
{
    lock_guard lock(_mutex);
    if(proxy)
    {
        _adapters[adapterId] = proxy;
        map<string, set<string> >::iterator p = _replicaGroups.find(replicaGroupId);
        if(p == _replicaGroups.end())
        {
            p = _replicaGroups.insert(make_pair(replicaGroupId, set<string>())).first;
        }
        p->second.insert(adapterId);
    }
    else
    {
        _adapters.erase(adapterId);
        map<string, set<string> >::iterator p = _replicaGroups.find(replicaGroupId);
        if(p != _replicaGroups.end())
        {
            p->second.erase(adapterId);
            if(p->second.empty())
            {
                _replicaGroups.erase(p);
            }
        }
    }
    response();
}

void
LocatorRegistryI::setServerProcessProxyAsync(string,
                                              ProcessPrxPtr,
                                              function<void()> response,
                                              function<void(exception_ptr)>,
                                              const Ice::Current&)
{
    response();
}

Ice::ObjectPrxPtr
LocatorRegistryI::findObject(const Ice::Identity& id) const
{
    lock_guard lock(_mutex);
    if(id.name.empty())
    {
        return nullopt;
    }

    Ice::ObjectPrxPtr prx = _wellKnownProxy->ice_identity(id);

    vector<string> adapterIds;
    for(map<string, set<string> >::const_iterator p = _replicaGroups.begin(); p != _replicaGroups.end(); ++p)
    {
        try
        {
            prx->ice_adapterId(p->first)->ice_ping();
            adapterIds.push_back(p->first);
        }
        catch(const Ice::Exception&)
        {
            // Ignore
        }
    }

    if(adapterIds.empty())
    {
        for(map<string, Ice::ObjectPrxPtr>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
        {
            try
            {
                prx->ice_adapterId(p->first)->ice_ping();
                adapterIds.push_back(p->first);
            }
            catch(const Ice::Exception&)
            {
                // Ignore
            }
        }
    }

    if(adapterIds.empty())
    {
        return nullopt;
    }

    IceUtilInternal::shuffle(adapterIds.begin(), adapterIds.end());
    return prx->ice_adapterId(adapterIds[0]);
}

Ice::ObjectPrxPtr
LocatorRegistryI::findAdapter(const string& adapterId, bool& isReplicaGroup) const
{
    lock_guard lock(_mutex);

    map<string, Ice::ObjectPrxPtr>::const_iterator p = _adapters.find(adapterId);
    if(p != _adapters.end())
    {
        isReplicaGroup = false;
        return p->second;
    }

    map<string, set<string> >::const_iterator q = _replicaGroups.find(adapterId);
    if(q != _replicaGroups.end())
    {
        Ice::EndpointSeq endpoints;
        Ice::ObjectPrxPtr prx;
        for(set<string>::const_iterator r = q->second.begin(); r != q->second.end(); ++r)
        {
            map<string, Ice::ObjectPrxPtr>::const_iterator s = _adapters.find(*r);
            if(s == _adapters.end())
            {
                continue; // TODO: Inconsistency
            }

            if(!prx)
            {
                prx = s->second;
            }

            Ice::EndpointSeq endpts = s->second->ice_getEndpoints();
            copy(endpts.begin(), endpts.end(), back_inserter(endpoints));
        }

        if(prx)
        {
            isReplicaGroup = true;
            return prx->ice_endpoints(endpoints);
        }
    }

    isReplicaGroup = false;
    return nullopt;
}

LocatorI::LocatorI(const LookupIPtr& lookup, const LocatorRegistryPrxPtr& registry) : _lookup(lookup), _registry(registry)
{
}

void
LocatorI::findObjectByIdAsync(Ice::Identity id,
                              function<void(const ObjectPrxPtr&)> response,
                              function<void(exception_ptr)> ex,
                              const Ice::Current&) const
{
    _lookup->findObject(make_pair(response, ex), id);
}

void
LocatorI::findAdapterByIdAsync(string adapterId,
                               function<void(const ObjectPrxPtr&)> response,
                               function<void(exception_ptr)> ex,
                               const Ice::Current&) const
{
    _lookup->findAdapter(make_pair(response, ex), adapterId);
}

LocatorRegistryPrxPtr
LocatorI::getRegistry(const Current&) const
{
    return _registry;
}
