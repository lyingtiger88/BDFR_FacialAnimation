#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <unordered_map>

namespace bdfr {

struct CurveTuningRule {
    float inputMin = 0.0F;
    float inputMax = 1.0F;
    float gain = 1.0F;
    float bias = 0.0F;
    float responseExponent = 1.0F;
    float deadZone = 0.0F;
    float outputMin = 0.0F;
    float outputMax = 1.0F;
};

class CurveTuningProfile {
public:
    std::string name;

    bool setRule(const std::string& curveId, CurveTuningRule rule);
    const CurveTuningRule* rule(const std::string& curveId) const;

    float applyValue(const std::string& curveId, float value) const;
    CurveMap apply(const CurveMap& curves) const;

private:
    std::unordered_map<std::string, CurveTuningRule> rules_;
};

} // namespace bdfr
