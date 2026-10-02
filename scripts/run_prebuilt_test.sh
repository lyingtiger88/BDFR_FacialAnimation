#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

echo "=== BDFR Prebuilt Initial Test ==="

if [[ ! -x "./bdfr_initial_smoke_tests" ]]; then
  echo "bdfr_initial_smoke_tests not found next to this script." >&2
  exit 2
fi

if [[ ! -x "./bdfr_cli" ]]; then
  echo "bdfr_cli not found next to this script." >&2
  exit 2
fi

echo
echo "1/2 End-to-end smoke test"
./bdfr_initial_smoke_tests

echo
echo "2/2 CLI text pipeline"
./bdfr_cli text "Hello BDFR"

echo
echo "BDFR PREBUILT INITIAL TEST PASSED."
