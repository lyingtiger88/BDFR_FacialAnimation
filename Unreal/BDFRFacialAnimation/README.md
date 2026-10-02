# BDFR Unreal Engine Plugin

Initial UE5 runtime plugin skeleton.

Current surface:
- Runtime module
- `UBDFRFacialComponent`
- normalized facial-curve storage
- Blueprint curve setters/getters
- bulk curve update

Next UE tasks:
- BDFR core frame adapter
- SkeletalMesh / AnimInstance curve application
- Retarget Profile asset
- prerecorded sequence playback
- live frame receiver
- Live Link source
- debug visualization
- runtime stats panel

The standalone BDFR C++ core remains engine-independent. Unreal is an adapter layer.
