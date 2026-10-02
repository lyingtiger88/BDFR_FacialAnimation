$ErrorActionPreference = "Stop"

Write-Host "=== BDFR Facial Animation Initial Test ==="

$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root

if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
}

cmake -S . -B build
cmake --build build --config Release

ctest --test-dir build -C Release --output-on-failure

$Smoke = Join-Path $Root "build\Release\bdfr_initial_smoke_tests.exe"
if (-not (Test-Path $Smoke)) {
    $Smoke = Join-Path $Root "build\bdfr_initial_smoke_tests.exe"
}

if (-not (Test-Path $Smoke)) {
    throw "Smoke test executable not found."
}

Write-Host ""
Write-Host "=== Direct smoke-test output ==="
& $Smoke

Write-Host ""
Write-Host "=== CLI text test ==="

$Cli = Join-Path $Root "build\Release\bdfr_cli.exe"
if (-not (Test-Path $Cli)) {
    $Cli = Join-Path $Root "build\bdfr_cli.exe"
}

& $Cli text "Hello BDFR"

Write-Host ""
Write-Host "BDFR initial desktop test completed successfully."
