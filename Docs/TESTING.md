# Testing

The first executable test target is **bdfr_core_tests**.

## Local build

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Current coverage

The bootstrap suite checks:

- canonical FACS/practical curve registration
- custom curve registration
- facial-frame validation
- curve sanitization and clamping
- binary frame encode/decode roundtrip
- malformed/truncated binary rejection
- weighted override mixing
- temporal exponential smoothing
- timeline track creation
- Text/Dialogue track clips
- Emotion track clips
- Instant Event clips
- Mocap track clips
- timeline validation and duration
- timeline evaluation at time
- Mute and Solo behavior
- track/clip priority ordering
- facial-region masks
- region-specific curve fusion
- Session/Take model
- duplicate Take rejection
- partial re-solve dirty ranges
- dirty-range merging

## CI

GitHub Actions runs the core build/test job on:

- Ubuntu
- Windows

## Next test expansion

Before M1 is considered complete, add:

- deterministic codec golden vectors
- endian-safe codec format
- schema migration tests
- duplicate/invalid timeline clip tests
- track mute/solo evaluation tests
- region masks and priority tests
- partial re-solve range tests
- stress tests for large curve streams
- sanitizer jobs where supported
