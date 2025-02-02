//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_Endpoint_h__
#define __Ice_Endpoint_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/Version.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/EndpointF.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Ice
{

class EndpointInfo;
class Endpoint;
class IPEndpointInfo;
class TCPEndpointInfo;
class UDPEndpointInfo;
class WSEndpointInfo;
class OpaqueEndpointInfo;

}

namespace Ice
{

/**
 * Base class providing access to the endpoint details.
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) EndpointInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~EndpointInfo();

    EndpointInfo() = default;

    EndpointInfo(const EndpointInfo&) = default;
    EndpointInfo(EndpointInfo&&) = default;
    EndpointInfo& operator=(const EndpointInfo&) = default;
    EndpointInfo& operator=(EndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     */
    EndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress) :
        underlying(underlying),
        timeout(timeout),
        compress(compress)
    {
    }

    /**
     * Returns the type of the endpoint.
     * @return The endpoint type.
     */
    virtual short type() const noexcept = 0;

    /**
     * Returns true if this endpoint is a datagram endpoint.
     * @return True for a datagram endpoint.
     */
    virtual bool datagram() const noexcept = 0;

    /**
     * @return True for a secure endpoint.
     */
    virtual bool secure() const noexcept = 0;

    /**
     * The information of the underyling endpoint or null if there's no underlying endpoint.
     */
    ::std::shared_ptr<::Ice::EndpointInfo> underlying;
    /**
     * The timeout for the endpoint in milliseconds. 0 means non-blocking, -1 means no timeout.
     */
    int timeout;
    /**
     * Specifies whether or not compression should be used if available when using this endpoint.
     */
    bool compress;
};

/**
 * The user-level interface to an endpoint.
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) Endpoint
{
public:

    ICE_MEMBER(ICE_API) virtual ~Endpoint();

    virtual bool operator==(const Endpoint&) const = 0;
    virtual bool operator<(const Endpoint&) const = 0;

    /**
     * Return a string representation of the endpoint.
     * @return The string representation of the endpoint.
     */
    virtual ::std::string toString() const noexcept = 0;

    /**
     * Returns the endpoint information.
     * @return The endpoint information class.
     */
    virtual ::std::shared_ptr<::Ice::EndpointInfo> getInfo() const noexcept = 0;
};

/**
 * Provides access to the address details of a IP endpoint.
 * @see Endpoint
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) IPEndpointInfo : public ::Ice::EndpointInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~IPEndpointInfo();

    IPEndpointInfo() = default;

    IPEndpointInfo(const IPEndpointInfo&) = default;
    IPEndpointInfo(IPEndpointInfo&&) = default;
    IPEndpointInfo& operator=(const IPEndpointInfo&) = default;
    IPEndpointInfo& operator=(IPEndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     * @param host The host or address configured with the endpoint.
     * @param port The port number.
     * @param sourceAddress The source IP address.
     */
    IPEndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress, const ::std::string& host, int port, const ::std::string& sourceAddress) :
        EndpointInfo(underlying, timeout, compress),
        host(host),
        port(port),
        sourceAddress(sourceAddress)
    {
    }

    /**
     * The host or address configured with the endpoint.
     */
    ::std::string host;
    /**
     * The port number.
     */
    int port;
    /**
     * The source IP address.
     */
    ::std::string sourceAddress;
};

/**
 * Provides access to a TCP endpoint information.
 * @see Endpoint
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) TCPEndpointInfo : public ::Ice::IPEndpointInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~TCPEndpointInfo();

    TCPEndpointInfo() = default;

    TCPEndpointInfo(const TCPEndpointInfo&) = default;
    TCPEndpointInfo(TCPEndpointInfo&&) = default;
    TCPEndpointInfo& operator=(const TCPEndpointInfo&) = default;
    TCPEndpointInfo& operator=(TCPEndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     * @param host The host or address configured with the endpoint.
     * @param port The port number.
     * @param sourceAddress The source IP address.
     */
    TCPEndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress, const ::std::string& host, int port, const ::std::string& sourceAddress) :
        IPEndpointInfo(underlying, timeout, compress, host, port, sourceAddress)
    {
    }
};

/**
 * Provides access to an UDP endpoint information.
 * @see Endpoint
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) UDPEndpointInfo : public ::Ice::IPEndpointInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~UDPEndpointInfo();

    UDPEndpointInfo() = default;

    UDPEndpointInfo(const UDPEndpointInfo&) = default;
    UDPEndpointInfo(UDPEndpointInfo&&) = default;
    UDPEndpointInfo& operator=(const UDPEndpointInfo&) = default;
    UDPEndpointInfo& operator=(UDPEndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     * @param host The host or address configured with the endpoint.
     * @param port The port number.
     * @param sourceAddress The source IP address.
     * @param mcastInterface The multicast interface.
     * @param mcastTtl The multicast time-to-live (or hops).
     */
    UDPEndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress, const ::std::string& host, int port, const ::std::string& sourceAddress, const ::std::string& mcastInterface, int mcastTtl) :
        IPEndpointInfo(underlying, timeout, compress, host, port, sourceAddress),
        mcastInterface(mcastInterface),
        mcastTtl(mcastTtl)
    {
    }

    /**
     * The multicast interface.
     */
    ::std::string mcastInterface;
    /**
     * The multicast time-to-live (or hops).
     */
    int mcastTtl;
};

/**
 * Provides access to a WebSocket endpoint information.
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) WSEndpointInfo : public ::Ice::EndpointInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~WSEndpointInfo();

    WSEndpointInfo() = default;

    WSEndpointInfo(const WSEndpointInfo&) = default;
    WSEndpointInfo(WSEndpointInfo&&) = default;
    WSEndpointInfo& operator=(const WSEndpointInfo&) = default;
    WSEndpointInfo& operator=(WSEndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     * @param resource The URI configured with the endpoint.
     */
    WSEndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress, const ::std::string& resource) :
        EndpointInfo(underlying, timeout, compress),
        resource(resource)
    {
    }

    /**
     * The URI configured with the endpoint.
     */
    ::std::string resource;
};

/**
 * Provides access to the details of an opaque endpoint.
 * @see Endpoint
 * \headerfile Ice/Ice.h
 */
class ICE_CLASS(ICE_API) OpaqueEndpointInfo : public ::Ice::EndpointInfo
{
public:

    ICE_MEMBER(ICE_API) virtual ~OpaqueEndpointInfo();

    OpaqueEndpointInfo() = default;

    OpaqueEndpointInfo(const OpaqueEndpointInfo&) = default;
    OpaqueEndpointInfo(OpaqueEndpointInfo&&) = default;
    OpaqueEndpointInfo& operator=(const OpaqueEndpointInfo&) = default;
    OpaqueEndpointInfo& operator=(OpaqueEndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint or null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     * @param rawEncoding The encoding version of the opaque endpoint (to decode or encode the rawBytes).
     * @param rawBytes The raw encoding of the opaque endpoint.
     */
    OpaqueEndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress, const ::Ice::EncodingVersion& rawEncoding, const ::Ice::ByteSeq& rawBytes) :
        EndpointInfo(underlying, timeout, compress),
        rawEncoding(rawEncoding),
        rawBytes(rawBytes)
    {
    }

    /**
     * The encoding version of the opaque endpoint (to decode or encode the rawBytes).
     */
    ::Ice::EncodingVersion rawEncoding;
    /**
     * The raw encoding of the opaque endpoint.
     */
    ::Ice::ByteSeq rawBytes;
};

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

/// \cond INTERNAL
namespace Ice
{

using EndpointInfoPtr = ::std::shared_ptr<EndpointInfo>;

using EndpointPtr = ::std::shared_ptr<Endpoint>;

using IPEndpointInfoPtr = ::std::shared_ptr<IPEndpointInfo>;

using TCPEndpointInfoPtr = ::std::shared_ptr<TCPEndpointInfo>;

using UDPEndpointInfoPtr = ::std::shared_ptr<UDPEndpointInfo>;

using WSEndpointInfoPtr = ::std::shared_ptr<WSEndpointInfo>;

using OpaqueEndpointInfoPtr = ::std::shared_ptr<OpaqueEndpointInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
