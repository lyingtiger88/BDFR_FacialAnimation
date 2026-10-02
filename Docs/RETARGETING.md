# Retargeting Core

The BDFR retargeter is engine-agnostic. It converts canonical BDFR curves into names and value ranges expected by a target rig.

## Mapping controls

Each mapping supports:

- source curve
- target morph/bone curve name
- scale
- bias
- minimum / maximum clamp
- dead zone
- inversion

This lets one BDFR solve drive MetaHuman, ARKit-style rigs or custom facial rigs without changing the solver.

## Compatibility scan

A profile can be checked against a set of target curve names.

The compatibility report contains:
- mapped targets
- missing targets
- coverage ratio
- missing-name list

The future Rig Inspector and Auto Mapping tools will build on this core.
