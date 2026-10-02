#pragma once

#include "bdfr/expression/BehaviorEngine.h"

#include <string>

namespace bdfr::expression {

enum class PersonalityPreset {
    Neutral,
    Reserved,
    Nervous,
    Confident,
    Expressive
};

struct PersonalityProfile {
    std::string name;
    std::uint32_t seed = 0;
    float articulationScale = 1.0F;
    float emotionScale = 1.0F;
    BehaviorProfile behavior;
};

class PersonalityLibrary {
public:
    static PersonalityProfile preset(PersonalityPreset preset);
};

} // namespace bdfr::expression
