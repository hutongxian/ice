//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

#include <thread>
#include <chrono>

using namespace std;

namespace
{

class BatchRequestInterceptorI final
{
public:

    BatchRequestInterceptorI() : _enabled(false), _count(0), _size(0), _lastRequestSize(0)
    {
    }

    virtual void
    enqueue(const Ice::BatchRequest& request, int32_t count, int32_t size)
    {
        test(request.getOperation() == "opByteSOneway" || request.getOperation() == "ice_ping");
        test(request.getProxy()->ice_isBatchOneway());

        if(count > 0)
        {
            test(_lastRequestSize + _size == size);
        }
        _count = count;
        _size = size;

        if(_size + request.getSize() > 25000)
        {
            request.getProxy()->ice_flushBatchRequestsAsync();
            _size = 18; // header
        }

        if(_enabled)
        {
            _lastRequestSize = request.getSize();
            ++_count;
            request.enqueue();
        }
    }

    void
    enqueue(bool enabled)
    {
        _enabled = enabled;
    }

    int
    count()
    {
        return _count;
    }

private:

    bool _enabled;
    int _count;
    int _size;
    int _lastRequestSize;
};
using BatchRequestInterceptorIPtr = std::shared_ptr<BatchRequestInterceptorI>;

}

void
batchOneways(const Test::MyClassPrxPtr& p)
{
    const Test::ByteS bs1(10  * 1024);

    Test::MyClassPrxPtr batch = Ice::uncheckedCast<Test::MyClassPrx>(p->ice_batchOneway());

    batch->ice_flushBatchRequests(); // Empty flush
    if(batch->ice_getConnection())
    {
        batch->ice_getConnection()->flushBatchRequests(Ice::CompressBatch::BasedOnProxy);
    }
    batch->ice_getCommunicator()->flushBatchRequests(Ice::CompressBatch::BasedOnProxy);

    int i;
    p->opByteSOnewayCallCount(); // Reset the call count
    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
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

        batch1->ice_ping();
        batch2->ice_ping();
        batch1->ice_flushBatchRequests();
        batch1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
        batch1->ice_ping();
        batch2->ice_ping();

        batch1->ice_getConnection();
        batch2->ice_getConnection();

        batch1->ice_ping();
        batch1->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
        batch1->ice_ping();
        batch2->ice_ping();
    }

    Ice::Identity identity;
    identity.name = "invalid";
    {
        Ice::ObjectPrxPtr batch3 = batch->ice_identity(identity);
        batch3->ice_ping();
        batch3->ice_flushBatchRequests();
        // Make sure that a bogus batch request doesn't cause troubles to other ones.
        batch3->ice_ping();
        batch->ice_ping();
        batch->ice_flushBatchRequests();
        batch->ice_ping();
    }

    if(batch->ice_getConnection() &&
       p->ice_getCommunicator()->getProperties()->getProperty("Ice.Default.Protocol") != "bt")
    {
        Ice::InitializationData initData;
        initData.properties = p->ice_getCommunicator()->getProperties()->clone();
        BatchRequestInterceptorIPtr interceptor = std::make_shared<BatchRequestInterceptorI>();

        initData.batchRequestInterceptor = [=](const Ice::BatchRequest& request, int countP, int size)
        {
            interceptor->enqueue(request, countP, size);
        };
        Ice::CommunicatorPtr ic = Ice::initialize(initData);

        Test::MyClassPrxPtr batch4 =
            Ice::uncheckedCast<Test::MyClassPrx>(ic->stringToProxy(p->ice_toString()))->ice_batchOneway();

        test(interceptor->count() == 0);
        batch4->ice_ping();
        batch4->ice_ping();
        batch4->ice_ping();
        test(interceptor->count() == 0);

        interceptor->enqueue(true);
        batch4->ice_ping();
        batch4->ice_ping();
        batch4->ice_ping();
        test(interceptor->count() == 3);

        batch4->ice_flushBatchRequests();
        batch4->ice_ping();
        test(interceptor->count() == 1);

        batch4->opByteSOneway(bs1);
        test(interceptor->count() == 2);
        batch4->opByteSOneway(bs1);
        test(interceptor->count() == 3);

        batch4->opByteSOneway(bs1); // This should trigger the flush
        batch4->ice_ping();
        test(interceptor->count() == 2);

        ic->destroy();
    }

    bool supportsCompress = true;
    try
    {
        supportsCompress = p->supportsCompress();
    }
    catch(const Ice::OperationNotExistException&)
    {
    }

    if(supportsCompress && batch->ice_getConnection() &&
       p->ice_getCommunicator()->getProperties()->getProperty("Ice.Override.Compress") == "")
    {
        Ice::ObjectPrxPtr prx = batch->ice_getConnection()->createProxy(batch->ice_getIdentity())->ice_batchOneway();

        Test::MyClassPrxPtr batch1 = Ice::uncheckedCast<Test::MyClassPrx>(prx->ice_compress(false));
        Test::MyClassPrxPtr batch2 = Ice::uncheckedCast<Test::MyClassPrx>(prx->ice_compress(true));
        Test::MyClassPrxPtr batch3 = Ice::uncheckedCast<Test::MyClassPrx>(prx->ice_identity(identity));

        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::CompressBatch::Yes);

        batch2->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::CompressBatch::No);

        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::CompressBatch::BasedOnProxy);

        batch1->opByteSOneway(bs1);
        batch2->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::CompressBatch::BasedOnProxy);

        batch1->opByteSOneway(bs1);
        batch3->opByteSOneway(bs1);
        batch1->opByteSOneway(bs1);
        batch1->ice_getConnection()->flushBatchRequests(Ice::CompressBatch::BasedOnProxy);
    }
}
