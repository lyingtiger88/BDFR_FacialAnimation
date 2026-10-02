#pragma once

#include "bdfr/core/Sequence.h"

namespace bdfr {

class SequenceCleanup {
public:
    static FacialSequence despike(
        const FacialSequence& input,
        float spikeThreshold = 0.45F,
        float neighborAgreementTolerance = 0.12F);
};

} // namespace bdfr
