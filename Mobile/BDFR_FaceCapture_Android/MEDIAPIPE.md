# MediaPipe Face Landmarker Integration

The Android client now has the first on-device solve provider using MediaPipe Tasks Vision.

## Current implementation

`MediaPipeFaceTracker` supports:

- IMAGE mode
- VIDEO mode
- 52 blendshape output
- transformation-matrix output enabled
- conversion of blendshape categories into `BdfrFacialFrame`

The resulting curve names align with the BDFR ARKit-style 52 interoperability profile.

## Model

The model binary is not committed.

Place:

```text
app/src/main/assets/face_landmarker.task
```

and construct the tracker with that asset name.

## Next step

CameraX ImageAnalysis will feed the same tracker using MediaPipe `LIVE_STREAM` mode.

That path will then connect:

```text
CameraX
 -> MediaPipe Face Landmarker
 -> BdfrFacialFrame
 -> BdfrPacketCodec
 -> LiveStreamClient
 -> Desktop / UE
```
