CPP_BUILD_DIR = build/cpp-service

.PHONY: build proto cpp rust python test test-cpp test-rust test-python

build: proto cpp rust python

proto:
	./scripts/gen-protos.sh

cpp: proto
	cmake -S cpp-service -B $(CPP_BUILD_DIR)
	cmake --build $(CPP_BUILD_DIR)

rust:
	cargo build --target=aarch64-unknown-linux-gnu --manifest-path rust-agent/Cargo.toml

python: proto
	python -m py_compile python-worker/*.py

test: build test-cpp test-rust test-python

test-cpp:
	ctest --test-dir $(CPP_BUILD_DIR)

test-rust:
	cargo test --target=aarch64-unknown-linux-gnu --manifest-path rust-agent/Cargo.toml

test-python:
	pytest || [ $$? -eq 5 ]
