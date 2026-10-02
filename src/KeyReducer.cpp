#include "bdfr/core/KeyReducer.h"

#include <algorithm>
#include <cmath>

namespace bdfr {

namespace {

float interpolate(const CurveKey& a, const CurveKey& b, double timeSeconds) {
    const double span = b.timeSeconds - a.timeSeconds;
    if (span <= 0.0) return a.value;
    const double t = std::clamp((timeSeconds - a.timeSeconds) / span, 0.0, 1.0);
    return static_cast<float>(a.value + (b.value - a.value) * t);
}

void reduceRange(const std::vector<CurveKey>& keys,
                 std::size_t first,
                 std::size_t last,
                 float tolerance,
                 std::vector<bool>& keep) {
    if (last <= first + 1) return;

    float maxDistance = -1.0F;
    std::size_t index = first;

    for (std::size_t i = first + 1; i < last; ++i) {
        const float expected = interpolate(keys[first], keys[last], keys[i].timeSeconds);
        const float distance = std::fabs(keys[i].value - expected);
        if (distance > maxDistance) {
            maxDistance = distance;
            index = i;
        }
    }

    if (maxDistance > tolerance) {
        keep[index] = true;
        reduceRange(keys, first, index, tolerance, keep);
        reduceRange(keys, index, last, tolerance, keep);
    }
}

bool validKeys(const std::vector<CurveKey>& keys) {
    double previous = -1.0;
    for (const auto& key : keys) {
        if (!std::isfinite(key.timeSeconds) || !std::isfinite(key.value) ||
            key.timeSeconds < 0.0 || key.timeSeconds <= previous) {
            return false;
        }
        previous = key.timeSeconds;
    }
    return true;
}

} // namespace

std::vector<CurveKey> KeyReducer::reduce(const std::vector<CurveKey>& keys, float tolerance) {
    if (keys.size() <= 2 || !std::isfinite(tolerance) || tolerance < 0.0F || !validKeys(keys))
        return keys;

    std::vector<bool> keep(keys.size(), false);
    keep.front() = true;
    keep.back() = true;
    reduceRange(keys, 0, keys.size() - 1, tolerance, keep);

    std::vector<CurveKey> out;
    out.reserve(keys.size());
    for (std::size_t i = 0; i < keys.size(); ++i)
        if (keep[i]) out.push_back(keys[i]);
    return out;
}

float KeyReducer::maxError(const std::vector<CurveKey>& original,
                           const std::vector<CurveKey>& reduced) {
    if (original.empty() || reduced.empty() || !validKeys(original) || !validKeys(reduced))
        return 0.0F;

    float maxErrorValue = 0.0F;
    std::size_t segment = 0;

    for (const auto& key : original) {
        while (segment + 1 < reduced.size() &&
               key.timeSeconds > reduced[segment + 1].timeSeconds) {
            ++segment;
        }

        float reconstructed = reduced.back().value;
        if (segment + 1 < reduced.size())
            reconstructed = interpolate(reduced[segment], reduced[segment + 1], key.timeSeconds);

        maxErrorValue = std::max(maxErrorValue, std::fabs(key.value - reconstructed));
    }
    return maxErrorValue;
}

} // namespace bdfr
