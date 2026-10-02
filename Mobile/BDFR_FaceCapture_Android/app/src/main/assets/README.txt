Place the MediaPipe Face Landmarker model asset in this directory as:

face_landmarker.task

The model binary is intentionally not committed to this repository.

BDFR Android expects a model asset path to be supplied to MediaPipeFaceTracker.

Official MediaPipe Face Landmarker supports:
- IMAGE
- VIDEO
- LIVE_STREAM
- 3D face landmarks
- 52 face blendshape scores
- facial transformation matrices

BDFR currently implements IMAGE and VIDEO helper modes first. Camera LIVE_STREAM integration is the next mobile slice.
