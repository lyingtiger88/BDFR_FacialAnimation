#pragma once

#include "bdfr/core/Timeline.h"
#include "bdfr/speech/DialogueMarkup.h"
#include "bdfr/speech/TextSpeech.h"

#include <string>

namespace bdfr::speech {

struct DialogueCompileOptions {
    TextSpeechOptions speech;
    double defaultEmotionDurationSeconds = 1.0;
    double blinkDurationSeconds = 0.12;
};

struct DialogueCompileResult {
    Timeline timeline;
    double durationSeconds = 0.0;
};

class DialogueCompiler {
public:
    static bool compile(const DialogueScript& script,
                        DialogueCompileResult& result,
                        const DialogueCompileOptions& options = {},
                        std::string* error = nullptr);
};

} // namespace bdfr::speech
