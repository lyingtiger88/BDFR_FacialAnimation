# Expression Engine

The Expression Engine supplies editable emotional layers and short event-driven facial actions.

## Emotion presets

Initial categorical presets:
- Neutral
- Happiness
- Sadness
- Anger
- Fear
- Surprise
- Disgust
- Contempt

Each preset emits BDFR curves rather than target-rig morph names.

Presets are deliberately data-like starting points. Production versions will support actor calibration, valence/arousal and learned performance models.

## Instant events

Initial event types:
- Blink
- Double Blink
- Brow Flick
- Lip Twitch Left/Right
- Nose Flare
- Jaw Clench

Events are sampled over a start time and duration using a smooth temporal envelope.

These are intended for the Timeline Instant Event track.
