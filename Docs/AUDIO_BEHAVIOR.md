# Audio-Driven Behavior Cues

The first audio behavior planner converts prosody into editable high-level cues.

Current cue types:
- Emphasis
- Head Nod
- Blink
- Breath

Rules are intentionally transparent and deterministic:
- emphasis regions may create a head-nod cue
- sufficiently long pauses may create blink candidates
- longer pauses may create breath candidates

These cues are **not baked animation**.

They are intended to feed:
- Instant Event tracks
- Head/Gaze tracks
- Procedural Behavior
- future learned performance models

This keeps automatic behavior editable by an animator.
