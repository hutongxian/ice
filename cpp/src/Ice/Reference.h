//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REFERENCE_H
#define ICE_REFERENCE_H

#include <Ice/ReferenceF.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/EndpointIF.h>
#include <Ice/InstanceF.h>
#include <Ice/Router.h>
#include <Ice/Locator.h>
#include <Ice/RouterInfoF.h>
#include <Ice/LocatorInfoF.h>
#include <Ice/ConnectionIF.h>
#include <Ice/RequestHandlerF.h>
#include <Ice/BatchRequestQueue.h>
#include <Ice/SharedContext.h>
#include <Ice/Identity.h>
#include <Ice/Protocol.h>
#include <Ice/Properties.h>

#include <mutex>

namespace Ice
{

class OutputStream;

}

namespace IceInternal
{

class Reference : public std::enable_shared_from_this<Reference>
{
public:

    virtual ~Reference() = default;

    enum Mode
    {
        ModeTwoway,
        ModeOneway,
        ModeBatchOneway,
        ModeDatagram,
        ModeBatchDatagram,
        ModeLast = ModeBatchDatagram
    };

    Mode getMode() const { return _mode; }
    bool isBatch() const { return _mode == ModeBatchOneway || _mode == ModeBatchDatagram; }
    bool isTwoway() const { return _mode == ModeTwoway; }
    bool getSecure() const { return _secure; }
    const Ice::ProtocolVersion& getProtocol() const { return _protocol; }
    const Ice::EncodingVersion& getEncoding() const { return _encoding; }
    const Ice::Identity& getIdentity() const { return _identity; }
    const std::string& getFacet() const { return _facet; }
    const InstancePtr& getInstance() const { return _instance; }
    const SharedContextPtr& getContext() const { return _context; }
    int getInvocationTimeout() const { return _invocationTimeout; }
    std::optional<bool> getCompress() const
    {
        return _overrideCompress ? std::optional<bool>(_compress) : std::nullopt;
    }

    Ice::CommunicatorPtr getCommunicator() const;

    virtual std::vector<EndpointIPtr> getEndpoints() const = 0;
    virtual std::string getAdapterId() const = 0;
    virtual LocatorInfoPtr getLocatorInfo() const { return 0; }
    virtual RouterInfoPtr getRouterInfo() const { return 0; }
    virtual bool getCollocationOptimized() const = 0;
    virtual bool getCacheConnection() const = 0;
    virtual bool getPreferSecure() const = 0;
    virtual Ice::EndpointSelectionType getEndpointSelection() const = 0;
    virtual int getLocatorCacheTimeout() const = 0;
    virtual std::string getConnectionId() const = 0;
    virtual std::optional<int> getTimeout() const = 0;

    //
    // The change* methods (here and in derived classes) create
    // a new reference based on the existing one, with the
    // corresponding value changed.
    //
    ReferencePtr changeContext(const Ice::Context&) const;
    ReferencePtr changeSecure(bool) const;
    ReferencePtr changeIdentity(const Ice::Identity&) const;
    ReferencePtr changeFacet(const std::string&) const;
    ReferencePtr changeInvocationTimeout(int) const;

    virtual ReferencePtr changeEncoding(const Ice::EncodingVersion&) const;
    virtual ReferencePtr changeCompress(bool) const;
    virtual ReferencePtr changeMode(Mode) const;
    virtual ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const = 0;
    virtual ReferencePtr changeAdapterId(const std::string&) const = 0;
    virtual ReferencePtr changeLocator(const std::optional<Ice::LocatorPrx>&) const = 0;
    virtual ReferencePtr changeRouter(const std::optional<Ice::RouterPrx>&) const = 0;
    virtual ReferencePtr changeCollocationOptimized(bool) const = 0;
    virtual ReferencePtr changeLocatorCacheTimeout(int) const = 0;
    virtual ReferencePtr changeCacheConnection(bool) const = 0;
    virtual ReferencePtr changePreferSecure(bool) const = 0;
    virtual ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const = 0;

    virtual ReferencePtr changeTimeout(int) const = 0;
    virtual ReferencePtr changeConnectionId(const std::string&) const = 0;
    virtual ReferencePtr changeConnection(const Ice::ConnectionIPtr&) const = 0;

    int hash() const; // Conceptually const.

    bool getCompressOverride(bool&) const;

    //
    // Utility methods.
    //
    virtual bool isIndirect() const = 0;
    virtual bool isWellKnown() const = 0;

    //
    // Marshal the reference.
    //
    virtual void streamWrite(Ice::OutputStream*) const;

    //
    // Convert the reference to its string form.
    //
    virtual std::string toString() const;

    //
    // Convert the reference to its property form.
    //
    virtual Ice::PropertyDict toProperty(const std::string&) const = 0;

    //
    // Get a suitable connection for this reference.
    //
    virtual RequestHandlerPtr getRequestHandler() const = 0;

    //
    // Get batch request queue.
    //
    virtual const BatchRequestQueuePtr& getBatchRequestQueue() const = 0;

    virtual bool operator==(const Reference&) const;
    virtual bool operator<(const Reference&) const;

    virtual ReferencePtr clone() const = 0;

protected:

    Reference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const std::string&, Mode, bool,
              const Ice::ProtocolVersion&, const Ice::EncodingVersion&, int, const Ice::Context& ctx);
    Reference(const Reference&);

    virtual std::int32_t hashInit() const;

    const InstancePtr _instance;
    bool _overrideCompress;
    bool _compress; // Only used if _overrideCompress == true

private:

    const Ice::CommunicatorPtr _communicator;

    Mode _mode;
    bool _secure;
    Ice::Identity _identity;
    SharedContextPtr _context;
    std::string _facet;
    mutable std::int32_t _hashValue;
    mutable bool _hashInitialized;
    mutable std::mutex _hashMutex;
    Ice::ProtocolVersion _protocol;
    Ice::EncodingVersion _encoding;
    int _invocationTimeout;
};

class FixedReference final : public Reference
{
public:

    FixedReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const std::string&, Mode,
                   bool, const Ice::ProtocolVersion&, const Ice::EncodingVersion&, const Ice::ConnectionIPtr&,
                   int, const Ice::Context&, const std::optional<bool>&);

    FixedReference(const FixedReference&);

    std::vector<EndpointIPtr> getEndpoints() const final;
    std::string getAdapterId() const final;
    bool getCollocationOptimized() const final;
    bool getCacheConnection() const final;
    bool getPreferSecure() const final;
    Ice::EndpointSelectionType getEndpointSelection() const final;
    int getLocatorCacheTimeout() const final;
    std::string getConnectionId() const final;
    std::optional<int> getTimeout() const final;

    ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const final;
    ReferencePtr changeAdapterId(const std::string&) const final;
    ReferencePtr changeLocator(const std::optional<Ice::LocatorPrx>&) const final;
    ReferencePtr changeRouter(const std::optional<Ice::RouterPrx>&) const final;
    ReferencePtr changeCollocationOptimized(bool) const final;
    ReferencePtr changeCacheConnection(bool) const final;
    ReferencePtr changePreferSecure(bool) const final;
    ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const final;
    ReferencePtr changeLocatorCacheTimeout(int) const final;

    ReferencePtr changeTimeout(int) const final;
    ReferencePtr changeConnectionId(const std::string&) const final;
    ReferencePtr changeConnection(const Ice::ConnectionIPtr&) const final;

    bool isIndirect() const final;
    bool isWellKnown() const final;

    void streamWrite(Ice::OutputStream*) const final;
    Ice::PropertyDict toProperty(const std::string&) const final;

    RequestHandlerPtr getRequestHandler() const final;
    const BatchRequestQueuePtr& getBatchRequestQueue() const final;

    bool operator==(const Reference&) const final;
    bool operator<(const Reference&) const final;

    ReferencePtr clone() const final;

private:

    Ice::ConnectionIPtr _fixedConnection;
};

using FixedReferencePtr = std::shared_ptr<FixedReference>;

class RoutableReference final : public Reference
{
public:

    RoutableReference(const InstancePtr&, const Ice::CommunicatorPtr&, const Ice::Identity&, const std::string&, Mode,
                      bool, const Ice::ProtocolVersion&, const Ice::EncodingVersion&, const std::vector<EndpointIPtr>&,
                      const std::string&, const LocatorInfoPtr&, const RouterInfoPtr&, bool, bool, bool,
                      Ice::EndpointSelectionType, int, int, const Ice::Context&);

    RoutableReference(const RoutableReference&);

    std::vector<EndpointIPtr> getEndpoints() const final;
    std::string getAdapterId() const final;
    LocatorInfoPtr getLocatorInfo() const final;
    RouterInfoPtr getRouterInfo() const final;
    bool getCollocationOptimized() const final;
    bool getCacheConnection() const final;
    bool getPreferSecure() const final;
    Ice::EndpointSelectionType getEndpointSelection() const final;
    int getLocatorCacheTimeout() const final;
    std::string getConnectionId() const final;
    std::optional<int> getTimeout() const final;

    ReferencePtr changeEncoding(const Ice::EncodingVersion&) const final;
    ReferencePtr changeCompress(bool) const final;
    ReferencePtr changeMode(Mode) const final;
    ReferencePtr changeEndpoints(const std::vector<EndpointIPtr>&) const final;
    ReferencePtr changeAdapterId(const std::string&) const final;
    ReferencePtr changeLocator(const std::optional<Ice::LocatorPrx>&) const final;
    ReferencePtr changeRouter(const std::optional<Ice::RouterPrx>&) const final;
    ReferencePtr changeCollocationOptimized(bool) const final;
    ReferencePtr changeCacheConnection(bool) const final;
    ReferencePtr changePreferSecure(bool) const final;
    ReferencePtr changeEndpointSelection(Ice::EndpointSelectionType) const final;
    ReferencePtr changeLocatorCacheTimeout(int) const final;

    ReferencePtr changeTimeout(int) const final;
    ReferencePtr changeConnectionId(const std::string&) const final;
    ReferencePtr changeConnection(const Ice::ConnectionIPtr&) const final;

    bool isIndirect() const final;
    bool isWellKnown() const final;

    void streamWrite(Ice::OutputStream*) const final;
    std::string toString() const final;
    Ice::PropertyDict toProperty(const std::string&) const final;

    bool operator==(const Reference&) const final;
    bool operator<(const Reference&) const final;

    ReferencePtr clone() const final;

    RequestHandlerPtr getRequestHandler() const final;
    const BatchRequestQueuePtr& getBatchRequestQueue() const final;

    void getConnectionAsync(
        std::function<void(Ice::ConnectionIPtr, bool)> response,
        std::function<void(std::exception_ptr)> exception) const;

    void applyOverrides(std::vector<EndpointIPtr>&) const;

protected:

    std::vector<EndpointIPtr> filterEndpoints(const std::vector<EndpointIPtr>&) const;

    int hashInit() const final;

private:

    void createConnectionAsync(
        const std::vector<EndpointIPtr>&,
        std::function<void(Ice::ConnectionIPtr, bool)> response,
        std::function<void(std::exception_ptr)> exception) const;

    void getConnectionNoRouterInfoAsync(
        std::function<void(Ice::ConnectionIPtr, bool)> response,
        std::function<void(std::exception_ptr)> exception) const;

    // Sets or resets _batchRequestQueue based on _mode.
    void setBatchRequestQueue();

    BatchRequestQueuePtr _batchRequestQueue;

    std::vector<EndpointIPtr> _endpoints; // Empty if indirect proxy.
    std::string _adapterId; // Empty if direct proxy.

    LocatorInfoPtr _locatorInfo; // Null if no locator is used.
    RouterInfoPtr _routerInfo; // Null if no router is used.
    bool _collocationOptimized;
    bool _cacheConnection;
    bool _preferSecure;
    Ice::EndpointSelectionType _endpointSelection;
    int _locatorCacheTimeout;

    bool _overrideTimeout;
    int _timeout; // Only used if _overrideTimeout == true
    std::string _connectionId;
};

using RoutableReferencePtr = std::shared_ptr<RoutableReference>;

}

#endif
