//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_TRACE_LEVELS_H
#define ICE_TRACE_LEVELS_H

#include <Ice/TraceLevelsF.h>
#include <Ice/PropertiesF.h>

namespace IceInternal
{

class TraceLevels
{
public:

    TraceLevels(const ::Ice::PropertiesPtr&);

    const int network;
    const char* networkCat;

    const int protocol;
    const char* protocolCat;

    const int retry;
    const char* retryCat;

    const int location;
    const char* locationCat;

    const int slicing;
    const char* slicingCat;

    const int gc;
    const char* gcCat;

    const int threadPool;
    const char* threadPoolCat;
};

}

#endif
