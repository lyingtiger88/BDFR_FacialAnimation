#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bdfr {

enum class TrackType : std::uint8_t {
    Audio,
    TextDialogue,
    GeneratedSpeech,
    Emotion,
    InstantEvent,
    Mocap,
    GazeHead,
    ManualOverride
};

struct TimelineClip {
    std::string id;
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    std::string payload;
    float weight = 1.0F;
};

struct TimelineTrack {
    std::string name;
    TrackType type = TrackType::ManualOverride;
    bool muted = false;
    bool solo = false;
    bool locked = false;
    std::vector<TimelineClip> clips;
};

class Timeline {
public:
    bool addTrack(TimelineTrack track);
    bool addClip(std::size_t trackIndex, TimelineClip clip);
    const std::vector<TimelineTrack>& tracks() const noexcept;
    double durationSeconds() const noexcept;
    bool validate(std::string* error = nullptr) const;

private:
    std::vector<TimelineTrack> tracks_;
};

} // namespace bdfr
