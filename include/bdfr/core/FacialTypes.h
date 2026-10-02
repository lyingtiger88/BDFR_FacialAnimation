#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace bdfr {

using CurveMap = std::unordered_map<std::string, float>;

struct HeadPose {
    float pitch = 0.0F;
    float yaw = 0.0F;
    float roll = 0.0F;
};

struct Gaze {
    float x = 0.0F;
    float y = 0.0F;
    float confidence = 0.0F;
};

struct FacialFrame {
    std::uint32_t schemaVersion = 1;
    double timestampSeconds = 0.0;
    float confidence = 1.0F;
    CurveMap curves;
    HeadPose head;
    Gaze gaze;
};

inline float clamp01(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}

inline bool isFinite(float value) { return std::isfinite(value); }
inline bool isFinite(double value) { return std::isfinite(value); }

} // namespace bdfr
