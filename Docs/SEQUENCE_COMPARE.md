# Sequence Comparison

BDFR sequence comparison samples two facial sequences on a shared timeline and reports quantitative differences.

Current metrics:

- overall mean absolute error
- overall maximum absolute error
- per-curve mean absolute error
- per-curve maximum absolute error
- sample count
- comparison duration and sample rate

This is useful for:

- RAW vs Filtered comparison
- source vs retargeted comparison
- regression testing
- solver version comparison
- mobile solve vs desktop Studio solve
- key-reduction quality checks

The comparison layer is deterministic and independent from any rendering engine.
