#include "bdfr/audio/AudioFeatures.h"

#include <algorithm>
#include <cmath>

namespace bdfr::audio {

std::vector<AudioFeatureFrame> AudioFeatures::analyze(
    const AudioBuffer& audio,
    double windowSeconds,
    double hopSeconds) {

    std::vector<AudioFeatureFrame> out;
    if (!audio.valid() ||
        !std::isfinite(windowSeconds) || !std::isfinite(hopSeconds) ||
        windowSeconds <= 0.0 || hopSeconds <= 0.0) {
        return out;
    }

    const std::size_t windowFrames = std::max<std::size_t>(
        1, static_cast<std::size_t>(std::llround(windowSeconds * audio.sampleRate)));
    const std::size_t hopFrames = std::max<std::size_t>(
        1, static_cast<std::size_t>(std::llround(hopSeconds * audio.sampleRate)));

    for (std::size_t start = 0; start < audio.frameCount(); start += hopFrames) {
        const std::size_t end = std::min(audio.frameCount(), start + windowFrames);
        if (end <= start) break;

        double sumSquares = 0.0;
        float peak = 0.0F;
        std::size_t zeroCrossings = 0;
        float previousMono = 0.0F;
        bool havePrevious = false;

        for (std::size_t frame = start; frame < end; ++frame) {
            float mono = 0.0F;
            for (std::size_t ch = 0; ch < audio.channels; ++ch) {
                mono += audio.sample(frame, ch);
            }
            mono /= static_cast<float>(audio.channels);

            sumSquares += static_cast<double>(mono) * mono;
            peak = std::max(peak, std::fabs(mono));

            if (havePrevious &&
                ((mono >= 0.0F && previousMono < 0.0F) ||
                 (mono < 0.0F && previousMono >= 0.0F))) {
                ++zeroCrossings;
            }
            previousMono = mono;
            havePrevious = true;
        }

        const std::size_t count = end - start;
        AudioFeatureFrame feature;
        feature.startSeconds =
            static_cast<double>(start) / static_cast<double>(audio.sampleRate);
        feature.durationSeconds =
            static_cast<double>(count) / static_cast<double>(audio.sampleRate);
        feature.rms = static_cast<float>(
            std::sqrt(sumSquares / static_cast<double>(count)));
        feature.peak = peak;
        feature.zeroCrossingRate = count > 1
            ? static_cast<float>(zeroCrossings) / static_cast<float>(count - 1)
            : 0.0F;
        out.push_back(feature);

        if (end == audio.frameCount() && start + hopFrames >= audio.frameCount())
            break;
    }

    return out;
}

} // namespace bdfr::audio
