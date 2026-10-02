# Prosody Foundation

BDFR's first prosody layer uses windowed audio energy to establish timing and expressive hooks.

Current outputs:
- normalized energy
- speech-active flag
- silence windows
- emphasis candidates
- active/silent duration summary

This is intentionally a bootstrap implementation.

Future SpeechFace versions will extend it with:
- pitch / F0
- voiced/unvoiced probability
- speaking rate
- phrase boundaries
- breath detection
- stress/accent estimation
- learned expressive embeddings

The important architectural point is that prosody is represented independently from phoneme/viseme timing, allowing both to influence facial behavior without being baked together.
