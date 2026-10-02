# Frame Rate and Timecode

BDFR now has an engine-independent frame-rate/timecode foundation.

Built-in rates:

- 24
- 25
- 30
- 50
- 60
- 30000/1001 (~29.97)
- 60000/1001 (~59.94)

## Uses

- Take metadata
- Timeline snapping
- mocap/audio synchronization
- mobile/desktop capture alignment
- virtual production
- DCC export
- Unreal/Sequencer integration

## Current limitation

The initial display timecode uses nominal non-drop frame numbering.

Explicit SMPTE drop-frame handling for 29.97/59.94 is a separate planned feature so that drop-frame semantics are never silently inferred.
