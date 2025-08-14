PROTO = proto/data.proto
CPP_BUILD_DIR = build/cpp-service

.PHONY: build test

build:
	protoc -I proto --cpp_out=cpp-service --python_out=python-worker $(PROTO)
	cmake -S cpp-service -B $(CPP_BUILD_DIR)
	cmake --build $(CPP_BUILD_DIR)
	cargo build --target=aarch64-unknown-linux-gnu --manifest-path rust-agent/Cargo.toml
	python -m py_compile python-worker/*.py

test:
	ctest --test-dir $(CPP_BUILD_DIR)
	cargo test --target=aarch64-unknown-linux-gnu --manifest-path rust-agent/Cargo.toml
	pytest || [ $$? -eq 5 ]
