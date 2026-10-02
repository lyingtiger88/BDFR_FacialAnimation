#include "bdfr/audio/PitchTracker.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace bdfr::audio {

namespace {

float monoSample(const AudioBuffer& audio, std::size_t frame) {
    if (audio.channels == 0) return 0.0F;
    float sum = 0.0F;
    for (std::size_t channel = 0; channel < audio.channels; ++channel) {
        sum += audio.sample(frame, channel);
    }
    return sum / static_cast<float>(audio.channels);
}

} // namespace

std::vector<PitchFrame> PitchTracker::analyze(
    const AudioBuffer& audio,
    double windowSeconds,
    double hopSeconds,
    float minimumHz,
    float maximumHz,
    float voicedThreshold) {

    std::vector<PitchFrame> out;

    if (!audio.valid() ||
        windowSeconds <= 0.0 ||
        hopSeconds <= 0.0 ||
        minimumHz <= 0.0F ||
        maximumHz <= minimumHz) {
        return out;
    }

    const std::size_t windowFrames = std::max<std::size_t>(
        2,
        static_cast<std::size_t>(
            std::llround(windowSeconds * audio.sampleRate)));

    const std::size_t hopFrames = std::max<std::size_t>(
        1,
        static_cast<std::size_t>(
            std::llround(hopSeconds * audio.sampleRate)));

    const std::size_t minimumLag = std::max<std::size_t>(
        1,
        static_cast<std::size_t>(
            std::floor(audio.sampleRate / maximumHz)));

    const std::size_t maximumLag = std::min<std::size_t>(
        windowFrames - 1,
        static_cast<std::size_t>(
            std::ceil(audio.sampleRate / minimumHz)));

    for (std::size_t start = 0;
         start + windowFrames <= audio.frameCount();
         start += hopFrames) {

        std::vector<float> samples(windowFrames);
        double mean = 0.0;

        for (std::size_t i = 0; i < windowFrames; ++i) {
            samples[i] = monoSample(audio, start + i);
            mean += samples[i];
        }

        mean /= static_cast<double>(windowFrames);

        double energy = 0.0;
        for (float& sample : samples) {
            sample -= static_cast<float>(mean);
            energy += static_cast<double>(sample) * sample;
        }

        PitchFrame frame;
        frame.startSeconds =
            static_cast<double>(start) / audio.sampleRate;
        frame.durationSeconds =
            static_cast<double>(windowFrames) / audio.sampleRate;

        if (energy <= 1e-9) {
            out.push_back(frame);
            continue;
        }

        float bestCorrelation = 0.0F;
        std::size_t bestLag = 0;

        for (std::size_t lag = minimumLag;
             lag <= maximumLag;
             ++lag) {

            double numerator = 0.0;
            double leftEnergy = 0.0;
            double rightEnergy = 0.0;

            for (std::size_t i = 0;
                 i + lag < windowFrames;
                 ++i) {
                const double a = samples[i];
                const double b = samples[i + lag];
                numerator += a * b;
                leftEnergy += a * a;
                rightEnergy += b * b;
            }

            const double denom =
                std::sqrt(leftEnergy * rightEnergy);

            const float correlation =
                denom > 1e-12
                    ? static_cast<float>(numerator / denom)
                    : 0.0F;

            if (correlation > bestCorrelation) {
                bestCorrelation = correlation;
                bestLag = lag;
            }
        }

        frame.confidence =
            std::clamp(bestCorrelation, 0.0F, 1.0F);

        frame.voiced =
            bestLag > 0 &&
            frame.confidence >=
                std::clamp(voicedThreshold, 0.0F, 1.0F);

        if (frame.voiced) {
            frame.frequencyHz =
                static_cast<float>(audio.sampleRate) /
                static_cast<float>(bestLag);
        }

        out.push_back(frame);
    }

    return out;
}

} // namespace bdfr::audio
