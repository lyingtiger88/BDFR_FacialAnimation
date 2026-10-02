#include "bdfr/core/Arkit52.h"
#include "bdfr/core/Retargeter.h"
#include "bdfr/core/SequenceCompare.h"
#include "bdfr/runtime/FramePacketCodec.h"
#include "bdfr/runtime/LiveSessionReceiver.h"
#include "bdfr/runtime/SessionStream.h"
#include "bdfr/runtime/UdpTransport.h"
#include "bdfr/speech/DialogueCompiler.h"
#include "bdfr/speech/DialogueMarkup.h"

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
    } else {
        std::cout << "PASS: " << message << '\n';
    }
}

bool near(float a, float b, float epsilon = 0.01F) {
    return std::fabs(a - b) <= epsilon;
}

} // namespace

int main() {
    std::string error;

    std::cout << "BDFR INITIAL SMOKE TEST\n";
    std::cout << "=======================\n";

    bdfr::speech::DialogueScript script;
    expect(
        bdfr::speech::DialogueMarkup::parse(
            "[emotion=happy intensity=0.5] Hello BDFR [pause=0.1] [blink]",
            script,
            &error),
        "dialogue markup parses");

    bdfr::speech::DialogueCompileResult compiled;
    expect(
        bdfr::speech::DialogueCompiler::compile(
            script,
            compiled,
            {},
            &error),
        "dialogue compiles to timeline");

    expect(
        compiled.timeline.tracks().size() >= 4,
        "compiled timeline contains layered tracks");

    const double sampleTime =
        compiled.durationSeconds > 0.05
            ? compiled.durationSeconds * 0.4
            : 0.01;

    const bdfr::CurveMap generatedCurves =
        compiled.timeline.evaluate(sampleTime);

    expect(
        !generatedCurves.empty(),
        "timeline produces facial curves");

    bdfr::RetargetProfile arkitProfile =
        bdfr::Arkit52::identityProfile();

    const bdfr::CurveMap retargeted =
        arkitProfile.apply(generatedCurves);

    expect(
        !retargeted.empty(),
        "generated curves pass through ARKit52 retarget profile");

    bdfr::FacialFrame sourceFrame;
    sourceFrame.timestampSeconds = 10.0;
    sourceFrame.confidence = 0.95F;
    sourceFrame.curves = retargeted;

    bdfr::mocap::MocapPacket packet;
    packet.sourceId = "initial-smoke";
    packet.sequenceNumber = 1;
    packet.frame = sourceFrame;

    const auto encoded =
        bdfr::runtime::FramePacketCodec::encode(packet);

    bdfr::mocap::MocapPacket decoded;
    expect(
        bdfr::runtime::FramePacketCodec::decode(encoded, decoded),
        "BDFP packet binary roundtrip succeeds");

    expect(
        decoded.sourceId == "initial-smoke" &&
        decoded.sequenceNumber == 1,
        "BDFP packet metadata survives roundtrip");

    bdfr::runtime::UdpFrameSender sender;
    bdfr::runtime::LiveSessionReceiver receiver(0.0);

    expect(
        receiver.open(0, "127.0.0.1", &error),
        "live receiver opens on loopback");

    expect(
        sender.open(&error),
        "UDP sender opens");

    expect(
        sender.sendTo(
            "127.0.0.1",
            receiver.localPort(),
            packet,
            &error),
        "facial packet sends over UDP");

    expect(
        receiver.poll(1000, 10.2, &error),
        "live receiver accepts UDP facial packet");

    bdfr::FacialFrame liveFrame;
    expect(
        receiver.popReady(11.0, liveFrame),
        "jitter/clock pipeline releases playback-ready frame");

    expect(
        !liveFrame.curves.empty(),
        "live frame preserves facial curves");

    bdfr::mocap::MocapPacket recordedPacket = packet;
    recordedPacket.frame = liveFrame;

    const std::vector<bdfr::mocap::MocapPacket> recorded = {
        packet,
        recordedPacket
    };

    const auto sessionBytes =
        bdfr::runtime::SessionStream::encode(recorded);

    std::vector<bdfr::mocap::MocapPacket> restored;
    expect(
        bdfr::runtime::SessionStream::decode(
            sessionBytes,
            restored,
            &error),
        "BDFS solved-session roundtrip succeeds");

    expect(
        restored.size() == 2,
        "BDFS preserves recorded frame count");

    bdfr::FacialSequence sequenceA;
    bdfr::FacialSequence sequenceB;

    bdfr::FacialFrame a0 = packet.frame;
    a0.timestampSeconds = 0.0;

    bdfr::FacialFrame a1 = packet.frame;
    a1.timestampSeconds = 0.1;

    bdfr::FacialFrame b0 = a0;
    bdfr::FacialFrame b1 = a1;

    sequenceA.addFrame(a0);
    sequenceA.addFrame(a1);
    sequenceB.addFrame(b0);
    sequenceB.addFrame(b1);

    const auto comparison =
        bdfr::SequenceCompare::compare(
            sequenceA,
            sequenceB,
            60.0);

    expect(
        comparison.maximumAbsoluteError < 0.0001F,
        "record/replay sequence comparison is deterministic");

    const auto stats = receiver.stats();

    expect(
        stats.packetsReceived == 1 &&
        stats.packetsLost == 0,
        "live receiver reports clean network statistics");

    std::cout << "\nSummary\n";
    std::cout << "-------\n";
    std::cout << "Timeline duration: "
              << compiled.durationSeconds << " s\n";
    std::cout << "Generated curves: "
              << generatedCurves.size() << "\n";
    std::cout << "BDFP bytes: "
              << encoded.size() << "\n";
    std::cout << "BDFS bytes: "
              << sessionBytes.size() << "\n";
    std::cout << "UDP received: "
              << stats.packetsReceived << "\n";

    if (failures == 0) {
        std::cout << "\nBDFR INITIAL SMOKE TEST PASSED.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << "\n"
              << failures
              << " initial smoke test(s) failed.\n";
    return EXIT_FAILURE;
}
