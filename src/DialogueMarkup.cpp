#include "bdfr/speech/DialogueMarkup.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace bdfr::speech {

namespace {

std::string trim(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() &&
           std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;

    std::size_t end = value.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;

    return value.substr(begin, end - begin);
}

bool parseFloat(const std::string& text, float& value) {
    try {
        std::size_t consumed = 0;
        value = std::stof(text, &consumed);
        return consumed == text.size();
    } catch (...) {
        return false;
    }
}

void pushText(DialogueScript& script, const std::string& text) {
    const std::string cleaned = trim(text);
    if (!cleaned.empty()) {
        script.items.push_back({DialogueItemType::Text, cleaned, {}, 0.0F});
    }
}

} // namespace

bool DialogueMarkup::parse(const std::string& source,
                           DialogueScript& script,
                           std::string* error) {
    DialogueScript out;
    std::size_t cursor = 0;

    while (cursor < source.size()) {
        const std::size_t open = source.find('[', cursor);
        if (open == std::string::npos) {
            pushText(out, source.substr(cursor));
            break;
        }

        pushText(out, source.substr(cursor, open - cursor));

        const std::size_t close = source.find(']', open + 1);
        if (close == std::string::npos) {
            if (error) *error = "unterminated dialogue markup";
            return false;
        }

        const std::string command = trim(source.substr(open + 1, close - open - 1));
        std::istringstream stream(command);
        std::string first;
        stream >> first;

        DialogueItem item;

        if (first == "blink") {
            item.type = DialogueItemType::Blink;
        } else if (first.rfind("pause=", 0) == 0) {
            item.type = DialogueItemType::Pause;
            if (!parseFloat(first.substr(6), item.value) || item.value < 0.0F) {
                if (error) *error = "invalid pause duration";
                return false;
            }
        } else if (first.rfind("gaze=", 0) == 0) {
            item.type = DialogueItemType::Gaze;
            item.argument = first.substr(5);
            if (item.argument.empty()) {
                if (error) *error = "empty gaze target";
                return false;
            }
        } else if (first.rfind("style=", 0) == 0) {
            item.type = DialogueItemType::Style;
            item.argument = first.substr(6);
            if (item.argument.empty()) {
                if (error) *error = "empty style";
                return false;
            }
        } else if (first.rfind("emotion=", 0) == 0) {
            item.type = DialogueItemType::Emotion;
            item.argument = first.substr(8);
            item.value = 1.0F;

            std::string extra;
            while (stream >> extra) {
                if (extra.rfind("intensity=", 0) == 0) {
                    if (!parseFloat(extra.substr(10), item.value) ||
                        item.value < 0.0F || item.value > 1.0F) {
                        if (error) *error = "invalid emotion intensity";
                        return false;
                    }
                }
            }

            if (item.argument.empty()) {
                if (error) *error = "empty emotion";
                return false;
            }
        } else {
            if (error) *error = "unknown dialogue markup: " + command;
            return false;
        }

        out.items.push_back(std::move(item));
        cursor = close + 1;
    }

    script = std::move(out);
    return true;
}

} // namespace bdfr::speech
