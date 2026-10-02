# BDFR Facial Animation — Roadmap

This roadmap tracks implementation status, not just planned scope.

## Status legend

- [x] Completed and present in the repository
- [~] In progress / partially implemented
- [ ] Not started

## Current snapshot

- [x] Repository bootstrap
- [x] Architecture documentation
- [x] CMake-based C++17 core build
- [x] Versioned facial-frame data model
- [x] Canonical facial/FACS curve registry
- [x] Validation and sanitization
- [x] Curve mixer
- [x] Temporal exponential smoothing
- [x] Binary facial-frame codec
- [x] Initial multi-track timeline data model
- [x] Timeline curve evaluator with Mute/Solo/Priority
- [x] Region-aware curve fusion
- [x] Session/Take model
- [x] Partial re-solve dirty-range model
- [x] Initial automated core tests
- [x] GitHub Actions CI for Windows and Ubuntu
- [~] M0 Foundation — core foundation largely implemented
- [~] M1 FACS Core — expression/dynamics/tooling foundations implemented
- [~] M2 UE5 Runtime — plugin skeleton + Blueprint curve component
- [~] M3 SpeechFace — text/viseme/markup prototype
- [~] M4 Expressive Speech — emotion/events/behavior/personality foundations
- [~] M5 FaceCapture — capture contracts + Android camera/import skeleton
- [~] M6 Solver & Calibration — calibration core + confidence contracts
- [~] M7 Retargeting — generic retarget/auto-map/mocap fusion foundations
- [~] M8 Real-Time Runtime — queue/jitter/clock/packet foundations
- [~] M9 Quality Program
- [ ] M10 AI Character Integration

---

## Product direction

BDFR Facial Animation is designed as a modular facial-performance platform.

Planned first-class inputs:

- [x] Text
- [ ] Prerecorded audio
- [ ] Live microphone
- [ ] Prerecorded video
- [ ] Live camera
- [ ] Live facial mocap
- [ ] Imported facial mocap files
- [~] Android capture / live transfer
- [~] External curves / API streams

All input paths must converge on the shared BDFR facial representation.

---

# Milestone M0 — Foundation

**Status: [~] In progress**

## Goals

- [x] Freeze initial top-level architecture
- [x] Define initial naming conventions
- [x] Define common facial-frame schema
- [x] Define timestamps
- [x] Define frame confidence
- [x] Define normalized curve range
- [x] Establish initial dependency policy
- [x] Establish initial coding rules
- [x] Establish test/build conventions
- [x] Define logging standard
- [x] Define profiling standard
- [ ] Define benchmark asset conventions
- [x] Define initial session/take metadata conventions
- [~] Define project-file/version compatibility policy

## Deliverables

- [x] BDFR facial-frame data model
- [x] Binary serialization prototype
- [x] Initial module interfaces
- [x] Test harness skeleton
- [x] CMake build
- [x] GitHub CI
- [x] Development documentation
- [x] Testing documentation
- [x] JSON serialization/interchange
- [~] Programmatic sample facial sequences / test vectors
- [x] Session/Take data model
- [x] Project persistence format
- [x] Undo/Redo snapshot-history foundation
- [x] Crash recovery/autosave snapshot foundation

## Exit criteria

- [x] A facial frame can be serialized, loaded and validated
- [x] Unit tests cover basic curve range validation
- [x] Unit tests cover binary roundtrip
- [x] Unit tests cover schema compatibility classification
- [x] A complete facial sequence can be stored, sampled and replayed deterministically
- [x] Session/Take metadata survives save/load

---

# Milestone M1 — FACS Core

**Status: [~] In progress**

## Goals

- [x] Initial Action Unit representation
- [x] Normalized facial curve storage
- [x] Basic temporal smoothing
- [x] Basic blending/layering
- [x] Left/right curve support in registry
- [~] Initial FACS/curve metadata catalog
- [x] Region masks
- [x] Layer priority
- [x] Additive/override layers with region masks
- [x] Curve constraints
- [~] Velocity constraints
- [~] Conditional corrective-rule foundation
- [~] Per-curve dynamics foundation
- [x] Region freeze
- [x] Partial re-solve primitives

## Deliverables

- [x] Initial FACS/practical curve registry
- [x] Curve mixer
- [x] Exponential smoothing
- [x] Binary facial-frame codec
- [x] Initial timeline types
- [x] Expression layer stack
- [x] Region mask system
- [x] Priority resolver
- [x] Corrective-rule engine
- [~] Sequence linear interpolation + smoothing
- [x] JSON interchange
- [~] Deterministic little-endian codec/header tests
- [x] Curve key reduction
- [ ] Curve compression

## Timeline foundation

- [x] Audio track type
- [x] Text/Dialogue track type
- [x] Generated Speech track type
- [x] Emotion track type
- [x] Instant Event track type
- [x] Mocap track type
- [x] Gaze/Head track type
- [x] Manual Override track type
- [x] Clip start/duration/weight
- [x] Track lock primitive
- [x] Timeline evaluator
- [x] Mute behavior
- [x] Solo behavior
- [x] Track priority
- [x] Track masks
- [x] Basic clip blending
- [~] Non-destructive layer/history foundations
- [ ] Curve editor
- [ ] Snapping
- [ ] Bezier/tangent editing
- [x] Key reduction
- [ ] Quality/confidence heatmap
- [x] Partial range re-solve primitives

## Exit criteria

- [x] Multiple curve layers can be mixed without invalid normalized values
- [~] Expression stack and timed Timeline curve evaluation
- [x] Region-specific source fusion primitives work
- [x] Deterministic sequence sampling/playback verified
- [x] Conditional corrective rules resolve combined-expression cases

---

# Milestone M2 — Unreal Engine 5 Runtime

**Status: [~] In progress**

## Goals

- [x] Create UE5 plugin skeleton
- [ ] Consume BDFR facial frames
- [ ] Map curves to morph targets
- [ ] Map curves to facial bones
- [ ] Support prerecorded playback
- [ ] Support live playback
- [~] Blueprint curve API
- [~] C++ component API
- [ ] Live Link adapter
- [ ] Debug visualization
- [ ] Runtime profiling

## Deliverables

- [x] UE5 plugin skeleton
- [x] BDFR Facial Component
- [ ] Curve receiver
- [ ] Retarget profile asset
- [ ] Test character profile
- [ ] Debug UI
- [ ] Live Link source
- [ ] Runtime stats panel

## Exit criteria

- [ ] Prerecorded BDFR curves animate a UE5 test face
- [ ] Live BDFR frames animate a UE5 test face
- [ ] Runtime profiling is available
- [ ] Game-thread cost is measurable

---

# Milestone M3 — SpeechFace Prototype

**Status: [~] In progress**

## Input modes

- [ ] Audio file
- [ ] Live microphone
- [x] Text-only fallback prototype
- [ ] Text + TTS timing
- [x] Text + performance markup parser

## Goals

- [ ] Speech segmentation
- [ ] Phoneme timing
- [x] Viseme generation prototype
- [~] Jaw/lip viseme pose coordination
- [~] Neighbor-blend coarticulation prototype
- [ ] Anticipatory articulation
- [~] MBP lip-closure prototype
- [~] FV viseme prototype
- [ ] Tongue approximation
- [ ] Offline WAV processing
- [ ] Multilingual phoneme profiles

## Deliverables

- [ ] Audio frontend
- [x] Text frontend
- [~] Timed orthographic/viseme event timeline
- [x] Viseme curve synthesizer prototype
- [~] Neighbor blending prototype
- [ ] TTS timing adapter
- [ ] Curve export
- [ ] Speaker profile
- [ ] Language profile

## Exit criteria

- [ ] Clean speech produces synchronized editable mouth animation
- [x] Text-only input can generate a timed facial-performance draft
- [ ] Text+TTS path can produce tighter lip timing
- [ ] Transitions are context-sensitive rather than pose switching

---

# Milestone M4 — Expressive Speech & Performance

**Status: [~] In progress**

## Goals

- [ ] Extract prosody
- [ ] Estimate emphasis
- [ ] Estimate pauses/breath events
- [x] Generate procedural full-face behavior foundation
- [x] Emotion layering/presets foundation
- [x] Blink generation
- [x] Eye-dart procedural foundation
- [x] Gaze drift behavior foundation
- [~] Procedural head yaw/pitch/roll foundation
- [~] Instant micro-event foundation
- [ ] Breathing cues
- [~] Personality/performance preset foundation
- [x] Character personality profiles
- [ ] Intent tags
- [ ] Text + audio emotion fusion

## Deliverables

- [ ] Prosody feature extractor
- [ ] Emphasis events
- [x] Blink generator
- [x] Gaze/eye-dart generator foundation
- [x] Head-motion generator foundation
- [~] Instant event generator
- [x] Emotion system foundation
- [ ] Valence/Arousal support
- [x] Performance fusion/mixer foundation
- [x] Personality profile format

## Exit criteria

- [ ] Speech-driven output includes believable full-face secondary motion
- [ ] Behavior layers remain independently editable
- [x] Two personality profiles produce distinct deterministic behavior

---

# Milestone M5 — FaceCapture Prototype

**Status: [~] In progress**

## Desktop live capture

- [ ] Camera input
- [ ] Face detection
- [ ] Facial landmarks
- [ ] Dense face mesh
- [x] Mocap frame head-pose transport
- [ ] Eye/eyelid tracking
- [ ] Gaze estimation
- [ ] Temporal tracking
- [ ] Occlusion detection
- [ ] Tracking confidence
- [ ] Auto re-acquire

## Offline desktop video

- [ ] Video-file input
- [ ] Full-sequence analysis
- [ ] Look-ahead/look-behind solve
- [ ] Bidirectional smoothing
- [ ] Offline high-quality Studio mode
- [ ] Partial clip re-solve
- [ ] Region-specific re-solve
- [~] Bad-take quality scoring foundation
- [ ] Quality/confidence heatmap

## Android — BDFR FaceCapture Mobile

- [x] Android app skeleton
- [~] Front-camera CameraX preview
- [ ] Live facial tracking
- [ ] On-device solve
- [x] Prerecorded-video picker/import entry point
- [ ] Offline video-to-animation extraction
- [ ] Record then solve
- [ ] Local session storage
- [x] Live camera preview
- [ ] FPS/confidence display
- [ ] Actor calibration
- [~] UDP live-stream client foundation
- [ ] USB transfer mode
- [ ] Remote Record/Stop
- [ ] QR/pairing flow
- [ ] Send video to PC for high-quality solve
- [ ] Send intermediate tracking data to PC
- [ ] Mobile Mocap Track import

## Deliverables

- [~] Capture interfaces/contracts
- [ ] Tracking pipeline
- [x] Normalized FaceObservation contract
- [ ] Debug overlays
- [ ] Recording/replay system
- [ ] Studio offline solver
- [ ] Android capture-analysis path
- [x] Shared BDFR/BDFP binary packet protocol + Android encoder
- [~] Clock-offset estimator foundation

## Exit criteria

- [ ] Stable tracking on benchmark footage
- [ ] Desktop prerecorded video converts to editable BDFR facial curves
- [ ] Android prerecorded video converts to editable BDFR facial curves
- [ ] Android live stream reaches desktop
- [ ] No severe frame-to-frame jitter under normal conditions

---

# Milestone M6 — Solver & Calibration

**Status: [~] In progress**

## Goals

- [ ] Convert tracked observations to BDFR curves/FACS
- [x] Actor-specific calibration normalization core
- [x] Neutral reference in calibration ranges
- [x] Range-of-motion normalization core
- [~] Confidence-aware data/fusion foundation
- [ ] Anatomical constraints
- [~] Left/right independent curve/calibration support
- [ ] Neutral drift correction
- [ ] Adaptive solver
- [~] Actor profile + calibration/personality foundations
- [ ] Depth-aware solving
- [ ] Face mesh support

## Deliverables

- [x] Calibration profile
- [ ] Solver
- [~] Frame/region confidence contracts
- [ ] Temporal regularization
- [~] Independent left/right curve support
- [ ] Drift correction
- [x] Region-confidence observation model
- [ ] Occlusion recovery
- [ ] Calibration UI

## Exit criteria

- [ ] Calibrated actors outperform generic mapping on benchmark sequences
- [ ] Solver degrades gracefully under uncertainty
- [ ] Solver recovers after temporary occlusion
- [ ] Long sessions do not develop unacceptable neutral drift

---

# Milestone M7 — Mocap, MetaHuman & Advanced Retargeting

**Status: [~] In progress**

## Live mocap input

- [~] Android UDP sender foundation input
- [ ] TCP input
- [ ] WebSocket input
- [ ] OSC input
- [ ] Unreal Live Link input
- [ ] ARKit-style curves
- [~] Generic external-curve/blendshape adapter
- [~] External curve to BDFR/FACS mapping foundation
- [ ] Head pose
- [x] Mocap frame gaze transport
- [x] Third-party live mocap adapter interface

## Mocap file import

- [ ] CSV
- [x] JSON
- [ ] FBX animation curves
- [ ] BVH where facial channels are available
- [ ] ARKit curve files
- [ ] Unreal curve exports
- [x] BDFR binary sessions

## Retargeting

- [ ] MetaHuman mapping profile
- [ ] ARKit 52 compatibility profile
- [~] Generic retarget profile core
- [ ] Facial bone mapping
- [ ] Retarget calibration UI
- [x] Auto mapping by curve/morph naming foundation
- [x] Scale/Bias/Clamp
- [ ] Response curves
- [x] Invert/Dead zone
- [ ] Neutral offsets
- [ ] Pose-space correctives
- [x] Character compatibility scanner foundation
- [ ] Auto retarget calibration
- [ ] Retarget preview

## Mocap fusion

- [ ] Mocap + Audio
- [ ] Mocap + Emotion
- [ ] Mocap + Manual Override
- [x] Per-region source weighting
- [x] Per-region priorities
- [ ] Mocap fallback to Audio/Text when tracking fails

## Exit criteria

- [ ] One performance retargets to multiple characters with minimal edits
- [ ] External live mocap can drive BDFR in real time
- [ ] Imported mocap can be edited in the BDFR timeline
- [~] Region-level fusion core is implemented and unit-tested

---

# Milestone M8 — Real-Time Runtime

**Status: [~] In progress**

## Goals

- [ ] Live microphone streaming
- [ ] Live camera streaming
- [ ] Live mocap streaming
- [ ] Incremental inference
- [~] Thread-safe bounded frame queue foundation
- [x] Bounded frame queue memory
- [ ] Low latency
- [x] Jitter buffer
- [~] Sequence/drop accounting foundation
- [~] Configurable jitter delay foundation
- [~] Clock-offset estimator
- [~] Drop counts + shared diagnostics primitives
- [ ] Curve compression
- [ ] Facial LOD
- [ ] Network LOD
- [ ] Failover mode
- [ ] Multi-device capture
- [ ] Multi-actor support

## Transport

- [ ] UDP
- [ ] TCP
- [ ] WebSocket
- [ ] OSC
- [ ] Shared memory
- [ ] Unreal Live Link bridge

## Initial performance targets

- [ ] Audio-driven end-to-end latency below 100 ms where practical
- [ ] Stretch target around 50 ms on supported hardware
- [ ] Stable camera/mocap latency without queue buildup
- [ ] Measurable UE5 game-thread contribution

## Exit criteria

- [ ] Stable real-time demo
- [ ] Latency is measured automatically
- [ ] Frame timing is measured automatically
- [ ] Network loss/jitter are visible in diagnostics

---

# Milestone M9 — Quality, Testing & Production Tooling

**Status: [~] In progress**

## Already implemented

- [x] Core unit-test executable
- [x] Windows CI
- [x] Ubuntu CI
- [x] Basic frame validation tests
- [x] Binary codec roundtrip test
- [x] Truncated binary rejection
- [x] Mixer tests
- [x] Smoothing test
- [x] Timeline construction/validation tests
- [x] Timeline evaluator tests
- [x] Mute/Solo behavior tests
- [x] Region-mask tests
- [x] Layer-priority tests
- [x] Session/Take tests
- [x] Partial re-solve range tests

## Remaining quality systems

- [ ] Regression suite
- [ ] Visual benchmark suite
- [ ] Objective timing tests
- [ ] Quality review workflow
- [x] Profiling primitives
- [ ] Failure-mode catalog implementation
- [ ] Performance Cleanup Assistant
- [ ] Smart Cleanup
- [~] Capture quality/bad-take scoring foundation
- [~] Logging/timing diagnostics foundation
- [ ] Take quality heatmap
- [ ] A/B solver preview
- [~] Curve diff primitive
- [~] Curve diff/freeze primitives
- [ ] Source vs Retargeted comparison
- [ ] Stress tests
- [ ] Sanitizer CI
- [ ] Large stream tests

## Benchmark categories

- [ ] Fast speech
- [ ] Slow speech
- [ ] Whisper-like speech
- [ ] High-energy speech
- [ ] Asymmetric expression
- [ ] Head rotation
- [ ] Partial occlusion
- [ ] Glasses/facial hair
- [ ] Low contrast
- [ ] Strong blink
- [ ] Sustained vowels
- [ ] Plosives
- [ ] Lip closure
- [ ] Emotional speech
- [ ] Noisy audio
- [ ] Low-light capture
- [ ] Mobile capture

## Exit criteria

- [ ] Every release can be compared against prior versions
- [ ] Quality claims are backed by stored test evidence
- [ ] Regression clips can identify facial-region regressions
- [ ] Performance budgets are enforced

---

# Milestone M10 — AI Character Integration

**Status: [~] Foundation only**

## Goals

- [ ] TTS integration
- [ ] LLM/NPC hooks
- [~] Emotion/markup metadata foundations
- [ ] Intent metadata
- [ ] Live conversational performance
- [ ] Network transport
- [ ] AI Performance Director
- [x] Character Personality Profiles
- [x] Gaze target dialogue markup
- [x] Performance style dialogue markup

## Example pipeline

```text
LLM / Game AI
      |
      v
Text + Intent + Emotion
      |
      +------> TTS
      |         |
      |         v
      +--> BDFR SpeechFace
                |
                v
         Expression Engine
                |
                v
            Retargeter
                |
                v
      UE5 / MetaHuman / Custom Rig
```

## Exit criteria

- [ ] AI character can receive text and generate facial performance
- [ ] AI character can use TTS timing for accurate speech animation
- [ ] Personality profile alters performance consistently
- [ ] System can recover gracefully if one live input disappears

---

# Production Tooling Backlog

These features span multiple milestones.

## Project / Session management

- [~] Project model/persistence foundation
- [~] Actor profile foundation
- [~] Session model / manager foundation
- [~] Take model / manager foundation
- [x] Scene/Shot metadata in Session
- [ ] Multi-take comparison
- [ ] Facial slate
- [ ] Timecode
- [ ] 24/25/30/50/60 fps workflows
- [~] Rotating recovery snapshot foundation
- [~] Recovery snapshot discovery foundation

## Editing

- [ ] Professional curve editor
- [x] Undo/Redo snapshot-history foundation
- [~] Expression/Timeline layering foundations
- [x] Partial re-solve core ranges
- [x] Region freeze
- [ ] Smart cleanup
- [x] Key reduction
- [~] Key-reduction optimization foundation
- [ ] Pose library
- [x] Emotion expression presets foundation
- [~] Personality presets foundation

## Automation / SDK

- [~] Modular public C++ core API
- [ ] Python API
- [ ] Blueprint API
- [x] CLI/headless foundation
- [ ] Batch processing
- [ ] Plugin SDK
- [x] IFaceTracker capture provider interface
- [ ] FaceSolver provider interface
- [ ] SpeechSolver provider interface
- [~] Generic retarget profile API
- [ ] RuntimeOutput interface

## DCC / export

- [~] UE5 plugin skeleton
- [ ] MetaHuman
- [ ] ARKit
- [ ] FBX animation curves
- [ ] Blender
- [ ] Maya
- [ ] JSON
- [ ] CSV
- [ ] BDFR binary

## Model management

- [ ] Model manager
- [ ] Model versioning
- [ ] Model checksums
- [ ] License tracking
- [ ] CPU backend selection
- [ ] GPU backend selection
- [ ] NPU backend selection where available
- [ ] Benchmark manager
- [ ] Rollback

## Security / connectivity

- [ ] Device pairing
- [ ] Token authentication
- [ ] Optional encryption
- [ ] Trusted-device profiles
- [ ] Network monitor

---

# Release strategy

- [~] **0.1.x — Core curves, schema and timeline foundation**
- [ ] **0.2.x — UE5 runtime**
- [ ] **0.3.x — Offline SpeechFace**
- [ ] **0.4.x — Expressive Speech**
- [ ] **0.5.x — FaceCapture Desktop + Android**
- [ ] **0.6.x — Calibration and solver**
- [ ] **0.7.x — Mocap + advanced retargeting**
- [ ] **0.8.x — Real-time runtime**
- [ ] **0.9.x — Quality hardening**
- [ ] **1.0.0 — Production-ready baseline**

---

## Non-goals for early versions

- [ ] Photorealistic neural rendering
- [ ] Replacing character modeling/rigging packages
- [ ] Unsupported proprietary-format reverse engineering
- [ ] Unsupported claims of parity with commercial tools
- [ ] Training on datasets without clear licensing

These remain intentionally outside the early implementation scope.
