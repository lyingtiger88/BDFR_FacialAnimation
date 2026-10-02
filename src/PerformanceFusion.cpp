#include "bdfr/core/PerformanceFusion.h"

#include <algorithm>

namespace bdfr {

float PerformanceFusion::weightForRegion(const FusionSourceConfig& config,
                                         CurveRegion region) {
    float weight = clamp01(config.defaultWeight);
    for (const auto& rule : config.regionWeights) {
        if (CurveMixer::regionEnabled(rule.mask, region)) {
            weight = clamp01(rule.weight);
        }
    }
    return weight;
}

CurveMap PerformanceFusion::fuse(const CurveMap& base,
                                 const std::vector<FusionInput>& inputs,
                                 const std::vector<FusionSourceConfig>& configs) {
    std::vector<CurveLayer> layers;

    for (const auto& input : inputs) {
        auto configIt = std::find_if(configs.begin(), configs.end(),
            [&](const FusionSourceConfig& config) {
                return config.sourceId == input.sourceId;
            });

        FusionSourceConfig fallback;
        fallback.sourceId = input.sourceId;
        const FusionSourceConfig& config =
            configIt == configs.end() ? fallback : *configIt;

        if (!config.enabled) continue;

        for (const auto& [curveId, value] : input.curves) {
            const CurveRegion region = CurveMixer::classify(curveId);
            const float weight =
                clamp01(weightForRegion(config, region) * clamp01(input.confidence));
            if (weight <= 0.0F) continue;

            CurveLayer layer;
            layer.curves[curveId] = value;
            layer.weight = weight;
            layer.priority = config.priority;
            layer.regionMask = regionMask(region);
            layers.push_back(std::move(layer));
        }
    }

    return CurveMixer::mix(base, std::move(layers));
}

} // namespace bdfr
