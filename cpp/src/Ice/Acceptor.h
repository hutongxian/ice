//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ACCEPTOR_H
#define ICE_ACCEPTOR_H

#include <Ice/AcceptorF.h>
#include <Ice/TransceiverF.h>
#include <Ice/EndpointIF.h>
#include <Ice/Network.h>

namespace IceInternal
{

class ICE_API Acceptor
{
public:
    virtual ~Acceptor();

    virtual NativeInfoPtr getNativeInfo() = 0;
    virtual void close() = 0;
    virtual EndpointIPtr listen() = 0;
#if defined(ICE_USE_IOCP)
    virtual void startAccept() = 0;
    virtual void finishAccept() = 0;
#endif
    virtual TransceiverPtr accept() = 0;
    virtual std::string protocol() const = 0;
    virtual std::string toString() const = 0;
    virtual std::string toDetailedString() const = 0;
};

}

#endif
