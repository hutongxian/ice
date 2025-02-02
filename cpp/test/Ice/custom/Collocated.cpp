//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>
#include <WstringI.h>
#include <StringConverterI.h>

using namespace std;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    setProcessStringConverter(make_shared<Test::StringConverterI>());
    setProcessWstringConverter(make_shared<Test::WstringConverterI>());

    Ice::CommunicatorHolder communicator = initialize(argc, argv);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(std::make_shared<TestIntfI>(), Ice::stringToIdentity("TEST"));
    adapter->add(make_shared<Test1::WstringClassI>(), Ice::stringToIdentity("WSTRING1"));
    adapter->add(make_shared<Test2::WstringClassI>(), Ice::stringToIdentity("WSTRING2"));

    Test::TestIntfPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
