#pragma once

#include "bdfr/audio/WavAudio.h"

#include <vector>

namespace bdfr::audio {

struct PitchFrame {
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    float frequencyHz = 0.0F;
    float confidence = 0.0F;
    bool voiced = false;
};

class PitchTracker {
public:
    static std::vector<PitchFrame> analyze(
        const AudioBuffer& audio,
        double windowSeconds = 0.040,
        double hopSeconds = 0.010,
        float minimumHz = 70.0F,
        float maximumHz = 400.0F,
        float voicedThreshold = 0.35F);
};

} // namespace bdfr::audio
