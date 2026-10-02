#include "bdfr/core/SequenceFilter.h"

#include <algorithm>
#include <unordered_map>

namespace bdfr {

FacialSequence SequenceFilter::movingAverage(
    const FacialSequence& input,
    std::size_t radiusFrames) {

    FacialSequence output;
    const auto& frames = input.frames();
    if (frames.empty()) return output;

    for (std::size_t i = 0; i < frames.size(); ++i) {
        const std::size_t begin = i > radiusFrames ? i - radiusFrames : 0;
        const std::size_t end = std::min(frames.size() - 1, i + radiusFrames);

        FacialFrame frame = frames[i];
        std::unordered_map<std::string, double> sums;
        std::unordered_map<std::string, std::size_t> counts;

        double confidenceSum = 0.0;
        std::size_t confidenceCount = 0;

        for (std::size_t j = begin; j <= end; ++j) {
            confidenceSum += frames[j].confidence;
            ++confidenceCount;
            for (const auto& [id, value] : frames[j].curves) {
                sums[id] += value;
                counts[id] += 1;
            }
        }

        frame.curves.clear();
        for (const auto& [id, sum] : sums) {
            frame.curves[id] = clamp01(
                static_cast<float>(sum / static_cast<double>(counts[id])));
        }
        frame.confidence = confidenceCount == 0 ? frame.confidence :
            clamp01(static_cast<float>(
                confidenceSum / static_cast<double>(confidenceCount)));

        output.addFrame(std::move(frame));
    }

    return output;
}

} // namespace bdfr
