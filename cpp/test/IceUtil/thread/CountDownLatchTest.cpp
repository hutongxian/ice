//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>
#include <IceUtil/CountDownLatch.h>

#include <CountDownLatchTest.h>
#include <TestHelper.h>

#include <thread>
#include <chrono>

using namespace std;
using namespace IceUtil;
using namespace IceUtilInternal;

static const string testName("countDownLatch");

static const int magic = 0xbeef;

class CountDownLatchTestThread final : public Thread
{
public:

    CountDownLatchTestThread(CountDownLatch& latch, int& val, bool takeOne) :
        _latch(latch),
        _val(val),
        _takeOne(takeOne)
    {
    }

    void run() final
    {
        if(_takeOne)
        {
            _latch.countDown();
        }

        if(_latch.getCount() == 0)
        {
            test(_val == magic);
        }

        _latch.await();
        test(_latch.getCount() == 0);
        test(_val == magic);
    }

private:

    CountDownLatch& _latch;
    int& _val;
    bool _takeOne;
};

CountDownLatchTest::CountDownLatchTest() :
    TestBase(testName)
{
}

void
CountDownLatchTest::run()
{
    const int fullCount = 11;

    int val = 0xabcd;

    CountDownLatch latch(fullCount);
    test(latch.getCount() == fullCount);

    const int wave1Count = 6;

    int i = 0;
    ThreadPtr t1[wave1Count];
    for(i = 0; i < wave1Count; i++)
    {
        t1[i] = make_shared<CountDownLatchTestThread>(latch, val, false);
        t1[i]->start();
    }

    //
    // Sleep a little bit, and check count
    //
    this_thread::sleep_for(chrono::seconds(1));
    test(latch.getCount() == fullCount);

    //
    // Let's count down all except 1
    //
    ThreadPtr t2[fullCount - 1];
    for(i = 0; i < fullCount - 1; i++)
    {
        t2[i] = make_shared<CountDownLatchTestThread>(latch, val, true);
        t2[i]->start();
    }

    //
    // Sleep until count == 1
    //
    do
    {
        this_thread::sleep_for(chrono::milliseconds(100));

        for(i = 0; i < wave1Count; i++)
        {
            test(t1[i]->isAlive());
        }

        for(i = 0; i < fullCount - 1; i++)
        {
            test(t2[i]->isAlive());
        }

    } while(latch.getCount() > 1);

    //
    // Set val and release last count
    //
    val = magic;
    latch.countDown();
    test(latch.getCount() == 0);

    //
    // Join them all
    //
    for(i = 0; i < wave1Count; i++)
    {
        t1[i]->getThreadControl().join();
    }

    for(i = 0; i < fullCount - 1; i++)
    {
        t2[i]->getThreadControl().join();
    }

    test(latch.getCount() == 0);

    const int wave2Count = 4;
    ThreadPtr t3[wave2Count];
    for(i = 0; i < wave2Count; i++)
    {
        t3[i] = make_shared<CountDownLatchTestThread>(latch, val, true);
        t3[i]->start();
    }
    test(latch.getCount() == 0);

    for(i = 0; i < wave2Count; i++)
    {
        t3[i]->getThreadControl().join();
    }
    test(latch.getCount() == 0);
}
