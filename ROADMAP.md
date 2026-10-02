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
- [~] M0 Foundation
- [~] M1 FACS Core
- [ ] M2 UE5 Runtime
- [ ] M3 SpeechFace
- [ ] M4 Expressive Speech
- [ ] M5 FaceCapture
- [ ] M6 Solver & Calibration
- [ ] M7 Retargeting
- [ ] M8 Real-Time Runtime
- [~] M9 Quality Program
- [ ] M10 AI Character Integration

---

## Product direction

BDFR Facial Animation is designed as a modular facial-performance platform.

Planned first-class inputs:

- [ ] Text
- [ ] Prerecorded audio
- [ ] Live microphone
- [ ] Prerecorded video
- [ ] Live camera
- [ ] Live facial mocap
- [ ] Imported facial mocap files
- [ ] Android capture / live transfer
- [ ] External curves / API streams

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
- [ ] Define logging standard
- [ ] Define profiling standard
- [ ] Define benchmark asset conventions
- [x] Define initial session/take metadata conventions
- [ ] Define project-file/version migration policy

## Deliverables

- [x] BDFR facial-frame data model
- [x] Binary serialization prototype
- [x] Initial module interfaces
- [x] Test harness skeleton
- [x] CMake build
- [x] GitHub CI
- [x] Development documentation
- [x] Testing documentation
- [ ] JSON serialization/interchange
- [ ] Sample facial-sequence files
- [x] Session/Take data model
- [ ] Project persistence format
- [ ] Undo/Redo transaction model
- [ ] Crash recovery/autosave design

## Exit criteria

- [x] A facial frame can be serialized, loaded and validated
- [x] Unit tests cover basic curve range validation
- [x] Unit tests cover binary roundtrip
- [ ] Unit tests cover schema migration/version compatibility
- [ ] A complete facial sequence can be stored and replayed deterministically
- [ ] Session/Take metadata survives save/load

---

# Milestone M1 — FACS Core

**Status: [~] In progress**

## Goals

- [x] Initial Action Unit representation
- [x] Normalized facial curve storage
- [x] Basic temporal smoothing
- [x] Basic blending/layering
- [x] Left/right curve support in registry
- [ ] Full FACS registry and metadata
- [x] Region masks
- [x] Layer priority
- [ ] Additive/override policy per region
- [ ] Curve constraints
- [ ] Velocity/acceleration constraints
- [ ] Pose-space correctives
- [ ] Facial dynamics model
- [ ] Region freeze
- [x] Partial re-solve primitives

## Deliverables

- [x] Initial FACS/practical curve registry
- [x] Curve mixer
- [x] Exponential smoothing
- [x] Binary facial-frame codec
- [x] Initial timeline types
- [ ] Expression layer stack
- [x] Region mask system
- [x] Priority resolver
- [ ] Corrective-rule engine
- [ ] Advanced temporal interpolation
- [ ] JSON interchange
- [ ] Golden deterministic codec vectors
- [ ] Curve key reduction
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
- [ ] Non-destructive editing
- [ ] Curve editor
- [ ] Snapping
- [ ] Bezier/tangent editing
- [ ] Key reduction
- [ ] Quality/confidence heatmap
- [x] Partial range re-solve primitives

## Exit criteria

- [x] Multiple curve layers can be mixed without invalid normalized values
- [ ] Full expression layers can be evaluated over time
- [x] Region-specific source fusion primitives work
- [ ] Deterministic playback is verified across stored sequences
- [ ] Corrective rules can resolve problematic expression combinations

---

# Milestone M2 — Unreal Engine 5 Runtime

**Status: [ ] Not started**

## Goals

- [ ] Create UE5 plugin
- [ ] Consume BDFR facial frames
- [ ] Map curves to morph targets
- [ ] Map curves to facial bones
- [ ] Support prerecorded playback
- [ ] Support live playback
- [ ] Blueprint API
- [ ] C++ API
- [ ] Live Link adapter
- [ ] Debug visualization
- [ ] Runtime profiling

## Deliverables

- [ ] UE5 plugin skeleton
- [ ] BDFR Facial Component
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

**Status: [ ] Not started**

## Input modes

- [ ] Audio file
- [ ] Live microphone
- [ ] Text-only
- [ ] Text + TTS timing
- [ ] Text + performance markup

## Goals

- [ ] Speech segmentation
- [ ] Phoneme timing
- [ ] Viseme generation
- [ ] Jaw/lip coordination
- [ ] Context-aware coarticulation
- [ ] Anticipatory articulation
- [ ] Plosive closure handling
- [ ] F/V handling
- [ ] Tongue approximation
- [ ] Offline WAV processing
- [ ] Multilingual phoneme profiles

## Deliverables

- [ ] Audio frontend
- [ ] Text frontend
- [ ] Phoneme timeline
- [ ] Viseme solver
- [ ] Coarticulation engine
- [ ] TTS timing adapter
- [ ] Curve export
- [ ] Speaker profile
- [ ] Language profile

## Exit criteria

- [ ] Clean speech produces synchronized editable mouth animation
- [ ] Text-only input can generate a timed facial-performance draft
- [ ] Text+TTS path can produce tighter lip timing
- [ ] Transitions are context-sensitive rather than pose switching

---

# Milestone M4 — Expressive Speech & Performance

**Status: [ ] Not started**

## Goals

- [ ] Extract prosody
- [ ] Estimate emphasis
- [ ] Estimate pauses/breath events
- [ ] Generate procedural full-face behavior
- [ ] Emotion layering
- [ ] Blink generation
- [ ] Eye darts
- [ ] Gaze behavior
- [ ] Head nod/tilt/turn
- [ ] Micro-expressions
- [ ] Breathing cues
- [ ] Performance style presets
- [ ] Character personality profiles
- [ ] Intent tags
- [ ] Text + audio emotion fusion

## Deliverables

- [ ] Prosody feature extractor
- [ ] Emphasis events
- [ ] Blink generator
- [ ] Gaze/eye-dart generator
- [ ] Head-motion generator
- [ ] Micro-expression generator
- [ ] Emotion system
- [ ] Valence/Arousal support
- [ ] Performance mixer
- [ ] Personality profile format

## Exit criteria

- [ ] Speech-driven output includes believable full-face secondary motion
- [ ] Behavior layers remain independently editable
- [ ] Two personality profiles can produce different performances from identical dialogue

---

# Milestone M5 — FaceCapture Prototype

**Status: [ ] Not started**

## Desktop live capture

- [ ] Camera input
- [ ] Face detection
- [ ] Facial landmarks
- [ ] Dense face mesh
- [ ] Head pose
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
- [ ] Bad-take detection
- [ ] Quality/confidence heatmap

## Android — BDFR FaceCapture Mobile

- [ ] Android app skeleton
- [ ] Front-camera capture
- [ ] Live facial tracking
- [ ] On-device solve
- [ ] Prerecorded-video import
- [ ] Offline video-to-animation extraction
- [ ] Record then solve
- [ ] Local session storage
- [ ] Live preview
- [ ] FPS/confidence display
- [ ] Actor calibration
- [ ] Wi-Fi live transfer
- [ ] USB transfer mode
- [ ] Remote Record/Stop
- [ ] QR/pairing flow
- [ ] Send video to PC for high-quality solve
- [ ] Send intermediate tracking data to PC
- [ ] Mobile Mocap Track import

## Deliverables

- [ ] Capture frontend
- [ ] Tracking pipeline
- [ ] Normalized face observations
- [ ] Debug overlays
- [ ] Recording/replay system
- [ ] Studio offline solver
- [ ] Android capture-analysis path
- [ ] Android live-stream protocol
- [ ] Desktop/mobile clock sync

## Exit criteria

- [ ] Stable tracking on benchmark footage
- [ ] Desktop prerecorded video converts to editable BDFR facial curves
- [ ] Android prerecorded video converts to editable BDFR facial curves
- [ ] Android live stream reaches desktop
- [ ] No severe frame-to-frame jitter under normal conditions

---

# Milestone M6 — Solver & Calibration

**Status: [ ] Not started**

## Goals

- [ ] Convert tracked observations to BDFR curves/FACS
- [ ] Actor-specific calibration
- [ ] Neutral reference
- [ ] Range-of-motion calibration
- [ ] Confidence-aware solve
- [ ] Anatomical constraints
- [ ] Natural asymmetry preservation
- [ ] Neutral drift correction
- [ ] Adaptive solver
- [ ] Speaker/actor profiles
- [ ] Depth-aware solving
- [ ] Face mesh support

## Deliverables

- [ ] Calibration profile
- [ ] Solver
- [ ] Confidence model
- [ ] Temporal regularization
- [ ] Asymmetry support
- [ ] Drift correction
- [ ] Region confidence
- [ ] Occlusion recovery
- [ ] Calibration UI

## Exit criteria

- [ ] Calibrated actors outperform generic mapping on benchmark sequences
- [ ] Solver degrades gracefully under uncertainty
- [ ] Solver recovers after temporary occlusion
- [ ] Long sessions do not develop unacceptable neutral drift

---

# Milestone M7 — Mocap, MetaHuman & Advanced Retargeting

**Status: [ ] Not started**

## Live mocap input

- [ ] UDP input
- [ ] TCP input
- [ ] WebSocket input
- [ ] OSC input
- [ ] Unreal Live Link input
- [ ] ARKit-style curves
- [ ] Generic blendshape weights
- [ ] FACS/AU streams
- [ ] Head pose
- [ ] Gaze/eye data
- [ ] Third-party adapter interface

## Mocap file import

- [ ] CSV
- [ ] JSON
- [ ] FBX animation curves
- [ ] BVH where facial channels are available
- [ ] ARKit curve files
- [ ] Unreal curve exports
- [ ] BDFR binary sessions

## Retargeting

- [ ] MetaHuman mapping profile
- [ ] ARKit 52 compatibility profile
- [ ] Arbitrary morph-target rigs
- [ ] Facial bone mapping
- [ ] Retarget calibration UI
- [ ] Auto mapping by curve/morph naming
- [ ] Scale/Offset/Clamp
- [ ] Response curves
- [ ] Invert/Dead zone
- [ ] Neutral offsets
- [ ] Pose-space correctives
- [ ] Character compatibility scanner
- [ ] Auto retarget calibration
- [ ] Retarget preview

## Mocap fusion

- [ ] Mocap + Audio
- [ ] Mocap + Emotion
- [ ] Mocap + Manual Override
- [ ] Per-region source weighting
- [ ] Per-region priorities
- [ ] Mocap fallback to Audio/Text when tracking fails

## Exit criteria

- [ ] One performance retargets to multiple characters with minimal edits
- [ ] External live mocap can drive BDFR in real time
- [ ] Imported mocap can be edited in the BDFR timeline
- [ ] Region-level fusion works reliably

---

# Milestone M8 — Real-Time Runtime

**Status: [ ] Not started**

## Goals

- [ ] Live microphone streaming
- [ ] Live camera streaming
- [ ] Live mocap streaming
- [ ] Incremental inference
- [ ] Thread-safe runtime
- [ ] Bounded memory
- [ ] Low latency
- [ ] Jitter buffer
- [ ] Packet-loss handling
- [ ] Delay compensation
- [ ] Clock synchronization
- [ ] Network diagnostics
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

**Status: [~] Started**

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
- [ ] Profiling framework
- [ ] Failure-mode catalog implementation
- [ ] Performance Cleanup Assistant
- [ ] Smart Cleanup
- [ ] Automatic bad-take detection
- [ ] Technical performance diagnostics
- [ ] Take quality heatmap
- [ ] A/B solver preview
- [ ] Session Diff
- [ ] RAW vs Filtered comparison
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

**Status: [ ] Not started**

## Goals

- [ ] TTS integration
- [ ] LLM/NPC hooks
- [ ] Emotion metadata input
- [ ] Intent metadata
- [ ] Live conversational performance
- [ ] Network transport
- [ ] AI Performance Director
- [ ] Character Personality Profiles
- [ ] Gaze target metadata
- [ ] Performance style metadata

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

- [ ] Project manager
- [ ] Actor profiles
- [~] Session model / manager foundation
- [~] Take model / manager foundation
- [ ] Scene/Shot metadata
- [ ] Multi-take comparison
- [ ] Facial slate
- [ ] Timecode
- [ ] 24/25/30/50/60 fps workflows
- [ ] Autosave
- [ ] Crash recovery

## Editing

- [ ] Professional curve editor
- [ ] Undo/Redo
- [ ] Non-destructive layers
- [x] Partial re-solve core ranges
- [ ] Region freeze
- [ ] Smart cleanup
- [ ] Key reduction
- [ ] Curve optimization
- [ ] Pose library
- [ ] Expression presets
- [ ] Performance presets

## Automation / SDK

- [ ] C++ SDK
- [ ] Python API
- [ ] Blueprint API
- [ ] CLI/headless processing
- [ ] Batch processing
- [ ] Plugin SDK
- [ ] CaptureProvider interface
- [ ] FaceSolver provider interface
- [ ] SpeechSolver provider interface
- [ ] RetargetProvider interface
- [ ] RuntimeOutput interface

## DCC / export

- [ ] UE5
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
