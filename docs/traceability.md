# Traceability Matrix

This document maps project features to the tests that validate them.

| Requirement | Test Files |
| --- | --- |
| Update user data | rust-agent/src/main.rs (tests module `tests::it_runs`) |
| Start data service | cpp-service/tests/test_dataservice.sh |
| Activate Python worker | tests/test_worker.py |
| Reliable sensor data roundtrip | tests/test_worker.py::test_sensor_reading_pub_sub_roundtrip |

The sensor roundtrip test polls ZeroMQ sockets instead of using time-based delays,
demonstrating that messages arrive without relying on arbitrary sleeps.
