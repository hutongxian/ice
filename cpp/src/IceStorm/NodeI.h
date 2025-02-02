//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef NODE_I_H
#define NODE_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <IceStorm/Election.h>
#include <IceStorm/Replica.h>
#include <IceStorm/Instance.h>
#include <IceUtil/Timer.h>

#include <set>
#include <condition_variable>

namespace IceStormElection
{

class Observers;

class NodeI final : public Node, public std::enable_shared_from_this<NodeI>
{
public:

    NodeI(const std::shared_ptr<IceStorm::Instance>&, std::shared_ptr<Replica>, Ice::ObjectPrxPtr,
          int, const std::map<int, NodePrxPtr>&);

    void start();

    void check();
    void timeout();
    void merge(const std::set<int>&);
    void mergeContinue();
    void invitation(int, std::string, const Ice::Current&) override;
    void ready(int, std::string, Ice::ObjectPrxPtr, int, std::int64_t, const Ice::Current&) override;
    void accept(int, std::string, Ice::IntSeq, Ice::ObjectPrxPtr, LogUpdate, int,
                const Ice::Current&) override;
    bool areYouCoordinator(const Ice::Current&) const override;
    bool areYouThere(std::string, int, const Ice::Current&) const override;
    Ice::ObjectPrxPtr sync(const Ice::Current&) const override;
    NodeInfoSeq nodes(const Ice::Current&) const override;
    QueryInfo query(const Ice::Current&) const override;
    void recovery(std::int64_t = -1);

    void destroy();

    // Notify the node that we're about to start an update.
    void checkObserverInit(std::int64_t);
    Ice::ObjectPrxPtr startUpdate(std::int64_t&, const char*, int);
    Ice::ObjectPrxPtr startCachedRead(std::int64_t&, const char*, int);
    void startObserverUpdate(std::int64_t, const char*, int);
    bool updateMaster(const char*, int);

    // The node has completed the update.
    void finishUpdate();

private:

    void setState(NodeState);

    const IceUtil::TimerPtr _timer;
    const std::shared_ptr<IceStorm::TraceLevels> _traceLevels;
    const std::shared_ptr<IceStormElection::Observers> _observers;
    const std::shared_ptr<Replica> _replica; // The replica.
    const Ice::ObjectPrxPtr _replicaProxy; // A proxy to the individual replica.

    const int _id; // My node id.
    const std::map<int, NodePrxPtr> _nodes; // The nodes indexed by their id.
    const std::map<int, NodePrxPtr> _nodesOneway; // The nodes indexed by their id (as oneway proxies).

    const std::chrono::seconds _masterTimeout;
    const std::chrono::seconds _electionTimeout;
    const std::chrono::seconds _mergeTimeout;

    NodeState _state;
    int _updateCounter;

    int _coord; // Id of the coordinator.
    std::string _group; // My group id.

    std::set<GroupNodeInfo> _up; // Set of nodes in my group.
    std::set<int> _invitesIssued; // The issued invitations.
    std::set<int> _invitesAccepted; // The accepted invitations.

    unsigned int _max; // The highest group count I've seen.
    std::int64_t _generation; // The current generation (or -1 if not set).

    Ice::ObjectPrxPtr _coordinatorProxy;
    bool _destroy;

    IceUtil::TimerTaskPtr _mergeTask;
    IceUtil::TimerTaskPtr _timeoutTask;
    IceUtil::TimerTaskPtr _checkTask;
    IceUtil::TimerTaskPtr _mergeContinueTask;

    mutable std::recursive_mutex _mutex;
    std::condition_variable_any _condVar;
};

class FinishUpdateHelper
{
public:

    FinishUpdateHelper(std::shared_ptr<NodeI> node) :
        _node(std::move(node))
    {
    }

    ~FinishUpdateHelper()
    {
        if(_node)
        {
            _node->finishUpdate();
        }
    }

private:

    const std::shared_ptr<NodeI> _node;
};

class CachedReadHelper
{
public:

    CachedReadHelper(std::shared_ptr<NodeI> node, const char* file, int line) :
        _node(std::move(node))
    {
        if(_node)
        {
            _master = _node->startCachedRead(_generation, file, line);
        }
    }

    ~CachedReadHelper()
    {
        if(_node)
        {
            _node->finishUpdate();
        }
    }

    Ice::ObjectPrxPtr
    getMaster() const
    {
        return _master;
    }

    std::int64_t
    generation() const
    {
        return _generation;
    }

    bool
    observerPrecondition(std::int64_t generation) const
    {
        return generation == _generation && _master;
    }

private:

    const std::shared_ptr<NodeI> _node;
    Ice::ObjectPrxPtr _master;
    std::int64_t _generation;
};

class ObserverUpdateHelper
{
public:

    ObserverUpdateHelper(std::shared_ptr<NodeI> node, std::int64_t generation, const char* file, int line) :
        _node(std::move(node))
    {
        if(_node)
        {
            _node->startObserverUpdate(generation, file, line);
        }
    }

    ~ObserverUpdateHelper()
    {
        if(_node)
        {
            _node->finishUpdate();
        }
    }

private:

    const std::shared_ptr<NodeI> _node;
};

}

#endif // NODE_I_H
