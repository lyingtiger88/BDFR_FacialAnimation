# Quality Targets

## Objective

BDFR Facial Animation aims for production-oriented facial performance.

Commercial systems may be used as workflow/quality references, but project claims must be based on BDFR's own benchmark evidence.

## Quality dimensions

### Synchronization
- mouth closures line up with relevant consonants
- vowel timing remains stable
- jaw timing does not visibly lead/lag speech

### Coarticulation
- neighboring phonemes influence transitions
- rounding may begin before the target sound
- plosives preserve closure
- transitions avoid pose popping

### Temporal stability
- minimal landmark/curve jitter
- no frame-to-frame snapping
- stable neutral pose
- stable slow head motion

### Expressiveness
- prosody influences performance
- emphasis can influence brows/head/gaze
- blinks are plausible rather than periodic
- eye darts remain subtle and context-aware

### Retargeting
- character identity is preserved
- target limits are respected
- asymmetry survives mapping
- corrective shapes do not overfire

### Capture robustness
Test conditions should eventually include:
- frontal face
- moderate head rotation
- glasses
- facial hair
- uneven lighting
- partial hand occlusion
- fast expression changes
- slow subtle expression
- blink
- squint
- asymmetric smile

## Performance metrics

Planned runtime metrics:
- end-to-end latency
- solver latency
- inference latency
- frame queue depth
- dropped frames
- Unreal game-thread cost
- memory footprint
- CPU utilization
- optional GPU utilization

## Initial real-time targets

Audio-driven:
- initial target: <100 ms end-to-end where practical
- stretch goal: approximately 50 ms on supported hardware

Camera-driven:
- interactive frame rate on supported hardware
- bounded latency
- no accumulating queue delay

These are engineering targets, not guarantees.

## Regression testing

Every meaningful solver update should be evaluated on fixed benchmark clips.

Recommended outputs:
- curve CSV/JSON
- rendered preview
- latency report
- error/warning log
- solver version
- model version
- hardware summary

## Visual review protocol

Reviewers should inspect:
- lips
- jaw
- cheeks
- brows
- eyelids
- gaze
- head motion
- neutral return
- transitions

A change should not be accepted solely because one region improves if another visibly regresses.

## Failure-mode catalog

Known failure types should be named and tracked, for example:
- lip chatter
- jaw pumping
- blink flutter
- brow noise
- gaze swimming
- neutral drift
- phoneme popping
- over-rounding
- under-closure
- asymmetric collapse
- retarget overshoot
- delayed consonant
- head jitter

## Production-quality definition

Version 1.0 should require:
- reproducible benchmarks
- documented latency
- stable serialization
- UE5 integration
- editable output
- robust failure handling
- retarget profiles
- automated regression tests
- documented hardware assumptions
