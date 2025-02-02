//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_EndpointF_h__
#define __Ice_EndpointF_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
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
class IPEndpointInfo;
class TCPEndpointInfo;
class UDPEndpointInfo;
class WSEndpointInfo;
class Endpoint;

}

namespace Ice
{

/**
 * A sequence of endpoints.
 */
using EndpointSeq = ::std::vector<::std::shared_ptr<Endpoint>>;

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

using IPEndpointInfoPtr = ::std::shared_ptr<IPEndpointInfo>;

using TCPEndpointInfoPtr = ::std::shared_ptr<TCPEndpointInfo>;

using UDPEndpointInfoPtr = ::std::shared_ptr<UDPEndpointInfo>;

using WSEndpointInfoPtr = ::std::shared_ptr<WSEndpointInfo>;

using EndpointPtr = ::std::shared_ptr<Endpoint>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
