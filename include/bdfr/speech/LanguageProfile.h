#pragma once

#include "bdfr/speech/TextSpeech.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace bdfr::speech {

struct LanguageVisemeRule {
    std::string phoneme;
    Viseme viseme = Viseme::Rest;
    float weight = 1.0F;
};

class LanguageProfile {
public:
    std::string languageCode = "en";
    std::string displayName = "English";

    bool addRule(LanguageVisemeRule rule);
    const LanguageVisemeRule* findRule(
        const std::string& phoneme) const;

    Viseme mapPhoneme(
        const std::string& phoneme,
        Viseme fallback = Viseme::Rest) const;

    static LanguageProfile englishBootstrap();
    static LanguageProfile persianBootstrap();

private:
    std::unordered_map<std::string, LanguageVisemeRule> rules_;
};

} // namespace bdfr::speech
