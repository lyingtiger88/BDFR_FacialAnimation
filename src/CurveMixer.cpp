#include "bdfr/core/CurveMixer.h"

#include <algorithm>
#include <unordered_set>

namespace bdfr {

CurveRegion CurveMixer::classify(const std::string& curveId) {
    if (curveId.rfind("jaw", 0) == 0 || curveId == "AU26") return CurveRegion::Jaw;
    if (curveId.rfind("mouth", 0) == 0 || curveId == "AU10" || curveId == "AU12" ||
        curveId == "AU14" || curveId == "AU15" || curveId == "AU17" ||
        curveId == "AU20" || curveId == "AU23" || curveId == "AU25" || curveId == "AU28")
        return CurveRegion::Mouth;
    if (curveId.rfind("eye", 0) == 0 || curveId == "AU05" || curveId == "AU06" ||
        curveId == "AU07" || curveId == "AU45") return CurveRegion::Eyes;
    if (curveId.rfind("brow", 0) == 0 || curveId == "AU01" || curveId == "AU02" ||
        curveId == "AU04") return CurveRegion::Brows;
    if (curveId.rfind("cheek", 0) == 0) return CurveRegion::Cheeks;
    if (curveId.rfind("nose", 0) == 0 || curveId == "AU09") return CurveRegion::Nose;
    return CurveRegion::Mouth;
}

bool CurveMixer::regionEnabled(CurveRegionMask mask, CurveRegion region) {
    return (mask & regionMask(region)) != 0;
}

CurveMap CurveMixer::mix(const CurveMap& base, std::vector<CurveLayer> layers) {
    std::stable_sort(layers.begin(), layers.end(),
        [](const CurveLayer& a, const CurveLayer& b) { return a.priority < b.priority; });

    CurveMap result = base;
    for (const CurveLayer& layer : layers) {
        const float weight = clamp01(layer.weight);
        for (const auto& [id, target] : layer.curves) {
            if (!regionEnabled(layer.regionMask, classify(id))) continue;
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
