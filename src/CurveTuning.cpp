#include "bdfr/core/CurveTuning.h"

#include <algorithm>
#include <cmath>

namespace bdfr {

bool CurveTuningProfile::setRule(const std::string& curveId, CurveTuningRule ruleValue) {
    if (curveId.empty() ||
        !std::isfinite(ruleValue.inputMin) ||
        !std::isfinite(ruleValue.inputMax) ||
        !std::isfinite(ruleValue.gain) ||
        !std::isfinite(ruleValue.bias) ||
        !std::isfinite(ruleValue.responseExponent) ||
        !std::isfinite(ruleValue.deadZone) ||
        !std::isfinite(ruleValue.outputMin) ||
        !std::isfinite(ruleValue.outputMax) ||
        ruleValue.inputMax <= ruleValue.inputMin ||
        ruleValue.responseExponent <= 0.0F ||
        ruleValue.deadZone < 0.0F ||
        ruleValue.outputMax < ruleValue.outputMin) {
        return false;
    }

    rules_[curveId] = ruleValue;
    return true;
}

const CurveTuningRule* CurveTuningProfile::rule(const std::string& curveId) const {
    const auto it = rules_.find(curveId);
    return it == rules_.end() ? nullptr : &it->second;
}

float CurveTuningProfile::applyValue(const std::string& curveId, float value) const {
    const CurveTuningRule* tuning = rule(curveId);
    if (!tuning) return clamp01(value);

    const float normalized = std::clamp(
        (value - tuning->inputMin) / (tuning->inputMax - tuning->inputMin),
        0.0F, 1.0F);

    float shaped = normalized <= tuning->deadZone
        ? 0.0F
        : (normalized - tuning->deadZone) / std::max(0.000001F, 1.0F - tuning->deadZone);

    shaped = std::pow(std::clamp(shaped, 0.0F, 1.0F), tuning->responseExponent);
    shaped = shaped * tuning->gain + tuning->bias;
    return std::clamp(shaped, tuning->outputMin, tuning->outputMax);
}

CurveMap CurveTuningProfile::apply(const CurveMap& curves) const {
    CurveMap out;
    for (const auto& [id, value] : curves) {
        out[id] = applyValue(id, value);
    }
    return out;
}

} // namespace bdfr
