# Autosave and Recovery Foundation

BDFR recovery snapshots use the same versioned project JSON format as normal project persistence.

## Rotation

A recovery set is written as:

```text
project.recovery.0.bdfr.json   newest
project.recovery.1.bdfr.json
project.recovery.2.bdfr.json
...
```

Before a new snapshot is written, older snapshots are rotated.

The newest snapshot itself is written through the atomic persistence path.

## Future desktop behavior

The desktop editor will:
- periodically autosave when a project is dirty
- write recovery snapshots separately from the user project
- detect recovery files on startup
- show original-vs-recovery timestamps
- never overwrite the user's main project until recovery is accepted
