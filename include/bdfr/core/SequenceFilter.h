#pragma once

#include "bdfr/core/Sequence.h"

#include <cstddef>

namespace bdfr {

class SequenceFilter {
public:
    static FacialSequence movingAverage(const FacialSequence& input,
                                        std::size_t radiusFrames = 1);
};

} // namespace bdfr
