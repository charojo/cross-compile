# Test Plan

This document outlines unit, integration, and system test objectives for each subsystem and identifies scenarios required for MVP 1.

## C++ Data Service

### Unit Tests
- Verify cache insert and retrieval logic.
- Ensure SQLite writes and reads persist data correctly.

### Integration Tests
- Request/response over ZeroMQ with a test client.
- Published updates reach subscribed components.

### System Tests
- Run alongside Python and Rust services to confirm end-to-end storage and event propagation.

## Python Worker

### Unit Tests
- Build and parse sensor messages.
- Handle retry or backoff behavior.

### Integration Tests
- Exchange messages with the C++ data service over ZeroMQ.

### System Tests
- Participate in sensor flow with the data service and Rust agent.

## Rust Agent

### Unit Tests
- Serialize and deserialize command messages.
- Validate command handling logic.

### Integration Tests
- Subscribe to data service events and publish responses.

### System Tests
- Operate with the full stack to verify command messages flow through the bus.

## MVP 1 Scenarios

The following scenarios must be covered before declaring MVP 1 complete:

- **C++ service storage:** Store a sensor reading in SQLite and retrieve it on request.
- **Python/Rust message flow:** Python worker publishes a message that the C++ service processes and the Rust agent consumes.
