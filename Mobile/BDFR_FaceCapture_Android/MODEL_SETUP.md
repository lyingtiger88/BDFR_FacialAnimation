# Face Landmarker Model Setup

The Android app no longer requires the Face Landmarker model to be committed to Git.

It supports two sources:

1. Bundled asset:
   `app/src/main/assets/face_landmarker.task`
2. Downloaded private app model:
   `files/models/face_landmarker.task`

The **Get Model** action downloads the recommended MediaPipe Face Landmarker bundle from Google's official MediaPipe model storage.

The downloaded file is loaded using MediaPipe `BaseOptions.setModelAssetBuffer()` with a memory-mapped buffer.

This keeps the model separate from BDFR source history while still allowing a normal installed APK to become capture-ready.
