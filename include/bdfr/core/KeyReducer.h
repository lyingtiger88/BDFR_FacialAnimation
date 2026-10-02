#pragma once

#include <string>
#include <vector>

namespace bdfr {

struct CurveKey {
    double timeSeconds = 0.0;
    float value = 0.0F;
};

class KeyReducer {
public:
    static std::vector<CurveKey> reduce(const std::vector<CurveKey>& keys, float tolerance);
    static float maxError(const std::vector<CurveKey>& original,
                          const std::vector<CurveKey>& reduced);
};

} // namespace bdfr
