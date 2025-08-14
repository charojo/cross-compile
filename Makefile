.PHONY: build test

build:
  ./scripts/gen-protos.sh && g++ -std=c++17 cpp-service/DataService.cpp -o cpp-service/data_service && cargo build --manifest-path rust-agent/Cargo.toml && python -m py_compile python-worker/worker.py
	cmake -S cpp-service -B build/cpp-service
	cmake --build build/cpp-service
	cargo build --manifest-path rust-agent/Cargo.toml
	python -m py_compile python-worker/worker.py

test:
  ./scripts/gen-protos.sh && cargo test --manifest-path rust-agent/Cargo.toml && python -m py_compile python-worker/worker.py
	cmake -S cpp-service -B build/cpp-service
	cmake --build build/cpp-service
	cargo test --manifest-path rust-agent/Cargo.toml
	python -m py_compile python-worker/worker.py

