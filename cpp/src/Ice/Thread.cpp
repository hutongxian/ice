//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifdef __sun
//
// Solaris 10 bug: it's supposed to be defined in pthread.h
//
#ifndef __EXTENSIONS__
#define __EXTENSIONS__
#endif
#endif

#include <IceUtil/Thread.h>
#include <IceUtil/ThreadException.h>
#include <Ice/ConsoleUtil.h>
#include <climits>
#include <exception>

#ifndef _WIN32
    #include <sys/time.h>
    #include <sys/resource.h>
#endif

using namespace std;
using namespace IceInternal;

#if defined(_WIN32)

IceUtil::ThreadControl::ThreadControl() :
    _handle(0),
    _id(GetCurrentThreadId())
{
}

IceUtil::ThreadControl::ThreadControl(HANDLE handle, IceUtil::ThreadControl::ID id) :
    _handle(handle),
    _id(id)
{
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    return _id == rhs._id;
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return _id != rhs._id;
}

void
IceUtil::ThreadControl::join()
{
    if(_handle == 0)
    {
        throw BadThreadControlException(__FILE__, __LINE__);
    }

    DWORD rc = WaitForSingleObjectEx(_handle, INFINITE, true);
    if(rc != WAIT_OBJECT_0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }

    detach();
}

void
IceUtil::ThreadControl::detach()
{
    if(_handle == 0)
    {
        throw BadThreadControlException(__FILE__, __LINE__);
    }

    if(CloseHandle(_handle) == 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

IceUtil::ThreadControl::ID
IceUtil::ThreadControl::id() const
{
    return _id;
}

void
IceUtil::ThreadControl::yield()
{
    //
    // A value of zero causes the thread to relinquish the remainder
    // of its time slice to any other thread of equal priority that is
    // ready to run.
    //
    Sleep(0);
}

IceUtil::Thread::Thread() :
    _started(false),
    _running(false),
    _handle(0),
    _id(0)
{
}

IceUtil::Thread::Thread(const string& name) :
    _name(name),
    _started(false),
    _running(false),
    _handle(0),
    _id(0)
{
}

IceUtil::Thread::~Thread()
{
}

static unsigned int
WINAPI startHook(void* arg)
{
    // Ensure that the thread doesn't go away until run() has
    // completed.
    //
    IceUtil::ThreadPtr thread;

    try
    {
        auto rawThread = static_cast<shared_ptr<IceUtil::Thread>*>(arg);

        //
        // Ensure that the thread doesn't go away until run() has
        // completed.
        //
        thread = *rawThread;

        //
        // See the comment in IceUtil::Thread::start() for details.
        //
        delete rawThread;
        thread->run();
    }
    catch(...)
    {
        if(!thread->name().empty())
        {
            consoleErr << thread->name() << " terminating" << endl;
        }
        std::terminate();
    }

    thread->_done();

    return 0;
}

#include <process.h>

IceUtil::ThreadControl
IceUtil::Thread::start(size_t stackSize)
{
    return start(stackSize, THREAD_PRIORITY_NORMAL);
}

IceUtil::ThreadControl
IceUtil::Thread::start(size_t stackSize, int priority)
{
    lock_guard lock(_stateMutex);

    if(_started)
    {
        throw ThreadStartedException(__FILE__, __LINE__);
    }

    auto self = new shared_ptr<Thread>(shared_from_this());
    unsigned int id;
    _handle =
        reinterpret_cast<HANDLE>(
            _beginthreadex(
                0,
                static_cast<unsigned int>(stackSize),
                startHook,
                self,
                CREATE_SUSPENDED,
                &id));
    _id = id;
    assert(_handle != (HANDLE)-1L);
    if(_handle == 0)
    {
        delete self;
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
    if(SetThreadPriority(_handle, priority) == 0)
    {
        delete self;
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
    if(static_cast<int>(ResumeThread(_handle)) == -1)
    {
        delete self;
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }

    _started = true;
    _running = true;

    return ThreadControl(_handle, _id);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl() const
{
    lock_guard lock(_stateMutex);
    if(!_started)
    {
        throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_handle, _id);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    return this == &rhs;
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    return this < &rhs;
}

bool
IceUtil::Thread::isAlive() const
{
    lock_guard lock(_stateMutex);
    return _running;
}

void
IceUtil::Thread::_done()
{
    lock_guard lock(_stateMutex);
    _running = false;
}

const string&
IceUtil::Thread::name() const
{
    return _name;
}

#else

IceUtil::ThreadControl::ThreadControl(pthread_t thread) :
    _thread(thread),
    _detachable(true)
{
}

IceUtil::ThreadControl::ThreadControl() :
    _thread(pthread_self()),
    _detachable(false)
{
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    return pthread_equal(_thread, rhs._thread) != 0;
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

void
IceUtil::ThreadControl::join()
{
    if(!_detachable)
    {
        throw BadThreadControlException(__FILE__, __LINE__);
    }

    void* status = 0;
    int rc = pthread_join(_thread, &status);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
IceUtil::ThreadControl::detach()
{
    if(!_detachable)
    {
        throw BadThreadControlException(__FILE__, __LINE__);
    }

    int rc = pthread_detach(_thread);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

IceUtil::ThreadControl::ID
IceUtil::ThreadControl::id() const
{
    return _thread;
}

void
IceUtil::ThreadControl::yield()
{
    sched_yield();
}

IceUtil::Thread::Thread() :
    _started(false),
    _running(false)
{
}

IceUtil::Thread::Thread(const string& name) :
    _name(name),
    _started(false),
    _running(false)
{
}

IceUtil::Thread::~Thread()
{
}

extern "C"
{
static void*
startHook(void* arg)
{
    //
    // Ensure that the thread doesn't go away until run() has
    // completed.
    //
    shared_ptr<IceUtil::Thread> thread;

    try
    {
        auto rawThread = static_cast<shared_ptr<IceUtil::Thread>*>(arg);

        thread = *rawThread;
        delete rawThread;
        thread->run();
    }
    catch(...)
    {
        if(!thread->name().empty())
        {
            consoleErr << thread->name() << " terminating" << endl;
        }
        std::terminate();
    }

    thread->_done();

    return 0;
}
}

IceUtil::ThreadControl
IceUtil::Thread::start(size_t stackSize)
{
    return start(stackSize, false, 0);
}

IceUtil::ThreadControl
IceUtil::Thread::start(size_t stackSize, int priority)
{
    return start(stackSize, true, priority);
}
IceUtil::ThreadControl
IceUtil::Thread::start(size_t stackSize, bool realtimeScheduling, int priority)
{
    lock_guard lock(_stateMutex);

    if(_started)
    {
        throw ThreadStartedException(__FILE__, __LINE__);
    }

    pthread_attr_t attr;
    int rc = pthread_attr_init(&attr);
    if(rc != 0)
    {
        pthread_attr_destroy(&attr);
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    if(stackSize > 0)
    {
        if(stackSize < static_cast<size_t>(PTHREAD_STACK_MIN))
        {
            stackSize = static_cast<size_t>(PTHREAD_STACK_MIN);
        }
#ifdef __APPLE__
        if(stackSize % 4096 > 0)
        {
            stackSize = stackSize / 4096 * 4096 + 4096;
        }
#endif
        rc = pthread_attr_setstacksize(&attr, stackSize);
        if(rc != 0)
        {
            pthread_attr_destroy(&attr);
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }

    if(realtimeScheduling)
    {
        rc = pthread_attr_setschedpolicy(&attr, SCHED_RR);
        if(rc != 0)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
        sched_param param;
        param.sched_priority = priority;
        rc = pthread_attr_setschedparam(&attr, &param);
        if(rc != 0)
        {
            pthread_attr_destroy(&attr);
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
        pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    }

    auto self = new shared_ptr<Thread>(shared_from_this());

    rc = pthread_create(&_thread, &attr, startHook, self);
    pthread_attr_destroy(&attr);
    if(rc != 0)
    {
        delete self;
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    _started = true;
    _running = true;
    return ThreadControl(_thread);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl() const
{
    lock_guard lock(_stateMutex);
    if(!_started)
    {
        throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_thread);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    return this == &rhs;
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    return this < &rhs;
}

bool
IceUtil::Thread::isAlive() const
{
    lock_guard lock(_stateMutex);
    return _running;
}

void
IceUtil::Thread::_done()
{
    lock_guard lock(_stateMutex);
    _running = false;
}

const string&
IceUtil::Thread::name() const
{
    return _name;
}

#endif
