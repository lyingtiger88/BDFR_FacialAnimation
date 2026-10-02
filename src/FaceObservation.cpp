#include "bdfr/capture/FaceObservation.h"

#include <cmath>

namespace bdfr::capture {

bool ImageView::valid() const noexcept {
    return data != nullptr && sizeBytes > 0 && width > 0 && height > 0 &&
           strideBytes > 0 && std::isfinite(timestampSeconds) && timestampSeconds >= 0.0;
}

bool FaceObservation::valid() const noexcept {
    if (!std::isfinite(timestampSeconds) || timestampSeconds < 0.0 ||
        !std::isfinite(confidence) || confidence < 0.0F || confidence > 1.0F) {
        return false;
    }

    for (const auto& landmark : landmarks) {
        if (!std::isfinite(landmark.x) || !std::isfinite(landmark.y) ||
            !std::isfinite(landmark.z) || !std::isfinite(landmark.confidence) ||
            landmark.confidence < 0.0F || landmark.confidence > 1.0F) {
            return false;
        }
    }
    return true;
}

} // namespace bdfr::capture
