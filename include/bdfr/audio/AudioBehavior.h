#pragma once

#include "bdfr/audio/Prosody.h"

#include <vector>

namespace bdfr::audio {

enum class AudioBehaviorCueType {
    Blink,
    HeadNod,
    Breath,
    Emphasis
};

struct AudioBehaviorCue {
    AudioBehaviorCueType type = AudioBehaviorCueType::Emphasis;
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    float strength = 0.0F;
};

class AudioBehaviorPlanner {
public:
    static std::vector<AudioBehaviorCue> plan(
        const std::vector<ProsodyFrame>& prosody,
        double minimumPauseForBlinkSeconds = 0.16,
        double minimumPauseForBreathSeconds = 0.30);
};

} // namespace bdfr::audio
