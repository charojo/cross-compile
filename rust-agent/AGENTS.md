# Rust Agent Guide

See [../AGENTS.md](../AGENTS.md) for repository-wide rules.

## Formatting
- Format with Rustfmt:
  
  ```
  cargo fmt --all
  ```
- Lint with Clippy:
  
  ```
  cargo clippy --all-targets --all-features
  ```

## Build
- Build for the target architecture:
  
  ```
  cargo build --target=aarch64-unknown-linux-gnu
  ```

## Tests
- Run agent tests with `pytest`:

  ```
  pytest
  ```
