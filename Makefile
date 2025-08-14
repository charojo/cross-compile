PROTO = proto/data.proto
CPP_BUILD_DIR = build/cpp-service

.PHONY: build test

build:
	./scripts/gen-protos.sh
	cmake -S cpp-service -B $(CPP_BUILD_DIR)
	cmake --build $(CPP_BUILD_DIR)
	cargo build --target=aarch64-unknown-linux-gnu --manifest-path rust-agent/Cargo.toml
	python -m py_compile python-worker/worker.py

test:
	./scripts/gen-protos.sh
	cmake -S cpp-service -B $(CPP_BUILD_DIR)
	cmake --build $(CPP_BUILD_DIR)
	ctest --test-dir $(CPP_BUILD_DIR)
	cargo test --target=aarch64-unknown-linux-gnu --manifest-path rust-agent/Cargo.toml
	pytest || [ $$? -eq 5 ]
	python -m py_compile python-worker/worker.py
