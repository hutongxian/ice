//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_PluginF_h__
#define __Ice_PluginF_h__

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

class Plugin;
class PluginManager;

}

/// \cond STREAM
namespace Ice
{

}
/// \endcond

/// \cond INTERNAL
namespace Ice
{

using PluginPtr = ::std::shared_ptr<Plugin>;

using PluginManagerPtr = ::std::shared_ptr<PluginManager>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
