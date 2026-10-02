# External Mocap Adapters

External facial mocap applications should not be coupled directly to MetaHuman or any target character inside BDFR.

The adapter path is:

```text
External Software
      |
      v
External curve names / head / gaze
      |
      v
ExternalMocapProfile
      |
      v
BDFR FacialFrame
      |
      +--> Timeline
      +--> Fusion
      +--> Retargeter
      +--> UE5
```

## Profile rules

Each source rule supports:

- external curve name
- BDFR target curve
- scale
- bias
- min/max clamp
- invert

This is the base for future adapters for:
- MewFace
- custom Retargeter software
- ARKit streams
- OSC curve streams
- Live Link sources
- proprietary studio capture tools where documented APIs are available

## Live source interface

`ILiveMocapSource` provides a transport-agnostic polling contract that returns timestamped `MocapPacket` values.

UDP, TCP, WebSocket, OSC and engine-specific adapters can implement that interface without changing the facial solver.
