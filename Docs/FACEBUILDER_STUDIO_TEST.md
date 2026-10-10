# BDFR FaceBuilder — Photo to 3D Face

BDFR Studio can create a personalized 3D face using the optional eos backend.

## Current workflow

1. Open BDFR Studio.
2. In **FaceBuilder / eos**, click **Create 3D Face from Photo…**
3. Select a face photo.
4. Select a 68-point ibug `.pts` landmark file for that photo.
5. Select an eos-compatible morphable-model `.bin`.
6. Select the matching landmark mapping file.
7. BDFR fits the 3DMM identity shape, estimates camera pose, generates a personalized mesh and extracts a UV texture from the source photo.
8. The result is shown directly in the existing 3D viewport.

## Output

The FaceBuilder runtime returns:

- personalized render vertices
- triangle topology
- normals
- UV coordinates
- extracted RGBA face texture
- fitted shape coefficients
- yaw / pitch / roll
- number of valid landmark correspondences used

## CI

The eos integration test uses the sample assets distributed by the eos project strictly as automated integration-test data.

The BDFR release does **not** bundle the Surrey Face Model. Studio asks the user for a compatible model file and mapping file because the model asset has licensing terms separate from the Apache-2.0 eos source code.

## Next automation slice

The explicit `.pts` input is the first deterministic desktop workflow. The next step is to add an automatic desktop landmark provider so **Photo → 3D Face** becomes one-click.
