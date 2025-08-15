# Test Report
This document summarizes the latest `make test` run for each merged feature. Include the abbreviated commit ID and a link to the relevant pull request or commit for traceability.

## Command
`make test` run on 2025-08-14 at 23:53 UTC for commit 0530771.

## Output
```
./scripts/gen-protos.sh
cmake -S cpp-service -B build/cpp-service
-- The CXX compiler identification is GNU 13.3.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found PkgConfig: /usr/bin/pkg-config (found version "1.8.1") 
-- Checking for module 'libzmq'
--   Package 'libzmq', required by 'virtual:world', not found
CMake Error at /usr/share/cmake-3.28/Modules/FindPkgConfig.cmake:619 (message):
  The following required packages were not found:

   - libzmq

Call Stack (most recent call first):
  /usr/share/cmake-3.28/Modules/FindPkgConfig.cmake:841 (_pkg_check_modules_internal)
  CMakeLists.txt:9 (pkg_check_modules)


-- Configuring incomplete, errors occurred!
make: *** [Makefile:13: test] Error 1
```

