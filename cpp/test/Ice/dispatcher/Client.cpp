//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>
#include <Dispatcher.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);
    //
    // Limit the send buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    initData.properties->setProperty("Ice.TCP.SndSize", "50000");

    auto dispatcher = Dispatcher::create();
    initData.dispatcher = [=](function<void()> call, const shared_ptr<Ice::Connection>& conn)
        {
            dispatcher->dispatch(make_shared<DispatcherCall>(call), conn);
        };
    // The communicator must be destroyed before the dispatcher is terminated.
    {
        Ice::CommunicatorHolder communicator = initialize(argc, argv, initData);

        void allTests(Test::TestHelper*);
        allTests(this);
    }
    dispatcher->terminate();
}

DEFINE_TEST(Client)
