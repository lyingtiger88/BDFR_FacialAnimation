# Performance Fusion

BDFR can combine multiple facial-performance sources instead of forcing one solver to own the whole face.

Example:

```text
SpeechFace  -> Mouth / Jaw
Live Mocap  -> Eyes / Head
Emotion     -> Brows / Cheeks
Manual      -> selected corrective curves
                    |
                    v
            PerformanceFusion
                    |
                    v
             Final BDFR Curves
```

Each source has:
- enabled state
- priority
- default weight
- per-region weights
- input confidence

This allows graceful fallback when one source becomes unreliable.

Example policy:
- Speech: Mouth 1.0, Jaw 1.0, Eyes 0.0
- Mocap: Eyes 1.0, Mouth 0.3, Jaw 0.3
- Emotion: Brows 0.5, Cheeks 0.4
- Manual: high-priority local overrides

The same mechanism will power live capture fusion and offline Studio re-solving.
