#!/usr/bin/env bash
set -e
timeout 1 ./cpp-service/data_service | grep -m 1 "Data Service running"
