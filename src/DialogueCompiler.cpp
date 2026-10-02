#include "bdfr/speech/DialogueCompiler.h"

#include "bdfr/expression/EmotionEngine.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace bdfr::speech {

namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool parseEmotion(const std::string& name, expression::Emotion& emotion) {
    const std::string n = lower(name);
    if (n == "neutral") emotion = expression::Emotion::Neutral;
    else if (n == "happy" || n == "happiness") emotion = expression::Emotion::Happiness;
    else if (n == "sad" || n == "sadness") emotion = expression::Emotion::Sadness;
    else if (n == "angry" || n == "anger") emotion = expression::Emotion::Anger;
    else if (n == "fear" || n == "afraid") emotion = expression::Emotion::Fear;
    else if (n == "surprise" || n == "surprised") emotion = expression::Emotion::Surprise;
    else if (n == "disgust" || n == "disgusted") emotion = expression::Emotion::Disgust;
    else if (n == "contempt") emotion = expression::Emotion::Contempt;
    else return false;
    return true;
}

} // namespace

bool DialogueCompiler::compile(const DialogueScript& script,
                               DialogueCompileResult& result,
                               const DialogueCompileOptions& options,
                               std::string* error) {
    DialogueCompileResult out;

    TimelineTrack textTrack{"Dialogue", TrackType::TextDialogue};
    TimelineTrack speechTrack{"Generated Speech", TrackType::GeneratedSpeech};
    speechTrack.regionMask = regionMask(CurveRegion::Mouth) | regionMask(CurveRegion::Jaw);
    speechTrack.priority = 10;

    TimelineTrack emotionTrack{"Emotion", TrackType::Emotion};
    emotionTrack.priority = 5;

    TimelineTrack eventTrack{"Instant Events", TrackType::InstantEvent};
    eventTrack.priority = 20;

    TimelineTrack gazeTrack{"Gaze / Head", TrackType::GazeHead};
    gazeTrack.priority = 15;

    out.timeline.addTrack(textTrack);
    out.timeline.addTrack(speechTrack);
    out.timeline.addTrack(emotionTrack);
    out.timeline.addTrack(eventTrack);
    out.timeline.addTrack(gazeTrack);

    double cursor = 0.0;
    std::size_t idCounter = 0;
    expression::Emotion activeEmotion = expression::Emotion::Neutral;
    float activeEmotionIntensity = 0.0F;

    auto nextId = [&](const std::string& prefix) {
        return prefix + "_" + std::to_string(++idCounter);
    };

    for (const DialogueItem& item : script.items) {
        switch (item.type) {
            case DialogueItemType::Text: {
                const auto events = TextSpeechPlanner::plan(item.text, options.speech);
                double textDuration = 0.0;
                if (!events.empty()) {
                    const auto& last = events.back();
                    textDuration = last.startSeconds + last.durationSeconds;
                }

                TimelineClip textClip;
                textClip.id = nextId("text");
                textClip.startSeconds = cursor;
                textClip.durationSeconds = textDuration;
                textClip.payload = item.text;
                if (!out.timeline.addClip(0, textClip)) {
                    if (error) *error = "failed to add text clip";
                    return false;
                }

                for (const SpeechEvent& event : events) {
                    TimelineClip speechClip;
                    speechClip.id = nextId("viseme");
                    speechClip.startSeconds = cursor + event.startSeconds;
                    speechClip.durationSeconds = event.durationSeconds;
                    speechClip.payload = event.token;
                    speechClip.curves = VisemeSynthesizer::pose(event.viseme);
                    speechClip.regionMask =
                        regionMask(CurveRegion::Mouth) | regionMask(CurveRegion::Jaw);
                    if (!out.timeline.addClip(1, std::move(speechClip))) {
                        if (error) *error = "failed to add speech clip";
                        return false;
                    }
                }

                if (activeEmotionIntensity > 0.0F && textDuration > 0.0) {
                    TimelineClip emotionClip;
                    emotionClip.id = nextId("emotion");
                    emotionClip.startSeconds = cursor;
                    emotionClip.durationSeconds = textDuration;
                    emotionClip.weight = activeEmotionIntensity;
                    emotionClip.curves = expression::EmotionEngine::pose(activeEmotion, 1.0F);
                    if (!out.timeline.addClip(2, std::move(emotionClip))) {
                        if (error) *error = "failed to add emotion clip";
                        return false;
                    }
                }

                cursor += textDuration;
                break;
            }

            case DialogueItemType::Emotion: {
                expression::Emotion parsed;
                if (!parseEmotion(item.argument, parsed)) {
                    if (error) *error = "unsupported emotion: " + item.argument;
                    return false;
                }
                activeEmotion = parsed;
                activeEmotionIntensity = clamp01(item.value);
                break;
            }

            case DialogueItemType::Pause:
                cursor += std::max(0.0F, item.value);
                break;

            case DialogueItemType::Blink: {
                TimelineClip clip;
                clip.id = nextId("blink");
                clip.startSeconds = cursor;
                clip.durationSeconds = std::max(0.04, options.blinkDurationSeconds);
                clip.payload = "blink";
                clip.curves = {{"eyeBlinkLeft", 1.0F}, {"eyeBlinkRight", 1.0F}};
                clip.regionMask = regionMask(CurveRegion::Eyes);
                if (!out.timeline.addClip(3, std::move(clip))) {
                    if (error) *error = "failed to add blink";
                    return false;
                }
                break;
            }

            case DialogueItemType::Gaze: {
                TimelineClip clip;
                clip.id = nextId("gaze");
                clip.startSeconds = cursor;
                clip.durationSeconds = 0.0;
                clip.payload = item.argument;
                clip.regionMask = regionMask(CurveRegion::Head);
                if (!out.timeline.addClip(4, std::move(clip))) {
                    if (error) *error = "failed to add gaze event";
                    return false;
                }
                break;
            }

            case DialogueItemType::Style:
                // Stored by higher-level performance/personality orchestration in a later slice.
                break;
        }
    }

    out.durationSeconds = std::max(cursor, out.timeline.durationSeconds());
    if (!out.timeline.validate(error)) return false;
    result = std::move(out);
    return true;
}

} // namespace bdfr::speech
