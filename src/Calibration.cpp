#include "bdfr/core/Calibration.h"

#include <algorithm>
#include <cmath>

namespace bdfr {

bool CalibrationProfile::setRange(const std::string& curveId, CalibrationRange rangeValue) {
    if (curveId.empty() ||
        !std::isfinite(rangeValue.neutral) ||
        !std::isfinite(rangeValue.minimum) ||
        !std::isfinite(rangeValue.maximum) ||
        rangeValue.maximum <= rangeValue.minimum ||
        rangeValue.neutral < rangeValue.minimum ||
        rangeValue.neutral > rangeValue.maximum) {
        return false;
    }
    ranges_[curveId] = rangeValue;
    return true;
}

const CalibrationRange* CalibrationProfile::range(const std::string& curveId) const {
    auto it = ranges_.find(curveId);
    return it == ranges_.end() ? nullptr : &it->second;
}

float CalibrationProfile::normalizeValue(const std::string& curveId, float rawValue) const {
    const CalibrationRange* calibration = range(curveId);
    if (!calibration || !std::isfinite(rawValue)) {
        return std::isfinite(rawValue) ? clamp01(rawValue) : 0.0F;
    }

    float normalized = 0.0F;
    if (rawValue >= calibration->neutral) {
        const float span = calibration->maximum - calibration->neutral;
        normalized = span > 0.0F ? (rawValue - calibration->neutral) / span : 0.0F;
    } else {
        const float span = calibration->neutral - calibration->minimum;
        normalized = span > 0.0F ? (calibration->neutral - rawValue) / span : 0.0F;
    }

    normalized = clamp01(normalized);
    return calibration->invert ? 1.0F - normalized : normalized;
}

CurveMap CalibrationProfile::normalize(const CurveMap& rawCurves) const {
    CurveMap out;
    for (const auto& [id, raw] : rawCurves) {
        out[id] = normalizeValue(id, raw);
    }
    return out;
}

const std::unordered_map<std::string, CalibrationRange>&
CalibrationProfile::ranges() const noexcept {
    return ranges_;
}

} // namespace bdfr
