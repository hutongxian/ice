//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/CountDownLatch.h>
#include <IceUtil/ThreadException.h>

IceUtilInternal::CountDownLatch::CountDownLatch(int count) :
    _count(count)
{
    if(count < 0)
    {
        throw IceUtil::IllegalArgumentException(__FILE__, __LINE__, "count must be greater than 0");
    }

#ifdef _WIN32
    _event = CreateEvent(0, TRUE, FALSE, 0);
    if(_event == 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
#else
    int rc = pthread_mutex_init(&_mutex, 0);
    if(rc != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    rc = pthread_cond_init(&_cond, 0);
    if(rc != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif
}

IceUtilInternal::CountDownLatch::~CountDownLatch()
{
#ifdef _WIN32
    CloseHandle(_event);
#else
#  ifndef NDEBUG
    int rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
    rc = pthread_cond_destroy(&_cond);
    assert(rc == 0);
#  else
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_cond);
#  endif
#endif
}

void
IceUtilInternal::CountDownLatch::await() const
{
#ifdef _WIN32
    while(InterlockedExchangeAdd(&_count, 0) > 0)
    {
        DWORD rc = WaitForSingleObject(_event, INFINITE);
        assert(rc == WAIT_OBJECT_0 || rc == WAIT_FAILED);

        if(rc == WAIT_FAILED)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
    }
#else
    lock();
    while(_count > 0)
    {
        int rc = pthread_cond_wait(&_cond, &_mutex);
        if(rc != 0)
        {
            pthread_mutex_unlock(&_mutex);
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    unlock();

#endif
}

void
IceUtilInternal::CountDownLatch::countDown()
{
#ifdef _WIN32
    if(InterlockedDecrement(&_count) == 0)
    {
        if(SetEvent(_event) == 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
    }
#else
    bool broadcast = false;

    lock();
    if(_count > 0 && --_count == 0)
    {
        broadcast = true;
    }
#if defined(__APPLE__)
    //
    // On macOS we do the broadcast with the mutex held. This seems to
    // be necessary to prevent the broadcast call to hang (spinning in
    // an infinite loop).
    //
    if(broadcast)
    {
        int rc = pthread_cond_broadcast(&_cond);
        if(rc != 0)
        {
            pthread_mutex_unlock(&_mutex);
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    unlock();

#else
    unlock();

    if(broadcast)
    {
        int rc = pthread_cond_broadcast(&_cond);
        if(rc != 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
#endif

#endif
}

int
IceUtilInternal::CountDownLatch::getCount() const
{
#ifdef _WIN32
    int count = InterlockedExchangeAdd(&_count, 0);
    return count > 0 ? count : 0;
#else
    lock();
    int result = _count;
    unlock();
    return result;
#endif
}

#ifndef _WIN32
void
IceUtilInternal::CountDownLatch::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
IceUtilInternal::CountDownLatch::unlock() const
{
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

#endif
