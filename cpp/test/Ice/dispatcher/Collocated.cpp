//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>
#include <Dispatcher.h>

using namespace std;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    auto dispatcher = Dispatcher::create();
    initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
        {
            dispatcher->dispatch(make_shared<DispatcherCall>(call), conn);
        };
    Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
    communicator->getProperties()->setProperty("ControllerAdapter.ThreadPool.Size", "1");

    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectAdapterPtr adapter2 = communicator->createObjectAdapter("ControllerAdapter");

    TestIntfControllerIPtr testController = make_shared<TestIntfControllerI>(adapter);

    adapter->add(make_shared<TestIntfI>(), Ice::stringToIdentity("test"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    adapter2->add(testController, Ice::stringToIdentity("testController"));
    //adapter2->activate(); // Don't activate OA to ensure collocation is used.

    void allTests(Test::TestHelper*);
    allTests(this);

    dispatcher->terminate();
}

DEFINE_TEST(Collocated)
