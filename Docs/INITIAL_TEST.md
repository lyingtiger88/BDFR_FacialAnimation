# BDFR Initial Test Kit

This is the first milestone intended for hands-on testing before the full desktop editor and final Unreal integration exist.

## What this milestone verifies

The automated smoke test covers:

```text
Dialogue markup
  -> multi-track Timeline
  -> generated facial curves
  -> ARKit52 retarget
  -> BDFP binary packet
  -> UDP loopback
  -> LiveSessionReceiver
  -> clock/jitter path
  -> BDFS recorded session
  -> deterministic SequenceCompare
```

It is deliberately wider than a unit test.

## Windows

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_initial_test.ps1
```

A successful run ends with:

```text
BDFR INITIAL SMOKE TEST PASSED.
BDFR initial desktop test completed successfully.
```

## Linux

```bash
chmod +x scripts/run_initial_test.sh
./scripts/run_initial_test.sh
```

## Android initial test

The Android project is under:

```text
Mobile/BDFR_FaceCapture_Android
```

Automated build:

```bash
gradle :app:testDebugUnitTest
gradle :app:assembleDebug
```

For on-device face solving, provision:

```text
app/src/main/assets/face_landmarker.task
```

Then test these paths on a real phone:

1. Launch the app and grant camera permission.
2. Confirm front-camera preview.
3. Confirm status shows tracking curves/confidence/FPS.
4. Enter the PC IP and UDP port.
5. Start **Live**.
6. On the PC run:
   ```text
   bdfr_cli listen 5000 10000
   ```
7. Move jaw, blink and smile.
8. Confirm the PC receives BDFR curve values.
9. Start **Record**, perform facial motion, stop recording.
10. Confirm a `.bdfs` session is written.
11. Copy it to the PC and run:
    ```text
    bdfr_cli inspect-session recording.bdfs
    ```
12. Use **Import** on the Android app and select a prerecorded face video.
13. Confirm the offline solve completes and emits another `.bdfs` file.

## Unreal initial test

The Unreal plugin currently has:

- `UBDFRFacialComponent`
- Retarget Profile Data Asset
- direct SkeletalMesh morph-target application
- Live Link source foundation

The first Unreal manual test should use a character whose morph names match ARKit-style curves or a manually authored retarget profile.

The UE plugin is not yet compile-verified by repository CI because Unreal Engine itself is not installed on the CI runners.

## Current pass criteria

Desktop/Core initial milestone is considered ready when:

- Windows CI passes
- Ubuntu CI passes
- `bdfr_initial_smoke_tests` passes
- Android CI passes

Hardware milestone is considered ready when:

- Android model loads on a real device
- Android live curves reach the PC
- Android BDFS recording reopens on PC
- prerecorded Android video produces BDFS data

Those hardware checks are intentionally kept separate from compile CI.
