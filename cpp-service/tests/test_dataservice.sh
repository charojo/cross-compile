#!/usr/bin/env bash
set -euo pipefail
pytest "$(dirname "$0")/test_dataservice.py"
