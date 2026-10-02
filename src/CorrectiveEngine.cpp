#include "bdfr/core/CorrectiveEngine.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bdfr {

bool CorrectiveEngine::addRule(CorrectiveRule rule) {
    if (rule.id.empty() || rule.targetCurve.empty() || !std::isfinite(rule.targetValue) ||
        !std::isfinite(rule.weight) || rule.weight < 0.0F || rule.weight > 1.0F)
        return false;

    for (const auto& condition : rule.conditions) {
        if (condition.curveId.empty() || !std::isfinite(condition.minValue) ||
            !std::isfinite(condition.maxValue) || condition.minValue < 0.0F ||
            condition.maxValue > 1.0F || condition.maxValue < condition.minValue)
            return false;
    }

    rules_.push_back(std::move(rule));
    return true;
}

const std::vector<CorrectiveRule>& CorrectiveEngine::rules() const noexcept {
    return rules_;
}

CurveMap CorrectiveEngine::apply(const CurveMap& input) const {
    CurveMap out = input;

    for (const auto& rule : rules_) {
        bool matches = true;
        for (const auto& condition : rule.conditions) {
            const auto it = input.find(condition.curveId);
            const float value = it == input.end() ? 0.0F : it->second;
            if (value < condition.minValue || value > condition.maxValue) {
                matches = false;
                break;
            }
        }
        if (!matches) continue;

        float& target = out[rule.targetCurve];
        const float v = clamp01(rule.targetValue);
        const float w = clamp01(rule.weight);
        switch (rule.mode) {
            case CorrectiveMode::Add:
                target = clamp01(target + v * w);
                break;
            case CorrectiveMode::Override:
                target = clamp01(target + (v - target) * w);
                break;
            case CorrectiveMode::Multiply:
                target = clamp01(target * (1.0F + (v - 1.0F) * w));
                break;
        }
    }
    return out;
}

} // namespace bdfr
