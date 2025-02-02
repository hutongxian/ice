//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class TestI : public Test::TestIntf
{
public:

    TestI();

    virtual void shutdown(const Ice::Current&);
    virtual void abort(const Ice::Current&);
    virtual void idempotentAbort(const Ice::Current&);
    virtual std::int32_t pid(const Ice::Current&);
};

#endif
