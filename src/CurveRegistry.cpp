#include "bdfr/core/CurveRegistry.h"

namespace bdfr {

namespace {
std::vector<std::string> makeCanonicalCurves() {
    return {
        "AU01", "AU02", "AU04", "AU05", "AU06", "AU07", "AU09", "AU10",
        "AU12", "AU14", "AU15", "AU17", "AU20", "AU23", "AU25", "AU26",
        "AU28", "AU45",
        "jawOpen", "jawLeft", "jawRight", "jawForward",
        "mouthClose", "mouthFunnel", "mouthPucker", "mouthSmileLeft",
        "mouthSmileRight", "mouthFrownLeft", "mouthFrownRight", "mouthPressLeft",
        "mouthPressRight", "mouthStretchLeft", "mouthStretchRight",
        "cheekRaiseLeft", "cheekRaiseRight", "noseSneerLeft", "noseSneerRight",
        "eyeBlinkLeft", "eyeBlinkRight", "eyeSquintLeft", "eyeSquintRight",
        "eyeWideLeft", "eyeWideRight", "browInnerUp", "browOuterUpLeft",
        "browOuterUpRight", "browDownLeft", "browDownRight"
    };
}
} // namespace

CurveRegistry::CurveRegistry() : canonical_(makeCanonicalCurves()) {
    known_.insert(canonical_.begin(), canonical_.end());
}

bool CurveRegistry::isRegistered(const std::string& id) const {
    return known_.find(id) != known_.end();
}

bool CurveRegistry::registerCustom(const std::string& id) {
    if (id.empty()) return false;
    return known_.insert(id).second;
}

const std::vector<std::string>& CurveRegistry::canonicalCurves() const noexcept {
    return canonical_;
}

bool CurveRegistry::validateFrame(const FacialFrame& frame, std::string* error) const {
    auto fail = [&](const std::string& message) {
        if (error) *error = message;
        return false;
    };

    if (frame.schemaVersion == 0) return fail("schemaVersion must be greater than zero");
    if (!isFinite(frame.timestampSeconds) || frame.timestampSeconds < 0.0)
        return fail("timestampSeconds must be finite and non-negative");
    if (!isFinite(frame.confidence) || frame.confidence < 0.0F || frame.confidence > 1.0F)
        return fail("frame confidence must be within [0,1]");

    for (const auto& [id, value] : frame.curves) {
        if (!isRegistered(id)) return fail("unknown curve: " + id);
        if (!isFinite(value) || value < 0.0F || value > 1.0F)
            return fail("curve out of range: " + id);
    }
    return true;
}

FacialFrame CurveRegistry::sanitized(const FacialFrame& frame) const {
    FacialFrame out = frame;
    if (!isFinite(out.timestampSeconds) || out.timestampSeconds < 0.0) out.timestampSeconds = 0.0;
    out.confidence = isFinite(out.confidence) ? clamp01(out.confidence) : 0.0F;

    for (auto it = out.curves.begin(); it != out.curves.end();) {
        if (!isRegistered(it->first)) {
            it = out.curves.erase(it);
            continue;
        }
        it->second = isFinite(it->second) ? clamp01(it->second) : 0.0F;
        ++it;
    }
    return out;
}

} // namespace bdfr
