# Architecture

## Architectural principle

BDFR Facial Animation separates **observation**, **solving**, **performance synthesis**, and **retargeting**.

A tracking model should never be directly coupled to a MetaHuman, and an audio solver should never emit engine-specific morph-target names.

## Layers

### 1. Input layer

Sources:
- webcam
- prerecorded video
- microphone
- prerecorded audio
- external facial-curve stream
- AI/TTS metadata

### 2. Observation layer

Video observations may include:
- facial landmarks
- dense landmarks
- head pose
- eye state
- eyelid aperture
- gaze estimate
- confidence
- occlusion indicators

Audio observations may include:
- waveform features
- phonemes
- phoneme timing
- pitch
- intensity
- spectral features
- speech rate
- pauses
- emphasis candidates
- breath candidates

### 3. Solver layer

Converts raw observations into normalized facial state.

Output requirements:
- timestamped
- confidence-aware
- temporally coherent
- independent of character rig

### 4. FACS/curve representation

The common representation contains:
- FACS Action Units where applicable
- practical speech curves
- eye curves
- jaw curves
- head transform
- optional gaze
- optional tongue parameters
- confidence

All normalized scalar curves should use a documented canonical range.

### 5. Expression Engine

Combines:
- solved expression
- speech articulation
- emotion layer
- blink
- gaze
- eye darts
- idle motion
- breathing
- authored overrides

Layer order and weights must be explicit and reproducible.

### 6. Retargeting

Retargeting is profile-driven.

A retarget profile may define:
- source curve
- target morph/bone
- scale
- bias
- clamp
- response curve
- left/right mapping
- corrective relationships
- neutral offsets

Target adapters:
- MetaHuman
- ARKit-compatible rigs
- custom morph targets
- custom facial bones
- future DCC/runtime adapters

### 7. Runtime adapter

The runtime layer is responsible for:
- threading
- streaming
- buffering
- transport
- engine callbacks
- profiling
- latency measurement

The core solver should remain engine-independent.

---

## Proposed directory ownership

```text
Engine/FaceCapture       computer-vision capture and tracking
Engine/FaceSolver        observation-to-curve solving
Engine/FACS              Action Units and common curve model
Engine/SpeechFace        audio-driven articulation/performance
Engine/ExpressionEngine  procedural and layered facial behavior
Engine/Retargeter        rig-neutral mapping system

Runtime/Core             shared runtime primitives
Runtime/Audio            streaming audio frontend
Runtime/Video            streaming video frontend
Runtime/Networking       curve streaming protocols

Unreal/BDFRFacialAnimation
                         UE5 plugin and adapters
```

---

## Time model

Every facial frame must carry a timestamp.

Recommended internal time:
- seconds as double precision for offline tools
- monotonic clock for real-time runtime
- engine time converted at adapter boundary

Never infer timing purely from frame index unless the source explicitly declares a fixed frame rate.

---

## Confidence model

Solvers should expose confidence for:
- full frame
- facial regions
- individual curves where feasible

Low confidence should not cause uncontrolled snapping.

Fallback policy may include:
- hold
- decay toward neutral
- predictive continuation
- region-specific suppression

---

## Temporal solving

A major quality requirement is that facial animation is treated as a temporal signal.

Potential techniques:
- exponential smoothing
- One Euro filter
- Kalman filtering
- temporal neural models
- sequence models
- optimization over windows
- velocity/acceleration constraints

The exact method may vary by module.

---

## Coarticulation

Speech animation must model neighboring phonetic context.

Requirements:
- look-ahead/look-behind where available
- plosive closures
- anticipatory rounding
- vowel influence
- timing overlap
- jaw continuity
- speaker calibration

A static phoneme-to-viseme table may exist as a fallback, not as the final-quality solver.

---

## Data contracts

Initial planned entities:

### BDFRFacialFrame
- schema_version
- timestamp
- frame_confidence
- curves
- head pose
- gaze
- metadata

### BDFRCurve
- id
- value
- confidence

### BDFRRetargetProfile
- source schema
- target rig identity
- mapping rules
- calibration
- corrective rules

### BDFRCalibrationProfile
- actor/session identity
- neutral reference
- motion ranges
- solver calibration parameters

---

## Transport

Candidate transport formats:
- in-process C++ API
- compact binary frames
- JSON for debugging/interchange
- UDP for low-overhead local streaming
- WebSocket/TCP where reliability matters

JSON is not intended to be the only production transport.

---

## Threading

Target architecture:
- capture thread
- inference/solver worker
- performance mixer
- output/transport
- game-engine adapter

No module should block the Unreal game thread with expensive inference.

---

## Model policy

Machine-learning models must have:
- source/provenance
- license
- version
- checksum
- documented input/output format
- documented hardware expectations

Model binaries should not be committed blindly to Git history.

---

## Compatibility philosophy

Compatibility layers are adapters, not architectural dependencies.

The BDFR core should continue working if:
- MetaHuman naming changes
- a new engine adapter is introduced
- an ARKit-style target is replaced by a custom rig
- the capture model is swapped
