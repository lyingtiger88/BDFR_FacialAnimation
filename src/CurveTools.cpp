#include "bdfr/core/CurveTools.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace bdfr {

CurveMap CurveTools::freezeRegions(const CurveMap& base,
                                   const CurveMap& candidate,
                                   CurveRegionMask frozenRegions) {
    CurveMap out = candidate;
    std::unordered_set<std::string> ids;
    for (const auto& [id, _] : base) ids.insert(id);
    for (const auto& [id, _] : candidate) ids.insert(id);

    for (const auto& id : ids) {
        const CurveRegion region = CurveMixer::classify(id);
        if ((frozenRegions & regionMask(region)) == 0) continue;

        const auto it = base.find(id);
        if (it == base.end())
            out.erase(id);
        else
            out[id] = it->second;
    }
    return out;
}

std::unordered_map<std::string, CurveDifference>
CurveTools::diff(const CurveMap& a, const CurveMap& b, float threshold) {
    const float minDelta = std::max(0.0F, threshold);
    std::unordered_set<std::string> ids;
    for (const auto& [id, _] : a) ids.insert(id);
    for (const auto& [id, _] : b) ids.insert(id);

    std::unordered_map<std::string, CurveDifference> out;
    for (const auto& id : ids) {
        const auto ia = a.find(id);
        const auto ib = b.find(id);
        const float av = ia == a.end() ? 0.0F : ia->second;
        const float bv = ib == b.end() ? 0.0F : ib->second;
        const float delta = std::fabs(av - bv);
        if (delta >= minDelta) out[id] = {av, bv, delta};
    }
    return out;
}

} // namespace bdfr
