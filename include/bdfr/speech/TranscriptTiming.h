#pragma once

#include "bdfr/speech/TextSpeech.h"

#include <vector>

namespace bdfr::speech {

struct TimedTranscriptResult {
    std::vector<SpeechEvent> events;
    double sourceDurationSeconds = 0.0;
    double targetDurationSeconds = 0.0;
    double timeScale = 1.0;
};

class TranscriptTiming {
public:
    static TimedTranscriptResult fitToDuration(
        const std::vector<SpeechEvent>& events,
        double targetDurationSeconds);

    static TimedTranscriptResult fitTextToDuration(
        const std::string& text,
        double targetDurationSeconds,
        const TextSpeechOptions& options = {});
};

} // namespace bdfr::speech
