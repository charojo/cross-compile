# Logging Guidelines

This project demonstrates consistent structured logging across the C++ Data Service, Python worker, and Rust agent. Each service accepts an optional `--trace-id` argument used to correlate log messages across languages. Protobuf messages include a `uint64 trace_id` field that should be passed through all services.

## C++ Data Service

```sh
./data_service --log-level info --trace-id 42
```

Uses [spdlog](https://github.com/gabime/spdlog) and supports a `--log-level` flag. Message IDs such as `DS1001` are embedded in log lines:

```
[DS1001][trace=42]: Data Service running
```

## Python Worker

```sh
python worker.py --log-level INFO --trace-id 42
```

Configures the `logging` module with a format that includes message and trace identifiers. Example output:

```
2024-01-01 00:00:00 INFO [PY1001] [trace=42] Python worker active
```

## Rust Agent

```sh
cargo run -- --trace-id 42
```

Uses the `env_logger` crate with a custom formatter. Example log:

```
2024-01-01T00:00:00Z INFO [trace=42] [RS1001] Rust agent ready
```

## Protobuf

`proto/data.proto` defines `trace_id` on all messages. Ensure this value is propagated through every service call.
