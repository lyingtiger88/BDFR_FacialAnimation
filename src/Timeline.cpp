#include "bdfr/core/Timeline.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <utility>

namespace bdfr {

bool Timeline::addTrack(TimelineTrack track) {
    if (track.name.empty()) return false;
    tracks_.push_back(std::move(track));
    return true;
}

bool Timeline::addClip(std::size_t trackIndex, TimelineClip clip) {
    if (trackIndex >= tracks_.size() || tracks_[trackIndex].locked) return false;
    if (clip.id.empty() || !std::isfinite(clip.startSeconds) || !std::isfinite(clip.durationSeconds) ||
        clip.startSeconds < 0.0 || clip.durationSeconds < 0.0 || !std::isfinite(clip.weight)) return false;
    clip.weight = std::clamp(clip.weight, 0.0F, 1.0F);
    tracks_[trackIndex].clips.push_back(std::move(clip));
    return true;
}

const std::vector<TimelineTrack>& Timeline::tracks() const noexcept {
    return tracks_;
}

double Timeline::durationSeconds() const noexcept {
    double result = 0.0;
    for (const auto& track : tracks_)
        for (const auto& clip : track.clips)
            result = std::max(result, clip.startSeconds + clip.durationSeconds);
    return result;
}

bool Timeline::validate(std::string* error) const {
    std::unordered_set<std::string> ids;
    for (const auto& track : tracks_) {
        if (track.name.empty()) {
            if (error) *error = "timeline contains unnamed track";
            return false;
        }
        for (const auto& clip : track.clips) {
            if (clip.id.empty()) {
                if (error) *error = "timeline contains clip with empty id";
                return false;
            }
            if (!ids.insert(clip.id).second) {
                if (error) *error = "duplicate clip id: " + clip.id;
                return false;
            }
            if (clip.startSeconds < 0.0 || clip.durationSeconds < 0.0 ||
                clip.weight < 0.0F || clip.weight > 1.0F) {
                if (error) *error = "invalid clip values: " + clip.id;
                return false;
            }
        }
    }
    return true;
}

} // namespace bdfr
