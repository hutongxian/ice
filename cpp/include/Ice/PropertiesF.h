//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_PropertiesF_h__
#define __Ice_PropertiesF_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <Ice/Proxy.h>
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

class Properties;
class PropertiesAdmin;
class PropertiesAdminPrx;

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

/// \cond INTERNAL
namespace Ice
{

using PropertiesPtr = ::std::shared_ptr<Properties>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
