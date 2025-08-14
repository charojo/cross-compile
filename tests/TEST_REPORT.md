# Test Report
This document summarizes the latest `make test` run for each merged feature. Include the abbreviated commit ID and a link to the relevant pull request or commit for traceability.

## Command
`make test` run on 2025-08-14 at 06:18 UTC.

## Output
```
./scripts/gen-protos.sh
cmake -S cpp-service -B build/cpp-service
CMake Error at /usr/share/cmake-3.28/Modules/FindPackageHandleStandardArgs.cmake:230 (message):
  Could NOT find Protobuf (missing: Protobuf_LIBRARIES Protobuf_INCLUDE_DIR)
Call Stack (most recent call first):
  /usr/share/cmake-3.28/Modules/FindPackageHandleStandardArgs.cmake:600 (_FPHSA_FAILURE_MESSAGE)
  /usr/share/cmake-3.28/Modules/FindProtobuf.cmake:749 (FIND_PACKAGE_HANDLE_STANDARD_ARGS)
  CMakeLists.txt:16 (find_package)


make: *** [Makefile:15: test] Error 1

```

## Summary
Tests failed: Could NOT find Protobuf (missing: Protobuf_LIBRARIES Protobuf_INCLUDE_DIR).

## Feature Commits
- C++ Data Service skeleton - `7f2ac3f` ([commit](https://github.com/example/commit/7f2ac3f))
- Python worker example - `97dc3be` ([commit](https://github.com/example/commit/97dc3be))
- Rust agent template - _pending_
- Sensor reading storage and command handling - `626feab` ([commit](https://github.com/example/commit/626feab))

## b2458e5
- **Commit**: [`b2458e5`](../../commit/b2458e5) (PR #25: split build into subtargets for languages)
- **Date**: 2025-08-14
- **Result**: `make test` failed – Could NOT find Protobuf (missing: Protobuf_LIBRARIES Protobuf_INCLUDE_DIR)

## TBD
- **Commit**: [`TBD`](../../commit/TBD) (PR #TBD: pending)
- **Date**: 2025-08-14
- **Result**: `make test` failed – Could NOT find Protobuf (missing: Protobuf_LIBRARIES Protobuf_INCLUDE_DIR)
