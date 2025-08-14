# C++ Service Guide

See [../AGENTS.md](../AGENTS.md) for shared repository guidelines.

## Formatting
- Format C++ sources with `clang-format` using the repo configuration:
  
  ```
  clang-format -i *.cpp *.h
  ```

## Build
- Configure and build with CMake:

  ```
  cmake -S . -B build
  cmake --build build
  ```

- To mirror CI locally and produce the shared library used by tests, run from
  the repository root:

  ```
  make build
  ```

  This generates `build/cpp-service/libsensor_service.so`.

## Tests
- Run the C++ tests via CTest:
  
  ```
  ctest --test-dir build
  ```
