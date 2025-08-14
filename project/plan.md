# Project Plan

## MVP Goal
Build a cross-platform development scaffold enabling C++, Python, and Rust services to communicate via ZeroMQ and Protobuf while persisting data with SQLite on the i.MX8MP platform.

## High-Level Architecture
```
+-------------+     +--------------+     +-----------+
| Python      | --> | ZeroMQ Bus   | <-- | Rust      |
| Worker      |     |              |     | Agent     |
+-------------+     +--------------+     +-----------+
          \            ^
           \          /
            v        /
         +------------------+
         | C++ Data Service |
         +------------------+
               |
               v
          SQLite DB
```

## Milestones
1. Containerized dev environment and cross-compilation toolchains.
2. C++ data service with Protobuf schema and SQLite persistence.
3. Python worker and Rust agent integration over ZeroMQ.
4. End-to-end tests and CI pipeline.
