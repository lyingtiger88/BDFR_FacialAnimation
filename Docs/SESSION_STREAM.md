# BDFS Recorded Facial Session Stream

BDFS is the first compact recorded live-session container.

Header:

```text
"BDFS"
uint16 version
uint32 packetCount
```

Then, for each frame:

```text
uint32 packetLength
bytes  BDFP packet
```

Because each entry is an ordinary BDFP live packet:
- Android live and record use the same facial data
- desktop can replay mobile recordings
- source ID and sequence numbers are retained
- the contained FacialFrame remains engine-independent

This format is intended for solved facial data, not raw camera footage.
Raw video/audio assets remain separate Take sources.
