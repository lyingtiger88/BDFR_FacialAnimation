#pragma once

#include "bdfr/core/Sequence.h"

#include <cstddef>
#include <string>
#include <unordered_map>

namespace bdfr {

struct CurveComparison {
    double meanAbsoluteError = 0.0;
    float maximumAbsoluteError = 0.0F;
    std::size_t samples = 0;
};

struct SequenceComparisonReport {
    double durationSeconds = 0.0;
    double sampleRateHz = 0.0;
    double meanAbsoluteError = 0.0;
    float maximumAbsoluteError = 0.0F;
    std::size_t sampleCount = 0;
    std::unordered_map<std::string, CurveComparison> curves;
};

class SequenceCompare {
public:
    static SequenceComparisonReport compare(
        const FacialSequence& a,
        const FacialSequence& b,
        double sampleRateHz = 60.0);
};

} // namespace bdfr
