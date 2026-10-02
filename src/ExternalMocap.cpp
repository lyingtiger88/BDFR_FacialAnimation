#include "bdfr/mocap/ExternalMocap.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bdfr::mocap {

bool ExternalMocapProfile::addRule(ExternalCurveRule rule) {
    if (rule.externalName.empty() || rule.bdfrCurve.empty() ||
        !std::isfinite(rule.scale) || !std::isfinite(rule.bias) ||
        !std::isfinite(rule.minValue) || !std::isfinite(rule.maxValue) ||
        rule.minValue > rule.maxValue) {
        return false;
    }
    rules_.push_back(std::move(rule));
    return true;
}

const std::vector<ExternalCurveRule>& ExternalMocapProfile::rules() const noexcept {
    return rules_;
}

FacialFrame ExternalMocapProfile::normalize(const CurveMap& externalCurves,
                                            double timestampSeconds,
                                            float confidence,
                                            const HeadPose& head,
                                            const Gaze& gaze) const {
    FacialFrame out;
    out.timestampSeconds = std::isfinite(timestampSeconds) && timestampSeconds >= 0.0
        ? timestampSeconds : 0.0;
    out.confidence = clamp01(confidence);
    out.head = head;
    out.gaze = gaze;

    for (const auto& rule : rules_) {
        const auto it = externalCurves.find(rule.externalName);
        if (it == externalCurves.end()) continue;

        float value = clamp01(it->second);
        if (rule.invert) value = 1.0F - value;
        value = value * rule.scale + rule.bias;
        value = std::clamp(value, rule.minValue, rule.maxValue);
        out.curves[rule.bdfrCurve] = value;
    }

    return out;
}

} // namespace bdfr::mocap
