#pragma once

#include "bdfr/core/FacialTypes.h"

#include <vector>

namespace bdfr {

struct CurveLayer {
    CurveMap curves;
    float weight = 1.0F;
    bool additive = false;
};

class CurveMixer {
public:
    static CurveMap mix(const CurveMap& base, const std::vector<CurveLayer>& layers);
    static CurveMap exponentialSmooth(const CurveMap& previous, const CurveMap& current, float alpha);
};

} // namespace bdfr
