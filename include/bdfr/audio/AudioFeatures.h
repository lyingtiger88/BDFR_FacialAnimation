#pragma once

#include "bdfr/audio/WavAudio.h"

#include <vector>

namespace bdfr::audio {

struct AudioFeatureFrame {
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    float rms = 0.0F;
    float peak = 0.0F;
    float zeroCrossingRate = 0.0F;
};

class AudioFeatures {
public:
    static std::vector<AudioFeatureFrame> analyze(
        const AudioBuffer& audio,
        double windowSeconds = 0.020,
        double hopSeconds = 0.010);
};

} // namespace bdfr::audio
