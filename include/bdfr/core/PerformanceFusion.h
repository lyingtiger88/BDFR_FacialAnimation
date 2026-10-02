#pragma once

#include "bdfr/core/CurveMixer.h"

#include <string>
#include <vector>

namespace bdfr {

struct RegionWeight {
    CurveRegionMask mask = static_cast<CurveRegionMask>(CurveRegion::All);
    float weight = 1.0F;
};

struct FusionSourceConfig {
    std::string sourceId;
    bool enabled = true;
    int priority = 0;
    float defaultWeight = 1.0F;
    std::vector<RegionWeight> regionWeights;
};

struct FusionInput {
    std::string sourceId;
    CurveMap curves;
    float confidence = 1.0F;
};

class PerformanceFusion {
public:
    static CurveMap fuse(const CurveMap& base,
                         const std::vector<FusionInput>& inputs,
                         const std::vector<FusionSourceConfig>& configs);

private:
    static float weightForRegion(const FusionSourceConfig& config,
                                 CurveRegion region);
};

} // namespace bdfr
