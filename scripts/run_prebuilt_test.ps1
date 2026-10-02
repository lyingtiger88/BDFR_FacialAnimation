$ErrorActionPreference = "Stop"

Write-Host "=== BDFR Prebuilt Initial Test ==="

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $Root

$Smoke = Join-Path $Root "bdfr_initial_smoke_tests.exe"
$Cli = Join-Path $Root "bdfr_cli.exe"

if (-not (Test-Path $Smoke)) {
    throw "bdfr_initial_smoke_tests.exe not found next to this script."
}

if (-not (Test-Path $Cli)) {
    throw "bdfr_cli.exe not found next to this script."
}

Write-Host ""
Write-Host "1/2 End-to-end smoke test"
& $Smoke
if ($LASTEXITCODE -ne 0) {
    throw "BDFR smoke test failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "2/2 CLI text pipeline"
& $Cli text "Hello BDFR"
if ($LASTEXITCODE -ne 0) {
    throw "BDFR CLI text test failed with exit code $LASTEXITCODE."
}

Write-Host ""
Write-Host "BDFR PREBUILT INITIAL TEST PASSED."
