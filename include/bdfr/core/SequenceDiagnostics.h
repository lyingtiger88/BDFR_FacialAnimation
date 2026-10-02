#pragma once

#include "bdfr/core/Sequence.h"

#include <cstddef>
#include <string>
#include <vector>

namespace bdfr {

struct CurveSpike {
    std::string curveId;
    double timestampSeconds = 0.0;
    float previousValue = 0.0F;
    float value = 0.0F;
    float nextValue = 0.0F;
    float magnitude = 0.0F;
};

struct SequenceQualityReport {
    float meanConfidence = 0.0F;
    float minimumConfidence = 1.0F;
    std::size_t lowConfidenceFrames = 0;
    std::vector<CurveSpike> spikes;
};

class SequenceDiagnostics {
public:
    static SequenceQualityReport analyze(
        const FacialSequence& sequence,
        float lowConfidenceThreshold = 0.5F,
        float spikeThreshold = 0.45F,
        float neighborAgreementTolerance = 0.12F);
};

} // namespace bdfr
