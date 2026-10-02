#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstdint>

namespace bdfr::expression {

struct BehaviorProfile {
    double blinkIntervalSeconds = 4.0;
    double blinkDurationSeconds = 0.12;
    float blinkIntensity = 1.0F;

    double eyeDartPeriodSeconds = 2.5;
    float eyeDartStrength = 0.08F;

    double headMotionPeriodSeconds = 6.0;
    float headYawDegrees = 2.0F;
    float headPitchDegrees = 1.0F;
};

class BehaviorEngine {
public:
    static FacialFrame sample(const BehaviorProfile& profile,
                              double timeSeconds,
                              std::uint32_t personalitySeed = 0);
};

} // namespace bdfr::expression
