#include "bdfr/speech/TextSpeech.h"

#include "bdfr/core/CurveMixer.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace bdfr::speech {

namespace {

Viseme classifyToken(const std::string& token) {
    if (token == "th") return Viseme::TH;
    if (token == "sh" || token == "ch" || token == "j") return Viseme::CHSH;
    if (token == "m" || token == "b" || token == "p") return Viseme::MBP;
    if (token == "f" || token == "v") return Viseme::FV;
    if (token == "l") return Viseme::L;
    if (token == "r") return Viseme::R;
    if (token == "w" || token == "q" || token == "u") return Viseme::WQ;
    if (token == "e" || token == "i" || token == "y") return Viseme::EE;
    if (token == "a") return Viseme::AA;
    if (token == "o") return Viseme::OH;
    if (token == "k" || token == "g") return Viseme::KNG;
    if (token == "s" || token == "z" || token == "t" || token == "d" || token == "n")
        return Viseme::SZTDN;
    return Viseme::Rest;
}

bool sentencePunctuation(char c) {
    return c == '.' || c == '!' || c == '?';
}

std::string lowerToken(const std::string& token) {
    std::string out = token;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

} // namespace

std::vector<SpeechEvent> TextSpeechPlanner::plan(const std::string& text,
                                                 const TextSpeechOptions& options) {
    std::vector<SpeechEvent> events;
    const float wpm = std::clamp(options.wordsPerMinute, 40.0F, 400.0F);
    const double averageWordSeconds = 60.0 / static_cast<double>(wpm);
    const double tokenSeconds = std::max(0.035, averageWordSeconds / 5.0);

    double cursor = 0.0;
    std::size_t i = 0;

    while (i < text.size()) {
        const unsigned char raw = static_cast<unsigned char>(text[i]);
        const char c = static_cast<char>(raw);

        if (std::isspace(raw)) {
            cursor += tokenSeconds * 0.25;
            ++i;
            continue;
        }
        if (c == ',') {
            cursor += std::max(0.0, options.commaPauseSeconds);
            ++i;
            continue;
        }
        if (sentencePunctuation(c)) {
            cursor += std::max(0.0, options.sentencePauseSeconds);
            ++i;
            continue;
        }
        if (!std::isalpha(raw)) {
            ++i;
            continue;
        }

        std::string token(1, static_cast<char>(std::tolower(raw)));
        if (i + 1 < text.size()) {
            std::string pair;
            pair.push_back(static_cast<char>(std::tolower(raw)));
            pair.push_back(static_cast<char>(std::tolower(
                static_cast<unsigned char>(text[i + 1]))));
            if (pair == "th" || pair == "sh" || pair == "ch") {
                token = pair;
                ++i;
            }
        }

        SpeechEvent event;
        event.token = lowerToken(token);
        event.viseme = classifyToken(event.token);
        event.startSeconds = cursor;
        event.durationSeconds = tokenSeconds;
        events.push_back(event);

        cursor += tokenSeconds;
        ++i;
    }

    return events;
}

CurveMap VisemeSynthesizer::pose(Viseme viseme, float intensity) {
    const float w = clamp01(intensity);
    CurveMap pose;

    auto set = [&](const std::string& id, float value) {
        pose[id] = clamp01(value * w);
    };

    switch (viseme) {
        case Viseme::Rest:
            break;
        case Viseme::MBP:
            set("mouthClose", 1.0F);
            set("mouthPressLeft", 0.55F);
            set("mouthPressRight", 0.55F);
            break;
        case Viseme::FV:
            set("mouthPressLeft", 0.35F);
            set("mouthPressRight", 0.35F);
            set("jawOpen", 0.12F);
            break;
        case Viseme::TH:
            set("jawOpen", 0.28F);
            set("mouthStretchLeft", 0.18F);
            set("mouthStretchRight", 0.18F);
            break;
        case Viseme::L:
            set("jawOpen", 0.32F);
            set("mouthStretchLeft", 0.15F);
            set("mouthStretchRight", 0.15F);
            break;
        case Viseme::R:
            set("jawOpen", 0.25F);
            set("mouthPucker", 0.22F);
            break;
        case Viseme::WQ:
            set("jawOpen", 0.18F);
            set("mouthPucker", 0.72F);
            set("mouthFunnel", 0.48F);
            break;
        case Viseme::EE:
            set("jawOpen", 0.24F);
            set("mouthStretchLeft", 0.62F);
            set("mouthStretchRight", 0.62F);
            break;
        case Viseme::AA:
            set("jawOpen", 0.78F);
            set("mouthFunnel", 0.08F);
            break;
        case Viseme::OH:
            set("jawOpen", 0.52F);
            set("mouthFunnel", 0.68F);
            set("mouthPucker", 0.35F);
            break;
        case Viseme::CHSH:
            set("jawOpen", 0.32F);
            set("mouthFunnel", 0.42F);
            set("mouthPucker", 0.22F);
            break;
        case Viseme::KNG:
            set("jawOpen", 0.35F);
            set("mouthStretchLeft", 0.08F);
            set("mouthStretchRight", 0.08F);
            break;
        case Viseme::SZTDN:
            set("jawOpen", 0.20F);
            set("mouthStretchLeft", 0.28F);
            set("mouthStretchRight", 0.28F);
            break;
    }
    return pose;
}

CurveMap VisemeSynthesizer::sample(const std::vector<SpeechEvent>& events, double timeSeconds) {
    if (!std::isfinite(timeSeconds) || timeSeconds < 0.0) return {};

    for (std::size_t i = 0; i < events.size(); ++i) {
        const auto& event = events[i];
        if (timeSeconds < event.startSeconds ||
            timeSeconds >= event.startSeconds + event.durationSeconds) continue;

        const double local = event.durationSeconds <= 0.0 ? 0.0 :
            (timeSeconds - event.startSeconds) / event.durationSeconds;
        const float centerWeight = static_cast<float>(
            1.0 - std::min(1.0, std::fabs(local - 0.5) * 1.4));

        CurveLayer current;
        current.curves = pose(event.viseme, std::max(0.35F, centerWeight));
        current.weight = 1.0F;

        std::vector<CurveLayer> layers{current};

        if (i > 0 && local < 0.25) {
            CurveLayer previous;
            previous.curves = pose(events[i - 1].viseme, 1.0F);
            previous.weight = static_cast<float>((0.25 - local) / 0.25) * 0.35F;
            layers.insert(layers.begin(), previous);
        }

        if (i + 1 < events.size() && local > 0.75) {
            CurveLayer next;
            next.curves = pose(events[i + 1].viseme, 1.0F);
            next.weight = static_cast<float>((local - 0.75) / 0.25) * 0.35F;
            layers.push_back(next);
        }

        return CurveMixer::mix({}, std::move(layers));
    }
    return {};
}

} // namespace bdfr::speech
