//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Glacier2_Instrumentation_h__
#define __Glacier2_Instrumentation_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/Instrumentation.h>
#include <IceUtil/UndefSysMacros.h>

namespace Glacier2
{

namespace Instrumentation
{

class SessionObserver;
class ObserverUpdater;
class RouterObserver;

}

}

namespace Glacier2
{

namespace Instrumentation
{

class SessionObserver : public virtual ::Ice::Instrumentation::Observer
{
public:

    virtual ~SessionObserver();

    /**
     * Notification of a forwarded request. This also implies removing the event from the queue.
     * @param client True if client request, false if server request.
     */
    virtual void forwarded(bool client) = 0;

    /**
     * Notification of a queued request.
     * @param client True if client request, false if server request.
     */
    virtual void queued(bool client) = 0;

    /**
     * Notification of a overridden request. This implies adding and removing an event to the queue.
     * @param client True if client request, false if server request.
     */
    virtual void overridden(bool client) = 0;

    /**
     * Notification of a routing table size change.
     * @param delta The size adjustement.
     */
    virtual void routingTableSize(int delta) = 0;
};

/**
 * The ObserverUpdater interface is implemented by Glacier2 and an instance of this interface is provided on
 * initialization to the RouterObserver object.
 * This interface can be used by add-ins imlementing the RouterObserver interface to update the obsevers of observed
 * objects.
 */
class ObserverUpdater
{
public:

    virtual ~ObserverUpdater();

    /**
     * Update the router sessions.
     * When called, this method goes through all the sessions and for each session RouterObserver::getSessionObserver
     * is called. The implementation of getSessionObserver has the possibility to return an updated observer if
     * necessary.
     */
    virtual void updateSessionObservers() = 0;
};

/**
 * The router observer interface used by Glacier2 to obtain and update observers for its observeable objects. This
 * interface should be implemented by add-ins that wish to observe Glacier2 objects in order to collect statistics.
 */
class RouterObserver
{
public:

    virtual ~RouterObserver();

    /**
     * This method should return an observer for the given session.
     * @param id The id of the session (the user id or the SSL DN).
     * @param con The connection associated to the session.
     * @param routingTableSize The size of the routing table for this session.
     * @param old The previous observer, only set when updating an existing observer.
     */
    virtual ::std::shared_ptr<::Glacier2::Instrumentation::SessionObserver> getSessionObserver(const ::std::string& id, const ::std::shared_ptr<::Ice::Connection>& con, int routingTableSize, const ::std::shared_ptr<SessionObserver>& old) = 0;

    /**
     * Glacier2 calls this method on initialization. The add-in implementing this interface can use this object to get
     * Glacier2 to re-obtain observers for topics and subscribers.
     * @param updater The observer updater object.
     */
    virtual void setObserverUpdater(const ::std::shared_ptr<ObserverUpdater>& updater) = 0;
};

}

}

/// \cond INTERNAL
namespace Glacier2
{

/// \cond INTERNAL
namespace Instrumentation
{

using SessionObserverPtr = ::std::shared_ptr<SessionObserver>;

using ObserverUpdaterPtr = ::std::shared_ptr<ObserverUpdater>;

using RouterObserverPtr = ::std::shared_ptr<RouterObserver>;

}
/// \endcond

}
/// \endcond

#endif
