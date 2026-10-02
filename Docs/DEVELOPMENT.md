# Development Plan

## Phase 1 implementation order

1. common facial frame schema
2. curve registry and validation
3. interpolation/smoothing
4. expression mixer
5. serialization
6. synthetic test generator
7. UE5 receiver
8. simple retarget profile
9. offline SpeechFace prototype
10. benchmark harness

## Initial languages

Planned:
- **C++** for core/runtime and Unreal integration
- **Python** for research, model experiments, data preparation and evaluation

Additional languages should be added only when they provide a clear benefit.

## Coding rules

- core code must not include Unreal headers
- no hidden global mutable solver state
- timestamps are explicit
- curve ranges are validated
- public interfaces are versioned
- allocations in hot real-time loops are minimized
- logging levels are standardized
- every model/dependency has license documentation

## Testing strategy

### Unit tests
- curve validation
- range clamps
- interpolation
- serialization
- retarget mapping
- timeline operations

### Golden tests
Known input sequence → known curve output.

### Performance tests
- processing time
- throughput
- allocation count where feasible
- latency

### Visual tests
Reference clips rendered through a stable test character.

## Branching

Initial recommendation:
- `main` — stable integration
- short-lived feature branches
- pull requests for substantial changes

## Commit style

Examples:
- `feat: add FACS curve registry`
- `feat(ue): add facial frame receiver`
- `fix(speech): preserve bilabial closure`
- `perf(runtime): reduce frame allocations`
- `docs: define calibration pipeline`
- `test: add coarticulation regression clips`

## Dependency policy

Before adding a dependency, document:
- why it is needed
- license
- maintenance status
- supported platforms
- binary size impact
- runtime cost
- replacement risk

## ML research workflow

Research implementations may begin in Python.

A model graduates to production only when:
- inference interface is documented
- preprocessing is reproducible
- model license is acceptable
- model version is fixed
- benchmark quality is recorded
- runtime performance is measured

## Unreal integration policy

The Unreal plugin should be an adapter over the core.

Responsibilities:
- receive frames
- map time
- apply curves
- manage components/assets
- expose Blueprint/C++ interfaces
- provide debug visualization

It should not become the only implementation of core solving logic.

## First implementation target

The first tangible demo should be:

```text
Synthetic/Recorded Facial Curves
              |
              v
       BDFR Core Runtime
              |
              v
        Retarget Profile
              |
              v
          UE5 Character
```

Once this path is stable, SpeechFace and FaceCapture can share the same output pipeline.
