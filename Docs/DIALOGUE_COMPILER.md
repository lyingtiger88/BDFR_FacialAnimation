# Dialogue-to-Timeline Compiler

The dialogue compiler converts parsed text performance markup into the multi-track BDFR Timeline.

Current compilation path:

```text
Dialogue Markup
    |
    +--> Text Track
    +--> Generated Speech / Viseme Track
    +--> Emotion Track
    +--> Instant Event Track
    +--> Gaze / Head Track
```

Example:

```text
[emotion=angry intensity=0.7]
Where have you been?
[pause=0.4]
[blink]
[gaze=player]
```

produces editable Timeline clips rather than one baked animation.

This is the first end-to-end text-to-Timeline pipeline. The current text speech planner is still a deterministic fallback and will later be replaced/augmented with language-specific G2P, TTS timing and audio alignment.
