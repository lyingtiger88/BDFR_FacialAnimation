#include "bdfr/core/AutoMapper.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

namespace bdfr {

namespace {

std::string canonicalizeSides(std::string value) {
    const std::vector<std::pair<std::string, std::string>> replacements = {
        {"left", "l"},
        {"right", "r"},
        {"_l", "l"},
        {"_r", "r"}
    };
    for (const auto& [from, to] : replacements) {
        std::size_t pos = 0;
        while ((pos = value.find(from, pos)) != std::string::npos) {
            value.replace(pos, from.size(), to);
            pos += to.size();
        }
    }
    return value;
}

float similarity(const std::string& a, const std::string& b) {
    if (a == b) return 1.0F;
    if (a.empty() || b.empty()) return 0.0F;
    if (a.find(b) != std::string::npos || b.find(a) != std::string::npos) return 0.88F;

    std::unordered_set<char> ca(a.begin(), a.end());
    std::unordered_set<char> cb(b.begin(), b.end());
    std::size_t common = 0;
    for (char c : ca) if (cb.find(c) != cb.end()) ++common;
    const std::size_t total = ca.size() + cb.size() - common;
    return total == 0 ? 0.0F : static_cast<float>(common) / static_cast<float>(total);
}

} // namespace

std::string AutoMapper::normalizeName(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)))
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return canonicalizeSides(out);
}

std::vector<MappingSuggestion>
AutoMapper::suggest(const std::vector<std::string>& sourceCurves,
                    const std::vector<std::string>& targetCurves,
                    float minimumConfidence) {
    const float threshold = std::clamp(minimumConfidence, 0.0F, 1.0F);
    std::vector<MappingSuggestion> out;
    std::unordered_set<std::string> usedTargets;

    for (const auto& source : sourceCurves) {
        const std::string normalizedSource = normalizeName(source);
        MappingSuggestion best;
        best.sourceCurve = source;

        for (const auto& target : targetCurves) {
            if (usedTargets.find(target) != usedTargets.end()) continue;
            const float score = similarity(normalizedSource, normalizeName(target));
            if (score > best.confidence) {
                best.targetCurve = target;
                best.confidence = score;
            }
        }

        if (!best.targetCurve.empty() && best.confidence >= threshold) {
            usedTargets.insert(best.targetCurve);
            out.push_back(best);
        }
    }

    return out;
}

} // namespace bdfr
