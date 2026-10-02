# Audio Frontend

BDFR now has a dependency-free PCM16 WAV frontend and basic windowed audio feature extraction.

## Supported bootstrap format

- RIFF/WAVE
- PCM integer
- 16-bit samples
- mono or multichannel
- arbitrary valid sample rate

Decoded samples are normalized to `[-1, 1]`.

## Features

Per analysis window:
- RMS energy
- absolute peak
- zero-crossing rate
- start time / duration

These are foundational features for:
- voice activity
- emphasis estimation
- breath candidates
- speaking intensity
- phoneme/viseme model inputs
- audio-driven head / blink behavior

The current frontend is intentionally small and deterministic. Higher-quality SpeechFace stages can later add ONNX/ML models without changing the audio data contract.
