# BDFR Facial Animation

**BDFR Facial Animation** is an open, modular facial-animation platform focused on high-quality real-time and offline character performance.

The project is designed around two complementary pipelines:

1. **Video / camera driven facial performance capture** — markerless tracking, facial solving, temporal filtering and retargeting.
2. **Audio driven facial animation** — speech analysis, phoneme/viseme timing, coarticulation, prosody, expressive behavior, blinks, gaze and head motion.

The long-term objective is a production-grade system that can approach the workflow quality of commercial facial-animation solutions while remaining modular, inspectable and extensible.

> BDFR Facial Animation is an independent project. Faceware, Speech Graphics, MetaHuman, Unreal Engine, ARKit and other product names are trademarks of their respective owners. They are mentioned only as interoperability or quality-reference targets.

## Download latest test builds

The latest hardware-test builds are published on the stable **BDFR Hardware Test — Latest** GitHub prerelease.

- **Windows Hardware Test Kit:** https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/download/hardware-test-latest/BDFR_Windows_Hardware_Test_Kit.zip
- **Android Hardware Test APK:** https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/download/hardware-test-latest/BDFR_Android_Hardware_Test.apk
- **Release page:** https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/tag/hardware-test-latest
- **BDFR Studio Windows:** https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/download/studio-test-latest/BDFR_Studio_Windows.zip
- **BDFR Studio release page:** https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/tag/studio-test-latest

These URLs stay the same. When a new test-relevant build passes Windows/Core and Android tests, the release assets are replaced automatically.

See [test download policy](Docs/DOWNLOADS.md) and [hardware live test guide](Docs/HARDWARE_LIVE_TEST.md).

## Core goals

- FACS-based facial representation
- Markerless video/camera facial capture
- Offline facial-animation extraction from prerecorded video on desktop and Android
- Audio-to-face animation
- Context-aware coarticulation instead of simple phoneme-to-viseme switching
- Prosody-driven facial performance
- Procedural blink, gaze, eye-dart, breathing and subtle head motion
- Temporal stability and anatomically plausible facial motion
- Rig-agnostic retargeting
- Unreal Engine 5 runtime integration
- MetaHuman support
- ARKit-compatible curve mapping
- Custom morph-target and bone-driven rigs
- Real-time and offline processing
- A common facial-curve interchange format
- Future AI-NPC integration

## Proposed architecture

```text
                  BDFR Facial Animation
                           |
          +----------------+----------------+
          |                                 |
    Video / Camera                      Audio / Voice
          |                                 |
    FaceCapture                        SpeechFace
          |                                 |
 Landmarks / Pose              Phonemes / Prosody / Emotion
          |                                 |
          +----------------+----------------+
                           |
                      FaceSolver
                           |
                     FACS / Curves
                           |
                 Expression Engine
                           |
                 Performance Mixer
                           |
                      Retargeter
                           |
        +------------------+------------------+
        |                  |                  |
     MetaHuman          ARKit 52         Custom Rigs
        |
   Unreal Engine 5
```

## Planned modules

### BDFR FaceCapture
Camera and prerecorded-video facial tracking on desktop and mobile.

Planned responsibilities:
- facial landmark detection
- head-pose estimation
- eye and eyelid tracking
- facial-region tracking
- temporal stabilization
- actor calibration
- expression solve preparation
- offline video-to-animation extraction
- full-sequence analysis with look-ahead/look-behind in Studio mode
- Android local solve or deferred PC solve

### BDFR FaceSolver
Converts observations into a normalized facial representation.

Planned responsibilities:
- FACS Action Units
- normalized facial curves
- asymmetry support
- temporal solving
- constraint handling
- confidence values
- calibration profiles

### BDFR SpeechFace
Audio-driven facial animation.

Planned responsibilities:
- speech segmentation
- phoneme timing
- viseme generation
- context-aware coarticulation
- jaw/tongue/lip coordination
- pitch, intensity and rhythm analysis
- emphasis and expressive speech cues
- optional emotion estimation
- breath-event estimation

### BDFR Expression Engine
Adds believable non-verbal facial behavior.

Planned responsibilities:
- blinks
- eye darts
- gaze
- brow behavior
- micro-expressions
- subtle head movement
- breathing cues
- emotion/expression layering
- procedural idle behavior

### BDFR Retargeter
Maps the common BDFR facial representation to target characters.

Initial targets:
- Unreal Engine 5
- MetaHuman
- ARKit-style 52 blendshape rigs
- custom morph-target rigs
- bone-driven facial rigs

### BDFR Runtime
Low-latency execution layer for games, interactive applications and AI characters.

Long-term target:
- real-time audio-to-face
- real-time camera capture
- low-latency streaming
- game-thread friendly runtime
- CPU/GPU backends where appropriate

## Common facial representation

All input systems should converge on a shared frame representation instead of coupling solvers directly to a character rig.

Example:

```json
{
  "timestamp": 1.266,
  "confidence": 0.96,
  "curves": {
    "AU01": 0.22,
    "AU02": 0.10,
    "AU04": 0.61,
    "AU06": 0.35,
    "AU12": 0.72,
    "jawOpen": 0.31,
    "mouthPucker": 0.12,
    "eyeBlinkLeft": 0.08,
    "eyeBlinkRight": 0.09
  },
  "head": {
    "pitch": -2.1,
    "yaw": 4.8,
    "roll": 0.4
  }
}
```

The exact schema will evolve, but the key rule is:

**Capture, audio analysis, animation synthesis and retargeting remain separate layers.**

## Quality targets

The project will prioritize:

- stable motion over noisy per-frame detection
- actor-specific calibration
- temporal coherence
- natural coarticulation
- asymmetrical expressions
- editable animation curves
- deterministic retargeting
- graceful confidence degradation
- reproducible evaluation clips
- measurable latency and accuracy

We will not claim commercial-product parity until objective test material supports it.

## Repository layout

```text
BDFR_FacialAnimation/
├─ Engine/
│  ├─ FaceCapture/
│  ├─ FaceSolver/
│  ├─ FACS/
│  ├─ SpeechFace/
│  ├─ ExpressionEngine/
│  └─ Retargeter/
├─ Mobile/
│  └─ BDFR_FaceCapture_Android/
├─ Runtime/
│  ├─ Core/
│  ├─ Audio/
│  ├─ Video/
│  └─ Networking/
├─ Unreal/
│  └─ BDFRFacialAnimation/
├─ Models/
├─ Tools/
├─ Tests/
├─ Samples/
├─ Docs/
└─ ThirdParty/
```

## Roadmap

See [ROADMAP.md](ROADMAP.md).

High-level milestones:

- **M0 — Foundation:** architecture, curve schema, test harness, project conventions
- **M1 — FACS Core:** Action Unit model, curve mixer, serialization, validation
- **M2 — UE5 Runtime:** Unreal plugin, curve transport, custom-rig test character
- **M3 — SpeechFace Prototype:** phoneme timing, visemes, coarticulation, offline audio-to-face
- **M4 — Expressive Speech:** prosody, blink/gaze/head behavior, performance layers
- **M5 — FaceCapture Prototype:** camera/video tracking, head pose, landmarks, temporal filtering
- **M6 — Solver & Calibration:** actor calibration, FACS solving, confidence/constraints
- **M7 — MetaHuman Retargeter:** production mapping and calibration workflow
- **M8 — Real-Time Runtime:** low-latency microphone and camera pipelines
- **M9 — Quality Program:** benchmark clips, regression tests, performance profiling and tuning
- **M10 — AI Character Integration:** TTS/LLM-driven facial performance hooks

## Initial engineering principles

1. Core modules must not depend on Unreal Engine.
2. Unreal integration lives behind an adapter/plugin layer.
3. All animation data is timestamped.
4. Solvers expose confidence values.
5. Retargeting is data-driven.
6. Offline and real-time paths share as much logic as practical.
7. Models and third-party assets must have explicit licenses.
8. Evaluation footage and expected outputs should be versioned where licensing permits.
9. Every performance optimization should be measurable.
10. Facial animation quality is treated as a temporal problem, not a collection of isolated poses.

## Current status

**Core bootstrap is now testable.**

Implemented in the first C++ core slice:
- versioned `FacialFrame` data model
- canonical FACS/practical curve registry
- frame validation and sanitization
- weighted curve mixing
- temporal exponential smoothing
- compact binary facial-frame codec
- multi-track timeline data model including Text, Emotion, Instant Event and Mocap tracks
- CMake build
- executable core unit tests
- GitHub Actions CI on Windows and Ubuntu

The next implementation slice is timeline evaluation, region masks/priorities, session/take model, partial re-solve primitives, and the first UE5 adapter.

## Documentation

- [Roadmap](ROADMAP.md)
- [Architecture](Docs/ARCHITECTURE.md)
- [Quality Targets](Docs/QUALITY_TARGETS.md)
- [Development Plan](Docs/DEVELOPMENT.md)
- [Testing](Docs/TESTING.md)

## Build and test

Requirements:
- CMake 3.20+
- C++17 compiler

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The bootstrap test suite has been compiled and executed successfully during development. CI repeats the build and tests on Windows and Ubuntu.

## License

Project licensing will be finalized before external code/assets are integrated. Third-party components keep their own licenses.

## Repository metadata

Suggested GitHub description:

> Production-oriented facial animation platform for FACS, markerless face capture, audio-driven performance, expressive behavior, retargeting, MetaHuman and Unreal Engine 5.

Suggested topics:

`facial-animation`, `facs`, `face-capture`, `facial-mocap`, `lip-sync`, `audio-to-face`, `speech-animation`, `unreal-engine`, `unreal-engine-5`, `metahuman`, `arkit`, `blendshapes`, `computer-vision`, `animation`, `ai-npc`


## MetaHuman / OpenRigLogic

BDFR Studio test builds now include the optional MetaHuman runtime based on Epic Games OpenRigLogic 5.8.

Current integration:

- MetaHuman DNA loading
- RigLogic / RigInstance initialization
- BDFR / ARKit-style curve to DNA raw-control mapping foundation
- realtime rig evaluation from live FacialFrame data
- joint output extraction
- named blendshape output extraction
- named animated-map output extraction
- Windows + Linux adapter CI
- Windows Studio package with OpenRigLogic enabled

OpenRigLogic remains isolated from the engine-independent BDFR Core.

See [MetaHuman / OpenRigLogic integration](Docs/METAHUMAN_OPENRIGLOGIC.md).
