#include "bdfr/expression/BehaviorEngine.h"

#include <algorithm>
#include <cmath>

namespace bdfr::expression {

namespace {
constexpr double Pi = 3.14159265358979323846;

double safePeriod(double value, double fallback) {
    return std::isfinite(value) && value > 0.001 ? value : fallback;
}

double phaseFromSeed(std::uint32_t seed) {
    return static_cast<double>(seed % 1000u) / 1000.0;
}
}

FacialFrame BehaviorEngine::sample(const BehaviorProfile& profile,
                                   double timeSeconds,
                                   std::uint32_t personalitySeed) {
    FacialFrame frame;
    frame.timestampSeconds = std::isfinite(timeSeconds) && timeSeconds >= 0.0
        ? timeSeconds : 0.0;

    const double seedPhase = phaseFromSeed(personalitySeed);

    const double blinkInterval = safePeriod(profile.blinkIntervalSeconds, 4.0);
    const double blinkDuration = std::clamp(profile.blinkDurationSeconds, 0.04, blinkInterval * 0.5);
    const double blinkTime = std::fmod(frame.timestampSeconds + seedPhase * blinkInterval, blinkInterval);
    if (blinkTime >= 0.0 && blinkTime <= blinkDuration) {
        const double normalized = blinkTime / blinkDuration;
        const float envelope = static_cast<float>(std::sin(normalized * Pi));
        const float blink = clamp01(envelope * clamp01(profile.blinkIntensity));
        frame.curves["eyeBlinkLeft"] = blink;
        frame.curves["eyeBlinkRight"] = blink;
    }

    const double eyePeriod = safePeriod(profile.eyeDartPeriodSeconds, 2.5);
    const double eyePhase = (frame.timestampSeconds / eyePeriod + seedPhase) * 2.0 * Pi;
    frame.gaze.x = std::sin(eyePhase) * std::max(0.0F, profile.eyeDartStrength);
    frame.gaze.y = std::sin(eyePhase * 0.61 + 1.2) *
                   std::max(0.0F, profile.eyeDartStrength) * 0.6F;
    frame.gaze.confidence = 1.0F;

    const double headPeriod = safePeriod(profile.headMotionPeriodSeconds, 6.0);
    const double headPhase = (frame.timestampSeconds / headPeriod + seedPhase) * 2.0 * Pi;
    frame.head.yaw = static_cast<float>(std::sin(headPhase)) * profile.headYawDegrees;
    frame.head.pitch = static_cast<float>(std::sin(headPhase * 0.53 + 0.7)) *
                       profile.headPitchDegrees;
    frame.head.roll = static_cast<float>(std::sin(headPhase * 0.37 + 1.9)) *
                      profile.headYawDegrees * 0.15F;

    return frame;
}

} // namespace bdfr::expression
