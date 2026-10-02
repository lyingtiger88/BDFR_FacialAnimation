# Research Alignment Notes — October 2026

This document records product/architecture observations from current official documentation and how BDFR responds to them.

## Faceware lessons

Current Faceware Studio emphasizes:
- markerless live or prerecorded facial tracking
- one-click / neutral-pose actor calibration
- actor-specific animation tuning
- per-control motion effects
- realtime streaming into DCC/game-engine clients
- recorded-video processing and timeline playback

BDFR response:
- CalibrationProfile
- CurveTuningProfile
- Corrective/Expression rules
- live UDP/BDFP transport
- prerecorded-video processing pipeline
- timeline-based, non-destructive editing

Faceware Portal also demonstrates the value of:
- automated prerecorded-video processing
- environment-agnostic animation control data
- JSON interchange
- asymmetry/subtle-motion preservation

BDFR response:
- engine-independent FacialFrame/FacialSequence
- JSON + binary interchange
- left/right canonical curves
- offline Studio pipeline and quality reports

## Speech Graphics lessons

Current SGX positions audio as a complete performance source, not only lip sync.

Important capabilities:
- accurate speech articulation
- procedural full-face emotion
- head motion
- blinks
- eye darts
- breath
- rig independence
- batch processing
- optional transcript support for higher-quality alignment

BDFR response:
- text/viseme SpeechFace prototype
- WAV audio frontend
- windowed audio feature extraction
- prosody/emphasis foundation
- procedural behavior
- emotion system
- personality profiles
- generic retargeting
- CLI/batch-ready architecture

Next SpeechFace priorities:
1. F0/pitch extraction
2. voice-activity segmentation
3. transcript/audio alignment
4. real phoneme recognition or forced alignment
5. breath-event model
6. learned coarticulation
7. audio-conditioned head/gaze behavior

## Unreal Engine / MetaHuman lessons

Current MetaHuman workflows support realtime animation from:
- mono video cameras
- audio sources
- mobile Live Link Face
- Live Link subjects

BDFR should therefore expose **multiple Live Link-style source types**, not only ARKit curves.

Planned BDFR UE sources:
- BDFR Curves
- BDFR Mobile
- BDFR Audio
- BDFR Mono Video
- external Live Mocap adapter

The BDFR core remains engine-independent.

## Android differentiation

Epic's current Live Link Face Android support targets realtime animation. Their documentation notes that Android does not support offline capture for offline processing in the same way as iOS capture workflows.

BDFR therefore keeps Android offline processing as a deliberate differentiator:

- import prerecorded video on Android
- record then solve on device
- solve locally when hardware permits
- upload/send clip or intermediate observations to desktop
- re-solve in higher-quality Studio mode
- preserve the same BDFR FacialFrame format

## ARKit interoperability

Apple ARKit provides normalized named facial blendshape coefficients and supports more than 50 expression controls.

BDFR now includes the standard ARKit-style 52-name profile as a first-class interoperability layer.

This is an adapter target, not the definition of the entire BDFR facial model.

## Product principle

BDFR should not copy a single commercial workflow.

The target architecture combines:

```text
Faceware-like video capture/calibration/tuning
+
SGX-like audio-driven full-face performance
+
MetaHuman/Live-Link-style realtime interoperability
+
BDFR-specific text, mocap fusion and Android offline solving
```

while keeping every stage independently editable and retargetable.
