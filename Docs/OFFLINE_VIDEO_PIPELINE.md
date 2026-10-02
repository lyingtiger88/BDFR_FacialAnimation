# Offline Video-to-Facial-Animation Pipeline

BDFR now has a provider-driven offline processing pipeline:

```text
IVideoSource
    |
    v
IFaceTracker
    |
    v
FaceObservation + CaptureQuality
    |
    v
IFaceSolver
    |
    v
FacialSequence
    |
    v
Offline smoothing / cleanup / retarget
```

## Why providers are separated

The pipeline does not depend on a specific:
- video decoder
- landmark tracker
- ML runtime
- mobile/desktop model
- engine

This lets us plug in a future MediaPipe/LiteRT/ONNX/custom tracker without changing Session, Timeline or Retargeting code.

## OfflineSolveResult

Reports:
- number of source frames
- solved frames
- tracker failures
- solver failures
- quality-skipped frames
- per-frame quality and inclusion state

## Studio quality path

`SequenceFilter::movingAverage` is the first offline temporal filter. It is deliberately simple; future Studio mode will use:
- bidirectional smoothing
- sequence optimization
- occlusion reconstruction
- neutral drift correction
- partial-region re-solving
- look-ahead/look-behind models
