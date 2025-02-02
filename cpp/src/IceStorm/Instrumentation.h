//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceStorm_Instrumentation_h__
#define __IceStorm_Instrumentation_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <Ice/Proxy.h>
#include <Ice/Object.h>
#include <Ice/Value.h>
#include <Ice/Incoming.h>
#include <Ice/FactoryTableInit.h>
#include <optional>
#include <Ice/ExceptionHelpers.h>
#include <Ice/Instrumentation.h>
#include <IceStorm/IceStorm.h>
#include <IceUtil/UndefSysMacros.h>

namespace IceStorm
{

namespace Instrumentation
{

class TopicObserver;
class SubscriberObserver;
class ObserverUpdater;
class TopicManagerObserver;

}

}

namespace IceStorm
{

namespace Instrumentation
{

enum class SubscriberState : unsigned char
{
    /**
     * Online waiting to send events.
     */
    SubscriberStateOnline,
    /**
     * Offline, retrying.
     */
    SubscriberStateOffline,
    /**
     * Error state, awaiting to be destroyed.
     */
    SubscriberStateError
};

}

}

namespace IceStorm
{

namespace Instrumentation
{

class TopicObserver : public virtual ::Ice::Instrumentation::Observer
{
public:

    virtual ~TopicObserver();

    /**
     * Notification of an event published on the topic by a publisher.
     */
    virtual void published() = 0;

    /**
     * Notification of an event forwared on the topic by another topic.
     */
    virtual void forwarded() = 0;
};

class SubscriberObserver : public virtual ::Ice::Instrumentation::Observer
{
public:

    virtual ~SubscriberObserver();

    /**
     * Notification of some events being queued.
     */
    virtual void queued(int count) = 0;

    /**
     * Notification of a some events being sent.
     */
    virtual void outstanding(int count) = 0;

    /**
     * Notification of some events being delivered.
     */
    virtual void delivered(int count) = 0;
};

/**
 * The ObserverUpdater interface is implemented by IceStorm and an instance of this interface is provided on
 * initialization to the TopicManagerObserver object.
 * This interface can be used by add-ins imlementing the TopicManagerObserver interface to update the obsevers of
 * observed objects.
 */
class ObserverUpdater
{
public:

    virtual ~ObserverUpdater();

    /**
     * Update topic observers associated with each topics.
     * When called, this method goes through all the topics and for each topic TopicManagerObserver::getTopicObserver
     * is called. The implementation of getTopicObserver has the possibility to return an updated observer if
     * necessary.
     */
    virtual void updateTopicObservers() = 0;

    /**
     * Update subscriber observers associated with each subscriber.
     * When called, this method goes through all the subscribers and for each subscriber
     * TopicManagerObserver::getSubscriberObserver is called. The implementation of getSubscriberObserver has the
     * possibility to return an updated observer if necessary.
     */
    virtual void updateSubscriberObservers() = 0;
};

/**
 * The topic manager observer interface used by the Ice run-time to obtain and update observers for its observeable
 * objects. This interface should be implemented by add-ins that wish to observe IceStorm objects in order to collect
 * statistics.
 */
class TopicManagerObserver
{
public:

    virtual ~TopicManagerObserver();

    /**
     * This method should return an observer for the given topic.
     * @param svc The service name.
     * @param name The topic name.
     * @param old The previous observer, only set when updating an existing observer.
     */
    virtual ::std::shared_ptr<::IceStorm::Instrumentation::TopicObserver> getTopicObserver(const ::std::string& svc, const ::std::string& name, const ::std::shared_ptr<TopicObserver>& old) = 0;

    /**
     * This method should return an observer for the given subscriber.
     * @param topic The name of the topic subscribed.
     * @param link The proxy of the linked topic if this subscriber forwards events to a linked topic.
     * @param old The previous observer, only set when updating an existing observer.
     */
    virtual ::std::shared_ptr<::IceStorm::Instrumentation::SubscriberObserver> getSubscriberObserver(const ::std::string& svc, const ::std::string& topic, const ::Ice::ObjectPrxPtr& prx, const ::IceStorm::QoS& q, const ::IceStorm::TopicPrxPtr& link, SubscriberState s, const ::std::shared_ptr<SubscriberObserver>& old) = 0;

    /**
     * IceStorm calls this method on initialization. The add-in implementing this interface can use this object to
     * get IceStorm to re-obtain observers for topics and subscribers.
     * @param updater The observer updater object.
     */
    virtual void setObserverUpdater(const ::std::shared_ptr<ObserverUpdater>& updater) = 0;
};

}

}

/// \cond STREAM
namespace Ice
{

template<>
struct StreamableTraits< ::IceStorm::Instrumentation::SubscriberState>
{
    static const StreamHelperCategory helper = StreamHelperCategoryEnum;
    static const int minValue = 0;
    static const int maxValue = 2;
    static const int minWireSize = 1;
    static const bool fixedLength = false;
};

}
/// \endcond

/// \cond INTERNAL
namespace IceStorm
{

/// \cond INTERNAL
namespace Instrumentation
{

using TopicObserverPtr = ::std::shared_ptr<TopicObserver>;

using SubscriberObserverPtr = ::std::shared_ptr<SubscriberObserver>;

using ObserverUpdaterPtr = ::std::shared_ptr<ObserverUpdater>;

using TopicManagerObserverPtr = ::std::shared_ptr<TopicManagerObserver>;

}
/// \endcond

}
/// \endcond

#endif
