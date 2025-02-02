//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SLICED_DATA_F_H
#define ICE_SLICED_DATA_F_H

#include <Ice/Config.h>

#include <memory>
#include <vector>

namespace Ice
{

struct SliceInfo;
class SlicedData;
class UnknownSlicedValue;

using SliceInfoPtr = ::std::shared_ptr<SliceInfo>;
using SlicedDataPtr = ::std::shared_ptr<SlicedData>;

/// \cond INTERNAL
using UnknownSlicedValuePtr = ::std::shared_ptr<UnknownSlicedValue>;
/// \endcond

/** The slices of unknown types. */
typedef ::std::vector<SliceInfoPtr> SliceInfoSeq;

}

#endif
