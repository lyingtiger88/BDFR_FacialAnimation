# ARKit 52 Interoperability

BDFR includes the complete standard ARKit-style 52 facial blendshape names as canonical curve identifiers.

This enables straightforward interchange with:
- iOS ARKit facial capture
- Unreal / Live Link workflows that use ARKit-style curves
- rigs authored around ARKit naming
- third-party mobile facial capture tools

## Design

BDFR does **not** require every solver to output all 52 curves.

Instead:
- available curves are represented directly
- missing curves may be derived, synthesized or left at zero
- retarget profiles decide how target rigs consume the data

`Arkit52::identityProfile()` creates a one-to-one retarget profile for rigs whose targets already use ARKit naming.

Apple documents ARKit blendshape coefficients as normalized facial-expression values, generally in the 0..1 range, which aligns well with BDFR's canonical normalized curve model.
