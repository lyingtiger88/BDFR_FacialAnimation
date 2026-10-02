# BDFR Live Link Source Foundation

BDFR now includes the first native Unreal Live Link source class.

## Current data role

The source publishes BDFR facial curves through the Live Link **Basic Role**:

- static data = ordered curve/property names
- frame data = normalized property values
- source timestamp = BDFR frame timestamp

This is suitable for expression curves before a character-specific retarget stage.

## Threading

The source intentionally separates ingestion from Live Link update:

```text
Network / BDFR Receiver Thread
        |
        v
EnqueueFrame()
        |
        v
Small bounded thread-safe queue
        |
        v
ILiveLinkSource::Update()
        |
        v
PushSubjectFrameData_AnyThread()
```

No socket receive, inference or blocking operation belongs in `Update()`.

Epic documents `ILiveLinkSource::Update()` as a critical game-thread path that should return quickly.

## Next UE slice

- BDFR UDP receiver worker
- Live Link source factory and source-creation UI
- configurable UDP port / subject name
- source/network statistics
- head-pose subject or transform role
- MetaHuman retarget profile
- UE integration build verification

The Unreal plugin is structural at this stage because this repository's CI does not include an Unreal Engine SDK.
