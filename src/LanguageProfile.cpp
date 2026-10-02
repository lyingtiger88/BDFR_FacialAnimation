#include "bdfr/speech/LanguageProfile.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace bdfr::speech {

namespace {

std::string normalize(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    return value;
}

} // namespace

bool LanguageProfile::addRule(
    LanguageVisemeRule rule) {

    rule.phoneme = normalize(rule.phoneme);

    if (rule.phoneme.empty() ||
        rule.weight < 0.0F ||
        rule.weight > 1.0F) {
        return false;
    }

    return rules_.emplace(
        rule.phoneme,
        std::move(rule)).second;
}

const LanguageVisemeRule*
LanguageProfile::findRule(
    const std::string& phoneme) const {

    const auto it =
        rules_.find(normalize(phoneme));

    return it == rules_.end()
        ? nullptr
        : &it->second;
}

Viseme LanguageProfile::mapPhoneme(
    const std::string& phoneme,
    Viseme fallback) const {

    const auto* rule = findRule(phoneme);

    return rule
        ? rule->viseme
        : fallback;
}

LanguageProfile
LanguageProfile::englishBootstrap() {

    LanguageProfile profile;
    profile.languageCode = "en";
    profile.displayName = "English";

    const std::vector<LanguageVisemeRule> rules = {
        {"m", Viseme::MBP},
        {"b", Viseme::MBP},
        {"p", Viseme::MBP},

        {"f", Viseme::FV},
        {"v", Viseme::FV},

        {"th", Viseme::TH},
        {"dh", Viseme::TH},

        {"l", Viseme::L},
        {"r", Viseme::R},

        {"w", Viseme::WQ},

        {"iy", Viseme::EE},
        {"ih", Viseme::EE},
        {"eh", Viseme::EE},

        {"aa", Viseme::AA},
        {"ae", Viseme::AA},
        {"ah", Viseme::AA},

        {"ao", Viseme::OH},
        {"ow", Viseme::OH},

        {"ch", Viseme::CHSH},
        {"sh", Viseme::CHSH},
        {"jh", Viseme::CHSH},

        {"k", Viseme::KNG},
        {"g", Viseme::KNG},
        {"ng", Viseme::KNG},

        {"s", Viseme::SZTDN},
        {"z", Viseme::SZTDN},
        {"t", Viseme::SZTDN},
        {"d", Viseme::SZTDN},
        {"n", Viseme::SZTDN}
    };

    for (const auto& rule : rules) {
        profile.addRule(rule);
    }

    return profile;
}


LanguageProfile
LanguageProfile::persianBootstrap() {

    LanguageProfile profile;
    profile.languageCode = "fa";
    profile.displayName = "Persian";

    // Provider-neutral phoneme token bootstrap.
    // Exact tokens may be adapted by concrete G2P/alignment providers.
    const std::vector<LanguageVisemeRule> rules = {
        {"m", Viseme::MBP},
        {"b", Viseme::MBP},
        {"p", Viseme::MBP},

        {"f", Viseme::FV},
        {"v", Viseme::FV},

        {"l", Viseme::L},
        {"r", Viseme::R},

        {"u", Viseme::WQ},
        {"ow", Viseme::WQ},

        {"i", Viseme::EE},
        {"e", Viseme::EE},

        {"a", Viseme::AA},
        {"aa", Viseme::AA},

        {"o", Viseme::OH},

        {"sh", Viseme::CHSH},
        {"ch", Viseme::CHSH},
        {"zh", Viseme::CHSH},
        {"j", Viseme::CHSH},

        {"k", Viseme::KNG},
        {"g", Viseme::KNG},
        {"gh", Viseme::KNG},
        {"kh", Viseme::KNG},

        {"s", Viseme::SZTDN},
        {"z", Viseme::SZTDN},
        {"t", Viseme::SZTDN},
        {"d", Viseme::SZTDN},
        {"n", Viseme::SZTDN},
        {"q", Viseme::KNG}
    };

    for (const auto& rule : rules) {
        profile.addRule(rule);
    }

    return profile;
}

} // namespace bdfr::speech
