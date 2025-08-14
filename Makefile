.PHONY: build test

build: ; g++ -std=c++17 cpp-service/DataService.cpp -o cpp-service/data_service && cargo build --manifest-path rust-agent/Cargo.toml && python -m py_compile python-worker/worker.py

test: ; cargo test --manifest-path rust-agent/Cargo.toml && python -m py_compile python-worker/worker.py
