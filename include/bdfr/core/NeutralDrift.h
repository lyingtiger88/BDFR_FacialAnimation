#pragma once

#include "bdfr/core/FacialTypes.h"
#include "bdfr/core/Sequence.h"

#include <string>
#include <unordered_map>

namespace bdfr {

struct NeutralDriftOptions {
    float baselineAlpha = 0.01F;
    float neutralThreshold = 0.18F;
    float maximumCorrection = 0.25F;
};

class NeutralDriftCorrector {
public:
    explicit NeutralDriftCorrector(
        NeutralDriftOptions options = {});

    void reset();

    CurveMap process(
        const CurveMap& input,
        float frameConfidence = 1.0F);

    const CurveMap& baseline() const noexcept;

    static FacialSequence correct(
        const FacialSequence& input,
        const NeutralDriftOptions& options = {});

private:
    NeutralDriftOptions options_;
    CurveMap baseline_;
};

} // namespace bdfr
