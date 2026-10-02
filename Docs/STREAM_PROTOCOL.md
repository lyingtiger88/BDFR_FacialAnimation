# BDFR Live Frame Packet Protocol

The first binary packet format is used to carry one solved BDFR facial frame between capture clients and runtime consumers.

## Packet v1

```text
uint32  magic      = "BDFP"
uint16  version    = 1
uint64  sequence
uint16  sourceIdLength
bytes   sourceId
uint32  facialFrameLength
bytes   BDFR BinaryCodec facial frame
```

## Design goals

- low overhead
- explicit packet version
- source/device identity
- sequence number for loss/reordering diagnostics
- reuse of the canonical BDFR facial frame
- usable over UDP, TCP, WebSocket binary messages or USB tunnels

## Mobile path

```text
Android FaceCapture
    -> BDFR curves
    -> FramePacketCodec equivalent
    -> Wi-Fi / USB
    -> Desktop receiver
    -> jitter buffer / clock sync
    -> Timeline / Retarget / UE5
```

Future protocol versions may add:
- compression flags
- authentication metadata
- session/take identifiers
- capture confidence summaries
- optional raw landmark payloads
