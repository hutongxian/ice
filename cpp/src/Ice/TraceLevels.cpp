//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/TraceLevels.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::TraceLevels::TraceLevels(const PropertiesPtr& properties) :
    network(0),
    networkCat("Network"),
    protocol(0),
    protocolCat("Protocol"),
    retry(0),
    retryCat("Retry"),
    location(0),
    locationCat("Locator"),
    slicing(0),
    slicingCat("Slicing"),
    gc(0),
    gcCat("GC"),
    threadPool(0),
    threadPoolCat("ThreadPool")
{
    const string keyBase = "Ice.Trace.";
    const_cast<int&>(network) = properties->getPropertyAsInt(keyBase + networkCat);
    const_cast<int&>(protocol) = properties->getPropertyAsInt(keyBase + protocolCat);
    const_cast<int&>(retry) = properties->getPropertyAsInt(keyBase + retryCat);
    const_cast<int&>(location) = properties->getPropertyAsInt(keyBase + locationCat);
    const_cast<int&>(slicing) = properties->getPropertyAsInt(keyBase + slicingCat);
    const_cast<int&>(gc) = properties->getPropertyAsInt(keyBase + gcCat);
    const_cast<int&>(threadPool) = properties->getPropertyAsInt(keyBase + threadPoolCat);
}
