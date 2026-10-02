#include "bdfr/capture/CaptureQuality.h"

#include <algorithm>

namespace bdfr::capture {

CaptureQualityReport CaptureQuality::evaluate(const FaceObservation& observation,
                                              float badTakeThreshold) {
    CaptureQualityReport report;

    if (!observation.valid()) {
        report.badTake = true;
        return report;
    }

    float landmarkSum = 0.0F;
    for (const auto& landmark : observation.landmarks) {
        landmarkSum += landmark.confidence;
    }
    report.landmarkConfidence = observation.landmarks.empty()
        ? observation.confidence
        : landmarkSum / static_cast<float>(observation.landmarks.size());

    report.regionConfidence =
        (observation.regions.mouth +
         observation.regions.leftEye +
         observation.regions.rightEye +
         observation.regions.brows +
         observation.regions.jaw) / 5.0F;

    report.overall =
        observation.confidence * 0.35F +
        report.landmarkConfidence * 0.35F +
        report.regionConfidence * 0.30F;

    if (observation.occluded) {
        report.overall *= 0.65F;
        report.occlusionPenaltyApplied = true;
    }

    report.overall = clamp01(report.overall);
    report.badTake = report.overall < clamp01(badTakeThreshold);
    return report;
}

} // namespace bdfr::capture
