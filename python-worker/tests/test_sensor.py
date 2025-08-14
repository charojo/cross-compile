import sys
import time
from pathlib import Path

import zmq

sys.path.append(str(Path(__file__).resolve().parents[1]))
from proto import data_pb2
from worker import (
    recv_sensor_reading,
    send_sensor_reading,
    setup_sensor_pubsub,
)


def test_sensor_roundtrip():
    ctx = zmq.Context()
    pub, sub = setup_sensor_pubsub(ctx, "inproc://worker-sensor")
    reading = data_pb2.SensorReading(sensor_id=5, value=6.7, timestamp=8)
    poller = zmq.Poller()
    poller.register(pub, zmq.POLLOUT)
    end_time = time.time() + 1
    while time.time() < end_time:
        if poller.poll(50):
            break
    else:
        raise TimeoutError("subscriber handshake timed out")
    send_sensor_reading(pub, reading)
    received = recv_sensor_reading(sub)
    assert received == reading
