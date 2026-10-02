# Mocap CSV Import / Export

BDFR now supports a simple curve-table CSV interchange format.

Example:

```csv
time,jawOpen,eyeBlinkLeft,eyeBlinkRight,mouthSmileLeft
0.000,0.10,0.00,0.00,0.05
0.033,0.15,0.00,0.00,0.08
0.066,0.20,0.85,0.82,0.10
```

## Import

- timestamp column is configurable
- delimiter is configurable
- all non-time columns become facial curves
- values may optionally be clamped to the canonical 0..1 range
- frames are placed into a timestamped `FacialSequence`

## Export

The exporter builds a deterministic sorted union of all curve names and writes missing curve values as zero.

This format is useful for:
- external mocap tools
- spreadsheets
- debugging
- research datasets
- simple DCC interchange

For compact live/recorded use, prefer BDFP/BDFS binary formats.
