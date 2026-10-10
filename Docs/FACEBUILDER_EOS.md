# BDFR FaceBuilder / eos

BDFR FaceBuilder is the optional photo-to-3D-face pipeline built on top of the eos 3D Morphable Model fitting library.

## Pipeline

```text
Photo + 2D landmarks
        |
        v
eos 3DMM fitting
        |
        +--> camera pose
        +--> identity coefficients
        +--> personalized 3D mesh
        +--> UV texture extraction
        |
        v
BDFR Studio FaceBuilder Preview
```

## Licensing

The eos source code is Apache-2.0.

The low-resolution Surrey Face Model distributed in the eos repository has separate licensing terms and is not bundled into BDFR Studio releases. BDFR Studio asks the user for a compatible morphable-model file and landmark mapping file.

The BDFR CI may use eos test assets only for automated integration validation.

## Initial input workflow

The first production-facing slice uses:

- source photo
- 68-point ibug `.pts` landmarks
- eos-compatible morphable model `.bin`
- landmark mapping `.txt`

A desktop landmark detector can later remove the explicit `.pts` step.
