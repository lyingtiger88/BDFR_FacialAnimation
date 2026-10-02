# Actor Calibration

BDFR calibration separates actor-specific motion range from the canonical normalized curve representation.

A profile stores per-curve:
- minimum observed value
- neutral value
- maximum observed value
- optional inversion

## Intended workflow

```text
Neutral
Smile
Jaw Open
Blink
Brow Up
Brow Down
Pucker
Range-of-motion poses
       |
       v
CalibrationProfile
       |
       v
Raw tracker / solver values
       |
       v
Normalized BDFR curves
```

Left and right curves can use independent ranges, preserving natural asymmetry.

The current implementation provides the normalization core. Capture-guided calibration UI and robust statistical range estimation come later.
