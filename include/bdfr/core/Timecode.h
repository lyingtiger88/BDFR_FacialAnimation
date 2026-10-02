#pragma once

#include <cstdint>
#include <string>

namespace bdfr {

struct FrameRate {
    std::uint32_t numerator = 30;
    std::uint32_t denominator = 1;

    bool valid() const noexcept;
    double framesPerSecond() const noexcept;
    double frameDurationSeconds() const noexcept;

    static FrameRate Fps24() { return {24, 1}; }
    static FrameRate Fps25() { return {25, 1}; }
    static FrameRate Fps30() { return {30, 1}; }
    static FrameRate Fps50() { return {50, 1}; }
    static FrameRate Fps60() { return {60, 1}; }
    static FrameRate Ntsc2997() { return {30000, 1001}; }
    static FrameRate Ntsc5994() { return {60000, 1001}; }
};

struct Timecode {
    std::uint32_t hours = 0;
    std::uint32_t minutes = 0;
    std::uint32_t seconds = 0;
    std::uint32_t frames = 0;

    std::string toString() const;
};

class TimecodeConverter {
public:
    static std::int64_t secondsToFrame(
        double seconds,
        FrameRate rate);

    static double frameToSeconds(
        std::int64_t frame,
        FrameRate rate);

    static Timecode frameToTimecode(
        std::int64_t frame,
        FrameRate rate);

    static bool timecodeToFrame(
        const Timecode& timecode,
        FrameRate rate,
        std::int64_t& frame);
};

} // namespace bdfr
