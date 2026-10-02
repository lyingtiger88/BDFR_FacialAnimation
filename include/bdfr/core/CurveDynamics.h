#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <unordered_map>

namespace bdfr {

struct CurveConstraint {
    float minValue = 0.0F;
    float maxValue = 1.0F;
    float maxVelocityPerSecond = 0.0F;
};

class CurveDynamics {
public:
    bool setConstraint(const std::string& curveId, CurveConstraint constraint);
    const CurveConstraint* constraint(const std::string& curveId) const;
    CurveMap apply(const CurveMap& previous, const CurveMap& target, double deltaSeconds) const;

private:
    std::unordered_map<std::string, CurveConstraint> constraints_;
};

} // namespace bdfr
