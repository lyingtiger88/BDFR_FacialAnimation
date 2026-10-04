# Real MetaHuman Mesh + Texture Preview

BDFR Studio can render the actual MetaHuman head mesh stored in a MetaHuman DNA file.

## What comes from DNA

The OpenRigLogic adapter extracts:

- mesh names
- vertex positions
- normals
- UV coordinates
- polygon topology
- blendshape targets
- blendshape channel bindings

Studio triangulates the authored polygons for GPU rendering and keeps the source-position mapping required to apply DNA blendshape deltas.

## What comes from the MetaHuman export

Textures are not stored in the DNA file.

Export the character's materials/textures from MetaHuman Creator / Unreal Engine and select the exported face BaseColor image in Studio.

BDFR does not redistribute Epic-owned MetaHuman character assets in the public repository or test build.

## Studio workflow

1. Open BDFR Studio.
2. In **MetaHuman / OpenRigLogic**, click **Load MetaHuman DNA...**
3. Select the character's head `.dna`.
4. Studio discovers a face/head mesh and extracts the real DNA geometry.
5. Switches the preview to **MetaHuman 3D**.
6. Click **Load BaseColor Texture...**
7. Select the face BaseColor PNG/JPG exported for that same character.
8. Start the Android receiver and enable Live.
9. RigLogic evaluates each FacialFrame.
10. Blendshape output weights deform the actual DNA mesh in the 3D viewport.

## Viewport controls

- Left mouse drag: orbit
- Mouse wheel: zoom
- **BDFR Rig** tab: lightweight diagnostic face
- **MetaHuman 3D** tab: DNA mesh + texture

## Current deformation coverage

The current viewport applies DNA blendshape target deltas from RigLogic output.

RigLogic joint outputs are already evaluated and exposed, but full skeletal skinning of DNA joint transforms is the next rendering slice. This means the first textured viewport is an actual MetaHuman DNA mesh with real blendshape deformation, while joint-driven deformation will become more complete as the viewport skinning path is added.
