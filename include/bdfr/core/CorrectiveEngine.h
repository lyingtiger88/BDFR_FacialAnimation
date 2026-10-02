#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <vector>

namespace bdfr {

enum class CorrectiveMode {
    Add,
    Override,
    Multiply
};

struct CurveCondition {
    std::string curveId;
    float minValue = 0.0F;
    float maxValue = 1.0F;
};

struct CorrectiveRule {
    std::string id;
    std::vector<CurveCondition> conditions;
    std::string targetCurve;
    float targetValue = 0.0F;
    float weight = 1.0F;
    CorrectiveMode mode = CorrectiveMode::Add;
};

class CorrectiveEngine {
public:
    bool addRule(CorrectiveRule rule);
    const std::vector<CorrectiveRule>& rules() const noexcept;
    CurveMap apply(const CurveMap& input) const;

private:
    std::vector<CorrectiveRule> rules_;
};

} // namespace bdfr
