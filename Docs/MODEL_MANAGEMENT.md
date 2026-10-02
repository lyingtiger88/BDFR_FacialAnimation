# Model Management

BDFR treats ML models as versioned runtime dependencies rather than anonymous binary files.

`ModelRegistry` tracks model manifests independently from the actual model storage.

This protects several production requirements:

- reproducibility
- license tracking
- deterministic benchmark comparisons
- model rollback
- backend/device selection
- debugging model-version regressions

A model is not considered production-ready until its provenance, license, version and expected input/output contracts are known.
