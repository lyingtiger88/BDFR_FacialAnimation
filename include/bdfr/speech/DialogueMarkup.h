#pragma once

#include <string>
#include <vector>

namespace bdfr::speech {

enum class DialogueItemType {
    Text,
    Emotion,
    Pause,
    Blink,
    Gaze,
    Style
};

struct DialogueItem {
    DialogueItemType type = DialogueItemType::Text;
    std::string text;
    std::string argument;
    float value = 0.0F;
};

struct DialogueScript {
    std::vector<DialogueItem> items;
};

class DialogueMarkup {
public:
    static bool parse(const std::string& source,
                      DialogueScript& script,
                      std::string* error = nullptr);
};

} // namespace bdfr::speech
