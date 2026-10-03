# BDFR Studio v0.1

BDFR Studio is the first desktop GUI for the BDFR Facial Animation runtime.

## Current live workflow

```text
Android FaceCapture
    |
    | BDFP / UDP
    v
BDFR Studio
    |
    +--> Live connection state
    +--> FPS / confidence
    +--> packet loss / clock offset
    +--> expression gauges
    +--> complete live curve table
    +--> head / gaze values
    +--> BDFS take recording
```

## Launch

Use the prebuilt Windows release when available:

```text
BDFR_Studio_Windows.zip
```

Extract it and run:

```text
BDFR Studio.exe
```

No Qt installation is required for the prebuilt package; the release is deployed with its required Qt runtime libraries.

## Android live test

1. Open BDFR Studio.
2. Leave UDP Port at `5000`.
3. Press **Start Receiver**.
4. Studio displays one or more detected PC IPv4 addresses.
5. On Android, enter the correct PC IPv4 and port `5000`.
6. Press **Test PC**.
7. Studio should move from **LISTENING** to **CONNECTED**.
8. The diagnostic curves should appear in the Live Curves panel.
9. On Android, ensure the face model is ready.
10. Press **Live**.
11. Blink, smile, raise the brows, and open the jaw.

## Recording

Press **Record** while the receiver is active.

Press **Stop & Save** to write an editable `.bdfs` solved-session file.

The recording uses the same BDFR SessionStream format as the CLI and mobile recorder.

## UI panels

### Project / Session

The left dock is the first project/take browser shell. It will become the full Project → Actor → Session → Take navigator.

### Live Face Monitor

The center panel currently shows:

- FPS
- frame confidence
- packet count
- packet loss
- clock offset
- recording state
- jaw opening
- left/right blink
- left/right smile
- inner-brow raise
- head pose
- gaze

### Live Curves

The right dock shows every curve present in the latest `FacialFrame`, sorted by canonical name.

## Build from source

Qt 6.8+ Widgets and Network are required.

```text
cmake -S . -B build-studio -DBDFR_BUILD_STUDIO=ON
cmake --build build-studio --config Release --target BDFRStudio
```

The CI test build currently uses Qt 6.11.2 because its package is reliably available from the automated Qt installer mirror.

## Next GUI milestone

v0.2 will add:

- load/open BDFS take
- playback controls
- take list
- live curve history plots
- timeline shell
- connection log
- retarget-profile selection
- Android pairing helpers
