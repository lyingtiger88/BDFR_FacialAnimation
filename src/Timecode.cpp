#include "bdfr/core/Timecode.h"

#include <cmath>
#include <iomanip>
#include <sstream>

namespace bdfr {

bool FrameRate::valid() const noexcept {
    return numerator > 0 && denominator > 0;
}

double FrameRate::framesPerSecond() const noexcept {
    return valid()
        ? static_cast<double>(numerator) /
          static_cast<double>(denominator)
        : 0.0;
}

double FrameRate::frameDurationSeconds() const noexcept {
    const double fps = framesPerSecond();
    return fps > 0.0 ? 1.0 / fps : 0.0;
}

std::string Timecode::toString() const {
    std::ostringstream out;
    out << std::setfill('0')
        << std::setw(2) << hours << ':'
        << std::setw(2) << minutes << ':'
        << std::setw(2) << seconds << ':'
        << std::setw(2) << frames;
    return out.str();
}

std::int64_t TimecodeConverter::secondsToFrame(
    double seconds,
    FrameRate rate) {

    if (!rate.valid() ||
        !std::isfinite(seconds)) {
        return 0;
    }

    return static_cast<std::int64_t>(
        std::llround(
            seconds * rate.framesPerSecond()));
}

double TimecodeConverter::frameToSeconds(
    std::int64_t frame,
    FrameRate rate) {

    if (!rate.valid()) {
        return 0.0;
    }

    return static_cast<double>(frame) /
           rate.framesPerSecond();
}

Timecode TimecodeConverter::frameToTimecode(
    std::int64_t frame,
    FrameRate rate) {

    Timecode out;

    if (!rate.valid() || frame < 0) {
        return out;
    }

    // Display timecode uses the nearest integer nominal frame rate.
    // Drop-frame labeling is intentionally a later explicit feature.
    const std::uint32_t nominalFps =
        static_cast<std::uint32_t>(
            std::llround(rate.framesPerSecond()));

    if (nominalFps == 0) {
        return out;
    }

    std::uint64_t remaining =
        static_cast<std::uint64_t>(frame);

    const std::uint64_t framesPerHour =
        static_cast<std::uint64_t>(nominalFps) * 3600ULL;

    const std::uint64_t framesPerMinute =
        static_cast<std::uint64_t>(nominalFps) * 60ULL;

    out.hours =
        static_cast<std::uint32_t>(
            remaining / framesPerHour);

    remaining %= framesPerHour;

    out.minutes =
        static_cast<std::uint32_t>(
            remaining / framesPerMinute);

    remaining %= framesPerMinute;

    out.seconds =
        static_cast<std::uint32_t>(
            remaining / nominalFps);

    out.frames =
        static_cast<std::uint32_t>(
            remaining % nominalFps);

    return out;
}

bool TimecodeConverter::timecodeToFrame(
    const Timecode& timecode,
    FrameRate rate,
    std::int64_t& frame) {

    if (!rate.valid() ||
        timecode.minutes >= 60 ||
        timecode.seconds >= 60) {
        return false;
    }

    const std::uint32_t nominalFps =
        static_cast<std::uint32_t>(
            std::llround(rate.framesPerSecond()));

    if (nominalFps == 0 ||
        timecode.frames >= nominalFps) {
        return false;
    }

    const std::uint64_t totalSeconds =
        static_cast<std::uint64_t>(timecode.hours) * 3600ULL +
        static_cast<std::uint64_t>(timecode.minutes) * 60ULL +
        static_cast<std::uint64_t>(timecode.seconds);

    frame =
        static_cast<std::int64_t>(
            totalSeconds * nominalFps +
            timecode.frames);

    return true;
}

} // namespace bdfr
