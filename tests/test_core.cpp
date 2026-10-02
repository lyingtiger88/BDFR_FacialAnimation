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
#include "bdfr/speech/TextSpeech.h"
#include "bdfr/core/Session.h"
#include "bdfr/core/Sequence.h"
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

    if (failures == 0) {
        std::cout << "All BDFR core tests passed.\n";
        return EXIT_SUCCESS;
    }

    std::cerr << failures << " test(s) failed.\n";
    return EXIT_FAILURE;
}
