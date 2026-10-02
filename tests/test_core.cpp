#include "bdfr/core/BinaryCodec.h"
#include "bdfr/core/CurveMixer.h"
#include "bdfr/core/CurveRegistry.h"
#include "bdfr/core/Timeline.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

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
}

int main() {
    bdfr::CurveRegistry registry;
    expect(registry.isRegistered("AU12"), "AU12 is canonical");
    expect(registry.isRegistered("jawOpen"), "jawOpen is canonical");
    expect(!registry.isRegistered("notARealCurve"), "unknown curve rejected");
    expect(registry.registerCustom("customTongueOut"), "custom curve can be registered");
    expect(registry.isRegistered("customTongueOut"), "custom curve becomes known");

    bdfr::FacialFrame valid;
    valid.timestampSeconds = 1.25;
    valid.confidence = 0.9F;
    valid.curves["AU12"] = 0.75F;
    valid.curves["jawOpen"] = 0.25F;
    std::string error;
    expect(registry.validateFrame(valid, &error), "valid frame accepted: " + error);

    bdfr::FacialFrame bad = valid;
    bad.curves["AU12"] = 1.5F;
    expect(!registry.validateFrame(bad, &error), "out-of-range curve rejected");
    const auto sanitized = registry.sanitized(bad);
    expect(near(sanitized.curves.at("AU12"), 1.0F), "sanitization clamps curve");

    const auto encoded = bdfr::BinaryCodec::encode(valid);
    bdfr::FacialFrame decoded;
    expect(bdfr::BinaryCodec::decode(encoded, decoded), "binary frame roundtrip decodes");
    expect(std::fabs(decoded.timestampSeconds - valid.timestampSeconds) < 0.000001, "binary timestamp preserved");
    expect(near(decoded.curves.at("AU12"), 0.75F), "binary curve preserved");
    auto truncated = encoded;
    truncated.pop_back();
    expect(!bdfr::BinaryCodec::decode(truncated, decoded), "truncated frame rejected");

    bdfr::CurveMap base{{"jawOpen", 0.2F}, {"AU12", 0.0F}};
    bdfr::CurveLayer speech{{{"jawOpen", 0.8F}}, 0.5F, false};
    bdfr::CurveLayer emotion{{{"AU12", 0.6F}}, 1.0F, false};
    const auto mixed = bdfr::CurveMixer::mix(base, {speech, emotion});
    expect(near(mixed.at("jawOpen"), 0.5F), "weighted override mix works");
    expect(near(mixed.at("AU12"), 0.6F), "emotion layer mixes independently");

    const auto smoothed = bdfr::CurveMixer::exponentialSmooth({{"jawOpen", 0.0F}}, {{"jawOpen", 1.0F}}, 0.25F);
    expect(near(smoothed.at("jawOpen"), 0.25F), "exponential smoothing works");

    bdfr::Timeline timeline;
    expect(timeline.addTrack({"Dialogue", bdfr::TrackType::TextDialogue}), "text track added");
    expect(timeline.addTrack({"Emotion", bdfr::TrackType::Emotion}), "emotion track added");
    expect(timeline.addTrack({"Ticks", bdfr::TrackType::InstantEvent}), "instant-event track added");
    expect(timeline.addTrack({"Mocap", bdfr::TrackType::Mocap}), "mocap track added");

    expect(timeline.addClip(0, {"line_001", 0.0, 2.5, "Where have you been?", 1.0F}), "text clip added");
    expect(timeline.addClip(1, {"emotion_001", 0.0, 2.5, "concerned:0.55", 1.0F}), "emotion clip added");
    expect(timeline.addClip(2, {"blink_001", 1.1, 0.12, "blink", 1.0F}), "instant event added");
    expect(timeline.addClip(3, {"mocap_001", 0.0, 3.0, "take_001.bdfr", 1.0F}), "mocap clip added");
    expect(timeline.validate(&error), "timeline validates: " + error);
    expect(std::fabs(timeline.durationSeconds() - 3.0) < 0.0001, "timeline duration computed");

    if (failures == 0) {
        std::cout << "All BDFR core tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
