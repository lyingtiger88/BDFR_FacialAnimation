#include "bdfr/core/Session.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace bdfr {

bool TimeRange::isValid() const noexcept {
    return std::isfinite(startSeconds) && std::isfinite(endSeconds) &&
           startSeconds >= 0.0 && endSeconds >= startSeconds;
}

bool TimeRange::contains(double timeSeconds) const noexcept {
    return isValid() && std::isfinite(timeSeconds) &&
           timeSeconds >= startSeconds && timeSeconds <= endSeconds;
}

bool TimeRange::intersects(const TimeRange& other) const noexcept {
    return isValid() && other.isValid() &&
           startSeconds <= other.endSeconds && other.startSeconds <= endSeconds;
}

bool Session::addTake(Take take) {
    if (take.id.empty() || take.name.empty() || !std::isfinite(take.durationSeconds) ||
        take.durationSeconds < 0.0 || take.frameRateNumerator == 0 ||
        take.frameRateDenominator == 0 || findTake(take.id) != nullptr) {
        return false;
    }
    takes.push_back(std::move(take));
    return true;
}

Take* Session::findTake(const std::string& takeId) {
    auto it = std::find_if(takes.begin(), takes.end(),
        [&](const Take& take) { return take.id == takeId; });
    return it == takes.end() ? nullptr : &(*it);
}

const Take* Session::findTake(const std::string& takeId) const {
    auto it = std::find_if(takes.begin(), takes.end(),
        [&](const Take& take) { return take.id == takeId; });
    return it == takes.end() ? nullptr : &(*it);
}

bool Session::markDirty(const std::string& takeId, TimeRange range) {
    Take* take = findTake(takeId);
    if (!take || !range.isValid() || range.endSeconds > take->durationSeconds) return false;

    take->dirtyRanges.push_back(range);
    std::sort(take->dirtyRanges.begin(), take->dirtyRanges.end(),
        [](const TimeRange& a, const TimeRange& b) { return a.startSeconds < b.startSeconds; });

    std::vector<TimeRange> merged;
    for (const TimeRange& current : take->dirtyRanges) {
        if (merged.empty() || merged.back().endSeconds < current.startSeconds) {
            merged.push_back(current);
        } else {
            merged.back().endSeconds = std::max(merged.back().endSeconds, current.endSeconds);
        }
    }
    take->dirtyRanges = std::move(merged);
    return true;
}

} // namespace bdfr
