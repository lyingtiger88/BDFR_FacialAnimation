# Live Runtime Foundation

The first live-runtime layer provides timing and buffering primitives shared by desktop, mobile and engine adapters.

## FrameQueue

A bounded thread-safe queue.

When producers outrun consumers:
- oldest frames are discarded
- dropped-frame count is incremented
- memory remains bounded

For facial animation this is preferable to allowing latency to grow forever.

## JitterBuffer

Frames may arrive out of order or with network timing variation.

The jitter buffer:
- sorts frames by timestamp
- holds a configurable playback delay
- releases frames only when they are ready

## ClockOffsetEstimator

Mobile devices, mocap software and the desktop do not share a perfect clock.

The estimator observes:

```text
remote timestamp
local arrival timestamp
```

and maintains a smoothed offset used to convert remote time into local runtime time.

Future network transport layers will use these primitives for UDP/WebSocket/Live Link paths.
