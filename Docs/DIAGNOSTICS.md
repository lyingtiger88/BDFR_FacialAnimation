# Diagnostics, Logging and Profiling

BDFR uses a shared diagnostics contract so desktop, runtime, mobile and engine adapters can report comparable information.

## Logging levels

- Trace — high-volume internal details
- Debug — development diagnostics
- Info — normal lifecycle/status
- Warning — recoverable degradation
- Error — failed operation or invalid state

Core code writes through `bdfr::Logger` rather than directly depending on an application UI.

Applications may install their own sink to route logs into:
- desktop console
- file logger
- Unreal log
- Android Logcat
- telemetry/debug panels

## Profiling

The core provides:
- `Stopwatch`
- `ScopedTimer`

Production modules should report at least:
- capture duration
- preprocessing duration
- inference duration
- solve duration
- expression-mix duration
- retarget duration
- transport latency
- end-to-end latency

## Naming

Recommended timing labels:

```text
capture.frame
video.decode
face.detect
face.track
face.solve
speech.frontend
speech.phoneme
speech.coarticulation
expression.mix
retarget.apply
network.encode
network.decode
ue.apply
```

## Rule

Performance claims must be backed by measured timings from a declared hardware/software configuration.
