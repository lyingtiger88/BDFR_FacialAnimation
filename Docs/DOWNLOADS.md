# Test Build Downloads

BDFR publishes the latest hardware-test builds on a stable GitHub prerelease:

**BDFR Hardware Test — Latest**

Direct downloads:

- Windows: `BDFR_Windows_Hardware_Test_Kit.zip`
- Android: `BDFR_Android_Hardware_Test.apk`

Stable URLs:

```text
https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/download/hardware-test-latest/BDFR_Windows_Hardware_Test_Kit.zip

https://github.com/lyingtiger88/BDFR_FacialAnimation/releases/download/hardware-test-latest/BDFR_Android_Hardware_Test.apk
```

## Publishing policy

When test-relevant Core, runtime, tools, scripts, or Android files are pushed to `main`:

1. Windows Core is built and tested.
2. Android is unit-tested and assembled.
3. The Windows hardware-test kit is packaged.
4. The Android debug APK is renamed consistently.
5. The `hardware-test-latest` tag is moved to the tested commit.
6. The GitHub prerelease assets are replaced using the same filenames.

Therefore documentation and tester instructions can always use the two stable URLs above instead of temporary GitHub Actions artifact links.
