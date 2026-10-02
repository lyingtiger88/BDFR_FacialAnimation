#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <unordered_map>

namespace bdfr {

struct CalibrationRange {
    float neutral = 0.0F;
    float minimum = 0.0F;
    float maximum = 1.0F;
    bool invert = false;
};

class CalibrationProfile {
public:
    std::string actorId;
    std::string name;

    bool setRange(const std::string& curveId, CalibrationRange range);
    const CalibrationRange* range(const std::string& curveId) const;

    float normalizeValue(const std::string& curveId, float rawValue) const;
    CurveMap normalize(const CurveMap& rawCurves) const;

    const std::unordered_map<std::string, CalibrationRange>& ranges() const noexcept;

private:
    std::unordered_map<std::string, CalibrationRange> ranges_;
};

} // namespace bdfr
