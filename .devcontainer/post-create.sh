#!/usr/bin/env bash
set -e

# Create initial build directories
mkdir -p build cpp-service/build python-worker/build rust-agent/target

# Generate code from protobuf to prime IntelliSense
protoc --proto_path=proto --cpp_out=cpp-service --python_out=python-worker proto/data.proto

# Create a local Cargo registry for offline dependency resolution
cargo local-registry --sync rust-agent/Cargo.lock /workspace/.cargo/local-registry
