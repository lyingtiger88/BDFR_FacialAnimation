#include "bdfr/speech/TranscriptTiming.h"

#include <algorithm>
#include <cmath>

namespace bdfr::speech {

namespace {

double eventTimelineDuration(
    const std::vector<SpeechEvent>& events) {

    double duration = 0.0;
    for (const auto& event : events) {
        duration = std::max(
            duration,
            event.startSeconds +
            std::max(0.0, event.durationSeconds));
    }
    return duration;
}

} // namespace

TimedTranscriptResult TranscriptTiming::fitToDuration(
    const std::vector<SpeechEvent>& events,
    double targetDurationSeconds) {

    TimedTranscriptResult result;
    result.sourceDurationSeconds =
        eventTimelineDuration(events);

    result.targetDurationSeconds =
        std::isfinite(targetDurationSeconds)
            ? std::max(0.0, targetDurationSeconds)
            : 0.0;

    result.events = events;

    if (result.events.empty() ||
        result.sourceDurationSeconds <= 0.0 ||
        result.targetDurationSeconds <= 0.0) {
        return result;
    }

    result.timeScale =
        result.targetDurationSeconds /
        result.sourceDurationSeconds;

    for (auto& event : result.events) {
        event.startSeconds *= result.timeScale;
        event.durationSeconds *= result.timeScale;
    }

    return result;
}

TimedTranscriptResult TranscriptTiming::fitTextToDuration(
    const std::string& text,
    double targetDurationSeconds,
    const TextSpeechOptions& options) {

    return fitToDuration(
        TextSpeechPlanner::plan(text, options),
        targetDurationSeconds);
}

} // namespace bdfr::speech
