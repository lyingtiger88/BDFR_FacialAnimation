# Prebuilt Initial Test

The CI-generated initial-test artifacts are intended to run without building the source tree.

## Windows artifact

Extract `BDFR_Initial_Test_Windows.zip`.

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\run_prebuilt_test.ps1
```

Expected final line:

```text
BDFR PREBUILT INITIAL TEST PASSED.
```

## Linux artifact

Extract `BDFR_Initial_Test_Linux.zip`.

Run:

```bash
chmod +x run_prebuilt_test.sh bdfr_cli bdfr_initial_smoke_tests
./run_prebuilt_test.sh
```

## What is exercised

The prebuilt smoke test verifies the first integrated runtime path:

- dialogue parsing
- Timeline compilation
- facial-curve generation
- ARKit52 retargeting
- BDFP binary encoding/decoding
- UDP loopback
- clock/jitter LiveSessionReceiver
- BDFS solved-session serialization
- deterministic sequence comparison

No camera, phone, Unreal Engine install, or ML model is required for this desktop smoke test.
