# Contributor Guide

## Code Style
- Run `pre-commit install` to enable automatic formatting checks.
- Rust: run `cargo fmt --all` and `cargo clippy --all-targets --all-features`.
- Python: format with `black` and lint with `flake8`.
- C++: format with `clang-format` using the repository configuration.

## Commit Messages
- Use the conventional commit format: `type: subject` (e.g., `feat: add cache`).
- Types include: `feat`, `fix`, `docs`, `test`, `chore`, `refactor`, `build`.
- Keep subjects ≤ 72 characters and in the imperative mood.

## Tests
- Run `make test` before pushing changes. All tests must pass.
- Docs-only changes must still run `make test` to confirm nothing breaks.

## Project Documentation
- Update `plan.md` when the feature roadmap changes or new features are added.
- Update `tracking.md` as feature work progresses or completes.

## Pull Requests
- Ensure this guide has been followed before submitting a PR.
- Each PR requires at least one reviewer approval and a clear description of changes.
