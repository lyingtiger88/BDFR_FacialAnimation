#!/usr/bin/env bash
set -euo pipefail

echo "=== BDFR Facial Animation Initial Test ==="

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

rm -rf build

cmake -S . -B build
cmake --build build --config Release -j

ctest --test-dir build -C Release --output-on-failure

SMOKE="$ROOT/build/bdfr_initial_smoke_tests"
CLI="$ROOT/build/bdfr_cli"

echo
echo "=== Direct smoke-test output ==="
"$SMOKE"

echo
echo "=== CLI text test ==="
"$CLI" text "Hello BDFR"

echo
echo "BDFR initial desktop test completed successfully."
