# Integration Testing and Benchmarking

This document outlines how to bring up the cross-platform demo and run
integration tests and benchmarks.

## Prerequisites

- `aarch64-linux-gnu-g++` and `qemu-aarch64`
- Python 3
- Rust toolchain and Cargo

## Integration Test

```
./scripts/integration_test.sh
```

The script builds all components, launches the C++ `data_service`
under QEMU, and then runs the Python worker and Rust agent on the host.
Each component's output is checked to ensure the expected messages are
observed.  Failures will print diagnostics from the failing component.

## Benchmark Harness

```
python scripts/bench.py
```

The benchmark iterates over several build profiles (O3, Os, LTO and
PGO) compiling the `data_service` for each, executing it under QEMU and
capturing runtime and maximum RSS. Results are written to
`bench_results.csv` and `bench_results.html` in the repository root.

## Troubleshooting

- Ensure QEMU and the cross compiler are installed and in your `$PATH`.
- Remove the `bench-build` directory if you need a clean benchmarking
  run.
- Use `make clean` to rebuild components from scratch if builds
  behave unexpectedly.
