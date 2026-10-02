# Android Offline Video Solve

The Android app can now process a prerecorded local video into solved BDFR facial-animation data.

Pipeline:

```text
Video URI
   |
   v
MediaMetadataRetriever
   |
   v
Timestamped Bitmaps
   |
   v
MediaPipe Face Landmarker (VIDEO mode)
   |
   v
52 Blendshape Curves
   |
   v
BdfrFacialFrame
   |
   v
BDFS Recorded Session
```

The bootstrap solver samples the video at a configurable time step.

The output is solved facial data (`.bdfs`), not a replacement copy of the source video.

## Future Studio improvements

- decode frames through MediaCodec rather than random-access extraction
- preserve source timecode exactly
- adaptive frame rate
- full quality/confidence reports
- local bidirectional cleanup
- send source video plus observations to desktop for higher-quality re-solve
- background WorkManager jobs for long clips
