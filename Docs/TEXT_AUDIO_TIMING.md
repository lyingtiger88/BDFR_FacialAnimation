# Text + Audio Timing Foundation

BDFR now has a simple transcript-duration fitting layer.

```text
Text
 -> SpeechEvent draft
 -> target audio duration
 -> time-scaled SpeechEvents
 -> Viseme / Timeline
```

This is useful when:
- transcript text is available
- audio duration is known
- a full forced-alignment model is not yet available

It is intentionally a baseline, **not** production forced alignment.

Future alignment will replace global time scaling with:
- voice-activity anchors
- word boundaries
- phoneme boundaries
- CTC / forced-alignment models
- TTS-supplied phoneme timestamps

The downstream Timeline does not need to change when the alignment provider improves.
