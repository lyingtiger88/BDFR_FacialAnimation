#include "bdfr/core/SequenceCleanup.h"

#include <cmath>
#include <unordered_set>

namespace bdfr {

FacialSequence SequenceCleanup::despike(
    const FacialSequence& input,
    float spikeThreshold,
    float neighborAgreementTolerance) {

    FacialSequence output;
    const auto& frames = input.frames();
    if (frames.empty()) return output;

    for (std::size_t i = 0; i < frames.size(); ++i) {
        FacialFrame frame = frames[i];

        if (i > 0 && i + 1 < frames.size()) {
            std::unordered_set<std::string> ids;
            for (const auto& [id, _] : frames[i - 1].curves) ids.insert(id);
            for (const auto& [id, _] : frames[i].curves) ids.insert(id);
            for (const auto& [id, _] : frames[i + 1].curves) ids.insert(id);

            for (const auto& id : ids) {
                const auto get = [&](const FacialFrame& f) {
                    const auto it = f.curves.find(id);
                    return it == f.curves.end() ? 0.0F : it->second;
                };

                const float previous = get(frames[i - 1]);
                const float current = get(frames[i]);
                const float next = get(frames[i + 1]);
                const float expected = (previous + next) * 0.5F;

                if (std::fabs(previous - next) <= neighborAgreementTolerance &&
                    std::fabs(current - expected) >= spikeThreshold) {
                    frame.curves[id] = clamp01(expected);
                }
            }
        }

        output.addFrame(std::move(frame));
    }

    return output;
}

} // namespace bdfr
