.PHONY: build test

build:
	cmake -S cpp-service -B build/cpp-service
	cmake --build build/cpp-service
	cargo build --manifest-path rust-agent/Cargo.toml
	python -m py_compile python-worker/worker.py

test:
	cmake -S cpp-service -B build/cpp-service
	cmake --build build/cpp-service
	cargo test --manifest-path rust-agent/Cargo.toml
	python -m py_compile python-worker/worker.py
