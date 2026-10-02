# Neutral Drift Correction

Long facial-capture sessions can slowly move away from the actor's calibrated neutral state because of:

- tracker bias
- changing lighting
- camera movement
- thermal/device changes
- gradual pose changes

BDFR now has a conservative online neutral-baseline estimator.

## Safety rule

The baseline adapts only when a curve is already inside a configurable neutral band.

This prevents obvious active expressions such as:

- smile
- strong jaw opening
- blink
- brow raise

from becoming the new neutral reference.

## Controls

- baseline adaptation alpha
- neutral threshold
- maximum allowed baseline correction
- input frame confidence

The correction is non-destructive and can also be applied to a complete FacialSequence offline.
