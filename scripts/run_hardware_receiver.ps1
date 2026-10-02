$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $Root

$Receiver = Join-Path $Root "bdfr_live_receiver.exe"
$Cli = Join-Path $Root "bdfr_cli.exe"
$Recording = Join-Path $Root "phone_test.bdfs"

if (-not (Test-Path $Receiver)) {
    throw "bdfr_live_receiver.exe not found next to this script."
}

Write-Host "=== BDFR Android -> PC Hardware Test ==="
Write-Host ""
Write-Host "1. Keep this window open."
Write-Host "2. On Android enter this PC IPv4 address."
Write-Host "3. Keep UDP port at 5000."
Write-Host "4. Tap Test PC first, then Live after model/tracking is ready."
Write-Host ""
Write-Host "Listening for 60 seconds and recording phone_test.bdfs..."
Write-Host ""

& $Receiver --port 5000 --duration 60 --record $Recording --no-curves

if ($LASTEXITCODE -ne 0) {
    throw "No valid BDFR packets were received. Check IP/firewall/Wi-Fi."
}

if ((Test-Path $Cli) -and (Test-Path $Recording)) {
    Write-Host ""
    Write-Host "=== Recorded session ==="
    & $Cli inspect-session $Recording
}

Write-Host ""
Write-Host "BDFR Android -> PC hardware transport test completed."
