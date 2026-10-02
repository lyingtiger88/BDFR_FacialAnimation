#include "bdfr/audio/Prosody.h"

#include <algorithm>
#include <cmath>

namespace bdfr::audio {

std::vector<ProsodyFrame> ProsodyAnalyzer::analyze(
    const std::vector<AudioFeatureFrame>& features,
    float activityThreshold,
    float emphasisMultiplier) {

    std::vector<ProsodyFrame> out;
    if (features.empty()) return out;

    float peak = 0.0F;
    double sum = 0.0;
    for (const auto& feature : features) {
        peak = std::max(peak, feature.rms);
        sum += feature.rms;
    }
    const float mean = static_cast<float>(sum / static_cast<double>(features.size()));
    const float threshold = std::clamp(activityThreshold, 0.0F, 1.0F);
    const float emphasisThreshold =
        std::max(threshold, mean * std::max(1.0F, emphasisMultiplier));

    out.reserve(features.size());
    for (const auto& feature : features) {
        ProsodyFrame frame;
        frame.startSeconds = feature.startSeconds;
        frame.durationSeconds = feature.durationSeconds;
        frame.energy = feature.rms;
        frame.normalizedEnergy = peak > 0.000001F
            ? std::clamp(feature.rms / peak, 0.0F, 1.0F)
            : 0.0F;
        frame.speechActive = frame.normalizedEnergy >= threshold;
        frame.emphasisCandidate =
            frame.speechActive && feature.rms >= emphasisThreshold;
        out.push_back(frame);
    }
    return out;
}

ProsodySummary ProsodyAnalyzer::summarize(
    const std::vector<ProsodyFrame>& frames) {

    ProsodySummary summary;
    if (frames.empty()) return summary;

    double sum = 0.0;
    for (const auto& frame : frames) {
        summary.peakEnergy = std::max(summary.peakEnergy, frame.energy);
        sum += frame.energy;
        if (frame.speechActive)
            summary.activeSpeechSeconds += frame.durationSeconds;
        else
            summary.silenceSeconds += frame.durationSeconds;
        if (frame.emphasisCandidate)
            ++summary.emphasisEvents;
    }
    summary.meanEnergy =
        static_cast<float>(sum / static_cast<double>(frames.size()));
    return summary;
}

} // namespace bdfr::audio
