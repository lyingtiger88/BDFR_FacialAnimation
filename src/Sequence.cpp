#include "bdfr/core/Sequence.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <utility>

namespace bdfr {

namespace {
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}
}

bool FacialSequence::addFrame(FacialFrame frame) {
    if (!isFinite(frame.timestampSeconds) || frame.timestampSeconds < 0.0) return false;
    auto it = std::lower_bound(frames_.begin(), frames_.end(), frame.timestampSeconds,
        [](const FacialFrame& value, double time) { return value.timestampSeconds < time; });

    if (it != frames_.end() && std::fabs(it->timestampSeconds - frame.timestampSeconds) < 0.0000001)
        *it = std::move(frame);
    else
        frames_.insert(it, std::move(frame));
    return true;
}

const std::vector<FacialFrame>& FacialSequence::frames() const noexcept {
    return frames_;
}

double FacialSequence::durationSeconds() const noexcept {
    return frames_.empty() ? 0.0 : frames_.back().timestampSeconds;
}

bool FacialSequence::validate(std::string* error) const {
    double previous = -1.0;
    for (const auto& frame : frames_) {
        if (!isFinite(frame.timestampSeconds) || frame.timestampSeconds < 0.0) {
            if (error) *error = "invalid frame timestamp";
            return false;
        }
        if (frame.timestampSeconds <= previous) {
            if (error) *error = "frame timestamps must be strictly increasing";
            return false;
        }
        previous = frame.timestampSeconds;
    }
    return true;
}

FacialFrame FacialSequence::sample(double timeSeconds) const {
    if (frames_.empty()) return {};

    if (!std::isfinite(timeSeconds) || timeSeconds <= frames_.front().timestampSeconds)
        return frames_.front();
    if (timeSeconds >= frames_.back().timestampSeconds)
        return frames_.back();

    auto upper = std::upper_bound(frames_.begin(), frames_.end(), timeSeconds,
        [](double time, const FacialFrame& frame) { return time < frame.timestampSeconds; });
    const auto lower = upper - 1;

    const double span = upper->timestampSeconds - lower->timestampSeconds;
    const float t = span <= 0.0 ? 0.0F :
        static_cast<float>((timeSeconds - lower->timestampSeconds) / span);

    FacialFrame out;
    out.schemaVersion = std::max(lower->schemaVersion, upper->schemaVersion);
    out.timestampSeconds = timeSeconds;
    out.confidence = lerp(lower->confidence, upper->confidence, t);
    out.head.pitch = lerp(lower->head.pitch, upper->head.pitch, t);
    out.head.yaw = lerp(lower->head.yaw, upper->head.yaw, t);
    out.head.roll = lerp(lower->head.roll, upper->head.roll, t);
    out.gaze.x = lerp(lower->gaze.x, upper->gaze.x, t);
    out.gaze.y = lerp(lower->gaze.y, upper->gaze.y, t);
    out.gaze.confidence = lerp(lower->gaze.confidence, upper->gaze.confidence, t);

    std::unordered_set<std::string> ids;
    for (const auto& [id, _] : lower->curves) ids.insert(id);
    for (const auto& [id, _] : upper->curves) ids.insert(id);

    for (const auto& id : ids) {
        const auto a = lower->curves.find(id);
        const auto b = upper->curves.find(id);
        const float av = a == lower->curves.end() ? 0.0F : a->second;
        const float bv = b == upper->curves.end() ? 0.0F : b->second;
        out.curves[id] = clamp01(lerp(av, bv, t));
    }
    return out;
}

} // namespace bdfr
