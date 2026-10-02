#include "bdfr/core/CurveDynamics.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace bdfr {

bool CurveDynamics::setConstraint(const std::string& curveId, CurveConstraint constraint) {
    if (curveId.empty() || !std::isfinite(constraint.minValue) ||
        !std::isfinite(constraint.maxValue) ||
        !std::isfinite(constraint.maxVelocityPerSecond) ||
        constraint.minValue < 0.0F || constraint.maxValue > 1.0F ||
        constraint.maxValue < constraint.minValue ||
        constraint.maxVelocityPerSecond < 0.0F) {
        return false;
    }
    constraints_[curveId] = constraint;
    return true;
}

const CurveConstraint* CurveDynamics::constraint(const std::string& curveId) const {
    auto it = constraints_.find(curveId);
    return it == constraints_.end() ? nullptr : &it->second;
}

CurveMap CurveDynamics::apply(const CurveMap& previous, const CurveMap& target, double deltaSeconds) const {
    CurveMap out;
    std::unordered_set<std::string> ids;
    for (const auto& [id, _] : previous) ids.insert(id);
    for (const auto& [id, _] : target) ids.insert(id);

    const double dt = std::isfinite(deltaSeconds) && deltaSeconds > 0.0 ? deltaSeconds : 0.0;

    for (const auto& id : ids) {
        const auto p = previous.find(id);
        const auto t = target.find(id);
        const float previousValue = p == previous.end() ? 0.0F : clamp01(p->second);
        float targetValue = t == target.end() ? 0.0F : clamp01(t->second);

        const CurveConstraint* rule = constraint(id);
        if (!rule) {
            out[id] = targetValue;
            continue;
        }

        targetValue = std::clamp(targetValue, rule->minValue, rule->maxValue);
        if (rule->maxVelocityPerSecond > 0.0F && dt > 0.0) {
            const float maxDelta = static_cast<float>(rule->maxVelocityPerSecond * dt);
            const float delta = std::clamp(targetValue - previousValue, -maxDelta, maxDelta);
            out[id] = std::clamp(previousValue + delta, rule->minValue, rule->maxValue);
        } else {
            out[id] = targetValue;
        }
    }
    return out;
}

} // namespace bdfr
