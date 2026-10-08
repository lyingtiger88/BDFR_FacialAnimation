#include "bdfr/core/BinaryCodec.h"
#include "bdfr/core/CurveMixer.h"
#include "bdfr/core/CurveRegistry.h"
#include "bdfr/core/CurveDynamics.h"
#include "bdfr/core/CurveCatalog.h"
#include "bdfr/core/CurveTools.h"
#include "bdfr/core/Diagnostics.h"
#include "bdfr/core/CorrectiveEngine.h"
#include "bdfr/core/ExpressionStack.h"
#include "bdfr/core/JsonCodec.h"
#include "bdfr/core/KeyReducer.h"
#include "bdfr/core/History.h"
#include "bdfr/core/Project.h"
#include "bdfr/core/Persistence.h"
#include "bdfr/core/Retargeter.h"
#include "bdfr/core/AutoMapper.h"
#include "bdfr/core/Arkit52.h"
#include "bdfr/speech/TextSpeech.h"
#include "bdfr/audio/WavAudio.h"
#include "bdfr/audio/AudioFeatures.h"
#include "bdfr/audio/Prosody.h"
#include "bdfr/audio/PitchTracker.h"
#include "bdfr/audio/AudioBehavior.h"
#include "bdfr/speech/DialogueMarkup.h"
#include "bdfr/speech/DialogueCompiler.h"
#include "bdfr/speech/TranscriptTiming.h"
#include "bdfr/speech/LanguageProfile.h"
#include "bdfr/runtime/LiveRuntime.h"
#include "bdfr/mocap/ExternalMocap.h"
#include "bdfr/mocap/MocapCsv.h"
#include "bdfr/models/ModelRegistry.h"
#include "bdfr/runtime/FramePacketCodec.h"
#include "bdfr/runtime/SessionStream.h"
#include "bdfr/runtime/UdpTransport.h"
#include "bdfr/runtime/LiveSessionReceiver.h"
#include "bdfr/core/Session.h"
#include "bdfr/core/Timecode.h"
#include "bdfr/core/Schema.h"
#include "bdfr/core/Recovery.h"
#include "bdfr/expression/EmotionEngine.h"
#include "bdfr/core/PerformanceFusion.h"
#include "bdfr/core/Calibration.h"
#include "bdfr/core/NeutralDrift.h"
#include "bdfr/core/CurveTuning.h"
#include "bdfr/expression/BehaviorEngine.h"
#include "bdfr/expression/Personality.h"
#include "bdfr/capture/FaceObservation.h"
#include "bdfr/capture/CaptureQuality.h"
#include "bdfr/core/Sequence.h"
#include "bdfr/core/SequencePlayback.h"
#include "bdfr/core/Timeline.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <filesystem>
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
    expect(std::fabs(decoded.timestampSeconds - valid.timestampSeconds) < 0.000001,
           "binary timestamp preserved");
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

    const auto smoothed = bdfr::CurveMixer::exponentialSmooth(
        {{"jawOpen", 0.0F}}, {{"jawOpen", 1.0F}}, 0.25F);
    expect(near(smoothed.at("jawOpen"), 0.25F), "exponential smoothing works");

    bdfr::CurveLayer mouthOnly;
    mouthOnly.curves = {{"mouthSmileLeft", 0.8F}, {"eyeBlinkLeft", 1.0F}};
    mouthOnly.regionMask = bdfr::regionMask(bdfr::CurveRegion::Mouth);
    const auto regionMixed = bdfr::CurveMixer::mix({}, {mouthOnly});
    expect(regionMixed.find("mouthSmileLeft") != regionMixed.end(), "mouth region passes mouth curve");
    expect(regionMixed.find("eyeBlinkLeft") == regionMixed.end(), "mouth region blocks eye curve");

    bdfr::CurveLayer low;
    low.curves = {{"AU12", 0.2F}};
    low.priority = 1;
    bdfr::CurveLayer high;
    high.curves = {{"AU12", 0.9F}};
    high.priority = 10;
    const auto priorityMixed = bdfr::CurveMixer::mix({}, {high, low});
    expect(near(priorityMixed.at("AU12"), 0.9F), "higher-priority override is applied last");

    bdfr::Timeline timeline;
    expect(timeline.addTrack({"Dialogue", bdfr::TrackType::TextDialogue}), "text track added");
    expect(timeline.addTrack({"Emotion", bdfr::TrackType::Emotion}), "emotion track added");
    expect(timeline.addTrack({"Ticks", bdfr::TrackType::InstantEvent}), "instant-event track added");
    expect(timeline.addTrack({"Mocap", bdfr::TrackType::Mocap}), "mocap track added");

    expect(timeline.addClip(0, {"line_001", 0.0, 2.5, "Where have you been?", 1.0F}),
           "text clip added");
    expect(timeline.addClip(1, {"emotion_001", 0.0, 2.5, "concerned:0.55", 1.0F}),
           "emotion clip added");
    expect(timeline.addClip(2, {"blink_001", 1.1, 0.12, "blink", 1.0F}),
           "instant event added");
    expect(timeline.addClip(3, {"mocap_001", 0.0, 3.0, "take_001.bdfr", 1.0F}),
           "mocap clip added");
    expect(timeline.validate(&error), "timeline validates: " + error);
    expect(std::fabs(timeline.durationSeconds() - 3.0) < 0.0001, "timeline duration computed");

    bdfr::Timeline evalTimeline;
    bdfr::TimelineTrack speechTrack{"Speech", bdfr::TrackType::GeneratedSpeech};
    speechTrack.priority = 1;
    speechTrack.regionMask = bdfr::regionMask(bdfr::CurveRegion::Mouth) |
                             bdfr::regionMask(bdfr::CurveRegion::Jaw);
    expect(evalTimeline.addTrack(speechTrack), "speech evaluation track added");

    bdfr::TimelineTrack mocapTrack{"Mocap", bdfr::TrackType::Mocap};
    mocapTrack.priority = 5;
    expect(evalTimeline.addTrack(mocapTrack), "mocap evaluation track added");

    bdfr::TimelineClip speechClip;
    speechClip.id = "speech_curves";
    speechClip.startSeconds = 0.0;
    speechClip.durationSeconds = 2.0;
    speechClip.curves = {{"jawOpen", 0.8F}, {"eyeBlinkLeft", 1.0F}};
    expect(evalTimeline.addClip(0, speechClip), "speech curve clip added");

    bdfr::TimelineClip mocapClip;
    mocapClip.id = "mocap_curves";
    mocapClip.startSeconds = 0.0;
    mocapClip.durationSeconds = 2.0;
    mocapClip.curves = {{"jawOpen", 0.3F}, {"eyeBlinkLeft", 0.9F}};
    expect(evalTimeline.addClip(1, mocapClip), "mocap curve clip added");

    auto evaluated = evalTimeline.evaluate(1.0);
    expect(near(evaluated.at("jawOpen"), 0.3F), "track priority resolves jaw override");
    expect(near(evaluated.at("eyeBlinkLeft"), 0.9F), "mocap provides eye curve");
    expect(evalTimeline.evaluate(3.0).empty(), "inactive clips do not evaluate");

    evalTimeline.tracks()[1].muted = true;
    evaluated = evalTimeline.evaluate(1.0);
    expect(near(evaluated.at("jawOpen"), 0.8F), "muted mocap reveals speech jaw");
    expect(evaluated.find("eyeBlinkLeft") == evaluated.end(),
           "speech region mask blocks eye curve after mocap mute");

    evalTimeline.tracks()[1].muted = false;
    evalTimeline.tracks()[0].solo = true;
    evaluated = evalTimeline.evaluate(1.0);
    expect(near(evaluated.at("jawOpen"), 0.8F), "solo track isolates speech");
    expect(evaluated.find("eyeBlinkLeft") == evaluated.end(), "solo respects region mask");

    bdfr::Session session;
    session.id = "session_001";
    session.project = "BDFR_Test";
    session.scene = "Scene01";
    session.shot = "Shot010";

    bdfr::Take take;
    take.id = "take_001";
    take.name = "Take 001";
    take.actorId = "actor_001";
    take.source = "desktop-video";
    take.durationSeconds = 10.0;
    take.frameRateNumerator = 30000;
    take.frameRateDenominator = 1001;
    expect(session.addTake(take), "session take added");
    expect(!session.addTake(take), "duplicate take id rejected");
    expect(session.findTake("take_001") != nullptr, "take lookup works");

    expect(session.markDirty("take_001", {2.0, 4.0}), "first partial re-solve range accepted");
    expect(session.markDirty("take_001", {3.5, 6.0}), "overlapping partial re-solve range accepted");
    const auto* storedTake = session.findTake("take_001");
    expect(storedTake != nullptr && storedTake->dirtyRanges.size() == 1,
           "overlapping dirty ranges merge");
    expect(storedTake != nullptr &&
           std::fabs(storedTake->dirtyRanges[0].startSeconds - 2.0) < 0.0001 &&
           std::fabs(storedTake->dirtyRanges[0].endSeconds - 6.0) < 0.0001,
           "merged partial re-solve range is correct");
    expect(!session.markDirty("take_001", {9.0, 11.0}), "out-of-take dirty range rejected");

    bdfr::TimeRange a{1.0, 3.0};
    bdfr::TimeRange b{2.5, 5.0};
    bdfr::TimeRange c{4.0, 5.0};
    expect(a.intersects(b), "time ranges intersect");
    expect(!a.intersects(c), "separated time ranges do not intersect");
    expect(a.contains(2.0), "time range contains point");



    bdfr::FacialSequence sequence;
    bdfr::FacialFrame f0;
    f0.timestampSeconds = 0.0;
    f0.confidence = 1.0F;
    f0.curves = {{"jawOpen", 0.0F}, {"AU12", 0.2F}};
    bdfr::FacialFrame f1;
    f1.timestampSeconds = 1.0;
    f1.confidence = 0.8F;
    f1.curves = {{"jawOpen", 1.0F}, {"AU12", 0.6F}};
    expect(sequence.addFrame(f1), "sequence accepts frame 1");
    expect(sequence.addFrame(f0), "sequence inserts frame 0 in timestamp order");
    expect(sequence.validate(&error), "sequence validates: " + error);
    expect(std::fabs(sequence.durationSeconds() - 1.0) < 0.0001, "sequence duration is final timestamp");
    const auto half = sequence.sample(0.5);
    expect(near(half.curves.at("jawOpen"), 0.5F), "sequence interpolates jaw curve");
    expect(near(half.curves.at("AU12"), 0.4F), "sequence interpolates AU curve");
    expect(near(half.confidence, 0.9F), "sequence interpolates confidence");

    bdfr::CorrectiveEngine correctiveEngine;
    bdfr::CorrectiveRule smileJawCorrective;
    smileJawCorrective.id = "jaw_smile_fix";
    smileJawCorrective.conditions = {
        {"jawOpen", 0.6F, 1.0F},
        {"AU12", 0.5F, 1.0F}
    };
    smileJawCorrective.targetCurve = "mouthPressLeft";
    smileJawCorrective.targetValue = 0.4F;
    smileJawCorrective.weight = 0.5F;
    smileJawCorrective.mode = bdfr::CorrectiveMode::Add;
    expect(correctiveEngine.addRule(smileJawCorrective), "corrective rule accepted");
    const auto corrected = correctiveEngine.apply({
        {"jawOpen", 0.8F},
        {"AU12", 0.7F},
        {"mouthPressLeft", 0.1F}
    });
    expect(near(corrected.at("mouthPressLeft"), 0.3F), "corrective rule modifies target curve");

    bdfr::ExpressionStack stack;
    bdfr::ExpressionLayer speechExpression;
    speechExpression.name = "Speech";
    speechExpression.layer.curves = {{"jawOpen", 0.7F}};
    speechExpression.layer.priority = 1;
    expect(stack.addLayer(speechExpression), "expression speech layer added");

    bdfr::ExpressionLayer emotionExpression;
    emotionExpression.name = "Emotion";
    emotionExpression.layer.curves = {{"AU12", 0.8F}};
    emotionExpression.layer.priority = 2;
    expect(stack.addLayer(emotionExpression), "expression emotion layer added");
    expect(!stack.addLayer(emotionExpression), "duplicate expression layer rejected");

    bdfr::CorrectiveRule expressionCorrective;
    expressionCorrective.id = "expression_fix";
    expressionCorrective.conditions = {
        {"jawOpen", 0.6F, 1.0F},
        {"AU12", 0.7F, 1.0F}
    };
    expressionCorrective.targetCurve = "mouthPressRight";
    expressionCorrective.targetValue = 0.5F;
    expressionCorrective.weight = 1.0F;
    expressionCorrective.mode = bdfr::CorrectiveMode::Override;
    expect(stack.correctives().addRule(expressionCorrective), "expression corrective added");

    const auto expressionOutput = stack.evaluate();
    expect(near(expressionOutput.at("jawOpen"), 0.7F), "expression stack keeps speech jaw");
    expect(near(expressionOutput.at("AU12"), 0.8F), "expression stack keeps emotion smile");
    expect(near(expressionOutput.at("mouthPressRight"), 0.5F), "expression stack applies corrective");
    expect(stack.setEnabled("Emotion", false), "expression layer can be disabled");
    const auto withoutEmotion = stack.evaluate();
    expect(withoutEmotion.find("AU12") == withoutEmotion.end(), "disabled expression layer is excluded");



    const std::string frameJson = bdfr::JsonCodec::encodeFrame(valid);
    bdfr::FacialFrame jsonFrame;
    expect(bdfr::JsonCodec::decodeFrame(frameJson, jsonFrame, &error),
           "JSON facial frame roundtrip decodes: " + error);
    expect(std::fabs(jsonFrame.timestampSeconds - valid.timestampSeconds) < 0.000001,
           "JSON frame timestamp preserved");
    expect(near(jsonFrame.curves.at("AU12"), valid.curves.at("AU12")),
           "JSON frame curve preserved");

    const std::string sequenceJson = bdfr::JsonCodec::encodeSequence(sequence);
    bdfr::FacialSequence decodedSequence;
    expect(bdfr::JsonCodec::decodeSequence(sequenceJson, decodedSequence, &error),
           "JSON sequence roundtrip decodes: " + error);
    expect(decodedSequence.frames().size() == 2, "JSON sequence preserves frame count");
    expect(near(decodedSequence.sample(0.5).curves.at("jawOpen"), 0.5F),
           "decoded JSON sequence remains sampleable");

    const std::string sessionJson = bdfr::JsonCodec::encodeSession(session);
    bdfr::Session decodedSession;
    expect(bdfr::JsonCodec::decodeSession(sessionJson, decodedSession, &error),
           "JSON session roundtrip decodes: " + error);
    expect(decodedSession.id == session.id, "JSON session id preserved");
    expect(decodedSession.takes.size() == 1, "JSON session preserves take count");
    expect(decodedSession.takes[0].dirtyRanges.size() == 1,
           "JSON session preserves merged dirty range");

    bdfr::FacialFrame malformedFrame;
    expect(!bdfr::JsonCodec::decodeFrame("{not valid json}", malformedFrame, &error),
           "malformed JSON frame rejected");



    bdfr::CurveDynamics dynamics;
    expect(dynamics.setConstraint("jawOpen", {0.1F, 0.9F, 2.0F}),
           "curve dynamics accepts valid constraint");
    expect(!dynamics.setConstraint("badCurve", {0.8F, 0.2F, 1.0F}),
           "curve dynamics rejects inverted range");

    const auto dynamicsStep = dynamics.apply(
        {{"jawOpen", 0.2F}},
        {{"jawOpen", 1.0F}},
        0.1);
    expect(near(dynamicsStep.at("jawOpen"), 0.4F),
           "curve velocity limit caps per-step motion");

    const auto dynamicsClamp = dynamics.apply(
        {{"jawOpen", 0.5F}},
        {{"jawOpen", 1.0F}},
        1.0);
    expect(near(dynamicsClamp.at("jawOpen"), 0.9F),
           "curve constraint clamps maximum value");



    std::vector<bdfr::CurveKey> denseKeys = {
        {0.0, 0.0F},
        {0.25, 0.25F},
        {0.5, 0.5F},
        {0.75, 0.75F},
        {1.0, 1.0F}
    };
    const auto reducedLinear = bdfr::KeyReducer::reduce(denseKeys, 0.001F);
    expect(reducedLinear.size() == 2, "linear curve reduces to endpoints");
    expect(bdfr::KeyReducer::maxError(denseKeys, reducedLinear) <= 0.0011F,
           "linear reduction stays within tolerance");

    std::vector<bdfr::CurveKey> shapedKeys = {
        {0.0, 0.0F},
        {0.25, 0.1F},
        {0.5, 0.9F},
        {0.75, 0.2F},
        {1.0, 0.0F}
    };
    const auto reducedShaped = bdfr::KeyReducer::reduce(shapedKeys, 0.05F);
    expect(reducedShaped.size() > 2, "nonlinear curve preserves shape keys");
    expect(bdfr::KeyReducer::maxError(shapedKeys, reducedShaped) <= 0.051F,
           "nonlinear reduction remains within tolerance");



    bdfr::Project project;
    project.name = "BDFR Demo";
    expect(project.addActor({"actor_001", "Test Actor", "baseline profile"}),
           "project actor added");
    expect(!project.addActor({"actor_001", "Duplicate", ""}),
           "duplicate actor rejected");
    expect(project.findActor("actor_001") != nullptr, "project actor lookup works");

    bdfr::Session projectSession;
    projectSession.id = "session_project_001";
    projectSession.project = "BDFR Demo";
    expect(project.addSession(projectSession), "project session added");
    expect(!project.addSession(projectSession), "duplicate project session rejected");
    expect(project.findSession("session_project_001") != nullptr, "project session lookup works");

    bdfr::SnapshotHistory<bdfr::CurveMap> history(3);
    history.reset({{"jawOpen", 0.1F}});
    history.push({{"jawOpen", 0.4F}});
    history.push({{"jawOpen", 0.8F}});
    expect(history.canUndo(), "history can undo");
    const auto* undoState = history.undo();
    expect(undoState != nullptr && near(undoState->at("jawOpen"), 0.4F),
           "history undo restores previous snapshot");
    expect(history.canRedo(), "history can redo");
    const auto* redoState = history.redo();
    expect(redoState != nullptr && near(redoState->at("jawOpen"), 0.8F),
           "history redo restores next snapshot");
    history.push({{"jawOpen", 0.6F}});
    expect(!history.canRedo(), "new edit invalidates redo branch");



    bdfr::CurveCatalog catalog;
    const auto* blinkLeftMeta = catalog.metadata("eyeBlinkLeft");
    expect(blinkLeftMeta != nullptr, "curve catalog returns eye blink metadata");
    expect(blinkLeftMeta != nullptr && blinkLeftMeta->region == bdfr::CurveRegion::Eyes,
           "curve catalog classifies eye blink region");
    expect(blinkLeftMeta != nullptr && blinkLeftMeta->counterpart == "eyeBlinkRight",
           "curve catalog exposes left/right counterpart");
    const auto* au12Meta = catalog.metadata("AU12");
    expect(au12Meta != nullptr && au12Meta->facsActionUnit,
           "curve catalog marks FACS action units");

    const bdfr::CurveMap freezeBase{{"jawOpen", 0.2F}, {"eyeBlinkLeft", 0.1F}};
    const bdfr::CurveMap freezeCandidate{{"jawOpen", 0.9F}, {"eyeBlinkLeft", 0.8F}};
    const auto frozenEyes = bdfr::CurveTools::freezeRegions(
        freezeBase, freezeCandidate, bdfr::regionMask(bdfr::CurveRegion::Eyes));
    expect(near(frozenEyes.at("jawOpen"), 0.9F), "region freeze leaves unfrozen jaw changed");
    expect(near(frozenEyes.at("eyeBlinkLeft"), 0.1F), "region freeze preserves frozen eye region");

    const auto curveDiff = bdfr::CurveTools::diff(
        {{"jawOpen", 0.2F}, {"AU12", 0.4F}},
        {{"jawOpen", 0.8F}, {"AU12", 0.42F}},
        0.1F);
    expect(curveDiff.find("jawOpen") != curveDiff.end(), "curve diff reports meaningful change");
    expect(curveDiff.find("AU12") == curveDiff.end(), "curve diff filters below threshold");



    bdfr::Project persistedProject;
    persistedProject.name = "Persistent Project";
    persistedProject.addActor({"actor_persist", "Persistent Actor", "notes"});
    bdfr::Session persistedSession;
    persistedSession.id = "persist_session";
    persistedSession.project = "Persistent Project";
    persistedSession.scene = "SceneA";
    persistedSession.shot = "ShotA";
    bdfr::Take persistedTake;
    persistedTake.id = "persist_take";
    persistedTake.name = "Take A";
    persistedTake.actorId = "actor_persist";
    persistedTake.source = "video";
    persistedTake.durationSeconds = 5.0;
    persistedSession.addTake(persistedTake);
    persistedProject.addSession(persistedSession);

    const std::string projectJson = bdfr::JsonCodec::encodeProject(persistedProject);
    bdfr::Project decodedProject;
    expect(bdfr::JsonCodec::decodeProject(projectJson, decodedProject, &error),
           "JSON project roundtrip decodes: " + error);
    expect(decodedProject.name == persistedProject.name, "JSON project name preserved");
    expect(decodedProject.actors.size() == 1, "JSON project actor preserved");
    expect(decodedProject.sessions.size() == 1, "JSON project session preserved");
    expect(decodedProject.sessions[0].takes.size() == 1, "JSON project take preserved");



    bdfr::LogLevel capturedLevel = bdfr::LogLevel::Trace;
    std::string capturedMessage;
    bdfr::Logger::setSink([&](bdfr::LogLevel level, const std::string& message) {
        capturedLevel = level;
        capturedMessage = message;
    });
    bdfr::Logger::write(bdfr::LogLevel::Info, "diagnostics-test");
    expect(capturedLevel == bdfr::LogLevel::Info && capturedMessage == "diagnostics-test",
           "logger routes messages through installed sink");

    bdfr::Stopwatch stopwatch;
    expect(stopwatch.elapsedMilliseconds() >= 0.0, "stopwatch reports non-negative duration");

    bool timerCalled = false;
    {
        bdfr::ScopedTimer timer("scope-test",
            [&](const std::string& label, double elapsedMs) {
                timerCalled = label == "scope-test" && elapsedMs >= 0.0;
            });
    }
    expect(timerCalled, "scoped timer reports duration on destruction");



    const auto persistencePath =
        std::filesystem::temp_directory_path() / "bdfr_core_persistence_test.bdfr.json";
    std::filesystem::remove(persistencePath);
    expect(bdfr::Persistence::saveProject(persistencePath, persistedProject, &error),
           "project saves atomically to disk: " + error);
    bdfr::Project diskProject;
    expect(bdfr::Persistence::loadProject(persistencePath, diskProject, &error),
           "project loads from disk: " + error);
    expect(diskProject.name == persistedProject.name, "disk project preserves name");
    expect(diskProject.actors.size() == 1 && diskProject.sessions.size() == 1,
           "disk project preserves hierarchy");
    expect(!std::filesystem::exists(persistencePath.string() + ".tmp"),
           "atomic save leaves no temporary file");
    std::filesystem::remove(persistencePath);



    bdfr::RetargetProfile retarget;
    retarget.name = "Test Rig";
    expect(retarget.addMapping({"jawOpen", "CTRL_JawOpen", 1.2F, 0.0F, 0.0F, 1.0F, 0.05F, false}),
           "retarget mapping added");
    expect(retarget.addMapping({"eyeBlinkLeft", "Blink_L", 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, false}),
           "second retarget mapping added");

    const auto retargeted = retarget.apply({
        {"jawOpen", 0.5F},
        {"eyeBlinkLeft", 0.8F}
    });
    expect(near(retargeted.at("CTRL_JawOpen"), 0.6F), "retarget scale is applied");
    expect(near(retargeted.at("Blink_L"), 0.8F), "retarget preserves direct mapping");

    const auto compatibility = retarget.scanCompatibility({"CTRL_JawOpen"});
    expect(compatibility.mappedTargets == 1, "compatibility counts mapped target");
    expect(compatibility.missingTargets == 1, "compatibility counts missing target");
    expect(near(compatibility.coverage, 0.5F), "compatibility coverage computed");



    expect(bdfr::AutoMapper::normalizeName("mouthSmile_Left") ==
           bdfr::AutoMapper::normalizeName("mouthSmileL"),
           "auto mapper normalizes left-side naming variants");

    const auto suggestions = bdfr::AutoMapper::suggest(
        {"jawOpen", "mouthSmileLeft", "eyeBlinkRight"},
        {"Jaw_Open", "mouthSmile_L", "EyeBlinkRight"},
        0.75F);
    expect(suggestions.size() == 3, "auto mapper finds normalized rig-name matches");



    const auto speechPlan = bdfr::speech::TextSpeechPlanner::plan(
        "Bob, we thought you were home.",
        bdfr::speech::TextSpeechOptions{180.0F, 0.10, 0.20});
    expect(!speechPlan.empty(), "text SpeechFace generates timed speech events");
    expect(speechPlan.front().startSeconds >= 0.0, "speech events have non-negative timing");

    bool foundMbp = false;
    bool foundTh = false;
    for (const auto& event : speechPlan) {
        foundMbp = foundMbp || event.viseme == bdfr::speech::Viseme::MBP;
        foundTh = foundTh || event.viseme == bdfr::speech::Viseme::TH;
    }
    expect(foundMbp, "text SpeechFace recognizes MBP viseme group");
    expect(foundTh, "text SpeechFace recognizes TH digraph");

    const auto mbpPose = bdfr::speech::VisemeSynthesizer::pose(bdfr::speech::Viseme::MBP);
    expect(mbpPose.find("mouthClose") != mbpPose.end() &&
           near(mbpPose.at("mouthClose"), 1.0F),
           "MBP viseme produces lip closure");

    const auto aaPose = bdfr::speech::VisemeSynthesizer::pose(bdfr::speech::Viseme::AA);
    expect(aaPose.find("jawOpen") != aaPose.end() && aaPose.at("jawOpen") > 0.7F,
           "AA viseme produces open jaw");

    const double speechSampleTime = speechPlan.front().startSeconds +
                                    speechPlan.front().durationSeconds * 0.5;
    const auto sampledSpeechPose = bdfr::speech::VisemeSynthesizer::sample(
        speechPlan, speechSampleTime);
    expect(!sampledSpeechPose.empty(), "SpeechFace timeline samples facial curves");



    bdfr::runtime::FrameQueue frameQueue(2);
    bdfr::FacialFrame q0; q0.timestampSeconds = 0.0;
    bdfr::FacialFrame q1; q1.timestampSeconds = 1.0;
    bdfr::FacialFrame q2; q2.timestampSeconds = 2.0;
    frameQueue.push(q0);
    frameQueue.push(q1);
    frameQueue.push(q2);
    expect(frameQueue.size() == 2, "bounded frame queue keeps capacity");
    expect(frameQueue.droppedCount() == 1, "bounded frame queue counts dropped oldest frame");
    bdfr::FacialFrame popped;
    expect(frameQueue.pop(popped) && std::fabs(popped.timestampSeconds - 1.0) < 0.0001,
           "frame queue drops oldest frame first");

    bdfr::runtime::JitterBuffer jitter(0.1);
    bdfr::FacialFrame j2; j2.timestampSeconds = 2.0;
    bdfr::FacialFrame j1; j1.timestampSeconds = 1.0;
    jitter.push(j2);
    jitter.push(j1);
    expect(jitter.popReady(1.05, popped) == false, "jitter buffer respects configured delay");
    expect(jitter.popReady(1.11, popped) &&
           std::fabs(popped.timestampSeconds - 1.0) < 0.0001,
           "jitter buffer sorts and releases ready frame");

    bdfr::runtime::ClockOffsetEstimator clock(0.5);
    clock.observe(10.0, 10.2);
    expect(clock.initialized(), "clock estimator initializes");
    expect(std::fabs(clock.offsetSeconds() - 0.2) < 0.0001,
           "clock estimator captures initial offset");
    clock.observe(11.0, 11.4);
    expect(std::fabs(clock.offsetSeconds() - 0.3) < 0.0001,
           "clock estimator smooths offset changes");
    expect(std::fabs(clock.remoteToLocal(20.0) - 20.3) < 0.0001,
           "clock estimator converts remote timestamp");



    bdfr::mocap::ExternalMocapProfile externalProfile;
    externalProfile.name = "MewFace-like Test";
    expect(externalProfile.addRule({"jaw_open", "jawOpen", 1.0F, 0.0F, 0.0F, 1.0F, false}),
           "external mocap rule added");
    expect(externalProfile.addRule({"blink_l", "eyeBlinkLeft", 1.0F, 0.0F, 0.0F, 1.0F, false}),
           "external mocap blink rule added");
    const auto normalizedMocap = externalProfile.normalize(
        {{"jaw_open", 0.7F}, {"blink_l", 0.4F}}, 2.5, 0.9F);
    expect(std::fabs(normalizedMocap.timestampSeconds - 2.5) < 0.0001,
           "external mocap preserves timestamp");
    expect(near(normalizedMocap.curves.at("jawOpen"), 0.7F),
           "external mocap maps jaw curve");
    expect(near(normalizedMocap.curves.at("eyeBlinkLeft"), 0.4F),
           "external mocap maps blink curve");



    bdfr::mocap::MocapPacket packet;
    packet.sourceId = "android-phone-01";
    packet.sequenceNumber = 42;
    packet.frame = normalizedMocap;
    const auto packetBytes = bdfr::runtime::FramePacketCodec::encode(packet);
    bdfr::mocap::MocapPacket decodedPacket;
    expect(bdfr::runtime::FramePacketCodec::decode(packetBytes, decodedPacket),
           "live frame packet roundtrip decodes");
    expect(decodedPacket.sourceId == packet.sourceId,
           "live frame packet preserves source id");
    expect(decodedPacket.sequenceNumber == 42,
           "live frame packet preserves sequence number");
    expect(near(decodedPacket.frame.curves.at("jawOpen"), 0.7F),
           "live frame packet preserves facial curve");

    auto badPacketBytes = packetBytes;
    badPacketBytes.pop_back();
    expect(!bdfr::runtime::FramePacketCodec::decode(badPacketBytes, decodedPacket),
           "truncated live packet is rejected");



    expect(encoded.size() >= 6 &&
           encoded[0] == static_cast<std::uint8_t>('B') &&
           encoded[1] == static_cast<std::uint8_t>('D') &&
           encoded[2] == static_cast<std::uint8_t>('F') &&
           encoded[3] == static_cast<std::uint8_t>('R') &&
           encoded[4] == 1 && encoded[5] == 0,
           "binary codec uses deterministic little-endian BDFR header");

    expect(packetBytes.size() >= 6 &&
           packetBytes[0] == static_cast<std::uint8_t>('B') &&
           packetBytes[1] == static_cast<std::uint8_t>('D') &&
           packetBytes[2] == static_cast<std::uint8_t>('F') &&
           packetBytes[3] == static_cast<std::uint8_t>('P') &&
           packetBytes[4] == 1 && packetBytes[5] == 0,
           "live packet uses deterministic little-endian BDFP header");



    expect(bdfr::Schema::frameCompatibility(1) == bdfr::SchemaCompatibility::Supported,
           "current frame schema is supported");
    expect(bdfr::Schema::frameCompatibility(2) == bdfr::SchemaCompatibility::TooNew,
           "future frame schema is rejected");
    expect(bdfr::Schema::projectCompatibility(0) == bdfr::SchemaCompatibility::TooOld,
           "old project schema is classified");

    bdfr::FacialFrame schemaFrame;
    schemaFrame.schemaVersion = 1;
    expect(bdfr::Schema::normalizeFrame(schemaFrame), "current frame schema normalizes");
    schemaFrame.schemaVersion = 2;
    expect(!bdfr::Schema::normalizeFrame(schemaFrame), "future frame schema does not normalize");

    const auto recoveryDir =
        std::filesystem::temp_directory_path() / "bdfr_recovery_test";
    std::filesystem::remove_all(recoveryDir);
    expect(bdfr::Recovery::saveRotatingSnapshot(
               recoveryDir, "project", persistedProject, 3, &error),
           "first recovery snapshot saves: " + error);

    bdfr::Project recoveryProject2 = persistedProject;
    recoveryProject2.name = "Persistent Project v2";
    expect(bdfr::Recovery::saveRotatingSnapshot(
               recoveryDir, "project", recoveryProject2, 3, &error),
           "second recovery snapshot rotates: " + error);

    const auto latestRecovery = bdfr::Recovery::latestSnapshot(recoveryDir, "project");
    expect(!latestRecovery.empty() && std::filesystem::exists(latestRecovery),
           "latest recovery snapshot is discoverable");
    expect(std::filesystem::exists(
               recoveryDir / "project.recovery.1.bdfr.json"),
           "older recovery snapshot is retained");

    bdfr::Project latestRecoveryProject;
    expect(bdfr::Persistence::loadProject(
               latestRecovery, latestRecoveryProject, &error),
           "latest recovery project loads");
    expect(latestRecoveryProject.name == "Persistent Project v2",
           "latest recovery snapshot contains newest project");
    std::filesystem::remove_all(recoveryDir);



    const auto happyPose = bdfr::expression::EmotionEngine::pose(
        bdfr::expression::Emotion::Happiness, 1.0F);
    expect(happyPose.find("AU12") != happyPose.end() &&
           happyPose.at("AU12") > 0.8F,
           "happiness preset drives smile AU");

    const auto mixedEmotion = bdfr::expression::EmotionEngine::blend(
        bdfr::expression::Emotion::Sadness, 0.5F,
        bdfr::expression::Emotion::Fear, 0.25F);
    expect(!mixedEmotion.empty(), "emotion presets can be blended");

    bdfr::expression::InstantEvent blinkEvent;
    blinkEvent.type = bdfr::expression::InstantEventType::Blink;
    blinkEvent.startSeconds = 1.0;
    blinkEvent.durationSeconds = 0.2;
    blinkEvent.intensity = 1.0F;
    const auto blinkMid = bdfr::expression::InstantEventGenerator::sample(blinkEvent, 1.1);
    expect(blinkMid.find("eyeBlinkLeft") != blinkMid.end() &&
           blinkMid.at("eyeBlinkLeft") > 0.9F,
           "instant blink peaks near event center");
    expect(bdfr::expression::InstantEventGenerator::sample(blinkEvent, 2.0).empty(),
           "instant event is inactive outside its time range");



    bdfr::FusionInput speechFusion;
    speechFusion.sourceId = "speech";
    speechFusion.curves = {{"jawOpen", 0.8F}, {"eyeBlinkLeft", 0.2F}};
    speechFusion.confidence = 1.0F;

    bdfr::FusionInput mocapFusion;
    mocapFusion.sourceId = "mocap";
    mocapFusion.curves = {{"jawOpen", 0.3F}, {"eyeBlinkLeft", 0.9F}};
    mocapFusion.confidence = 1.0F;

    bdfr::FusionSourceConfig speechConfig;
    speechConfig.sourceId = "speech";
    speechConfig.priority = 5;
    speechConfig.defaultWeight = 0.0F;
    speechConfig.regionWeights = {
        {bdfr::regionMask(bdfr::CurveRegion::Mouth), 1.0F},
        {bdfr::regionMask(bdfr::CurveRegion::Jaw), 1.0F}
    };

    bdfr::FusionSourceConfig mocapConfig;
    mocapConfig.sourceId = "mocap";
    mocapConfig.priority = 10;
    mocapConfig.defaultWeight = 1.0F;
    mocapConfig.regionWeights = {
        {bdfr::regionMask(bdfr::CurveRegion::Jaw), 0.0F}
    };

    const auto fusedPerformance = bdfr::PerformanceFusion::fuse(
        {}, {speechFusion, mocapFusion}, {speechConfig, mocapConfig});
    expect(near(fusedPerformance.at("jawOpen"), 0.8F),
           "fusion keeps speech-owned jaw region");
    expect(near(fusedPerformance.at("eyeBlinkLeft"), 0.9F),
           "fusion keeps mocap-owned eye region");



    bdfr::CalibrationProfile calibration;
    calibration.actorId = "actor_001";
    calibration.name = "Actor Calibration";
    expect(calibration.setRange("jawOpen", {0.2F, 0.1F, 0.9F, false}),
           "calibration range accepted");
    expect(!calibration.setRange("badRange", {0.5F, 0.8F, 0.2F, false}),
           "invalid calibration range rejected");
    expect(near(calibration.normalizeValue("jawOpen", 0.2F), 0.0F),
           "calibration neutral maps to zero");
    expect(near(calibration.normalizeValue("jawOpen", 0.9F), 1.0F),
           "calibration maximum maps to one");
    expect(calibration.normalizeValue("jawOpen", 0.55F) > 0.49F &&
           calibration.normalizeValue("jawOpen", 0.55F) < 0.51F,
           "calibration maps intermediate value proportionally");



    bdfr::expression::BehaviorProfile behaviorProfile;
    behaviorProfile.blinkIntervalSeconds = 2.0;
    behaviorProfile.blinkDurationSeconds = 0.2;
    behaviorProfile.eyeDartStrength = 0.1F;
    behaviorProfile.headYawDegrees = 2.0F;

    const auto behaviorBlink = bdfr::expression::BehaviorEngine::sample(
        behaviorProfile, 0.1, 0);
    expect(behaviorBlink.curves.find("eyeBlinkLeft") != behaviorBlink.curves.end() &&
           behaviorBlink.curves.at("eyeBlinkLeft") > 0.9F,
           "procedural behavior creates blink envelope");

    const auto behaviorMotion = bdfr::expression::BehaviorEngine::sample(
        behaviorProfile, 1.0, 123);
    expect(std::fabs(behaviorMotion.gaze.x) <= 0.1001F,
           "procedural gaze remains inside profile strength");
    expect(std::fabs(behaviorMotion.head.yaw) <= 2.001F,
           "procedural head yaw remains inside profile range");

    const auto behaviorRepeat = bdfr::expression::BehaviorEngine::sample(
        behaviorProfile, 1.0, 123);
    expect(std::fabs(behaviorMotion.head.yaw - behaviorRepeat.head.yaw) < 0.000001,
           "procedural behavior is deterministic for seed/time");



    std::uint8_t fakePixels[16] = {};
    bdfr::capture::ImageView imageView;
    imageView.data = fakePixels;
    imageView.sizeBytes = sizeof(fakePixels);
    imageView.width = 2;
    imageView.height = 2;
    imageView.strideBytes = 8;
    imageView.format = bdfr::capture::PixelFormat::RGBA32;
    imageView.timestampSeconds = 0.5;
    expect(imageView.valid(), "capture image view validates");

    bdfr::capture::FaceObservation observation;
    observation.timestampSeconds = 0.5;
    observation.confidence = 0.9F;
    observation.landmarks = {
        {0.1F, 0.2F, 0.0F, 0.9F},
        {0.8F, 0.2F, 0.0F, 0.8F}
    };
    observation.regions.mouth = 0.9F;
    observation.regions.leftEye = 0.8F;
    observation.regions.rightEye = 0.8F;
    observation.regions.brows = 0.75F;
    observation.regions.jaw = 0.85F;
    expect(observation.valid(), "face observation validates");

    const auto quality = bdfr::capture::CaptureQuality::evaluate(observation, 0.45F);
    expect(quality.overall > 0.7F && !quality.badTake,
           "good face observation passes capture quality");

    observation.occluded = true;
    const auto occludedQuality = bdfr::capture::CaptureQuality::evaluate(observation, 0.75F);
    expect(occludedQuality.occlusionPenaltyApplied,
           "capture quality applies occlusion penalty");
    expect(occludedQuality.badTake,
           "occluded low-quality observation can be flagged as bad take");



    const auto reservedPersonality =
        bdfr::expression::PersonalityLibrary::preset(
            bdfr::expression::PersonalityPreset::Reserved);
    const auto nervousPersonality =
        bdfr::expression::PersonalityLibrary::preset(
            bdfr::expression::PersonalityPreset::Nervous);
    expect(reservedPersonality.articulationScale < nervousPersonality.articulationScale,
           "personality presets vary articulation");
    expect(reservedPersonality.behavior.eyeDartStrength <
           nervousPersonality.behavior.eyeDartStrength,
           "personality presets vary gaze behavior");

    const auto reservedBehavior = bdfr::expression::BehaviorEngine::sample(
        reservedPersonality.behavior, 1.0, reservedPersonality.seed);
    const auto nervousBehavior = bdfr::expression::BehaviorEngine::sample(
        nervousPersonality.behavior, 1.0, nervousPersonality.seed);
    expect(std::fabs(reservedBehavior.gaze.x - nervousBehavior.gaze.x) > 0.0001F ||
           std::fabs(reservedBehavior.head.yaw - nervousBehavior.head.yaw) > 0.0001F,
           "same time produces distinct personality performance");



    bdfr::speech::DialogueScript dialogueScript;
    expect(bdfr::speech::DialogueMarkup::parse(
               "[emotion=angry intensity=0.7] Where have you been? [pause=0.4] [blink] [gaze=player]",
               dialogueScript, &error),
           "dialogue performance markup parses: " + error);
    expect(dialogueScript.items.size() == 5,
           "dialogue markup preserves text and four directives");
    expect(dialogueScript.items[0].type == bdfr::speech::DialogueItemType::Emotion &&
           dialogueScript.items[0].argument == "angry" &&
           near(dialogueScript.items[0].value, 0.7F),
           "dialogue markup parses emotion and intensity");
    expect(dialogueScript.items[1].type == bdfr::speech::DialogueItemType::Text &&
           dialogueScript.items[1].text == "Where have you been?",
           "dialogue markup keeps dialogue text");
    expect(dialogueScript.items[2].type == bdfr::speech::DialogueItemType::Pause &&
           near(dialogueScript.items[2].value, 0.4F),
           "dialogue markup parses pause");
    expect(dialogueScript.items[3].type == bdfr::speech::DialogueItemType::Blink,
           "dialogue markup parses blink event");
    expect(dialogueScript.items[4].type == bdfr::speech::DialogueItemType::Gaze &&
           dialogueScript.items[4].argument == "player",
           "dialogue markup parses gaze target");

    bdfr::speech::DialogueScript invalidDialogue;
    expect(!bdfr::speech::DialogueMarkup::parse("[unknown=value]", invalidDialogue, &error),
           "unknown dialogue markup is rejected");



    bdfr::speech::DialogueCompileResult compiledDialogue;
    expect(bdfr::speech::DialogueCompiler::compile(
               dialogueScript, compiledDialogue, {}, &error),
           "dialogue script compiles to multi-track timeline: " + error);
    expect(compiledDialogue.timeline.tracks().size() == 5,
           "dialogue compiler creates five editing tracks");
    expect(compiledDialogue.durationSeconds > 0.4,
           "dialogue compiler accounts for speech and pause duration");

    const auto& compiledTracks = compiledDialogue.timeline.tracks();
    expect(!compiledTracks[0].clips.empty(),
           "compiled dialogue contains text clip");
    expect(!compiledTracks[1].clips.empty(),
           "compiled dialogue contains generated speech/viseme clips");
    expect(!compiledTracks[2].clips.empty(),
           "compiled dialogue contains emotion clip");
    expect(!compiledTracks[3].clips.empty(),
           "compiled dialogue contains instant event clip");
    expect(!compiledTracks[4].clips.empty() &&
           compiledTracks[4].clips.front().payload == "player",
           "compiled dialogue contains gaze target event");

    const auto compiledFace = compiledDialogue.timeline.evaluate(
        compiledTracks[1].clips.front().startSeconds +
        compiledTracks[1].clips.front().durationSeconds * 0.5);
    expect(!compiledFace.empty(),
           "compiled text timeline evaluates to facial curves");



    bdfr::runtime::UdpFrameReceiver udpReceiver;
    expect(udpReceiver.open(0, "127.0.0.1", &error),
           "UDP receiver opens on loopback: " + error);
    expect(udpReceiver.localPort() != 0,
           "UDP receiver obtains an ephemeral local port");

    bdfr::runtime::UdpFrameSender udpSender;
    expect(udpSender.open(&error), "UDP sender opens: " + error);
    expect(udpSender.sendTo("127.0.0.1", udpReceiver.localPort(), packet, &error),
           "UDP sender transmits BDFP packet: " + error);

    bdfr::mocap::MocapPacket udpPacket;
    expect(udpReceiver.receive(udpPacket, 1000, &error),
           "UDP receiver decodes loopback BDFP packet: " + error);
    expect(udpPacket.sequenceNumber == packet.sequenceNumber &&
           udpPacket.sourceId == packet.sourceId,
           "UDP loopback preserves packet metadata");
    expect(near(udpPacket.frame.curves.at("jawOpen"), 0.7F),
           "UDP loopback preserves facial curves");



    expect(bdfr::Arkit52::curveNames().size() == 52,
           "ARKit interoperability exposes all 52 standard curves");
    for (const auto& curveName : bdfr::Arkit52::curveNames()) {
        expect(registry.isRegistered(curveName),
               "ARKit curve is registered in canonical BDFR registry: " + curveName);
    }

    const auto arkitIdentity = bdfr::Arkit52::identityProfile();
    expect(arkitIdentity.mappings().size() == 52,
           "ARKit identity retarget profile contains 52 mappings");
    const auto arkitMapped = arkitIdentity.apply({
        {"jawOpen", 0.65F},
        {"eyeBlinkLeft", 0.4F},
        {"tongueOut", 0.2F}
    });
    expect(near(arkitMapped.at("jawOpen"), 0.65F) &&
           near(arkitMapped.at("eyeBlinkLeft"), 0.4F) &&
           near(arkitMapped.at("tongueOut"), 0.2F),
           "ARKit identity retarget preserves normalized curves");



    bdfr::CurveTuningProfile tuning;
    tuning.name = "Actor/Rig Tuning";
    expect(tuning.setRule("jawOpen", {
               0.10F, 0.90F,
               1.10F, 0.0F,
               1.0F, 0.10F,
               0.0F, 1.0F}),
           "curve tuning accepts valid shaping rule");
    expect(!tuning.setRule("bad", {
               1.0F, 0.0F,
               1.0F, 0.0F,
               1.0F, 0.0F,
               0.0F, 1.0F}),
           "curve tuning rejects inverted input range");

    const float tunedNeutral = tuning.applyValue("jawOpen", 0.10F);
    expect(near(tunedNeutral, 0.0F),
           "curve tuning maps input minimum to zero");
    const float tunedMid = tuning.applyValue("jawOpen", 0.50F);
    expect(tunedMid > 0.40F && tunedMid < 0.60F,
           "curve tuning shapes intermediate values");
    const auto tunedMap = tuning.apply({{"jawOpen", 0.90F}, {"AU12", 0.25F}});
    expect(near(tunedMap.at("jawOpen"), 1.0F),
           "curve tuning applies gain and output clamp");
    expect(near(tunedMap.at("AU12"), 0.25F),
           "untuned curves pass through normalized");



    auto appendLe16 = [](std::vector<std::uint8_t>& bytes, std::uint16_t value) {
        bytes.push_back(static_cast<std::uint8_t>(value & 0xFFu));
        bytes.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFFu));
    };
    auto appendLe32 = [](std::vector<std::uint8_t>& bytes, std::uint32_t value) {
        for (int i = 0; i < 4; ++i)
            bytes.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFu));
    };

    std::vector<std::uint8_t> wav;
    wav.insert(wav.end(), {'R','I','F','F'});
    appendLe32(wav, 36 + 8);
    wav.insert(wav.end(), {'W','A','V','E'});
    wav.insert(wav.end(), {'f','m','t',' '});
    appendLe32(wav, 16);
    appendLe16(wav, 1);
    appendLe16(wav, 1);
    appendLe32(wav, 8000);
    appendLe32(wav, 16000);
    appendLe16(wav, 2);
    appendLe16(wav, 16);
    wav.insert(wav.end(), {'d','a','t','a'});
    appendLe32(wav, 8);
    appendLe16(wav, static_cast<std::uint16_t>(0));
    appendLe16(wav, static_cast<std::uint16_t>(16384));
    appendLe16(wav, static_cast<std::uint16_t>(0xC000));
    appendLe16(wav, static_cast<std::uint16_t>(0));

    bdfr::audio::AudioBuffer audioBuffer;
    expect(bdfr::audio::WavAudio::decodePcm16(wav, audioBuffer, &error),
           "PCM16 WAV decodes: " + error);
    expect(audioBuffer.valid() && audioBuffer.sampleRate == 8000 &&
           audioBuffer.channels == 1 && audioBuffer.frameCount() == 4,
           "decoded WAV metadata is correct");
    expect(audioBuffer.durationSeconds() > 0.00049 &&
           audioBuffer.durationSeconds() < 0.00051,
           "decoded WAV duration is correct");
    expect(audioBuffer.sample(1, 0) > 0.49F &&
           audioBuffer.sample(2, 0) < -0.49F,
           "decoded WAV samples are normalized");

    const auto audioFeatures = bdfr::audio::AudioFeatures::analyze(
        audioBuffer, 0.0005, 0.00025);
    expect(!audioFeatures.empty(),
           "audio frontend extracts windowed features");
    expect(audioFeatures.front().peak > 0.49F,
           "audio feature peak captures signal amplitude");
    expect(audioFeatures.front().rms > 0.2F,
           "audio feature RMS captures signal energy");



    std::vector<bdfr::audio::AudioFeatureFrame> syntheticFeatures = {
        {0.00, 0.02, 0.01F, 0.02F, 0.10F},
        {0.02, 0.02, 0.10F, 0.20F, 0.20F},
        {0.04, 0.02, 0.30F, 0.50F, 0.25F},
        {0.06, 0.02, 0.12F, 0.22F, 0.18F}
    };
    const auto prosodyFrames = bdfr::audio::ProsodyAnalyzer::analyze(
        syntheticFeatures, 0.20F, 1.5F);
    expect(prosodyFrames.size() == syntheticFeatures.size(),
           "prosody analyzer preserves feature frame count");
    expect(!prosodyFrames.front().speechActive,
           "prosody analyzer marks quiet window inactive");
    expect(prosodyFrames[2].speechActive &&
           prosodyFrames[2].emphasisCandidate,
           "prosody analyzer finds high-energy emphasis candidate");

    const auto prosodySummary =
        bdfr::audio::ProsodyAnalyzer::summarize(prosodyFrames);
    expect(prosodySummary.peakEnergy > 0.29F,
           "prosody summary records peak energy");
    expect(prosodySummary.activeSpeechSeconds > 0.0 &&
           prosodySummary.silenceSeconds > 0.0,
           "prosody summary tracks active speech and silence");



    std::vector<bdfr::audio::ProsodyFrame> cueProsody = {
        {0.00, 0.10, 0.2F, 0.5F, true, false},
        {0.10, 0.10, 0.8F, 1.0F, true, true},
        {0.20, 0.10, 0.1F, 0.1F, false, false},
        {0.30, 0.10, 0.05F, 0.05F, false, false},
        {0.40, 0.10, 0.05F, 0.05F, false, false},
        {0.50, 0.10, 0.2F, 0.5F, true, false}
    };
    const auto behaviorCues =
        bdfr::audio::AudioBehaviorPlanner::plan(
            cueProsody, 0.16, 0.25);
    bool foundCueBlink = false;
    bool foundCueBreath = false;
    bool foundCueNod = false;
    for (const auto& cue : behaviorCues) {
        foundCueBlink = foundCueBlink ||
            cue.type == bdfr::audio::AudioBehaviorCueType::Blink;
        foundCueBreath = foundCueBreath ||
            cue.type == bdfr::audio::AudioBehaviorCueType::Breath;
        foundCueNod = foundCueNod ||
            cue.type == bdfr::audio::AudioBehaviorCueType::HeadNod;
    }
    expect(foundCueBlink, "audio behavior planner creates blink from pause");
    expect(foundCueBreath, "audio behavior planner creates breath from long pause");
    expect(foundCueNod, "audio behavior planner creates head nod from emphasis");



    std::vector<bdfr::mocap::MocapPacket> sessionPackets = {packet, packet};
    sessionPackets[1].sequenceNumber = 43;
    sessionPackets[1].frame.timestampSeconds = 2.6;
    sessionPackets[1].frame.curves["jawOpen"] = 0.5F;

    const auto sessionBytes =
        bdfr::runtime::SessionStream::encode(sessionPackets);
    expect(sessionBytes.size() > packetBytes.size(),
           "BDFS session stream stores multiple BDFP packets");

    std::vector<bdfr::mocap::MocapPacket> decodedSessionPackets;
    expect(bdfr::runtime::SessionStream::decode(
               sessionBytes, decodedSessionPackets, &error),
           "BDFS recorded session roundtrip decodes: " + error);
    expect(decodedSessionPackets.size() == 2,
           "BDFS session preserves packet count");
    expect(decodedSessionPackets[1].sequenceNumber == 43 &&
           near(decodedSessionPackets[1].frame.curves.at("jawOpen"), 0.5F),
           "BDFS session preserves frame metadata and curves");



    bdfr::runtime::LiveSessionReceiver liveReceiver(0.0);
    expect(liveReceiver.open(0, "127.0.0.1", &error),
           "live session receiver opens on loopback: " + error);

    bdfr::mocap::MocapPacket livePacket1 = packet;
    livePacket1.sequenceNumber = 1;
    livePacket1.frame.timestampSeconds = 10.0;

    bdfr::mocap::MocapPacket livePacket3 = packet;
    livePacket3.sequenceNumber = 3;
    livePacket3.frame.timestampSeconds = 10.1;
    livePacket3.frame.curves["jawOpen"] = 0.45F;

    expect(udpSender.sendTo(
               "127.0.0.1",
               liveReceiver.localPort(),
               livePacket1,
               &error),
           "live receiver test sends first packet: " + error);

    expect(liveReceiver.poll(1000, 10.2, &error),
           "live receiver polls first packet: " + error);

    expect(udpSender.sendTo(
               "127.0.0.1",
               liveReceiver.localPort(),
               livePacket3,
               &error),
           "live receiver test sends packet with sequence gap: " + error);

    expect(liveReceiver.poll(1000, 10.3, &error),
           "live receiver polls sequence-gap packet: " + error);

    const auto liveStats = liveReceiver.stats();
    expect(liveStats.packetsReceived == 2,
           "live receiver counts received packets");
    expect(liveStats.lastSourceId == packet.sourceId,
           "live receiver exposes the latest source id");
    expect(liveStats.packetsLost == 1,
           "live receiver detects packet sequence loss");
    expect(liveStats.clockOffsetSeconds > 0.0,
           "live receiver estimates remote-to-local clock offset");

    bdfr::FacialFrame liveReady;
    expect(liveReceiver.popReady(11.0, liveReady),
           "live receiver releases playback-ready frame");
    expect(liveReady.timestampSeconds > 10.0,
           "live receiver converts remote timestamp to local time");



    bdfr::audio::AudioBuffer sineAudio;
    sineAudio.sampleRate = 8000;
    sineAudio.channels = 1;
    const double sineFrequency = 200.0;
    const std::size_t sineFrames = 800;
    sineAudio.samples.resize(sineFrames);
    for (std::size_t i = 0; i < sineFrames; ++i) {
        sineAudio.samples[i] = static_cast<float>(
            0.7 * std::sin(
                2.0 * 3.14159265358979323846 *
                sineFrequency *
                static_cast<double>(i) /
                static_cast<double>(sineAudio.sampleRate)));
    }

    const auto pitchFrames =
        bdfr::audio::PitchTracker::analyze(
            sineAudio,
            0.040,
            0.010,
            80.0F,
            350.0F,
            0.5F);

    expect(!pitchFrames.empty(),
           "pitch tracker returns analysis frames");

    bool foundVoicedPitch = false;
    for (const auto& pitchFrame : pitchFrames) {
        if (pitchFrame.voiced &&
            std::fabs(pitchFrame.frequencyHz - 200.0F) < 8.0F) {
            foundVoicedPitch = true;
            break;
        }
    }
    expect(foundVoicedPitch,
           "pitch tracker estimates synthetic 200 Hz tone");



    const auto fittedTranscript =
        bdfr::speech::TranscriptTiming::fitTextToDuration(
            "Hello world",
            2.0);

    expect(!fittedTranscript.events.empty(),
           "text-audio timing produces speech events");
    expect(std::fabs(
               fittedTranscript.targetDurationSeconds - 2.0) <
           0.000001,
           "text-audio timing preserves requested duration");
    if (!fittedTranscript.events.empty()) {
        const auto& lastEvent =
            fittedTranscript.events.back();
        const double fittedDuration =
            lastEvent.startSeconds +
            lastEvent.durationSeconds;
        expect(std::fabs(fittedDuration - 2.0) < 0.001,
               "text-audio timing scales event timeline to audio duration");
    }



    const auto englishProfile =
        bdfr::speech::LanguageProfile::englishBootstrap();
    expect(englishProfile.mapPhoneme("M") ==
               bdfr::speech::Viseme::MBP,
           "English language profile maps bilabial phoneme");
    expect(englishProfile.mapPhoneme("TH") ==
               bdfr::speech::Viseme::TH,
           "English language profile maps TH phoneme");
    expect(englishProfile.mapPhoneme(
               "unknown",
               bdfr::speech::Viseme::Rest) ==
               bdfr::speech::Viseme::Rest,
           "Language profile preserves fallback for unknown phoneme");



    const auto persianProfile =
        bdfr::speech::LanguageProfile::persianBootstrap();
    expect(persianProfile.languageCode == "fa",
           "Persian language profile exposes fa language code");
    expect(persianProfile.mapPhoneme("kh") ==
               bdfr::speech::Viseme::KNG,
           "Persian profile maps kh articulation group");
    expect(persianProfile.mapPhoneme("sh") ==
               bdfr::speech::Viseme::CHSH,
           "Persian profile maps sh articulation group");



    bdfr::NeutralDriftOptions driftOptions;
    driftOptions.baselineAlpha = 0.5F;
    driftOptions.neutralThreshold = 0.2F;
    driftOptions.maximumCorrection = 0.15F;

    bdfr::NeutralDriftCorrector driftCorrector(driftOptions);

    for (int i = 0; i < 8; ++i) {
        driftCorrector.process(
            {{"jawOpen", 0.10F}},
            1.0F);
    }

    const auto correctedNeutral =
        driftCorrector.process(
            {{"jawOpen", 0.10F}},
            1.0F);

    expect(correctedNeutral.at("jawOpen") < 0.02F,
           "neutral drift corrector removes persistent neutral offset");

    const auto correctedExpression =
        driftCorrector.process(
            {{"jawOpen", 0.80F}},
            1.0F);

    expect(correctedExpression.at("jawOpen") > 0.65F,
           "neutral drift corrector preserves strong active expression");

    expect(driftCorrector.baseline().at("jawOpen") <= 0.15F,
           "neutral drift baseline respects correction limit");



    bdfr::FacialSequence csvSequence;
    const std::string csvMocap =
        "time,jawOpen,eyeBlinkLeft\n"
        "0.000,0.10,0.00\n"
        "0.033,0.55,0.20\n"
        "0.066,1.20,0.90\n";

    expect(bdfr::mocap::MocapCsv::decode(
               csvMocap,
               csvSequence,
               {},
               &error),
           "mocap CSV decodes to facial sequence: " + error);

    expect(csvSequence.frames().size() == 3,
           "mocap CSV preserves frame count");

    expect(near(
               csvSequence.frames().back().curves.at("jawOpen"),
               1.0F),
           "mocap CSV clamps curves to normalized range");

    const std::string exportedCsv =
        bdfr::mocap::MocapCsv::encode(csvSequence);

    bdfr::FacialSequence csvRoundtrip;
    expect(bdfr::mocap::MocapCsv::decode(
               exportedCsv,
               csvRoundtrip,
               {},
               &error),
           "exported mocap CSV roundtrips: " + error);

    expect(csvRoundtrip.frames().size() ==
               csvSequence.frames().size(),
           "mocap CSV roundtrip preserves frame count");



    const bdfr::FrameRate fps25 = bdfr::FrameRate::Fps25();
    expect(fps25.valid() &&
           std::fabs(fps25.framesPerSecond() - 25.0) < 0.000001,
           "25 fps frame rate is valid");

    expect(bdfr::TimecodeConverter::secondsToFrame(
               2.0,
               fps25) == 50,
           "timecode converts seconds to frame index");

    expect(std::fabs(
               bdfr::TimecodeConverter::frameToSeconds(
                   50,
                   fps25) -
               2.0) < 0.000001,
           "timecode converts frame index to seconds");

    const auto tc25 =
        bdfr::TimecodeConverter::frameToTimecode(
            25 * (3600 + 2 * 60 + 3) + 12,
            fps25);

    expect(tc25.hours == 1 &&
           tc25.minutes == 2 &&
           tc25.seconds == 3 &&
           tc25.frames == 12,
           "timecode splits nominal frame number correctly");

    std::int64_t roundtripFrame = 0;
    expect(bdfr::TimecodeConverter::timecodeToFrame(
               tc25,
               fps25,
               roundtripFrame),
           "timecode parses valid nominal timecode");

    expect(roundtripFrame ==
               25 * (3600 + 2 * 60 + 3) + 12,
           "timecode roundtrip preserves frame index");

    expect(bdfr::FrameRate::Ntsc2997().framesPerSecond() > 29.96 &&
           bdfr::FrameRate::Ntsc2997().framesPerSecond() < 29.98,
           "29.97 rational frame rate is represented exactly");



    bdfr::models::ModelRegistry modelRegistry;

    bdfr::models::ModelManifest modelV1;
    modelV1.id = "face-landmarker";
    modelV1.version = "1.0.0";
    modelV1.fileName = "face_landmarker.task";
    modelV1.sha256 = "example-sha256";
    modelV1.license = "external-license";
    modelV1.source = "external";
    modelV1.backend =
        bdfr::models::RuntimeBackend::MediaPipe;
    modelV1.inputContract = "RGB image";
    modelV1.outputContract = "52 curves + landmarks";

    expect(modelRegistry.registerModel(modelV1),
           "model registry accepts complete model manifest");

    expect(!modelRegistry.registerModel(modelV1),
           "model registry rejects duplicate id/version");

    bdfr::models::ModelManifest modelV2 = modelV1;
    modelV2.version = "2.0.0";
    modelV2.fileName = "face_landmarker_v2.task";

    expect(modelRegistry.registerModel(modelV2),
           "model registry stores multiple versions");

    const auto* latestModel =
        modelRegistry.find("face-landmarker");

    expect(latestModel != nullptr &&
           latestModel->version == "2.0.0",
           "model registry resolves latest registered version");

    expect(modelRegistry.all().size() == 2,
           "model registry enumerates registered versions");


    bdfr::FacialSequence playbackSequence;
    bdfr::FacialFrame playbackA;
    playbackA.timestampSeconds = 0.0;
    playbackA.curves["jawOpen"] = 0.0F;
    bdfr::FacialFrame playbackB;
    playbackB.timestampSeconds = 1.0;
    playbackB.curves["jawOpen"] = 1.0F;
    expect(playbackSequence.addFrame(playbackA) &&
           playbackSequence.addFrame(playbackB),
           "sequence playback fixture accepts ordered frames");

    bdfr::SequencePlayback playback;
    playback.load(&playbackSequence);
    playback.play();
    const auto playbackMid = playback.update(0.5);
    expect(playback.state() == bdfr::PlaybackState::Playing &&
           playback.positionSeconds() > 0.49 &&
           playback.positionSeconds() < 0.51,
           "sequence playback advances transport while playing");
    expect(playbackMid.curves.at("jawOpen") > 0.49F &&
           playbackMid.curves.at("jawOpen") < 0.51F,
           "sequence playback samples interpolated facial values");

    playback.pause();
    const double pausedPosition = playback.positionSeconds();
    playback.update(0.25);
    expect(playback.positionSeconds() == pausedPosition,
           "paused sequence playback holds transport position");

    playback.setLooping(true);
    playback.play();
    playback.seek(0.9);
    playback.update(0.3);
    expect(playback.positionSeconds() < 0.3,
           "sequence playback loops across duration");

    if (failures == 0) {
        std::cout << "All BDFR core tests passed.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
