//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

#include <thread>
#include <chrono>

using namespace std;

void
batchOnewaysAMI(const Test::MyClassPrxPtr& p)
{
    const Test::ByteS bs1(10 * 1024);
    Test::MyClassPrxPtr batch = Ice::uncheckedCast<Test::MyClassPrx>(p->ice_batchOneway());
    promise<void> prom;
    batch->ice_flushBatchRequestsAsync(nullptr,
        [&](bool sentSynchronously)
        {
            test(sentSynchronously);
            prom.set_value();
        }); // Empty flush
    prom.get_future().get();

    for(int i = 0; i < 30; ++i)
    {
        batch->opByteSOnewayAsync(bs1, nullptr, [](exception_ptr){ test(false); });
    }

    int count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += p->opByteSOnewayCallCount();
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    if(batch->ice_getConnection() &&
       p->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "bt")
    {
        Test::MyClassPrxPtr batch1 = Ice::uncheckedCast<Test::MyClassPrx>(p->ice_batchOneway());
        Test::MyClassPrxPtr batch2 = Ice::uncheckedCast<Test::MyClassPrx>(p->ice_batchOneway());

        batch1->ice_pingAsync().get();
        batch2->ice_pingAsync().get();
        batch1->ice_flushBatchRequestsAsync().get();
        batch1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
        batch1->ice_pingAsync().get();
        batch2->ice_pingAsync().get();

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->ice_pingAsync().get();
        batch1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);

        batch1->ice_pingAsync().get();
        batch2->ice_pingAsync().get();
    }

    Ice::Identity identity;
    identity.name = "invalid";
    auto batch3 = batch->ice_identity(identity);
    batch3->ice_pingAsync();
    batch3->ice_flushBatchRequestsAsync().get();

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3->ice_pingAsync();
    batch->ice_pingAsync();
    batch->ice_flushBatchRequestsAsync().get();
    batch->ice_pingAsync();
}
