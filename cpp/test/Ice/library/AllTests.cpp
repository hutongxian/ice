//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Test.h>

ICE_DECLSPEC_IMPORT void
consume(const Ice::ObjectPtr&, const Ice::ObjectPrx&);

#if defined(_MSC_VER)
#   pragma comment(lib, ICE_LIBNAME("consumer"))
#   pragma comment(lib, ICE_LIBNAME("gencode"))
#endif

class TestI : public Test::MyInterface
{
public:

    void op(bool, const Ice::Current&);
};

void
TestI::op(bool throwIt, const Ice::Current&)
{
    if(throwIt)
    {
        throw Test::UserError("error message");
    }
}

ICE_DECLSPEC_EXPORT
void allTests(const Ice::ObjectAdapterPtr& oa)
{
    Test::MyInterfacePtr servant = std::make_shared<TestI>();
    Test::MyInterfacePrx proxy = Ice::uncheckedCast<Test::MyInterfacePrx>(oa->addWithUUID(servant));
    consume(servant, proxy);
}
