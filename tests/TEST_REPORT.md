# Test Report

## Command
`make test` run on 2025-08-14.

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


-- Configuring incomplete, errors occurred!
make: *** [Makefile:15: test] Error 1

```

## Summary
Tests failed: Could NOT find Protobuf (missing: Protobuf_LIBRARIES Protobuf_INCLUDE_DIR).

## Feature Commits
- C++ Data Service skeleton - `7f2ac3f` ([commit](https://github.com/example/commit/7f2ac3f))
- Python worker example - `97dc3be` ([commit](https://github.com/example/commit/97dc3be))
- Rust agent template - _pending_
- Sensor reading storage and command handling - `626feab` ([commit](https://github.com/example/commit/626feab))
