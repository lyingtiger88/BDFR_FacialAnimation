#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstdint>
#include <vector>

namespace bdfr {

enum class CurveRegion : std::uint32_t {
    None   = 0,
    Mouth  = 1u << 0,
    Eyes   = 1u << 1,
    Brows  = 1u << 2,
    Cheeks = 1u << 3,
    Nose   = 1u << 4,
    Jaw    = 1u << 5,
    Head   = 1u << 6,
    All    = 0xFFFFFFFFu
};

using CurveRegionMask = std::uint32_t;

constexpr CurveRegionMask regionMask(CurveRegion region) {
    return static_cast<CurveRegionMask>(region);
}

constexpr CurveRegionMask operator|(CurveRegion a, CurveRegion b) {
    return regionMask(a) | regionMask(b);
}

struct CurveLayer {
    CurveMap curves;
    float weight = 1.0F;
    bool additive = false;
    int priority = 0;
    CurveRegionMask regionMask = static_cast<CurveRegionMask>(CurveRegion::All);
};

class CurveMixer {
public:
    static CurveRegion classify(const std::string& curveId);
    static bool regionEnabled(CurveRegionMask mask, CurveRegion region);
    static CurveMap mix(const CurveMap& base, std::vector<CurveLayer> layers);
    static CurveMap exponentialSmooth(const CurveMap& previous, const CurveMap& current, float alpha);
};

} // namespace bdfr
