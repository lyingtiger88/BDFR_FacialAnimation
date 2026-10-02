#pragma once

#include "bdfr/audio/AudioFeatures.h"

#include <vector>

namespace bdfr::audio {

struct ProsodyFrame {
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    float energy = 0.0F;
    float normalizedEnergy = 0.0F;
    bool speechActive = false;
    bool emphasisCandidate = false;
};

struct ProsodySummary {
    float peakEnergy = 0.0F;
    float meanEnergy = 0.0F;
    double activeSpeechSeconds = 0.0;
    double silenceSeconds = 0.0;
    std::size_t emphasisEvents = 0;
};

class ProsodyAnalyzer {
public:
    static std::vector<ProsodyFrame> analyze(
        const std::vector<AudioFeatureFrame>& features,
        float activityThreshold = 0.08F,
        float emphasisMultiplier = 1.5F);

    static ProsodySummary summarize(
        const std::vector<ProsodyFrame>& frames);
};

} // namespace bdfr::audio
