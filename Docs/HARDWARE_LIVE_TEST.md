# Hardware Live Test — Android to PC

This is the first real-device network test.

## PC

Run:

```text
bdfr_live_receiver --port 5000 --duration 30 --record phone_test.bdfs
```

Expected startup:

```text
BDFR LIVE RECEIVER READY
UDP port: 5000
Waiting for Android / mocap BDFP packets...
```

## Android

1. Put the phone and PC on the same LAN/Wi-Fi.
2. Enter the PC IPv4 address in the BDFR app.
3. Use port `5000`.
4. First use **Test PC**. This sends a synthetic BDFP frame and does not require the face model.
5. The PC should immediately print a frame containing diagnostic curves.
6. Once the model is available and tracking is active, press **Live**.
7. Blink, open the jaw, smile and raise the brows.

The PC should show changing values such as:

```text
jawOpen
eyeBlinkLeft
eyeBlinkRight
mouthSmileLeft
mouthSmileRight
browInnerUp
```

## Firewall

If **Test PC** produces no packet:

- allow `bdfr_live_receiver.exe` through Windows Defender Firewall on Private networks
- verify the PC IPv4 address with `ipconfig`
- make sure phone and PC are not isolated by guest Wi-Fi/client isolation
- keep UDP port 5000 identical on both devices

## Recording

With `--record phone_test.bdfs`, every playback-ready frame is stored.

Then inspect:

```text
bdfr_cli inspect-session phone_test.bdfs
```

A successful hardware test proves:

```text
Android
 -> BDFP/UDP
 -> PC socket
 -> packet decoder
 -> sequence tracking
 -> clock sync
 -> jitter buffer
 -> BDFS recording
```

independently of Unreal Engine.
