#pragma once

#include "bdfr/core/Sequence.h"

#include <cstddef>

namespace bdfr {

class SequenceFilter {
public:
    static FacialSequence movingAverage(const FacialSequence& input,
                                        std::size_t radiusFrames = 1);

    static FacialSequence bidirectionalExponential(
        const FacialSequence& input,
        float alpha = 0.35F);
};

} // namespace bdfr
