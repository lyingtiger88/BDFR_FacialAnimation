#include "bdfr/audio/AudioBehavior.h"

#include <algorithm>

namespace bdfr::audio {

std::vector<AudioBehaviorCue> AudioBehaviorPlanner::plan(
    const std::vector<ProsodyFrame>& prosody,
    double minimumPauseForBlinkSeconds,
    double minimumPauseForBreathSeconds) {

    std::vector<AudioBehaviorCue> cues;
    if (prosody.empty()) return cues;

    bool inSilence = false;
    double silenceStart = 0.0;
    double silenceEnd = 0.0;

    auto flushSilence = [&]() {
        if (!inSilence) return;
        const double duration = std::max(0.0, silenceEnd - silenceStart);
        if (duration >= minimumPauseForBlinkSeconds) {
            cues.push_back({
                AudioBehaviorCueType::Blink,
                silenceStart + duration * 0.35,
                0.12,
                1.0F
            });
        }
        if (duration >= minimumPauseForBreathSeconds) {
            cues.push_back({
                AudioBehaviorCueType::Breath,
                silenceStart,
                duration,
                static_cast<float>(std::clamp(duration / 0.8, 0.25, 1.0))
            });
        }
        inSilence = false;
    };

    bool previousEmphasis = false;
    for (const auto& frame : prosody) {
        const double frameEnd = frame.startSeconds + frame.durationSeconds;

        if (!frame.speechActive) {
            if (!inSilence) {
                inSilence = true;
                silenceStart = frame.startSeconds;
            }
            silenceEnd = frameEnd;
        } else {
            flushSilence();
        }

        if (frame.emphasisCandidate && !previousEmphasis) {
            cues.push_back({
                AudioBehaviorCueType::Emphasis,
                frame.startSeconds,
                frame.durationSeconds,
                frame.normalizedEnergy
            });
            cues.push_back({
                AudioBehaviorCueType::HeadNod,
                frame.startSeconds,
                std::max(0.10, frame.durationSeconds * 2.0),
                std::clamp(frame.normalizedEnergy, 0.2F, 1.0F)
            });
        }

        previousEmphasis = frame.emphasisCandidate;
    }

    flushSilence();

    std::stable_sort(cues.begin(), cues.end(),
        [](const AudioBehaviorCue& a, const AudioBehaviorCue& b) {
            return a.startSeconds < b.startSeconds;
        });

    return cues;
}

} // namespace bdfr::audio
