#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace bdfr::capture {

enum class PixelFormat {
    Gray8,
    RGB24,
    RGBA32,
    NV21,
    YUV420
};

struct ImageView {
    const std::uint8_t* data = nullptr;
    std::size_t sizeBytes = 0;
    int width = 0;
    int height = 0;
    int strideBytes = 0;
    PixelFormat format = PixelFormat::RGB24;
    double timestampSeconds = 0.0;

    bool valid() const noexcept;
};

struct FaceLandmark {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float confidence = 0.0F;
};

struct RegionConfidence {
    float mouth = 0.0F;
    float leftEye = 0.0F;
    float rightEye = 0.0F;
    float brows = 0.0F;
    float jaw = 0.0F;
};

struct FaceObservation {
    double timestampSeconds = 0.0;
    float confidence = 0.0F;
    std::vector<FaceLandmark> landmarks;
    HeadPose head;
    Gaze gaze;
    RegionConfidence regions;
    bool occluded = false;

    bool valid() const noexcept;
};

class IFaceTracker {
public:
    virtual ~IFaceTracker() = default;
    virtual bool process(const ImageView& image, FaceObservation& observation) = 0;
    virtual void reset() = 0;
};

} // namespace bdfr::capture
