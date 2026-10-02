# Live Session Receiver

The live receiver combines the lower-level network/timing primitives into one desktop-facing pipeline.

```text
UDP BDFP Packet
      |
      v
Sequence Tracking
      |
      +--> packet-loss / out-of-order metrics
      |
      v
Clock Offset Estimator
      |
      v
Local Timestamp Conversion
      |
      v
Jitter Buffer
      |
      v
Playback-ready FacialFrame
```

This is the intended base for:
- Desktop live preview
- Android-to-PC streaming
- UE Live Link bridge
- live recording
- network diagnostics

The receiver keeps latency bounded by relying on timestamped frames and a configurable jitter delay rather than an ever-growing queue.
