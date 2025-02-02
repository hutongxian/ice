//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/ImplicitContextI.h>
#include <Ice/OutputStream.h>
#include <Ice/Object.h>
#include <IceUtil/ThreadException.h>

#include <mutex>

using namespace std;
using namespace Ice;

namespace
{

class SharedImplicitContext : public ImplicitContextI
{
public:

    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual bool containsKey(const string&) const;
    virtual string get(const string&) const;
    virtual string put(const string&, const string&);
    virtual string remove(const string&);

    virtual void write(const Context&, ::Ice::OutputStream*) const;
    virtual void combine(const Context&, Context&) const;

private:
    Context _context;
    mutable std::mutex _mutex;
};

class PerThreadImplicitContext : public ImplicitContextI
{
public:

    PerThreadImplicitContext();
    virtual ~PerThreadImplicitContext();

    virtual Context getContext() const;
    virtual void setContext(const Context&);

    virtual bool containsKey(const string&) const;
    virtual string get(const string&) const;
    virtual string put(const string&, const string&);
    virtual string remove(const string&);

    virtual void write(const Context&, ::Ice::OutputStream*) const;
    virtual void combine(const Context&, Context&) const;

    struct Slot
    {
        Slot() :
            context(0),
            owner(-1) // just to avoid UMR; a random value would work as well
        {
        }

        Context* context;
        long owner;
    };

    //
    // Each thread maintains a SlotVector. Each PerThreadImplicitContext instance
    // is assigned a slot in this vector.
    //
    typedef std::vector<Slot> SlotVector;

    //
    // We remember which slot-indices are in use (to be able to reuse indices)
    //
    typedef std::vector<bool> IndexInUse;
    static IndexInUse* _indexInUse;
    static mutex _mutex;

    static long _nextId;
    static long _destroyedIds;
    static size_t _slotVectors;

#ifdef _WIN32
    static DWORD _key;
#else
    static pthread_key_t _key;
#endif

    static void tryCleanupKey(); // must be called with _mutex locked

private:

    Context* getThreadContext(bool) const;
    void clearThreadContext() const;

    size_t _index; // index in all SlotVector
    long _id; // corresponds to owner in the Slot
};
}

extern "C" void iceImplicitContextThreadDestructor(void*);

ImplicitContextIPtr
ImplicitContextI::create(const std::string& kind)
{
    if(kind == "None" || kind == "")
    {
        return 0;
    }
    else if(kind == "Shared")
    {
        return std::make_shared<SharedImplicitContext>();
    }
    else if(kind == "PerThread")
    {
        return std::make_shared<PerThreadImplicitContext>();
    }
    else
    {
        throw Ice::InitializationException(
            __FILE__, __LINE__,
            "'" + kind + "' is not a valid value for Ice.ImplicitContext");
    }
}

#if defined(_WIN32)
void
ImplicitContextI::cleanupThread()
{
    if(PerThreadImplicitContext::_nextId > 0)
    {
        iceImplicitContextThreadDestructor(TlsGetValue(PerThreadImplicitContext::_key));
    }
}
#endif

//
// SharedImplicitContext implementation
//

Context
SharedImplicitContext::getContext() const
{
    lock_guard lock(_mutex);
    return _context;
}

void
SharedImplicitContext::setContext(const Context& newContext)
{
    lock_guard lock(_mutex);
    _context = newContext;
}

bool
SharedImplicitContext::containsKey(const string& k) const
{
    lock_guard lock(_mutex);
    Context::const_iterator p = _context.find(k);
    return p != _context.end();
}

string
SharedImplicitContext::get(const string& k) const
{
    lock_guard lock(_mutex);
    Context::const_iterator p = _context.find(k);
    if(p == _context.end())
    {
        return "";
    }
    return p->second;
}

string
SharedImplicitContext::put(const string& k, const string& v)
{
    lock_guard lock(_mutex);
    string& val = _context[k];

    string oldVal = val;
    val = v;
    return oldVal;
}

string
SharedImplicitContext::remove(const string& k)
{
    lock_guard lock(_mutex);
    Context::iterator p = _context.find(k);
    if(p == _context.end())
    {
        return "";
    }
    else
    {
        string oldVal = p->second;
        _context.erase(p);
        return oldVal;
    }
}

void
SharedImplicitContext::write(const Context& proxyCtx, ::Ice::OutputStream* s) const
{
    unique_lock lock(_mutex);
    if(proxyCtx.size() == 0)
    {
        s->write(_context);
    }
    else if(_context.size() == 0)
    {
        lock.unlock();
        s->write(proxyCtx);
    }
    else
    {
        Context combined = proxyCtx;
        combined.insert(_context.begin(), _context.end());
        lock.unlock();
        s->write(combined);
    }
}

void
SharedImplicitContext::combine(const Context& proxyCtx, Context& ctx) const
{
    lock_guard lock(_mutex);
    if(proxyCtx.size() == 0)
    {
        ctx = _context;
    }
    else if(_context.size() == 0)
    {
        ctx = proxyCtx;
    }
    else
    {
        ctx = proxyCtx;
        ctx.insert(_context.begin(), _context.end());
    }
}

//
// PerThreadImplicitContext implementation
//
long PerThreadImplicitContext::_nextId;
long PerThreadImplicitContext::_destroyedIds;
size_t PerThreadImplicitContext::_slotVectors;
PerThreadImplicitContext::IndexInUse* PerThreadImplicitContext::_indexInUse;
mutex PerThreadImplicitContext::_mutex;

#   ifdef _WIN32
DWORD PerThreadImplicitContext::_key;
#   else
pthread_key_t PerThreadImplicitContext::_key;
#   endif

PerThreadImplicitContext::PerThreadImplicitContext()
{
    lock_guard lock(_mutex);
    _id = _nextId++;
    if(_id == 0)
    {
#   ifdef _WIN32
        _key = TlsAlloc();
        if(_key == TLS_OUT_OF_INDEXES)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#   else
        int err = pthread_key_create(&_key, &iceImplicitContextThreadDestructor);
        if(err != 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
        }
#   endif
    }

    //
    // Now grab an index
    //
    if(_indexInUse == 0)
    {
        _indexInUse = new IndexInUse(1);
    }
    size_t i = 0;
    while(i < _indexInUse->size() && (*_indexInUse)[i])
    {
        i++;
    }

    if(i == _indexInUse->size())
    {
        _indexInUse->resize(i + 1);
    }
    (*_indexInUse)[i] = true;
    _index = i;
}

PerThreadImplicitContext::~PerThreadImplicitContext()
{
    lock_guard lock(_mutex);
    (*_indexInUse)[_index] = false;

    if(find(_indexInUse->begin(), _indexInUse->end(), true) == _indexInUse->end())
    {
        delete _indexInUse;
        _indexInUse = 0;
    }

    _destroyedIds++;
    tryCleanupKey();
}

void
PerThreadImplicitContext::tryCleanupKey()
{
    if(_destroyedIds == _nextId && _slotVectors == 0)
    {
        //
        // We can do a full reset
        //
        _nextId = 0;
        _destroyedIds = 0;
#   ifdef _WIN32
        TlsFree(_key);
#   else
        pthread_key_delete(_key);
#   endif
    }
}

Context*
PerThreadImplicitContext::getThreadContext(bool allocate) const
{
#   ifdef _WIN32
    SlotVector* sv = static_cast<SlotVector*>(TlsGetValue(_key));
#   else
    SlotVector* sv = static_cast<SlotVector*>(pthread_getspecific(_key));
#   endif
    if(sv == 0)
    {
        if(!allocate)
        {
            return 0;
        }

        {
             lock_guard lock(_mutex);
             sv = new SlotVector(_index + 1);
             _slotVectors++;
        }

#   ifdef _WIN32

        if(TlsSetValue(_key, sv) == 0)
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
#   else
        if(int err = pthread_setspecific(_key, sv))
        {
            throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
        }
#   endif
    }
    else
    {
        if(sv->size() <= _index)
        {
            if(!allocate)
            {
                return 0;
            }
            else
            {
                sv->resize(_index + 1);
            }
        }
    }

    Slot& slot = (*sv)[_index];
    if(slot.context != 0)
    {
        if(slot.owner != _id)
        {
            //
            // Reuse the slot from another (dead) communicator
            //
            slot.context->clear();
            slot.owner = _id;
        }
        //
        // else keep this slot.context
        //
    }
    else
    {
        if(allocate)
        {
            slot.context = new Context;
            slot.owner = _id;
        }
        //
        // else keep null slot.context
        //
    }
    return slot.context;
}

void
PerThreadImplicitContext::clearThreadContext() const
{
#   ifdef _WIN32
    SlotVector* sv = static_cast<SlotVector*>(TlsGetValue(_key));
#   else
    SlotVector* sv = static_cast<SlotVector*>(pthread_getspecific(_key));
#   endif
    if(sv != 0 && _index < sv->size())
    {
        delete (*sv)[_index].context;
        (*sv)[_index].context = 0;

        //
        // Trim tailing empty contexts.
        //
        size_t i = sv->size();

        bool clear = true;
        while(i != 0)
        {
            i--;
            if((*sv)[i].context != 0)
            {
                clear = false;
                break;
            }
        }

        //
        // If we did not find any contexts, delete the SlotVector.
        //
        if(clear)
        {
            delete sv;
#   ifdef _WIN32
            if(TlsSetValue(_key, 0) == 0)
            {
                throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
            }
#   else
            if(int err = pthread_setspecific(_key, 0))
            {
                throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, err);
            }

            {
                lock_guard lock(_mutex);
                _slotVectors--;
            }
#   endif
        }
        else
        {
            sv->resize(i + 1);
        }
    }
}

Context
PerThreadImplicitContext::getContext() const
{
    Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
        return Context();
    }
    else
    {
        return *ctx;
    }
}

void
PerThreadImplicitContext::setContext(const Context& newContext)
{
    if(newContext.size() == 0)
    {
        clearThreadContext();
    }
    else
    {
        Context* ctx = getThreadContext(true);
        assert(ctx != 0);
        *ctx = newContext;
    }
}

bool
PerThreadImplicitContext::containsKey(const string& k) const
{
    const Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
        return false;
    }
    Context::const_iterator p = ctx->find(k);
    return p != ctx->end();
}

string
PerThreadImplicitContext::get(const string& k) const
{
    const Context* ctx = getThreadContext(false);
    if(ctx == 0)
    {
        return "";
    }
    Context::const_iterator p = ctx->find(k);
    if(p == ctx->end())
    {
        return "";
    }
    return p->second;
}

string
PerThreadImplicitContext::put(const string& k, const string& v)
{
    Context* ctx = getThreadContext(true);

    string& val = (*ctx)[k];

    string oldVal = val;
    val = v;
    return oldVal;
}

string
PerThreadImplicitContext::remove(const string& k)
{
     Context* ctx = getThreadContext(false);
     if(ctx == 0)
     {
         return "";
     }

     Context::iterator p = ctx->find(k);
     if(p == ctx->end())
     {
         return "";
     }
     else
     {
         string oldVal = p->second;
         ctx->erase(p);

         if(ctx->size() == 0)
         {
             clearThreadContext();
         }
         return oldVal;
    }
}

void
PerThreadImplicitContext::write(const Context& proxyCtx, ::Ice::OutputStream* s) const
{
    const Context* threadCtx = getThreadContext(false);

    if(threadCtx == 0 || threadCtx->size() == 0)
    {
        s->write(proxyCtx);
    }
    else if(proxyCtx.size() == 0)
    {
        s->write(*threadCtx);
    }
    else
    {
        Context combined = proxyCtx;
        combined.insert(threadCtx->begin(), threadCtx->end());
        s->write(combined);
    }
}

void
PerThreadImplicitContext::combine(const Context& proxyCtx, Context& ctx) const
{
    const Context* threadCtx = getThreadContext(false);

    if(threadCtx == 0 || threadCtx->size() == 0)
    {
        ctx = proxyCtx;
    }
    else if(proxyCtx.size() == 0)
    {
        ctx = *threadCtx;
    }
    else
    {
        ctx = proxyCtx;
        ctx.insert(threadCtx->begin(), threadCtx->end());
    }
}

extern "C" void iceImplicitContextThreadDestructor(void* v)
{
    PerThreadImplicitContext::SlotVector* sv = static_cast<PerThreadImplicitContext::SlotVector*>(v);
    if(sv != 0)
    {
        //
        // Cleanup each slot
        //
        for(PerThreadImplicitContext::SlotVector::iterator p = sv->begin(); p != sv->end(); ++p)
        {
            delete p->context;
        }
        //
        // Then the vector
        //
        delete sv;

        {
            lock_guard lock(PerThreadImplicitContext::_mutex);
            PerThreadImplicitContext::_slotVectors--;
            PerThreadImplicitContext::tryCleanupKey();
        }
    }
}
