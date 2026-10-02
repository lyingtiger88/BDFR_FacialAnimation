# SpeechFace Provider Architecture

BDFR now separates SpeechFace contracts from concrete speech models.

Provider interfaces:

- `IGraphemeToPhonemeProvider`
- `IPhonemeAlignmentProvider`
- `ITtsTimingProvider`
- `IAudioPhonemeProvider`

This allows the deterministic bootstrap implementation to be replaced by:

- multilingual G2P
- forced alignment
- cloud/local TTS phoneme timing
- ONNX phoneme recognition
- language-specific research models

without changing the Timeline, Viseme Synthesizer, Expression Engine or Retargeter.

## Language profiles

`LanguageProfile` maps provider phoneme tokens onto the BDFR viseme system.

The first built-in profile is an English bootstrap profile.

Future profiles should be data-driven and versioned, including Persian and Arabic rather than forcing every language through English phonetics.
