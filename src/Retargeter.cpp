#include "bdfr/core/Retargeter.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bdfr {

bool RetargetProfile::addMapping(RetargetMapping mapping) {
    if (mapping.sourceCurve.empty() || mapping.targetCurve.empty() ||
        !std::isfinite(mapping.scale) || !std::isfinite(mapping.bias) ||
        !std::isfinite(mapping.minValue) || !std::isfinite(mapping.maxValue) ||
        !std::isfinite(mapping.deadZone) || mapping.minValue > mapping.maxValue ||
        mapping.deadZone < 0.0F || mapping.deadZone > 1.0F) {
        return false;
    }

    mappings_.push_back(std::move(mapping));
    return true;
}

const std::vector<RetargetMapping>& RetargetProfile::mappings() const noexcept {
    return mappings_;
}

CurveMap RetargetProfile::apply(const CurveMap& source) const {
    CurveMap out;

    for (const auto& mapping : mappings_) {
        const auto it = source.find(mapping.sourceCurve);
        if (it == source.end()) continue;

        float value = clamp01(it->second);
        if (mapping.invert) value = 1.0F - value;
        if (std::fabs(value) < mapping.deadZone) value = 0.0F;
        value = value * mapping.scale + mapping.bias;
        value = std::clamp(value, mapping.minValue, mapping.maxValue);
        out[mapping.targetCurve] = value;
    }

    return out;
}

RetargetCompatibility
RetargetProfile::scanCompatibility(const std::unordered_set<std::string>& targetNames) const {
    RetargetCompatibility report;

    for (const auto& mapping : mappings_) {
        if (targetNames.find(mapping.targetCurve) != targetNames.end()) {
            ++report.mappedTargets;
        } else {
            ++report.missingTargets;
            report.missing.push_back(mapping.targetCurve);
        }
    }

    const std::size_t total = report.mappedTargets + report.missingTargets;
    report.coverage = total == 0 ? 1.0F :
        static_cast<float>(report.mappedTargets) / static_cast<float>(total);
    return report;
}

} // namespace bdfr
