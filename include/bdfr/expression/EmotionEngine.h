#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <vector>

namespace bdfr::expression {

enum class Emotion {
    Neutral,
    Happiness,
    Sadness,
    Anger,
    Fear,
    Surprise,
    Disgust,
    Contempt
};

class EmotionEngine {
public:
    static CurveMap pose(Emotion emotion, float intensity = 1.0F);
    static CurveMap blend(Emotion a, float weightA,
                          Emotion b, float weightB);
};

enum class InstantEventType {
    Blink,
    DoubleBlink,
    BrowFlick,
    LipTwitchLeft,
    LipTwitchRight,
    NoseFlare,
    JawClench
};

struct InstantEvent {
    InstantEventType type = InstantEventType::Blink;
    double startSeconds = 0.0;
    double durationSeconds = 0.12;
    float intensity = 1.0F;
};

class InstantEventGenerator {
public:
    static CurveMap sample(const InstantEvent& event, double timeSeconds);
};

} // namespace bdfr::expression
