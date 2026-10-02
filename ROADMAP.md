# BDFR Facial Animation — Roadmap

This roadmap describes the staged development of BDFR Facial Animation. Dates are intentionally omitted until the foundation and benchmark suite are stable.

## Product direction

BDFR Facial Animation is planned as a modular platform with two primary input pipelines:

- **Video/Camera → Facial Performance**
- **Audio/Voice → Facial Performance**

Both pipelines converge on a shared time-based facial representation before expression layering and retargeting.

---

## Milestone M0 — Foundation

### Goals
- freeze top-level architecture
- define naming conventions
- define common facial frame schema
- define timestamps, confidence and curve ranges
- create benchmark/test asset conventions
- define coding, logging and profiling standards
- establish dependency policy

### Deliverables
- BDFR facial-frame data model
- serialization format
- module interfaces
- test harness skeleton
- sample curve data
- development documentation

### Exit criteria
- a synthetic facial sequence can be serialized, loaded and validated
- unit tests cover curve range, timestamps and schema versioning

---

## Milestone M1 — FACS Core

### Goals
- Action Unit representation
- normalized facial curve storage
- curve interpolation and smoothing
- blending/layering
- asymmetry support
- constraints

### Deliverables
- FACS registry
- curve mixer
- expression layer stack
- temporal interpolation
- JSON/binary interchange prototype

### Exit criteria
- multiple expressions can be layered without invalid curve values
- deterministic playback across test runs

---

## Milestone M2 — Unreal Engine 5 Runtime

### Goals
- create UE5 plugin
- consume BDFR facial frames
- map curves to morph targets and bones
- support live and prerecorded playback

### Deliverables
- UE5 plugin skeleton
- BDFR facial component
- curve receiver
- test character profile
- debug UI

### Exit criteria
- prerecorded BDFR curves animate a UE5 test face correctly
- runtime profiling is available

---

## Milestone M3 — SpeechFace Prototype

### Goals
- speech segmentation
- phoneme timing
- viseme generation
- jaw/lip coordination
- context-aware coarticulation
- offline WAV processing

### Deliverables
- audio frontend
- phoneme timeline
- viseme solver
- coarticulation engine
- curve export

### Exit criteria
- a clean speech clip produces synchronized editable mouth animation
- transitions are context-sensitive rather than pose switching

---

## Milestone M4 — Expressive Speech

### Goals
- extract prosody
- estimate emphasis
- generate procedural facial behavior
- add blink, gaze and head motion layers
- support emotion/expression overrides

### Deliverables
- prosody feature extractor
- emphasis events
- blink generator
- gaze/eye-dart generator
- head-motion generator
- performance mixer

### Exit criteria
- speech-driven performance includes believable full-face secondary motion
- behavior layers remain independently editable

---

## Milestone M5 — FaceCapture Prototype

### Goals
- camera input
- video-file input
- face detection
- landmarks
- head pose
- eyes/eyelids
- temporal tracking

### Deliverables
- capture frontend
- tracking pipeline
- normalized face observations
- debug overlays
- recording/replay system

### Exit criteria
- stable tracking on benchmark footage
- no severe frame-to-frame jitter under normal lighting

---

## Milestone M6 — Solver & Calibration

### Goals
- convert tracked observations to facial curves/FACS
- actor-specific calibration
- neutral reference
- range-of-motion calibration
- confidence-aware solve
- anatomical/rig constraints

### Deliverables
- calibration profile
- solver
- confidence model
- temporal regularization
- asymmetry support

### Exit criteria
- calibrated actors outperform generic mapping on benchmark sequences
- solver degrades gracefully when observations are uncertain

---

## Milestone M7 — MetaHuman & Advanced Retargeting

### Goals
- MetaHuman mapping profile
- ARKit-style 52-curve compatibility
- arbitrary morph-target rigs
- optional facial bone mapping
- retarget calibration UI

### Deliverables
- MetaHuman profile
- ARKit compatibility profile
- custom-rig editor
- profile import/export

### Exit criteria
- one solved performance can be retargeted to multiple characters with minimal manual edits

---

## Milestone M8 — Real-Time Runtime

### Goals
- microphone streaming
- camera streaming
- incremental inference
- thread-safe runtime
- bounded memory
- low latency

### Initial performance targets
- audio-driven latency target: **<100 ms**
- stretch target: **~50 ms**, subject to hardware/model constraints
- stable UE5 frame-time contribution appropriate for interactive use

### Exit criteria
- stable real-time demo
- latency and frame timing are measured automatically

---

## Milestone M9 — Quality Program

### Goals
- regression suite
- visual benchmark suite
- objective timing tests
- quality review process
- profiling
- failure-mode catalog

### Benchmark categories
- fast speech
- slow speech
- whisper-like speech
- high-energy speech
- asymmetric expression
- head rotation
- partial occlusion
- glasses/facial hair
- low contrast
- strong blink
- sustained vowels
- plosives
- lip closure
- emotional speech

### Exit criteria
- every release can be compared against prior versions
- quality claims are supported by stored test evidence

---

## Milestone M10 — AI Character Integration

### Goals
- TTS integration
- LLM/NPC hooks
- emotion metadata input
- live conversational performance
- network transport

### Example pipeline

```text
LLM / Game AI
      |
      v
TTS + expressive metadata
      |
      v
BDFR SpeechFace
      |
      v
Expression Engine
      |
      v
Retargeter
      |
      v
UE5 / MetaHuman / Custom Character
```

---

## Non-goals for early versions

The early project will not prioritize:
- photorealistic neural rendering
- replacing character modeling/rigging tools
- proprietary format compatibility without legal documentation
- unsupported claims of parity with commercial products
- training on unlicensed datasets

## Release strategy

Planned release sequence:

- **0.1.x** — core curves and schema
- **0.2.x** — UE5 runtime
- **0.3.x** — offline SpeechFace
- **0.4.x** — expressive speech
- **0.5.x** — FaceCapture
- **0.6.x** — calibration and solver
- **0.7.x** — advanced retargeting
- **0.8.x** — real-time runtime
- **0.9.x** — quality hardening
- **1.0.0** — production-ready baseline when exit criteria are actually met
