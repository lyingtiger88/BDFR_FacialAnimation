#pragma once

#include "bdfr/capture/FaceObservation.h"

namespace bdfr::capture {

struct CaptureQualityReport {
    float overall = 0.0F;
    float landmarkConfidence = 0.0F;
    float regionConfidence = 0.0F;
    bool occlusionPenaltyApplied = false;
    bool badTake = false;
};

class CaptureQuality {
public:
    static CaptureQualityReport evaluate(const FaceObservation& observation,
                                         float badTakeThreshold = 0.45F);
};

} // namespace bdfr::capture
