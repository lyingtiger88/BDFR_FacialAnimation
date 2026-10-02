#include "bdfr/capture/OfflineVideoProcessor.h"
#include "bdfr/core/SequenceFilter.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

bool near(float a, float b, float epsilon = 0.0001F) {
    return std::fabs(a - b) <= epsilon;
}

class FakeVideoSource final : public bdfr::capture::IVideoSource {
public:
    FakeVideoSource() {
        for (int i = 0; i < 5; ++i) {
            bdfr::capture::VideoFrame frame;
            frame.width = 2;
            frame.height = 2;
            frame.strideBytes = 8;
            frame.format = bdfr::capture::PixelFormat::RGBA32;
            frame.timestampSeconds = i * 0.1;
            frame.pixels.resize(16, static_cast<std::uint8_t>(i + 1));
            frames_.push_back(std::move(frame));
        }
    }

    bool next(bdfr::capture::VideoFrame& frame) override {
        if (index_ >= frames_.size()) return false;
        frame = frames_[index_++];
        return true;
    }

    void reset() override {
        index_ = 0;
    }

private:
    std::vector<bdfr::capture::VideoFrame> frames_;
    std::size_t index_ = 0;
};

class FakeTracker final : public bdfr::capture::IFaceTracker {
public:
    bool process(const bdfr::capture::ImageView& image,
                 bdfr::capture::FaceObservation& observation) override {
        if (!image.valid()) return false;

        observation.timestampSeconds = image.timestampSeconds;
        observation.confidence = image.timestampSeconds == 0.2 ? 0.2F : 0.95F;
        observation.landmarks = {
            {0.25F, 0.25F, 0.0F, observation.confidence},
            {0.75F, 0.25F, 0.0F, observation.confidence}
        };
        observation.regions.mouth = observation.confidence;
        observation.regions.leftEye = observation.confidence;
        observation.regions.rightEye = observation.confidence;
        observation.regions.brows = observation.confidence;
        observation.regions.jaw = observation.confidence;
        observation.occluded = false;
        return true;
    }

    void reset() override {}
};

class FakeSolver final : public bdfr::capture::IFaceSolver {
public:
    bool solve(const bdfr::capture::FaceObservation& observation,
               bdfr::FacialFrame& frame) override {
        frame.timestampSeconds = observation.timestampSeconds;
        frame.confidence = observation.confidence;
        frame.curves["jawOpen"] = static_cast<float>(
            std::min(1.0, observation.timestampSeconds * 2.0));
        frame.curves["eyeBlinkLeft"] = 0.1F;
        return true;
    }

    void reset() override {}
};

} // namespace

int main() {
    FakeVideoSource source;
    FakeTracker tracker;
    FakeSolver solver;

    bdfr::capture::OfflineSolveOptions options;
    options.minimumQuality = 0.5F;
    options.skipLowQualityFrames = true;

    const auto result =
        bdfr::capture::OfflineVideoProcessor::process(
            source, tracker, solver, options);

    expect(result.sourceFrames == 5,
           "offline pipeline reads all source frames");
    expect(result.skippedFrames == 1,
           "offline pipeline skips one low-quality frame");
    expect(result.solvedFrames == 4,
           "offline pipeline solves remaining frames");
    expect(result.sequence.frames().size() == 4,
           "offline pipeline produces editable facial sequence");
    expect(result.frames.size() == 5,
           "offline pipeline emits per-frame quality reports");

    const auto filtered =
        bdfr::SequenceFilter::movingAverage(result.sequence, 1);
    expect(filtered.frames().size() == result.sequence.frames().size(),
           "offline sequence filter preserves frame count");

    const auto sample = filtered.sample(0.3);
    expect(sample.curves.find("jawOpen") != sample.curves.end(),
           "filtered offline sequence remains sampleable");
    expect(sample.confidence > 0.0F && sample.confidence <= 1.0F,
           "filtered offline sequence preserves normalized confidence");

    bdfr::capture::OfflineSolveOptions includeLow;
    includeLow.minimumQuality = 0.5F;
    includeLow.skipLowQualityFrames = false;

    const auto fullResult =
        bdfr::capture::OfflineVideoProcessor::process(
            source, tracker, solver, includeLow);

    expect(fullResult.solvedFrames == 5,
           "offline pipeline can retain low-quality frames for manual cleanup");
    expect(fullResult.sequence.frames().size() == 5,
           "full offline solve preserves complete timing");



    const auto studioFiltered =
        bdfr::SequenceFilter::bidirectionalExponential(
            fullResult.sequence, 0.4F);
    expect(studioFiltered.frames().size() ==
           fullResult.sequence.frames().size(),
           "Studio bidirectional smoothing preserves frame count");
    expect(std::fabs(
               studioFiltered.frames()[2].timestampSeconds -
               fullResult.sequence.frames()[2].timestampSeconds) < 0.000001,
           "Studio smoothing preserves source timestamps");

    if (failures == 0) {
        std::cout << "All BDFR offline pipeline tests passed.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
