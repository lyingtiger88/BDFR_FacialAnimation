# Project Persistence

BDFR project metadata uses a versioned JSON representation and is written through an atomic replacement path.

## Current persisted hierarchy

```text
Project
├── schemaVersion
├── name
├── Actors
└── Sessions
    └── Takes
        ├── actor
        ├── source
        ├── duration
        ├── frame rate
        └── partial re-solve dirty ranges
```

Facial sequences have their own JSON interchange representation and can be associated with Takes as the asset model evolves.

## Atomic save strategy

`Persistence::saveTextAtomic` writes a temporary sibling file first and then renames it over the destination.

This reduces the risk of leaving a partially written project file if an application exits during a save.

## Planned extension

Autosave will use the same persistence layer with:
- rotating recovery snapshots
- last-known-good marker
- session/take dirty state
- recovery discovery at application startup
- explicit schema migration before overwriting older project files
