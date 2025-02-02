//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>

#include <stdio.h>

#include <StartTest.h>
#include <TestHelper.h>

#include <thread>
#include <chrono>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread start");

class StartTestThread final : public Thread
{
public:

    StartTestThread()
    {
    }

    void run() final
    {
    }
};

StartTest::StartTest() :
    TestBase(createTestName)
{
}

void
StartTest::run()
{
    //
    // Check that calling start() more than once raises ThreadStartedException.
    //
    auto t = make_shared<StartTestThread>();
    ThreadControl control = t->start();
    control.join();
    try
    {
        t->start();
        test(false);
    }
    catch(const ThreadStartedException&)
    {
    }

    //
    // Now let's create a bunch of short-lived threads
    //
    for(int i = 0; i < 40; i++)
    {
        for(int j = 0; j < 40; j++)
        {
            auto thread = make_shared<StartTestThread>();
            thread->start().detach();
        }
        this_thread::sleep_for(chrono::milliseconds(5));
    }
}
