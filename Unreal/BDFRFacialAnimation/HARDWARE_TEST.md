# UE5 Hardware Test — BDFR Android / UDP / Live Link

The plugin now contains a runtime `UBDFRLiveReceiverComponent`.

## Character setup

On an Actor, add:

1. `BDFRFacialComponent`
2. `BDFRLiveReceiverComponent`

Configure:

- Listen Port: `5000`
- Auto Start: enabled
- Publish Live Link: enabled
- Live Link Subject Name: `BDFR_Face`

The receiver thread decodes native BDFP packets off the Game Thread.

On Tick it collapses any backlog to the newest frame and:

- updates `BDFRFacialComponent`
- publishes the curve frame to the BDFR Live Link source

## Android network diagnostic

Before requiring face tracking:

1. Run the UE level.
2. Enter the PC IP and port 5000 on Android.
3. Tap **Test PC**.
4. Inspect `BDFRLiveReceiverComponent.GetStats()`.

Expected:

- PacketsReceived increases
- LastCurveCount is 6
- DecodeFailures remains zero

## Live tracking

After the Face Landmarker model is ready:

1. Tap **Live** on Android.
2. Blink / smile / open jaw.
3. `PacketsReceived` should rise continuously.
4. Live Link should expose subject `BDFR_Face`.
5. Direct BDFR facial curves are also available through `BDFRFacialComponent`.

## Threading

```text
Android UDP
   |
   v
UE socket worker thread
   |
   v
BDFP decode
   |
   v
MPSC frame queue
   |
   v
Game-thread Tick (latest frame only)
   +--> BDFRFacialComponent
   +--> FBDFRLiveLinkSource
```

Socket receive and packet parsing do not block the Game Thread.

## Current limitation

The Unreal plugin is not compiled in repository CI because no Unreal Engine SDK is installed on those runners. The Core protocol and Android sender are compile/integration tested separately.
