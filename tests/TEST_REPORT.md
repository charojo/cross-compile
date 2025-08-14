# Test Report
This document summarizes the latest `make test` run for each merged feature. Include the abbreviated commit ID and a link to the relevant pull request or commit for traceability.

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
```
