#include "bdfr/core/CurveMixer.h"

#include <unordered_set>

namespace bdfr {

CurveMap CurveMixer::mix(const CurveMap& base, const std::vector<CurveLayer>& layers) {
    CurveMap result = base;
    for (const CurveLayer& layer : layers) {
        const float weight = clamp01(layer.weight);
        for (const auto& [id, target] : layer.curves) {
            const float t = clamp01(target);
            float& value = result[id];
            if (layer.additive)
                value = clamp01(value + t * weight);
            else
                value = clamp01(value + (t - value) * weight);
        }
    }
    return result;
}

CurveMap CurveMixer::exponentialSmooth(const CurveMap& previous, const CurveMap& current, float alpha) {
    const float a = clamp01(alpha);
    CurveMap out;
    std::unordered_set<std::string> ids;
    for (const auto& [id, _] : previous) ids.insert(id);
    for (const auto& [id, _] : current) ids.insert(id);

    for (const std::string& id : ids) {
        const auto p = previous.find(id);
        const auto c = current.find(id);
        const float pv = (p != previous.end()) ? p->second : 0.0F;
        const float cv = (c != current.end()) ? c->second : 0.0F;
        out[id] = clamp01(pv + (cv - pv) * a);
    }
    return out;
}

} // namespace bdfr
