#pragma once

#include "bdfr/core/FacialTypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace bdfr::mocap {

struct ExternalCurveRule {
    std::string externalName;
    std::string bdfrCurve;
    float scale = 1.0F;
    float bias = 0.0F;
    float minValue = 0.0F;
    float maxValue = 1.0F;
    bool invert = false;
};

class ExternalMocapProfile {
public:
    std::string name;

    bool addRule(ExternalCurveRule rule);
    const std::vector<ExternalCurveRule>& rules() const noexcept;

    FacialFrame normalize(const CurveMap& externalCurves,
                          double timestampSeconds,
                          float confidence = 1.0F,
                          const HeadPose& head = {},
                          const Gaze& gaze = {}) const;

private:
    std::vector<ExternalCurveRule> rules_;
};

struct MocapPacket {
    std::string sourceId;
    std::uint64_t sequenceNumber = 0;
    FacialFrame frame;
};

class ILiveMocapSource {
public:
    virtual ~ILiveMocapSource() = default;
    virtual bool poll(MocapPacket& packet) = 0;
};

} // namespace bdfr::mocap
