# Pitch / F0 Tracking

SpeechFace now has a dependency-free bootstrap pitch tracker based on normalized autocorrelation.

Current output per window:
- estimated F0 in Hz
- voiced/unvoiced flag
- correlation confidence
- timestamp and duration

The implementation is intended as a deterministic baseline and test oracle, not the final production speech model.

Pitch will feed future:
- phrase intonation
- question/rising-tone behavior
- emphasis
- head motion
- brow activity
- expressive TTS alignment
- emotion/performance analysis

Higher-quality production paths can replace this estimator with an ML/ONNX pitch model while preserving the same `PitchFrame` contract.
