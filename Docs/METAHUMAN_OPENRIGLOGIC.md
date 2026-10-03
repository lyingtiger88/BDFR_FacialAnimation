# MetaHuman / OpenRigLogic Integration

BDFR keeps MetaHuman support optional and isolated from the engine-independent core.

## Architecture

```text
BDFR FacialFrame / ARKit-style curves
              |
              v
      BDFR_MetaHumanRig
              |
              v
       OpenRigLogic 5.8
              |
       MetaHuman DNA
              |
       +------+------+
       |             |
       v             v
   Joint outputs   Blendshape channels
       |
       +--> Animated maps
```

The BDFR Core does not depend on Epic code. OpenRigLogic is only fetched and linked when:

```text
-DBDFR_ENABLE_OPENRIGLOGIC=ON
```

## Runtime responsibilities

The adapter:

- loads a MetaHuman `.dna` file
- builds a RigLogic and RigInstance
- exposes DNA metadata
- discovers raw control names
- heuristically maps BDFR/ARKit curve names to DNA raw controls
- evaluates the MetaHuman rig
- returns joint output values
- returns named blendshape output values
- returns named animated-map values

## Mapping

Mapping is intentionally profile-friendly.

The bootstrap mapper normalizes names and handles common left/right suffix variants so names such as:

```text
jawOpen
eyeBlinkLeft
eyeBlinkRight
mouthSmileLeft
mouthSmileRight
```

can resolve against MetaHuman raw-control names that include prefixes such as `CTRL_expressions_`.

A future explicit MetaHuman profile layer can override heuristic mappings per character.

## Studio

When Studio is built with OpenRigLogic enabled, the MetaHuman panel can load a DCC-exported MetaHuman head DNA file and display the evaluated rig state.

The actual textured MetaHuman mesh is not embedded in the BDFR repository. MetaHuman assets remain external inputs with their own asset license.
