# SpeechFace

SpeechFace is the BDFR text/audio-driven facial-animation subsystem.

## Current implementation

The first implementation is a deterministic **text fallback path**:

```text
Text
 -> orthographic token timing
 -> rough viseme classification
 -> timed SpeechEvents
 -> viseme curve poses
 -> simple neighbor blending
 -> BDFR facial curves
```

This path is intentionally a fallback/prototyping layer. It is **not** the intended final phoneme/G2P quality.

## Why it exists

It establishes the stable contracts required by future systems:

- Text -> timed speech events
- speech event -> viseme
- viseme -> BDFR curves
- time sampling
- coarticulation hook points

Future layers can replace token classification with:

- language-specific grapheme-to-phoneme
- forced alignment
- TTS phoneme timing
- audio phoneme recognition
- sequence models

without changing the downstream Timeline or Retargeter contracts.
