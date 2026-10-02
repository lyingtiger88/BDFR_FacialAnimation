#pragma once

#include "bdfr/core/Retargeter.h"

#include <string>
#include <vector>

namespace bdfr {

struct MappingSuggestion {
    std::string sourceCurve;
    std::string targetCurve;
    float confidence = 0.0F;
};

class AutoMapper {
public:
    static std::string normalizeName(const std::string& name);
    static std::vector<MappingSuggestion>
    suggest(const std::vector<std::string>& sourceCurves,
            const std::vector<std::string>& targetCurves,
            float minimumConfidence = 0.75F);
};

} // namespace bdfr
