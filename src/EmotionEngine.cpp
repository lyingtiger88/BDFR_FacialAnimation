#include "bdfr/expression/EmotionEngine.h"

#include "bdfr/core/CurveMixer.h"

#include <algorithm>
#include <cmath>

namespace bdfr::expression {

CurveMap EmotionEngine::pose(Emotion emotion, float intensity) {
    const float w = clamp01(intensity);
    CurveMap out;
    auto set = [&](const std::string& id, float value) {
        out[id] = clamp01(value * w);
    };

    switch (emotion) {
        case Emotion::Neutral:
            break;
        case Emotion::Happiness:
            set("AU06", 0.55F);
            set("AU12", 0.85F);
            set("cheekRaiseLeft", 0.45F);
            set("cheekRaiseRight", 0.45F);
            break;
        case Emotion::Sadness:
            set("AU01", 0.55F);
            set("AU04", 0.35F);
            set("AU15", 0.65F);
            break;
        case Emotion::Anger:
            set("AU04", 0.80F);
            set("AU07", 0.45F);
            set("AU23", 0.60F);
            set("browDownLeft", 0.65F);
            set("browDownRight", 0.65F);
            break;
        case Emotion::Fear:
            set("AU01", 0.45F);
            set("AU02", 0.55F);
            set("AU04", 0.35F);
            set("AU05", 0.60F);
            set("AU20", 0.35F);
            set("AU26", 0.35F);
            break;
        case Emotion::Surprise:
            set("AU01", 0.60F);
            set("AU02", 0.70F);
            set("AU05", 0.65F);
            set("AU26", 0.65F);
            break;
        case Emotion::Disgust:
            set("AU09", 0.75F);
            set("AU10", 0.55F);
            set("AU17", 0.25F);
            break;
        case Emotion::Contempt:
            set("AU14", 0.60F);
            set("mouthSmileLeft", 0.25F);
            break;
    }
    return out;
}

CurveMap EmotionEngine::blend(Emotion a, float weightA,
                              Emotion b, float weightB) {
    CurveLayer layerA;
    layerA.curves = pose(a, 1.0F);
    layerA.weight = clamp01(weightA);

    CurveLayer layerB;
    layerB.curves = pose(b, 1.0F);
    layerB.weight = clamp01(weightB);
    layerB.additive = true;

    return CurveMixer::mix({}, {layerA, layerB});
}

CurveMap InstantEventGenerator::sample(const InstantEvent& event, double timeSeconds) {
    if (!std::isfinite(timeSeconds) || event.durationSeconds <= 0.0 ||
        timeSeconds < event.startSeconds ||
        timeSeconds > event.startSeconds + event.durationSeconds) {
        return {};
    }

    const double normalized =
        (timeSeconds - event.startSeconds) / event.durationSeconds;
    const float envelope = static_cast<float>(
        std::sin(std::clamp(normalized, 0.0, 1.0) * 3.14159265358979323846));
    const float w = clamp01(event.intensity) * envelope;
    CurveMap out;

    auto set = [&](const std::string& id, float value) {
        out[id] = clamp01(value * w);
    };

    switch (event.type) {
        case InstantEventType::Blink:
            set("eyeBlinkLeft", 1.0F);
            set("eyeBlinkRight", 1.0F);
            break;
        case InstantEventType::DoubleBlink: {
            const double phase = std::fmod(normalized * 2.0, 1.0);
            const float doubleEnvelope = static_cast<float>(
                std::sin(std::clamp(phase, 0.0, 1.0) * 3.14159265358979323846));
            out["eyeBlinkLeft"] = clamp01(doubleEnvelope * event.intensity);
            out["eyeBlinkRight"] = clamp01(doubleEnvelope * event.intensity);
            break;
        }
        case InstantEventType::BrowFlick:
            set("browInnerUp", 0.55F);
            set("browOuterUpLeft", 0.45F);
            set("browOuterUpRight", 0.45F);
            break;
        case InstantEventType::LipTwitchLeft:
            set("mouthSmileLeft", 0.35F);
            break;
        case InstantEventType::LipTwitchRight:
            set("mouthSmileRight", 0.35F);
            break;
        case InstantEventType::NoseFlare:
            set("noseSneerLeft", 0.35F);
            set("noseSneerRight", 0.35F);
            break;
        case InstantEventType::JawClench:
            set("mouthPressLeft", 0.55F);
            set("mouthPressRight", 0.55F);
            set("jawOpen", 0.05F);
            break;
    }

    return out;
}

} // namespace bdfr::expression
