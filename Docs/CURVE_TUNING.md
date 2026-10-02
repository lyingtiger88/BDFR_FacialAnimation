# Curve Tuning

Curve Tuning is the artist-facing post-solve shaping layer.

Each curve can independently define:
- input range
- gain
- bias
- response exponent
- dead zone
- output clamp

This lets an animator or technical artist tailor solved animation to:
- a specific actor
- a stylized character
- a limited rig
- a target expression range

The tuning layer is non-destructive and sits between solve/fusion and final retarget/application.

Typical use:

```text
Solver
  -> Calibration
  -> Performance Fusion
  -> Curve Tuning
  -> Correctives
  -> Retargeter
```
