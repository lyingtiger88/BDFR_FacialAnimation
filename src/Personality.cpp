#include "bdfr/expression/Personality.h"

namespace bdfr::expression {

PersonalityProfile PersonalityLibrary::preset(PersonalityPreset presetValue) {
    PersonalityProfile profile;

    switch (presetValue) {
        case PersonalityPreset::Neutral:
            profile.name = "Neutral";
            profile.seed = 100;
            break;

        case PersonalityPreset::Reserved:
            profile.name = "Reserved";
            profile.seed = 211;
            profile.articulationScale = 0.85F;
            profile.emotionScale = 0.75F;
            profile.behavior.blinkIntervalSeconds = 4.8;
            profile.behavior.eyeDartStrength = 0.04F;
            profile.behavior.headYawDegrees = 0.9F;
            profile.behavior.headPitchDegrees = 0.5F;
            break;

        case PersonalityPreset::Nervous:
            profile.name = "Nervous";
            profile.seed = 377;
            profile.articulationScale = 0.95F;
            profile.emotionScale = 1.1F;
            profile.behavior.blinkIntervalSeconds = 2.2;
            profile.behavior.blinkDurationSeconds = 0.10;
            profile.behavior.eyeDartPeriodSeconds = 1.1;
            profile.behavior.eyeDartStrength = 0.16F;
            profile.behavior.headMotionPeriodSeconds = 3.2;
            profile.behavior.headYawDegrees = 2.8F;
            profile.behavior.headPitchDegrees = 1.5F;
            break;

        case PersonalityPreset::Confident:
            profile.name = "Confident";
            profile.seed = 503;
            profile.articulationScale = 1.05F;
            profile.emotionScale = 0.95F;
            profile.behavior.blinkIntervalSeconds = 5.2;
            profile.behavior.eyeDartPeriodSeconds = 4.0;
            profile.behavior.eyeDartStrength = 0.025F;
            profile.behavior.headMotionPeriodSeconds = 7.0;
            profile.behavior.headYawDegrees = 1.4F;
            profile.behavior.headPitchDegrees = 0.7F;
            break;

        case PersonalityPreset::Expressive:
            profile.name = "Expressive";
            profile.seed = 701;
            profile.articulationScale = 1.2F;
            profile.emotionScale = 1.25F;
            profile.behavior.blinkIntervalSeconds = 3.6;
            profile.behavior.eyeDartStrength = 0.10F;
            profile.behavior.headMotionPeriodSeconds = 4.5;
            profile.behavior.headYawDegrees = 3.3F;
            profile.behavior.headPitchDegrees = 1.9F;
            break;
    }

    return profile;
}

} // namespace bdfr::expression
