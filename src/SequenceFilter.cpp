#include "bdfr/core/SequenceFilter.h"
#include "bdfr/core/CurveMixer.h"

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


FacialSequence SequenceFilter::bidirectionalExponential(
    const FacialSequence& input,
    float alpha) {

    FacialSequence output;
    const auto& source = input.frames();
    if (source.empty()) return output;

    const float a = clamp01(alpha);

    std::vector<FacialFrame> forward = source;
    for (std::size_t i = 1; i < forward.size(); ++i) {
        forward[i].curves = CurveMixer::exponentialSmooth(
            forward[i - 1].curves,
            source[i].curves,
            a);
        forward[i].confidence =
            forward[i - 1].confidence +
            (source[i].confidence - forward[i - 1].confidence) * a;
    }

    std::vector<FacialFrame> backward = source;
    for (std::size_t i = backward.size() - 1; i > 0; --i) {
        const std::size_t current = i - 1;
        backward[current].curves = CurveMixer::exponentialSmooth(
            backward[i].curves,
            source[current].curves,
            a);
        backward[current].confidence =
            backward[i].confidence +
            (source[current].confidence - backward[i].confidence) * a;
    }

    for (std::size_t i = 0; i < source.size(); ++i) {
        FacialFrame frame = source[i];

        std::unordered_set<std::string> ids;
        for (const auto& [id, _] : forward[i].curves) ids.insert(id);
        for (const auto& [id, _] : backward[i].curves) ids.insert(id);

        frame.curves.clear();
        for (const auto& id : ids) {
            const auto fit = forward[i].curves.find(id);
            const auto bit = backward[i].curves.find(id);
            const float fv = fit == forward[i].curves.end() ? 0.0F : fit->second;
            const float bv = bit == backward[i].curves.end() ? 0.0F : bit->second;
            frame.curves[id] = clamp01((fv + bv) * 0.5F);
        }

        frame.confidence = clamp01(
            (forward[i].confidence + backward[i].confidence) * 0.5F);

        output.addFrame(std::move(frame));
    }

    return output;
}

} // namespace bdfr
