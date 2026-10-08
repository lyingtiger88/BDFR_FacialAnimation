# MetaHuman Full Viewport Test

BDFR Studio now evaluates both blendshape and joint deformation for MetaHuman DNA meshes.

## Implemented

- DNA mesh extraction
- mesh selector for all meshes in the DNA
- vertex positions / normals / UVs / topology
- blendshape target extraction
- per-vertex skin weights
- joint hierarchy
- neutral joint transforms
- RigLogic quaternion joint deltas
- CPU linear blend skinning
- tangent-space basis generation
- BaseColor map
- Normal map
- Roughness map
- Specular map
- skin-oriented wrap-light / subsurface approximation
- live RigLogic deformation from BDFR FacialFrame input

## Studio test

1. Launch BDFR Studio.
2. Click **Load MetaHuman DNA...**
3. Select a valid MetaHuman Head DNA file.
4. Use the mesh dropdown to inspect head / eye / teeth / other DNA meshes.
5. Load the matching exported maps:
   - BaseColor
   - Normal
   - Roughness
   - Specular
6. Switch to **MetaHuman 3D**.
7. Start the BDFR receiver.
8. Connect the Android capture client.
9. Enable Live tracking.

Expected behavior:

- facial blendshapes deform the selected DNA mesh
- RigLogic joint outputs additionally skin the mesh
- eye/jaw/head-adjacent joint-driven motion is reflected where the DNA weights reference those joints
- loaded material maps influence the viewport shading

## Current rendering scope

The viewport renders one selected DNA mesh at a time. This makes it possible to validate head, eyes and teeth independently with the exact DNA topology and weights.

The next renderer expansion is a composed multi-mesh scene with per-mesh material assignments so head, eyes, teeth and other meshes can be shown simultaneously.
