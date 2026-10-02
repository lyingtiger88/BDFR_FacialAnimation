#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace bdfr {

struct RetargetMapping {
    std::string sourceCurve;
    std::string targetCurve;
    float scale = 1.0F;
    float bias = 0.0F;
    float minValue = 0.0F;
    float maxValue = 1.0F;
    float deadZone = 0.0F;
    bool invert = false;
};

struct RetargetCompatibility {
    std::size_t mappedTargets = 0;
    std::size_t missingTargets = 0;
    std::vector<std::string> missing;
    float coverage = 0.0F;
};

class RetargetProfile {
public:
    std::string name;

    bool addMapping(RetargetMapping mapping);
    const std::vector<RetargetMapping>& mappings() const noexcept;
    CurveMap apply(const CurveMap& source) const;
    RetargetCompatibility scanCompatibility(const std::unordered_set<std::string>& targetNames) const;

private:
    std::vector<RetargetMapping> mappings_;
};

} // namespace bdfr
