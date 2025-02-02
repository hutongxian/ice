//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/DefaultsAndOverrides.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::DefaultsAndOverrides::DefaultsAndOverrides(const PropertiesPtr& properties, const LoggerPtr& logger) :
    overrideTimeout(false),
    overrideTimeoutValue(-1),
    overrideConnectTimeout(false),
    overrideConnectTimeoutValue(-1),
    overrideCloseTimeout(false),
    overrideCloseTimeoutValue(-1),
    overrideCompress(false),
    overrideCompressValue(false),
    overrideSecure(false),
    overrideSecureValue(false)
{
    const_cast<string&>(defaultProtocol) = properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");

    const_cast<string&>(defaultHost) = properties->getProperty("Ice.Default.Host");

    string value;

    value = properties->getProperty("Ice.Default.SourceAddress");
    if(!value.empty())
    {
        const_cast<Address&>(defaultSourceAddress) = getNumericAddress(value);
        if(!isAddressValid(defaultSourceAddress))
        {
            throw InitializationException(__FILE__, __LINE__, "invalid IP address set for Ice.Default.SourceAddress: `" +
                                          value + "'");
        }
    }

    value = properties->getProperty("Ice.Override.Timeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideTimeout) = true;
        const_cast<int32_t&>(overrideTimeoutValue) = properties->getPropertyAsInt("Ice.Override.Timeout");
        if(overrideTimeoutValue < 1 && overrideTimeoutValue != -1)
        {
            const_cast<int32_t&>(overrideTimeoutValue) = -1;
            Warning out(logger);
            out << "invalid value for Ice.Override.Timeout `" << properties->getProperty("Ice.Override.Timeout")
                << "': defaulting to -1";
        }
    }

    value = properties->getProperty("Ice.Override.ConnectTimeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideConnectTimeout) = true;
        const_cast<int32_t&>(overrideConnectTimeoutValue) = properties->getPropertyAsInt("Ice.Override.ConnectTimeout");
        if(overrideConnectTimeoutValue < 1 && overrideConnectTimeoutValue != -1)
        {
            const_cast<int32_t&>(overrideConnectTimeoutValue) = -1;
            Warning out(logger);
            out << "invalid value for Ice.Override.ConnectTimeout `"
                << properties->getProperty("Ice.Override.ConnectTimeout") << "': defaulting to -1";
        }
    }

    value = properties->getProperty("Ice.Override.CloseTimeout");
    if(!value.empty())
    {
        const_cast<bool&>(overrideCloseTimeout) = true;
        const_cast<int32_t&>(overrideCloseTimeoutValue) = properties->getPropertyAsInt("Ice.Override.CloseTimeout");
        if(overrideCloseTimeoutValue < 1 && overrideCloseTimeoutValue != -1)
        {
            const_cast<int32_t&>(overrideCloseTimeoutValue) = -1;
            Warning out(logger);
            out << "invalid value for Ice.Override.CloseTimeout `"
                << properties->getProperty("Ice.Override.CloseTimeout") << "': defaulting to -1";
        }
    }

    value = properties->getProperty("Ice.Override.Compress");
    if(!value.empty())
    {
        const_cast<bool&>(overrideCompress) = true;
        const_cast<bool&>(overrideCompressValue) = properties->getPropertyAsInt("Ice.Override.Compress") > 0;
    }

    value = properties->getProperty("Ice.Override.Secure");
    if(!value.empty())
    {
        const_cast<bool&>(overrideSecure) = true;
        const_cast<bool&>(overrideSecureValue) = properties->getPropertyAsInt("Ice.Override.Secure") > 0;
    }

    const_cast<bool&>(defaultCollocationOptimization) =
        properties->getPropertyAsIntWithDefault("Ice.Default.CollocationOptimized", 1) > 0;

    value = properties->getPropertyWithDefault("Ice.Default.EndpointSelection", "Random");
    if(value == "Random")
    {
        defaultEndpointSelection = EndpointSelectionType::Random;
    }
    else if(value == "Ordered")
    {
        defaultEndpointSelection = EndpointSelectionType::Ordered;
    }
    else
    {
        throw EndpointSelectionTypeParseException(__FILE__, __LINE__, "illegal value `" + value +
                                                  "'; expected `Random' or `Ordered'");
    }

    const_cast<int&>(defaultTimeout) =
        properties->getPropertyAsIntWithDefault("Ice.Default.Timeout", 60000);
    if(defaultTimeout < 1 && defaultTimeout != -1)
    {
        const_cast<int32_t&>(defaultTimeout) = 60000;
        Warning out(logger);
        out << "invalid value for Ice.Default.Timeout `" << properties->getProperty("Ice.Default.Timeout")
            << "': defaulting to 60000";
    }

    const_cast<int&>(defaultInvocationTimeout) =
        properties->getPropertyAsIntWithDefault("Ice.Default.InvocationTimeout", -1);
    if(defaultInvocationTimeout < 1 && defaultInvocationTimeout != -1 && defaultInvocationTimeout != -2)
    {
        const_cast<int32_t&>(defaultInvocationTimeout) = -1;
        Warning out(logger);
        out << "invalid value for Ice.Default.InvocationTimeout `"
            << properties->getProperty("Ice.Default.InvocationTimeout") << "': defaulting to -1";
    }

    const_cast<int&>(defaultLocatorCacheTimeout) =
        properties->getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);
    if(defaultLocatorCacheTimeout < -1)
    {
        const_cast<int32_t&>(defaultLocatorCacheTimeout) = -1;
        Warning out(logger);
        out << "invalid value for Ice.Default.LocatorCacheTimeout `"
            << properties->getProperty("Ice.Default.LocatorCacheTimeout") << "': defaulting to -1";
    }

    const_cast<bool&>(defaultPreferSecure) =
        properties->getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

    value = properties->getPropertyWithDefault("Ice.Default.EncodingVersion", encodingVersionToString(currentEncoding));
    defaultEncoding = stringToEncodingVersion(value);
    checkSupportedEncoding(defaultEncoding);

    bool slicedFormat = properties->getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
    const_cast<FormatType&>(defaultFormat) = slicedFormat ?
        FormatType::SlicedFormat : FormatType::CompactFormat;
}
