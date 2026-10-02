# Sequence Diagnostics and Smart Cleanup

BDFR now has the first automatic cleanup primitives for solved facial sequences.

## Diagnostics

The sequence analyzer reports:
- mean confidence
- minimum confidence
- low-confidence frame count
- isolated curve spikes

A spike is defined conservatively:
- neighboring frames agree reasonably well
- the center frame deviates strongly from their expected value

## Despike cleanup

`SequenceCleanup::despike` replaces only isolated high-magnitude spikes with the average of neighboring values.

It does **not** smooth the entire performance.

This is important because aggressive smoothing can erase:
- plosive closures
- blinks
- micro-expressions
- intentional fast expressions

Future Smart Cleanup will add:
- region-specific thresholds
- blink-aware protection
- speech-event protection
- neutral-drift correction
- jaw-pumping detection
- brow-noise detection
- retarget saturation warnings
