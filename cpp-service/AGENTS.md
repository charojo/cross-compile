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

## Tests
- Run the C++ tests via CTest:
  
  ```
  ctest --test-dir build
  ```
