#include "bdfr/core/SequenceDiagnostics.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace bdfr {

SequenceQualityReport SequenceDiagnostics::analyze(
    const FacialSequence& sequence,
    float lowConfidenceThreshold,
    float spikeThreshold,
    float neighborAgreementTolerance) {

    SequenceQualityReport report;
    const auto& frames = sequence.frames();
    if (frames.empty()) {
        report.minimumConfidence = 0.0F;
        return report;
    }

    double confidenceSum = 0.0;
    report.minimumConfidence = 1.0F;

    for (const auto& frame : frames) {
        confidenceSum += frame.confidence;
        report.minimumConfidence =
            std::min(report.minimumConfidence, frame.confidence);
        if (frame.confidence < lowConfidenceThreshold)
            ++report.lowConfidenceFrames;
    }

    report.meanConfidence = clamp01(
        static_cast<float>(confidenceSum / static_cast<double>(frames.size())));

    if (frames.size() < 3) return report;

    for (std::size_t i = 1; i + 1 < frames.size(); ++i) {
        std::unordered_set<std::string> ids;
        for (const auto& [id, _] : frames[i - 1].curves) ids.insert(id);
        for (const auto& [id, _] : frames[i].curves) ids.insert(id);
        for (const auto& [id, _] : frames[i + 1].curves) ids.insert(id);

        for (const auto& id : ids) {
            const auto get = [&](const FacialFrame& frame) {
                const auto it = frame.curves.find(id);
                return it == frame.curves.end() ? 0.0F : it->second;
            };

            const float previous = get(frames[i - 1]);
            const float current = get(frames[i]);
            const float next = get(frames[i + 1]);

            const float neighborDelta = std::fabs(previous - next);
            const float expected = (previous + next) * 0.5F;
            const float magnitude = std::fabs(current - expected);

            if (neighborDelta <= neighborAgreementTolerance &&
                magnitude >= spikeThreshold) {
                report.spikes.push_back({
                    id,
                    frames[i].timestampSeconds,
                    previous,
                    current,
                    next,
                    magnitude
                });
            }
        }
    }

    return report;
}

} // namespace bdfr
