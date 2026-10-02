# Character Personality Profiles

Personality Profiles control how procedural performance is expressed without changing dialogue content.

Initial controls:
- stable personality seed
- articulation scale
- emotion scale
- blink timing
- eye-dart speed/strength
- head-motion period and amplitude

Initial presets:
- Neutral
- Reserved
- Nervous
- Confident
- Expressive

Example:

```text
Same dialogue
    |
    +--> Reserved  -> lower articulation, smaller head movement
    |
    +--> Nervous   -> faster blinks, stronger eye darts
    |
    +--> Confident -> stable gaze, restrained head motion
```

Profiles remain editable data. The built-in presets are starting points, not fixed personality judgments.
