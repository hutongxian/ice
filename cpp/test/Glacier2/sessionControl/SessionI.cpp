//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <SessionI.h>
#include <TestHelper.h>

using namespace std;
using namespace Test;

Glacier2::SessionPrxPtr
SessionManagerI::create(string userId, Glacier2::SessionControlPrxPtr sessionControl,
                        const Ice::Current& current)
{
    if(userId == "rejectme")
    {
        throw Glacier2::CannotCreateSessionException("");
    }
    if(userId == "localexception")
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return Ice::uncheckedCast<Glacier2::SessionPrx>(current.adapter->addWithUUID(make_shared<SessionI>(sessionControl)));
}

SessionI::SessionI(Glacier2::SessionControlPrxPtr sessionControl) :
    _sessionControl(std::move(sessionControl))
{
    assert(_sessionControl);
}

void
SessionI::destroyFromClientAsync(function<void()> response, function<void(exception_ptr)>, const Ice::Current&)
{
    _sessionControl->destroyAsync(std::move(response), [](exception_ptr){ test(false); });
}

void
SessionI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
SessionI::destroy(const Ice::Current& current)
{
    current.adapter->remove(current.id);
}
