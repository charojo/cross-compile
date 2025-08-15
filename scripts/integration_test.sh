#!/usr/bin/env bash
set -euo pipefail

# Integration test for cross-compile demo
# Builds all components, runs the aarch64 data service under QEMU,
# and launches the Python worker and Rust agent on the host.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$ROOT_DIR"

# Build components
make build

# Run data service under QEMU
DATA_LOG=$(mktemp)
if ! qemu-aarch64 -L /usr/aarch64-linux-gnu cpp-service/data_service >"$DATA_LOG" 2>&1; then
    echo "data_service failed to start" >&2
    cat "$DATA_LOG" >&2 || true
    exit 1
fi
if ! grep -q "Data Service running" "$DATA_LOG"; then
    echo "Unexpected data_service output" >&2
    cat "$DATA_LOG" >&2
    exit 1
fi

echo "data_service output: $(cat "$DATA_LOG")"

# Launch Python worker
PY_OUT=$(python -m worker)
if ! echo "$PY_OUT" | grep -q "Python worker active"; then
    echo "Python worker output unexpected" >&2
    echo "$PY_OUT" >&2
    exit 1
fi
echo "python worker output: $PY_OUT"

# Launch Rust agent
RS_OUT=$(./rust-agent/target/debug/rust-agent)
if ! echo "$RS_OUT" | grep -q "Rust agent ready"; then
    echo "Rust agent output unexpected" >&2
    echo "$RS_OUT" >&2
    exit 1
fi
echo "rust agent output: $RS_OUT"

echo "Integration test completed successfully" 
