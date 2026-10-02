# BDFR FaceCapture Android

Dedicated Android capture client for BDFR Facial Animation.

## Bootstrap status

Implemented:
- Android application skeleton
- AGP 9.4 / Gradle 9.6 configuration
- API 37 compile/target configuration
- CameraX 1.6.2
- front-camera PreviewView
- runtime camera permission
- Record / Live / Calibrate UI entry points

Not implemented yet:
- face landmark model
- on-device FACS solve
- video recording
- prerecorded-video import
- BDFR packet encoder
- Wi-Fi streaming
- USB tunnel
- QR pairing
- local session database
- remote desktop controls

## Intended pipeline

```text
Front Camera / Imported Video
        |
        v
Face Tracker
        |
        v
Mobile Face Solver
        |
        v
BDFR FacialFrame
        |
        +--> Local recording
        +--> Live packet stream
        +--> PC high-quality re-solve
```

The mobile app will implement the binary packet format documented in `Docs/STREAM_PROTOCOL.md`.

## Toolchain baseline

- Android Gradle Plugin 9.4.0
- Gradle 9.6
- JDK 17
- compileSdk / targetSdk 37
- CameraX 1.6.2

A Gradle wrapper JAR is intentionally not committed in this bootstrap; Android Studio can sync the project or a wrapper can be generated locally.
