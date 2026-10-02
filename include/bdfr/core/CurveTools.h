#pragma once

#include "bdfr/core/CurveMixer.h"

#include <string>
#include <unordered_map>

namespace bdfr {

struct CurveDifference {
    float a = 0.0F;
    float b = 0.0F;
    float absoluteDelta = 0.0F;
};

class CurveTools {
public:
    static CurveMap freezeRegions(const CurveMap& base,
                                  const CurveMap& candidate,
                                  CurveRegionMask frozenRegions);

    static std::unordered_map<std::string, CurveDifference>
    diff(const CurveMap& a, const CurveMap& b, float threshold = 0.0F);
};

} // namespace bdfr
