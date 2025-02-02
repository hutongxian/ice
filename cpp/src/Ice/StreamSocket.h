//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STREAM_SOCKET_H
#define ICE_STREAM_SOCKET_H

#include <Ice/Network.h>
#include <Ice/Buffer.h>
#include <Ice/ProtocolInstanceF.h>

#include <memory>

namespace IceInternal
{

class ICE_API StreamSocket : public NativeInfo
{
public:

    StreamSocket(const ProtocolInstancePtr&, const NetworkProxyPtr&, const Address&, const Address&);
    StreamSocket(const ProtocolInstancePtr&, SOCKET);
    virtual ~StreamSocket();

    SocketOperation connect(Buffer&, Buffer&);
    bool isConnected();
    size_t getSendPacketSize(size_t);
    size_t getRecvPacketSize(size_t);

    void setBufferSize(int rcvSize, int sndSize);

    SocketOperation read(Buffer&);
    SocketOperation write(Buffer&);

    ssize_t read(char*, size_t);
    ssize_t write(const char*, size_t);

#if defined(ICE_USE_IOCP)
    AsyncInfo* getAsyncInfo(SocketOperation);
#endif

#if defined(ICE_USE_IOCP)
    bool startWrite(Buffer&);
    void finishWrite(Buffer&);
    void startRead(Buffer&);
    void finishRead(Buffer&);
#endif

    void close();
    const std::string& toString() const;

private:

    void init();

    enum State
    {
        StateNeedConnect,
        StateConnectPending,
        StateProxyWrite,
        StateProxyRead,
        StateProxyConnected,
        StateConnected
    };
    State toState(SocketOperation) const;

    const ProtocolInstancePtr _instance;
    const NetworkProxyPtr _proxy;
    const Address _addr;
    const Address _sourceAddr;

    State _state;
    std::string _desc;

#if defined(ICE_USE_IOCP)
    size_t _maxSendPacketSize;
    size_t _maxRecvPacketSize;
    AsyncInfo _read;
    AsyncInfo _write;
#endif
};
using StreamSocketPtr = std::shared_ptr<StreamSocket>;

}

#endif
