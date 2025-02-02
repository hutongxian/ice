//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h> // For TestFacetI

using namespace std;
using namespace Test;
using namespace Ice;

static void
testFacets(const Ice::CommunicatorPtr& com, bool builtInFacets = true)
{
    if(builtInFacets)
    {
        test(com->findAdminFacet("Properties"));
        test(com->findAdminFacet("Process"));
        test(com->findAdminFacet("Logger"));
        test(com->findAdminFacet("Metrics"));
    }

    TestFacetPtr f1 = std::make_shared<TestFacetI>();
    TestFacetPtr f2 = std::make_shared<TestFacetI>();
    TestFacetPtr f3 = std::make_shared<TestFacetI>();

    com->addAdminFacet(f1, "Facet1");
    com->addAdminFacet(f2, "Facet2");
    com->addAdminFacet(f3, "Facet3");

    test(Ice::ObjectPtr(com->findAdminFacet("Facet1")) == f1);
    test(Ice::ObjectPtr(com->findAdminFacet("Facet2")) == f2);
    test(Ice::ObjectPtr(com->findAdminFacet("Facet3")) == f3);
    test(!com->findAdminFacet("Bogus"));

    const Ice::FacetMap facetMap = com->findAllAdminFacets();

    if(builtInFacets)
    {
        test(facetMap.size() == 7);
        test(facetMap.find("Properties") != facetMap.end());
        test(facetMap.find("Process") != facetMap.end());
        test(facetMap.find("Logger") != facetMap.end());
        test(facetMap.find("Metrics") != facetMap.end());
    }
    else
    {
        test(facetMap.size() >= 3);
    }
    test(facetMap.find("Facet1") != facetMap.end());
    test(facetMap.find("Facet2") != facetMap.end());
    test(facetMap.find("Facet3") != facetMap.end());

    try
    {
        com->addAdminFacet(f1, "Facet1");
        test(false);
    }
    catch(const Ice::AlreadyRegisteredException&)
    {
        // Expected
    }

    try
    {
        com->removeAdminFacet("Bogus");
        test(false);
    }
    catch(const Ice::NotRegisteredException&)
    {
        // Expected
    }

    com->removeAdminFacet("Facet1");
    com->removeAdminFacet("Facet2");
    com->removeAdminFacet("Facet3");

    try
    {
        com->removeAdminFacet("Facet1");
        test(false);
    }
    catch(const Ice::NotRegisteredException&)
    {
        // Expected
    }
}

class RemoteLoggerI : public Ice::RemoteLogger
{
public:

    RemoteLoggerI();

    virtual void init(string, Ice::LogMessageSeq, const Ice::Current&);
    virtual void log(Ice::LogMessage, const Ice::Current&);

    void checkNextInit(const string&, Ice::LogMessageType, const string&, const string& = "");
    void checkNextLog(Ice::LogMessageType, const string&, const string& = "");

    bool wait(int);

private:

    mutex _mutex;
    condition_variable _condition;
    int _receivedCalls;
    string _prefix;
    Ice::LogMessageSeq _initMessages;
    Ice::LogMessageSeq _logMessages;
};

using RemoteLoggerIPtr = std::shared_ptr<RemoteLoggerI>;

RemoteLoggerI::RemoteLoggerI() : _receivedCalls(0)
{
}

void
RemoteLoggerI::init(string prefix, Ice::LogMessageSeq logMessages, const Ice::Current&)
{
    lock_guard lock(_mutex);
    _prefix = prefix;
    _initMessages.insert(_initMessages.end(), logMessages.begin(), logMessages.end());
    _receivedCalls++;
    _condition.notify_all();
}

void
RemoteLoggerI::log(Ice::LogMessage logMessage, const Ice::Current&)
{
    lock_guard lock(_mutex);
    _logMessages.push_back(logMessage);
    _receivedCalls++;
    _condition.notify_all();
}

void
RemoteLoggerI::checkNextInit(const string& prefix, Ice::LogMessageType type, const string& message,
                             const string& category)
{
    lock_guard lock(_mutex);
    test(_prefix == prefix);
    test(_initMessages.size() > 0);
    Ice::LogMessage front = _initMessages.front();
    test(front.type == type);
    test(front.message == message);
    test(front.traceCategory == category);
    _initMessages.pop_front();
}

void
RemoteLoggerI::checkNextLog(Ice::LogMessageType type, const string& message, const string& category)
{
    lock_guard lock(_mutex);
    test(_logMessages.size() > 0);
    Ice::LogMessage front = _logMessages.front();
    test(front.type == type);
    test(front.message == message);
    test(front.traceCategory == category);
    _logMessages.pop_front();
}

bool
RemoteLoggerI::wait(int calls)
{
    unique_lock lock(_mutex);
    _receivedCalls -= calls;
    auto now = chrono::steady_clock::now();
    const auto start = now;
    auto delay = chrono::seconds(20);
    while(_receivedCalls < 0)
    {
        _condition.wait_for(lock, delay);
        now = chrono::steady_clock::now();
        if(now - start >= chrono::seconds(20))
        {
            cerr << "expected `" << calls << "' received: `" << (calls + _receivedCalls) << "'" << endl;
            return false; // Waited for more than 20s for close, something's wrong.
        }
        delay = chrono::duration_cast<chrono::seconds>(start + chrono::seconds(20) - now);
    }
    return true;
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing communicator operations... " << flush;
    {
        //
        // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        Ice::CommunicatorHolder ich(init);
        testFacets(ich.communicator());

        // Test move assignment on CommunicatorHolder
        Ice::CommunicatorHolder ich2;
        test(!ich2.communicator());
        ich2 = std::move(ich);
        test(ich2.communicator());
        test(!ich.communicator());

        // Equivalent with = and release
        Ice::CommunicatorHolder ich3;
        test(!ich3.communicator());
        ich3 = ich2.release();
        test(ich3.communicator());
        test(!ich2.communicator());
    }
    {
        //
        // Test: Verify that the operations work correctly in the presence of facet filters.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        init.properties->setProperty("Ice.Admin.Facets", "Properties");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        testFacets(com, false);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly with the Admin object disabled.
        //
        Ice::CommunicatorPtr com = Ice::initialize();
        testFacets(com, false);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Enabled", "1");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        test(!com->getAdmin());

        Ice::Identity id = stringToIdentity("test-admin");
        try
        {
            com->createAdmin(0, id);
            test(false);
        }
        catch(const Ice::InitializationException&)
        {
        }

        Ice::ObjectAdapterPtr adapter = com->createObjectAdapter("");
        com->createAdmin(adapter, id);
        test(com->getAdmin());

        testFacets(com);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        init.properties->setProperty("Ice.Admin.DelayCreation", "1");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        testFacets(com);
        com->getAdmin();
        testFacets(com);
        com->destroy();
    }
    cout << "ok" << endl;

    string ref = "factory:" + helper->getTestEndpoint() + " -t 10000";
    RemoteCommunicatorFactoryPrxPtr factory =
        Ice::uncheckedCast<RemoteCommunicatorFactoryPrx>(communicator->stringToProxy(ref));

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");

    cout << "testing process facet... " << flush;
    {
        //
        // Test: Verify that Process::shutdown() operation shuts down the communicator.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::ProcessPrxPtr proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing properties facet... " << flush;
    {
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Prop1"] = "1";
        props["Prop2"] = "2";
        props["Prop3"] = "3";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::PropertiesAdminPrxPtr pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        //
        // Test: PropertiesAdmin::getProperty()
        //
        test(pa->getProperty("Prop2") == "2");
        test(pa->getProperty("Bogus") == "");

        //
        // Test: PropertiesAdmin::getProperties()
        //
        Ice::PropertyDict pd = pa->getPropertiesForPrefix("");
        test(pd.size() == 5);
        test(pd["Ice.Admin.Endpoints"] == "tcp -h " + defaultHost);
        test(pd["Ice.Admin.InstanceName"] == "Test");
        test(pd["Prop1"] == "1");
        test(pd["Prop2"] == "2");
        test(pd["Prop3"] == "3");

        Ice::PropertyDict changes;

        //
        // Test: PropertiesAdmin::setProperties()
        //
        Ice::PropertyDict setProps;
        setProps["Prop1"] = "10"; // Changed
        setProps["Prop2"] = "20"; // Changed
        setProps["Prop3"] = ""; // Removed
        setProps["Prop4"] = "4"; // Added
        setProps["Prop5"] = "5"; // Added
        pa->setProperties(setProps);
        test(pa->getProperty("Prop1") == "10");
        test(pa->getProperty("Prop2") == "20");
        test(pa->getProperty("Prop3") == "");
        test(pa->getProperty("Prop4") == "4");
        test(pa->getProperty("Prop5") == "5");
        changes = com->getChanges();
        test(changes.size() == 5);
        test(changes["Prop1"] == "10");
        test(changes["Prop2"] == "20");
        test(changes["Prop3"] == "");
        test(changes["Prop4"] == "4");
        test(changes["Prop5"] == "5");
        pa->setProperties(setProps);
        changes = com->getChanges();
        test(changes.empty());

        com->removeUpdateCallback();
        Ice::PropertyDict moreProps;
        moreProps["Prop1"] = "11"; // Changed
        moreProps["Prop2"] = ""; // Removed
        moreProps["Prop6"] = "6"; // Added
        pa->setProperties(moreProps);
        changes = com->getChanges();
        test(changes.empty());
        com->addUpdateCallback();
        pa->setProperties(moreProps);
        changes = com->getChanges();
        test(changes.empty());
        moreProps["Prop1"] = "10"; // Changed back
        pa->setProperties(moreProps);
        changes = com->getChanges();
        test(changes.size() == 1);
        test(changes["Prop1"] == "10");

        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing logger facet... " << flush;
    {
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["NullLogger"] = "1";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);

        com->trace("testCat", "trace");
        com->warning("warning");
        com->error("error");
        com->print("print");

        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::LoggerAdminPrxPtr logger = Ice::checkedCast<Ice::LoggerAdminPrx>(obj, "Logger");
        test(logger);

        string prefix;

        //
        // Get all
        //
        Ice::LogMessageSeq logMessages =
            logger->getLog(Ice::LogMessageTypeSeq(), Ice::StringSeq(), -1, prefix);

        test(logMessages.size() == 4);
        test(prefix == "NullLogger");
        Ice::LogMessageSeq::const_iterator p = logMessages.begin();
        test(p->traceCategory == "testCat" && p++->message == "trace");
        test(p++->message == "warning");
        test(p++->message == "error");
        test(p++->message == "print");

        //
        // Get only errors and warnings
        //
        com->error("error2");
        com->print("print2");
        com->trace("testCat", "trace2");
        com->warning("warning2");

        Ice::LogMessageTypeSeq messageTypes;
        messageTypes.push_back(LogMessageType::ErrorMessage);
        messageTypes.push_back(LogMessageType::WarningMessage);

        logMessages =
            logger->getLog(messageTypes, Ice::StringSeq(), -1, prefix);
        test(logMessages.size() == 4);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        while(p != logMessages.end())
        {
            test(p->type == LogMessageType::ErrorMessage || p->type == LogMessageType::WarningMessage);
            ++p;
        }

        //
        // Get only errors and traces with Cat = "testCat"
        //
        com->trace("testCat2", "A");
        com->trace("testCat", "trace3");
        com->trace("testCat2", "B");

        messageTypes.clear();
        messageTypes.push_back(LogMessageType::ErrorMessage);
        messageTypes.push_back(LogMessageType::TraceMessage);

        Ice::StringSeq categories;
        categories.push_back("testCat");

        logMessages =
            logger->getLog(messageTypes, categories, -1, prefix);
        test(logMessages.size() == 5);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        while(p != logMessages.end())
        {
            test(p->type == LogMessageType::ErrorMessage ||
                (p->type == LogMessageType::TraceMessage && p->traceCategory == "testCat"));
            ++p;
        }

        //
        // Same, but limited to last 2 messages (trace3 + error3)
        //
        com->error("error3");

        logMessages =
            logger->getLog(messageTypes, categories, 2, prefix);
        test(logMessages.size() == 2);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        test(p++->message == "trace3");
        test(p->message == "error3");

        //
        // Now, test RemoteLogger
        //

        Ice::ObjectAdapterPtr adapter =
            communicator->createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

        RemoteLoggerIPtr remoteLogger = std::make_shared<RemoteLoggerI>();

        auto myProxy = Ice::uncheckedCast<Ice::RemoteLoggerPrx>(adapter->addWithUUID(remoteLogger));

        adapter->activate();

        //
        // No filtering
        //
        logMessages = logger->getLog(Ice::LogMessageTypeSeq(), Ice::StringSeq(), -1, prefix);
        logger->attachRemoteLogger(myProxy, Ice::LogMessageTypeSeq(), Ice::StringSeq(), -1);
        test(remoteLogger->wait(1));

        for(LogMessageSeq::const_iterator i = logMessages.begin(); i != logMessages.end(); ++i)
        {
            remoteLogger->checkNextInit(prefix, i->type, i->message, i->traceCategory);
        }

        com->trace("testCat", "rtrace");
        com->warning("rwarning");
        com->error("rerror");
        com->print("rprint");
        test(remoteLogger->wait(4));

        remoteLogger->checkNextLog(LogMessageType::TraceMessage, "rtrace", "testCat");
        remoteLogger->checkNextLog(LogMessageType::WarningMessage, "rwarning");
        remoteLogger->checkNextLog(LogMessageType::ErrorMessage, "rerror");
        remoteLogger->checkNextLog(LogMessageType::PrintMessage, "rprint");

        test(logger->detachRemoteLogger(myProxy));
        test(!logger->detachRemoteLogger(myProxy));

        //
        // Use Error + Trace with "traceCat" filter with 4 limit
        //
        logMessages = logger->getLog(messageTypes, categories, 4, prefix);
        test(logMessages.size() == 4);
        logger->attachRemoteLogger(myProxy, messageTypes, categories, 4);
        test(remoteLogger->wait(1));

        for(LogMessageSeq::const_iterator i = logMessages.begin(); i != logMessages.end(); ++i)
        {
            remoteLogger->checkNextInit(prefix, i->type, i->message, i->traceCategory);
        }

        com->warning("rwarning2");
        com->trace("testCat", "rtrace2");
        com->warning("rwarning3");
        com->error("rerror2");
        com->print("rprint2");
        test(remoteLogger->wait(2));

        remoteLogger->checkNextLog(LogMessageType::TraceMessage, "rtrace2", "testCat");
        remoteLogger->checkNextLog(LogMessageType::ErrorMessage, "rerror2");

        //
        // Attempt reconnection with slightly different proxy
        //
        try
        {
            logger->attachRemoteLogger(myProxy->ice_oneway(), messageTypes, categories, 4);
            test(false);
        }
        catch(const Ice::RemoteLoggerAlreadyAttachedException&)
        {
            // expected
        }

        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing custom facet... " << flush;
    {
        //
        // Test: Verify that the custom facet is present.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Test::TestFacetPrxPtr tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        tf->op();
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing facet filtering... " << flush;
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Properties facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::ProcessPrxPtr proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        test(!proc);
        Test::TestFacetPrxPtr tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        test(!tf);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Process facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Process";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::PropertiesAdminPrxPtr pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(!pa);
        Test::TestFacetPrxPtr tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        test(!tf);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::PropertiesAdminPrxPtr pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(!pa);
        Ice::ProcessPrxPtr proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        test(!proc);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
        // facet names.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties TestFacet";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::PropertiesAdminPrxPtr pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(pa->getProperty("Ice.Admin.InstanceName") == "Test");
        Test::TestFacetPrxPtr tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        tf->op();
        Ice::ProcessPrxPtr proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        test(!proc);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
        // facet names.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet, Process";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
        Ice::PropertiesAdminPrxPtr pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(!pa);
        Test::TestFacetPrxPtr tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        tf->op();
        Ice::ProcessPrxPtr proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    factory->shutdown();
}
