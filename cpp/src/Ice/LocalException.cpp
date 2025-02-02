//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif
#include <Ice/LocalException.h>
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

Ice::InitializationException::~InitializationException()
{
}

const ::std::string&
Ice::InitializationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::InitializationException";
    return typeId;
}

Ice::PluginInitializationException::~PluginInitializationException()
{
}

const ::std::string&
Ice::PluginInitializationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::PluginInitializationException";
    return typeId;
}

Ice::CollocationOptimizationException::~CollocationOptimizationException()
{
}

const ::std::string&
Ice::CollocationOptimizationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CollocationOptimizationException";
    return typeId;
}

Ice::AlreadyRegisteredException::~AlreadyRegisteredException()
{
}

const ::std::string&
Ice::AlreadyRegisteredException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::AlreadyRegisteredException";
    return typeId;
}

Ice::NotRegisteredException::~NotRegisteredException()
{
}

const ::std::string&
Ice::NotRegisteredException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::NotRegisteredException";
    return typeId;
}

Ice::TwowayOnlyException::~TwowayOnlyException()
{
}

const ::std::string&
Ice::TwowayOnlyException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::TwowayOnlyException";
    return typeId;
}

Ice::CloneNotImplementedException::~CloneNotImplementedException()
{
}

const ::std::string&
Ice::CloneNotImplementedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CloneNotImplementedException";
    return typeId;
}

Ice::UnknownException::~UnknownException()
{
}

const ::std::string&
Ice::UnknownException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownException";
    return typeId;
}

Ice::UnknownLocalException::~UnknownLocalException()
{
}

const ::std::string&
Ice::UnknownLocalException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownLocalException";
    return typeId;
}

Ice::UnknownUserException::~UnknownUserException()
{
}

const ::std::string&
Ice::UnknownUserException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownUserException";
    return typeId;
}

Ice::VersionMismatchException::~VersionMismatchException()
{
}

const ::std::string&
Ice::VersionMismatchException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::VersionMismatchException";
    return typeId;
}

Ice::CommunicatorDestroyedException::~CommunicatorDestroyedException()
{
}

const ::std::string&
Ice::CommunicatorDestroyedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CommunicatorDestroyedException";
    return typeId;
}

Ice::ObjectAdapterDeactivatedException::~ObjectAdapterDeactivatedException()
{
}

const ::std::string&
Ice::ObjectAdapterDeactivatedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ObjectAdapterDeactivatedException";
    return typeId;
}

Ice::ObjectAdapterIdInUseException::~ObjectAdapterIdInUseException()
{
}

const ::std::string&
Ice::ObjectAdapterIdInUseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ObjectAdapterIdInUseException";
    return typeId;
}

Ice::NoEndpointException::~NoEndpointException()
{
}

const ::std::string&
Ice::NoEndpointException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::NoEndpointException";
    return typeId;
}

Ice::EndpointParseException::~EndpointParseException()
{
}

const ::std::string&
Ice::EndpointParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::EndpointParseException";
    return typeId;
}

Ice::EndpointSelectionTypeParseException::~EndpointSelectionTypeParseException()
{
}

const ::std::string&
Ice::EndpointSelectionTypeParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::EndpointSelectionTypeParseException";
    return typeId;
}

Ice::VersionParseException::~VersionParseException()
{
}

const ::std::string&
Ice::VersionParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::VersionParseException";
    return typeId;
}

Ice::IdentityParseException::~IdentityParseException()
{
}

const ::std::string&
Ice::IdentityParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IdentityParseException";
    return typeId;
}

Ice::ProxyParseException::~ProxyParseException()
{
}

const ::std::string&
Ice::ProxyParseException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ProxyParseException";
    return typeId;
}

Ice::IllegalIdentityException::~IllegalIdentityException()
{
}

const ::std::string&
Ice::IllegalIdentityException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IllegalIdentityException";
    return typeId;
}

Ice::IllegalServantException::~IllegalServantException()
{
}

const ::std::string&
Ice::IllegalServantException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IllegalServantException";
    return typeId;
}

Ice::RequestFailedException::~RequestFailedException()
{
}

const ::std::string&
Ice::RequestFailedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::RequestFailedException";
    return typeId;
}

Ice::ObjectNotExistException::~ObjectNotExistException()
{
}

const ::std::string&
Ice::ObjectNotExistException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ObjectNotExistException";
    return typeId;
}

Ice::FacetNotExistException::~FacetNotExistException()
{
}

const ::std::string&
Ice::FacetNotExistException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FacetNotExistException";
    return typeId;
}

Ice::OperationNotExistException::~OperationNotExistException()
{
}

const ::std::string&
Ice::OperationNotExistException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::OperationNotExistException";
    return typeId;
}

Ice::SyscallException::~SyscallException()
{
}

const ::std::string&
Ice::SyscallException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::SyscallException";
    return typeId;
}

Ice::SocketException::~SocketException()
{
}

const ::std::string&
Ice::SocketException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::SocketException";
    return typeId;
}

Ice::CFNetworkException::~CFNetworkException()
{
}

const ::std::string&
Ice::CFNetworkException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CFNetworkException";
    return typeId;
}

Ice::FileException::~FileException()
{
}

const ::std::string&
Ice::FileException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FileException";
    return typeId;
}

Ice::ConnectFailedException::~ConnectFailedException()
{
}

const ::std::string&
Ice::ConnectFailedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectFailedException";
    return typeId;
}

Ice::ConnectionRefusedException::~ConnectionRefusedException()
{
}

const ::std::string&
Ice::ConnectionRefusedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionRefusedException";
    return typeId;
}

Ice::ConnectionLostException::~ConnectionLostException()
{
}

const ::std::string&
Ice::ConnectionLostException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionLostException";
    return typeId;
}

Ice::DNSException::~DNSException()
{
}

const ::std::string&
Ice::DNSException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::DNSException";
    return typeId;
}

Ice::OperationInterruptedException::~OperationInterruptedException()
{
}

const ::std::string&
Ice::OperationInterruptedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::OperationInterruptedException";
    return typeId;
}

Ice::TimeoutException::~TimeoutException()
{
}

const ::std::string&
Ice::TimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::TimeoutException";
    return typeId;
}

Ice::ConnectTimeoutException::~ConnectTimeoutException()
{
}

const ::std::string&
Ice::ConnectTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectTimeoutException";
    return typeId;
}

Ice::CloseTimeoutException::~CloseTimeoutException()
{
}

const ::std::string&
Ice::CloseTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CloseTimeoutException";
    return typeId;
}

Ice::ConnectionTimeoutException::~ConnectionTimeoutException()
{
}

const ::std::string&
Ice::ConnectionTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionTimeoutException";
    return typeId;
}

Ice::InvocationTimeoutException::~InvocationTimeoutException()
{
}

const ::std::string&
Ice::InvocationTimeoutException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::InvocationTimeoutException";
    return typeId;
}

Ice::InvocationCanceledException::~InvocationCanceledException()
{
}

const ::std::string&
Ice::InvocationCanceledException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::InvocationCanceledException";
    return typeId;
}

Ice::ProtocolException::~ProtocolException()
{
}

const ::std::string&
Ice::ProtocolException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ProtocolException";
    return typeId;
}

Ice::BadMagicException::~BadMagicException()
{
}

const ::std::string&
Ice::BadMagicException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::BadMagicException";
    return typeId;
}

Ice::UnsupportedProtocolException::~UnsupportedProtocolException()
{
}

const ::std::string&
Ice::UnsupportedProtocolException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnsupportedProtocolException";
    return typeId;
}

Ice::UnsupportedEncodingException::~UnsupportedEncodingException()
{
}

const ::std::string&
Ice::UnsupportedEncodingException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnsupportedEncodingException";
    return typeId;
}

Ice::UnknownMessageException::~UnknownMessageException()
{
}

const ::std::string&
Ice::UnknownMessageException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownMessageException";
    return typeId;
}

Ice::ConnectionNotValidatedException::~ConnectionNotValidatedException()
{
}

const ::std::string&
Ice::ConnectionNotValidatedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionNotValidatedException";
    return typeId;
}

Ice::UnknownRequestIdException::~UnknownRequestIdException()
{
}

const ::std::string&
Ice::UnknownRequestIdException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownRequestIdException";
    return typeId;
}

Ice::UnknownReplyStatusException::~UnknownReplyStatusException()
{
}

const ::std::string&
Ice::UnknownReplyStatusException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnknownReplyStatusException";
    return typeId;
}

Ice::CloseConnectionException::~CloseConnectionException()
{
}

const ::std::string&
Ice::CloseConnectionException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CloseConnectionException";
    return typeId;
}

Ice::ConnectionManuallyClosedException::~ConnectionManuallyClosedException()
{
}

const ::std::string&
Ice::ConnectionManuallyClosedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ConnectionManuallyClosedException";
    return typeId;
}

Ice::IllegalMessageSizeException::~IllegalMessageSizeException()
{
}

const ::std::string&
Ice::IllegalMessageSizeException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::IllegalMessageSizeException";
    return typeId;
}

Ice::CompressionException::~CompressionException()
{
}

const ::std::string&
Ice::CompressionException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::CompressionException";
    return typeId;
}

Ice::DatagramLimitException::~DatagramLimitException()
{
}

const ::std::string&
Ice::DatagramLimitException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::DatagramLimitException";
    return typeId;
}

Ice::MarshalException::~MarshalException()
{
}

const ::std::string&
Ice::MarshalException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::MarshalException";
    return typeId;
}

Ice::ProxyUnmarshalException::~ProxyUnmarshalException()
{
}

const ::std::string&
Ice::ProxyUnmarshalException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ProxyUnmarshalException";
    return typeId;
}

Ice::UnmarshalOutOfBoundsException::~UnmarshalOutOfBoundsException()
{
}

const ::std::string&
Ice::UnmarshalOutOfBoundsException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnmarshalOutOfBoundsException";
    return typeId;
}

Ice::NoValueFactoryException::~NoValueFactoryException()
{
}

const ::std::string&
Ice::NoValueFactoryException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::NoValueFactoryException";
    return typeId;
}

Ice::UnexpectedObjectException::~UnexpectedObjectException()
{
}

const ::std::string&
Ice::UnexpectedObjectException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::UnexpectedObjectException";
    return typeId;
}

Ice::MemoryLimitException::~MemoryLimitException()
{
}

const ::std::string&
Ice::MemoryLimitException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::MemoryLimitException";
    return typeId;
}

Ice::StringConversionException::~StringConversionException()
{
}

const ::std::string&
Ice::StringConversionException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::StringConversionException";
    return typeId;
}

Ice::EncapsulationException::~EncapsulationException()
{
}

const ::std::string&
Ice::EncapsulationException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::EncapsulationException";
    return typeId;
}

Ice::FeatureNotSupportedException::~FeatureNotSupportedException()
{
}

const ::std::string&
Ice::FeatureNotSupportedException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FeatureNotSupportedException";
    return typeId;
}

Ice::SecurityException::~SecurityException()
{
}

const ::std::string&
Ice::SecurityException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::SecurityException";
    return typeId;
}

Ice::FixedProxyException::~FixedProxyException()
{
}

const ::std::string&
Ice::FixedProxyException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::FixedProxyException";
    return typeId;
}

Ice::ResponseSentException::~ResponseSentException()
{
}

const ::std::string&
Ice::ResponseSentException::ice_staticId()
{
    static const ::std::string typeId = "::Ice::ResponseSentException";
    return typeId;
}
