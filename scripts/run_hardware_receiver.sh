#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

RECEIVER="./bdfr_live_receiver"
CLI="./bdfr_cli"
RECORDING="./phone_test.bdfs"

echo "=== BDFR Android -> PC Hardware Test ==="
echo
echo "On Android enter this PC IPv4 address and UDP port 5000."
echo "Tap Test PC first, then Live after tracking is ready."
echo
echo "Listening for 60 seconds..."

"$RECEIVER" --port 5000 --duration 60 --record "$RECORDING" --no-curves

if [[ -x "$CLI" && -f "$RECORDING" ]]; then
  echo
  echo "=== Recorded session ==="
  "$CLI" inspect-session "$RECORDING"
fi

echo
echo "BDFR Android -> PC hardware transport test completed."
