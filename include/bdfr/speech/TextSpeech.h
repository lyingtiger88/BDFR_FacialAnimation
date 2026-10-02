#pragma once

#include "bdfr/core/FacialTypes.h"

#include <string>
#include <vector>

namespace bdfr::speech {

enum class Viseme {
    Rest,
    MBP,
    FV,
    TH,
    L,
    R,
    WQ,
    EE,
    AA,
    OH,
    CHSH,
    KNG,
    SZTDN
};

struct SpeechEvent {
    Viseme viseme = Viseme::Rest;
    std::string token;
    double startSeconds = 0.0;
    double durationSeconds = 0.0;
    float emphasis = 0.0F;
};

struct TextSpeechOptions {
    float wordsPerMinute = 150.0F;
    double commaPauseSeconds = 0.15;
    double sentencePauseSeconds = 0.30;
};

class TextSpeechPlanner {
public:
    static std::vector<SpeechEvent> plan(const std::string& text,
                                         const TextSpeechOptions& options = {});
};

class VisemeSynthesizer {
public:
    static CurveMap pose(Viseme viseme, float intensity = 1.0F);
    static CurveMap sample(const std::vector<SpeechEvent>& events, double timeSeconds);
};

} // namespace bdfr::speech
