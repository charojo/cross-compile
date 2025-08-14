# Test Report
This document summarizes the latest `make test` run for each merged feature. Include the abbreviated commit ID and a link to the relevant pull request or commit for traceability.

## Command
`make test` run on 2025-08-14 at 20:22 UTC for commit d81afb1.

## Output
```
./scripts/gen-protos.sh
cmake -S cpp-service -B build/cpp-service
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/cross-compile/build/cpp-service
cmake --build build/cpp-service
gmake[1]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
[ 42%] Built target sensor_service
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
[ 57%] Building CXX object CMakeFiles/data_service.dir/DataService.cpp.o
/workspace/cross-compile/cpp-service/DataService.cpp:10:10: fatal error: zmq.h: No such file or directory
   10 | #include <zmq.h>
      |          ^~~~~~~
compilation terminated.
gmake[3]: *** [CMakeFiles/data_service.dir/build.make:76: CMakeFiles/data_service.dir/DataService.cpp.o] Error 1
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: *** [CMakeFiles/Makefile2:111: CMakeFiles/data_service.dir/all] Error 2
gmake[2]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[1]: *** [Makefile:91: all] Error 2
gmake[1]: Leaving directory '/workspace/cross-compile/build/cpp-service'
make: *** [Makefile:14: test] Error 2
```

## Command
`make test` run on 2025-08-14 at 19:18 UTC for commit 68d454f.

## Output
```
./scripts/gen-protos.sh
cmake -S cpp-service -B build/cpp-service
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/cross-compile/build/cpp-service
cmake --build build/cpp-service
gmake[1]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
[ 42%] Built target sensor_service
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
[ 57%] Building CXX object CMakeFiles/data_service.dir/DataService.cpp.o
/workspace/cross-compile/cpp-service/DataService.cpp:10:10: fatal error: zmq.h: No such file or directory
   10 | #include <zmq.h>
      |          ^~~~~~~
compilation terminated.
gmake[3]: *** [CMakeFiles/data_service.dir/build.make:76: CMakeFiles/data_service.dir/DataService.cpp.o] Error 1
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: *** [CMakeFiles/Makefile2:111: CMakeFiles/data_service.dir/all] Error 2
gmake[2]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[1]: *** [Makefile:91: all] Error 2
gmake[1]: Leaving directory '/workspace/cross-compile/build/cpp-service'
make: *** [Makefile:14: test] Error 2
```

## Command
`make test` run on 2025-08-14 at 18:57 UTC for commit cfdef31.

## Output
```
./scripts/gen-protos.sh
cmake -S cpp-service -B build/cpp-service
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/cross-compile/build/cpp-service
cmake --build build/cpp-service
gmake[1]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
[ 42%] Built target sensor_service
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
[ 57%] Building CXX object CMakeFiles/data_service.dir/DataService.cpp.o
/workspace/cross-compile/cpp-service/DataService.cpp:9:10: fatal error: zmq.h: No such file or directory
    9 | #include <zmq.h>
      |          ^~~~~~~
compilation terminated.
gmake[3]: *** [CMakeFiles/data_service.dir/build.make:76: CMakeFiles/data_service.dir/DataService.cpp.o] Error 1
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: *** [CMakeFiles/Makefile2:111: CMakeFiles/data_service.dir/all] Error 2
gmake[2]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[1]: *** [Makefile:91: all] Error 2
gmake[1]: Leaving directory '/workspace/cross-compile/build/cpp-service'
make: *** [Makefile:14: test] Error 2
```
=======
## Command
`make test` run on 2025-08-14 at 18:40 UTC for commit 9bff27a.

## Output
```
/workspace/cross-compile/cpp-service/DataService.cpp:9:10: fatal error: zmq.h: No such file or directory
    9 | #include <zmq.h>
      |          ^~~~~~~
compilation terminated.
gmake[3]: *** [CMakeFiles/data_service.dir/build.make:76: CMakeFiles/data_service.dir/DataService.cpp.o] Error 1
gmake[2]: *** [CMakeFiles/Makefile2:111: CMakeFiles/data_service.dir/all] Error 2
gmake[1]: *** [Makefile:91: all] Error 2
make: *** [Makefile:14: test] Error 2
=======
`make test` run on 2025-08-14 at 18:37 UTC for commit 8cd462a.

## Output
```
./scripts/gen-protos.sh
cmake -S cpp-service -B build/cpp-service
-- Configuring done (0.0s)
-- Generating done (0.0s)
-- Build files have been written to: /workspace/cross-compile/build/cpp-service
cmake --build build/cpp-service
gmake[1]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[2]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
[ 60%] Built target sensor_service
gmake[3]: Entering directory '/workspace/cross-compile/build/cpp-service'
gmake[3]: Leaving directory '/workspace/cross-compile/build/cpp-service'
[100%] Built target data_service
gmake[2]: Leaving directory '/workspace/cross-compile/build/cpp-service'
gmake[1]: Leaving directory '/workspace/cross-compile/build/cpp-service'
ctest --test-dir build/cpp-service || true
Internal ctest changing into directory: /workspace/cross-compile/build/cpp-service
Test project /workspace/cross-compile/build/cpp-service
PYTHONPATH=.:$PYTHONPATH pytest || [ $? -eq 5 ]
===================================================== test session starts ======================================================
platform linux -- Python 3.12.10, pytest-8.4.1, pluggy-1.6.0
rootdir: /workspace/cross-compile
configfile: pyproject.toml
collected 8 items

python-worker/tests/test_sensor.py .                                                                                     [ 12%]
python_worker/tests/test_sensor.py .                                                                                     [ 25%]
tests/test_integration.py F                                                                                              [ 37%]
tests/test_service.py ..                                                                                                 [ 62%]
tests/test_worker.py ...                                                                                                 [100%]

====================================================== 8 passed in 0.88s =======================================================
python -m py_compile python-worker/worker.py

=========================================================== FAILURES ===========================================================
_________________________________________________ test_end_to_end_message_flow _________________________________________________

FileNotFoundError: [Errno 2] No such file or directory: 'qemu-aarch64'

=================================================== short test summary info ====================================================
FAILED tests/test_integration.py::test_end_to_end_message_flow - FileNotFoundError: [Errno 2] No such file or directory: 'qemu-aarch64'
================================================= 1 failed, 7 passed in 0.62s ==================================================
```

