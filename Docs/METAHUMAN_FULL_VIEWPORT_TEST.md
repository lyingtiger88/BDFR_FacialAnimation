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

The viewport supports both:

- **Full LOD0 Scene** — composes the DNA meshes that belong to LOD0 into one live scene.
- **Individual mesh inspection** — select head, eye, teeth or any other DNA mesh from the dropdown.

The composed scene keeps exact DNA topology, UVs, morph targets, joint weights and RigLogic deformation. Preview material classes distinguish skin, eyes and teeth. Skin uses the loaded BaseColor/Normal/Roughness/Specular maps; eyes and teeth receive dedicated preview shading defaults.

This is still a Studio preview renderer, not a byte-for-byte reproduction of Unreal Engine's MetaHuman material graph. Exact UE material graph parity, strand hair rendering and production eye shaders remain separate rendering milestones.
