# Python Worker Guide

See [../AGENTS.md](../AGENTS.md) for repository-wide rules.

## Formatting
- Format with `black`:
  
  ```
  black .
  ```
- Lint with `flake8`:
  
  ```
  flake8
  ```

## Build
- Compile to check for syntax errors:
  
  ```
  python -m py_compile *.py
  ```

## Tests
- Run Python tests with `pytest`:
  
  ```
  pytest
  ```
