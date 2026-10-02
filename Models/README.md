# BDFR Model Assets

Do **not** commit large or externally licensed model binaries to this directory without an explicit licensing/provenance review.

Every production model should have a manifest containing:

- model ID
- version
- file name
- SHA-256 checksum
- license
- source/provenance
- expected runtime backend
- input contract
- output contract

Example logical entries:

```text
face-landmarker@1
speech-phoneme-en@1
speech-phoneme-fa@1
pitch-model@1
emotion-audio@1
```

The current Android MediaPipe Face Landmarker model is intentionally excluded from Git history. Its runtime asset should be provisioned separately.

Future model tooling will add:
- local model cache
- checksum verification
- supported-device information
- rollback
- CPU/GPU/NPU selection
- benchmark metadata
