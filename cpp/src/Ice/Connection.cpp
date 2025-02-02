//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/Connection.h>
#include <IceUtil/PushDisableWarnings.h>
#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>
#include <IceUtil/PopDisableWarnings.h>

#if defined(_MSC_VER)
#   pragma warning(disable:4458) // declaration of ... hides class member
#elif defined(__clang__)
#   pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wshadow"
#endif

Ice::ConnectionInfo::~ConnectionInfo()
{
}

Ice::Connection::~Connection()
{
}

Ice::IPConnectionInfo::~IPConnectionInfo()
{
}

Ice::TCPConnectionInfo::~TCPConnectionInfo()
{
}

Ice::UDPConnectionInfo::~UDPConnectionInfo()
{
}

Ice::WSConnectionInfo::~WSConnectionInfo()
{
}
