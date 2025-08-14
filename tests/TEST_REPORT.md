# Test Report
This document summarizes the latest `make test` run for each merged feature. Include the abbreviated commit ID and a link to the relevant pull request or commit for traceability.

## Command
`make test` run on 2025-08-14 at 15:25 UTC.

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
