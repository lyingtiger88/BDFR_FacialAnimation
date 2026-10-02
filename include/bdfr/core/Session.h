#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bdfr {

struct TimeRange {
    double startSeconds = 0.0;
    double endSeconds = 0.0;

    bool isValid() const noexcept;
    bool contains(double timeSeconds) const noexcept;
    bool intersects(const TimeRange& other) const noexcept;
};

struct Take {
    std::string id;
    std::string name;
    std::string actorId;
    std::string source;
    double durationSeconds = 0.0;
    std::uint32_t frameRateNumerator = 30;
    std::uint32_t frameRateDenominator = 1;
    std::vector<TimeRange> dirtyRanges;
};

struct Session {
    std::string id;
    std::string project;
    std::string scene;
    std::string shot;
    std::vector<Take> takes;

    bool addTake(Take take);
    Take* findTake(const std::string& takeId);
    const Take* findTake(const std::string& takeId) const;
    bool markDirty(const std::string& takeId, TimeRange range);
};

} // namespace bdfr
