//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceIAP_EndpointInfo_h__
#define __IceIAP_EndpointInfo_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/Endpoint.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICEIAP_API
#   if defined(ICE_STATIC_LIBS)
#       define ICEIAP_API /**/
#   elif defined(ICEIAP_API_EXPORTS)
#       define ICEIAP_API ICE_DECLSPEC_EXPORT
#   else
#       define ICEIAP_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceIAP
{

class EndpointInfo;

}

namespace IceIAP
{

/**
 * Provides access to an IAP endpoint information.
 * \headerfile IceIAP/IceIAP.h
 */
class ICE_CLASS(ICEIAP_API) EndpointInfo : public ::Ice::EndpointInfo
{
public:

    ICE_MEMBER(ICEIAP_API) virtual ~EndpointInfo();

    EndpointInfo() = default;

    EndpointInfo(const EndpointInfo&) = default;
    EndpointInfo(EndpointInfo&&) = default;
    EndpointInfo& operator=(const EndpointInfo&) = default;
    EndpointInfo& operator=(EndpointInfo&&) = default;

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint of null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     * @param manufacturer The accessory manufacturer or empty to not match against a manufacturer.
     * @param modelNumber The accessory model number or empty to not match against a model number.
     * @param name The accessory name or empty to not match against the accessory name.
     * @param protocol The protocol supported by the accessory.
     */
    EndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo>& underlying, int timeout, bool compress, const ::std::string& manufacturer, const ::std::string& modelNumber, const ::std::string& name, const ::std::string& protocol) :
        ::Ice::EndpointInfo(underlying, timeout, compress),
        manufacturer(manufacturer),
        modelNumber(modelNumber),
        name(name),
        protocol(protocol)
    {
    }

    /**
     * The accessory manufacturer or empty to not match against
     * a manufacturer.
     */
    ::std::string manufacturer;
    /**
     * The accessory model number or empty to not match against
     * a model number.
     */
    ::std::string modelNumber;
    /**
     * The accessory name or empty to not match against
     * the accessory name.
     */
    ::std::string name;
    /**
     * The protocol supported by the accessory.
     */
    ::std::string protocol;
};

}

/// \cond INTERNAL
namespace IceIAP
{

using EndpointInfoPtr = ::std::shared_ptr<EndpointInfo>;

}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
