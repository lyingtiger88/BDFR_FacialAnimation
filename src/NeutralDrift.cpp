#include "bdfr/core/NeutralDrift.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace bdfr {

NeutralDriftCorrector::NeutralDriftCorrector(
    NeutralDriftOptions options)
    : options_(options) {

    options_.baselineAlpha =
        std::clamp(options_.baselineAlpha, 0.0F, 1.0F);

    options_.neutralThreshold =
        std::clamp(options_.neutralThreshold, 0.0F, 1.0F);

    options_.maximumCorrection =
        std::clamp(options_.maximumCorrection, 0.0F, 1.0F);
}

void NeutralDriftCorrector::reset() {
    baseline_.clear();
}

CurveMap NeutralDriftCorrector::process(
    const CurveMap& input,
    float frameConfidence) {

    const float confidence =
        clamp01(frameConfidence);

    CurveMap output;
    std::unordered_set<std::string> ids;

    for (const auto& [id, _] : input) ids.insert(id);
    for (const auto& [id, _] : baseline_) ids.insert(id);

    for (const auto& id : ids) {
        const auto inIt = input.find(id);
        const float value =
            inIt == input.end()
                ? 0.0F
                : clamp01(inIt->second);

        float& baseline = baseline_[id];

        // Adapt the baseline only while the signal looks plausibly neutral.
        // This avoids teaching an active smile/blink/jaw pose as the new zero.
        if (confidence > 0.25F &&
            value <= options_.neutralThreshold) {

            const float alpha =
                options_.baselineAlpha * confidence;

            baseline +=
                (value - baseline) * alpha;

            baseline = std::clamp(
                baseline,
                0.0F,
                options_.maximumCorrection);
        }

        output[id] =
            clamp01(value - baseline);
    }

    return output;
}

const CurveMap&
NeutralDriftCorrector::baseline() const noexcept {
    return baseline_;
}

FacialSequence NeutralDriftCorrector::correct(
    const FacialSequence& input,
    const NeutralDriftOptions& options) {

    FacialSequence output;
    NeutralDriftCorrector corrector(options);

    for (const auto& source : input.frames()) {
        FacialFrame frame = source;
        frame.curves =
            corrector.process(
                source.curves,
                source.confidence);
        output.addFrame(std::move(frame));
    }

    return output;
}

} // namespace bdfr
