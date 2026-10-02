# FaceCapture Contract

BDFR keeps image capture, tracking and facial solving separate.

## ImageView

Capture backends expose a lightweight view describing:
- pixel buffer
- width / height / stride
- pixel format
- timestamp

Initial pixel formats:
- Gray8
- RGB24
- RGBA32
- NV21
- YUV420

## FaceObservation

A tracker returns:
- timestamp
- overall confidence
- sparse/dense landmarks
- head pose
- gaze
- region confidence
- occlusion flag

## IFaceTracker

Tracking implementations only need to implement:

```text
process(ImageView -> FaceObservation)
reset()
```

This lets BDFR swap:
- desktop CPU tracker
- GPU tracker
- Android on-device tracker
- future neural models

without changing the solver, Timeline or Retargeter.

## Capture quality

The first quality evaluator combines:
- detector confidence
- average landmark confidence
- facial-region confidence
- occlusion penalty

This becomes the foundation for bad-take detection and Timeline confidence heatmaps.
