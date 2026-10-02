#include "bdfr/core/Arkit52.h"

namespace bdfr {

const std::vector<std::string>& Arkit52::curveNames() {
    static const std::vector<std::string> names = {
        "browDownLeft", "browDownRight", "browInnerUp",
        "browOuterUpLeft", "browOuterUpRight",
        "cheekPuff", "cheekSquintLeft", "cheekSquintRight",
        "eyeBlinkLeft", "eyeBlinkRight",
        "eyeLookDownLeft", "eyeLookDownRight",
        "eyeLookInLeft", "eyeLookInRight",
        "eyeLookOutLeft", "eyeLookOutRight",
        "eyeLookUpLeft", "eyeLookUpRight",
        "eyeSquintLeft", "eyeSquintRight",
        "eyeWideLeft", "eyeWideRight",
        "jawForward", "jawLeft", "jawOpen", "jawRight",
        "mouthClose", "mouthDimpleLeft", "mouthDimpleRight",
        "mouthFrownLeft", "mouthFrownRight",
        "mouthFunnel", "mouthLeft",
        "mouthLowerDownLeft", "mouthLowerDownRight",
        "mouthPressLeft", "mouthPressRight",
        "mouthPucker", "mouthRight",
        "mouthRollLower", "mouthRollUpper",
        "mouthShrugLower", "mouthShrugUpper",
        "mouthSmileLeft", "mouthSmileRight",
        "mouthStretchLeft", "mouthStretchRight",
        "mouthUpperUpLeft", "mouthUpperUpRight",
        "noseSneerLeft", "noseSneerRight",
        "tongueOut"
    };
    return names;
}

RetargetProfile Arkit52::identityProfile() {
    RetargetProfile profile;
    profile.name = "ARKit 52 Identity";
    for (const auto& curve : curveNames()) {
        profile.addMapping({curve, curve, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, false});
    }
    return profile;
}

} // namespace bdfr
