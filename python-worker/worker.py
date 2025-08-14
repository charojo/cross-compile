"""ZeroMQ-based worker that reacts to user update events."""

from __future__ import annotations

import zmq
from proto import data_pb2

SUB_ENDPOINT = "tcp://localhost:5556"
REQ_ENDPOINT = "tcp://localhost:5555"


def setup(context: zmq.Context | None = None):
    """Create sockets and poller for the worker."""
    context = context or zmq.Context()
    sub_socket = context.socket(zmq.SUB)
    sub_socket.connect(SUB_ENDPOINT)
    # Subscribe to all topics
    try:
        sub_socket.setsockopt_string(zmq.SUBSCRIBE, "")
    except AttributeError:  # pragma: no cover - for simple mocks lacking method
        pass

    req_socket = context.socket(zmq.REQ)
    req_socket.connect(REQ_ENDPOINT)

    poller = zmq.Poller()
    poller.register(sub_socket, zmq.POLLIN)
    return poller, sub_socket, req_socket


def process(poller: zmq.Poller, sub_socket, req_socket):
    """Handle a single poll cycle.

    Returns the parsed UpdateUserEvent if one was received, otherwise ``None``.
    """
    events = dict(poller.poll())
    if sub_socket in events:
        raw = sub_socket.recv()
        event = data_pb2.UpdateUserEvent()
        event.ParseFromString(raw)
        request = data_pb2.GetUserRequest(user_id=event.user_id, field=event.field)
        req_socket.send(request.SerializeToString())
        return event
    return None


def send_sensor_reading(publisher, reading: data_pb2.SensorReading) -> None:
    """Serialize and publish a ``SensorReading`` message."""

    publisher.send(reading.SerializeToString())


def recv_sensor_reading(subscriber) -> data_pb2.SensorReading:
    """Receive and deserialize a ``SensorReading`` message."""

    raw = subscriber.recv()
    reading = data_pb2.SensorReading()
    reading.ParseFromString(raw)
    return reading


def setup_sensor_pubsub(
    context: zmq.Context | None = None, endpoint: str = "inproc://sensor"
):
    """Set up PUB and SUB sockets for sensor data."""

    context = context or zmq.Context()
    publisher = context.socket(zmq.PUB)
    publisher.bind(endpoint)
    subscriber = context.socket(zmq.SUB)
    subscriber.connect(endpoint)
    subscriber.setsockopt_string(zmq.SUBSCRIBE, "")
    return publisher, subscriber


def main() -> None:
    print("Python worker active")
    poller, sub_socket, req_socket = setup()
    while True:
        process(poller, sub_socket, req_socket)


if __name__ == "__main__":
    main()
