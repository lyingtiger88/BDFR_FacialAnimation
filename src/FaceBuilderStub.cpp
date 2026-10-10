#include "bdfr/facebuilder/FaceBuilder.h"

namespace bdfr::facebuilder {

bool FaceBuilder::backendAvailable() noexcept {
    return false;
}

const char* FaceBuilder::backendName() noexcept {
    return "Unavailable";
}

bool FaceBuilder::loadPtsLandmarks(
    const std::string&,
    std::vector<FaceBuilderLandmark>&,
    std::string* error) {
    if (error) {
        *error =
            "BDFR was built without eos FaceBuilder support.";
    }
    return false;
}

bool FaceBuilder::fit(
    const FaceBuilderImage&,
    const std::vector<FaceBuilderLandmark>&,
    const FaceBuilderOptions&,
    FaceBuilderResult&,
    std::string* error) const {
    if (error) {
        *error =
            "eos FaceBuilder backend is unavailable.";
    }
    return false;
}

} // namespace bdfr::facebuilder
