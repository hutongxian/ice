//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;

void
TestIntfI::ping(Test::PingReplyPrxPtr reply, const Current&)
{
    try
    {
        reply->reply();
    }
    catch(const Ice::Exception&)
    {
        assert(false);
    }
}

void
TestIntfI::sendByteSeq(Test::ByteSeq, Test::PingReplyPrxPtr reply, const Current&)
{
    try
    {
        reply->reply();
    }
    catch(const Ice::Exception&)
    {
        assert(false);
    }
}

void
TestIntfI::pingBiDir(Ice::Identity id, const Ice::Current& current)
{
    try
    {
        //
        // Ensure sending too much data doesn't cause the UDP connection
        // to be closed.
        //
        try
        {
            Test::ByteSeq seq;
            seq.resize(32 * 1024);
            Ice::uncheckedCast<Test::TestIntfPrx>(current.con->createProxy(id))->sendByteSeq(seq, nullopt);
        }
        catch(const DatagramLimitException&)
        {
            // Expected.
        }

        //
        // Send the reply through the incoming connection.
        //
        Ice::uncheckedCast<Test::PingReplyPrx>(current.con->createProxy(id))->replyAsync();
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        assert(false);
    }
}

void
TestIntfI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
