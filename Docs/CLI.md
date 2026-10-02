# BDFR CLI

The first command-line utility exposes core functionality without Unreal Engine or a desktop GUI.

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Text speech draft

```bash
bdfr_cli text "Where have you been?"
```

Outputs a timed event list from the current text-to-viseme fallback.

## Inspect project

```bash
bdfr_cli inspect project.bdfr.json
```

Outputs:
- project name
- schema version
- actor count
- session count
- take count

Future CLI commands will support:
- batch audio processing
- offline video solve
- retarget/export
- benchmark runs
- project migration
