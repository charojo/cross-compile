#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROTO_DIR="$SCRIPT_DIR/../proto"
CPP_OUT="$SCRIPT_DIR/../cpp-service/generated"
PY_OUT="$SCRIPT_DIR/../python-worker"

mkdir -p "$CPP_OUT"
protoc -I"$PROTO_DIR" --cpp_out="$CPP_OUT" "$PROTO_DIR/data.proto"
protoc -I"$PROTO_DIR" --python_out="$PY_OUT" "$PROTO_DIR/data.proto"
# Rust code is generated via build.rs during cargo build
