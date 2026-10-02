# Procedural Facial Behavior

The procedural behavior layer keeps a character subtly alive when there is no explicit authored facial action.

The first deterministic implementation produces:

- blink envelope
- low-amplitude gaze drift
- micro eye-dart motion
- subtle head yaw/pitch/roll

## Personality seed

The sampler accepts a stable personality seed.

Two characters using the same dialogue can therefore have different phase and timing behavior while remaining deterministic across replays.

## Important design rule

Procedural behavior is a **layer**, not a destructive edit.

It can be:
- muted
- weighted
- region masked
- overridden by mocap
- overridden by manual animation
- disabled for cinematic cleanup

Future versions will add statistically varied blink timing, attention targets, emotional modifiers and speaking-state modifiers.
