import sys
from unittest.mock import MagicMock, patch

from proto import data_pb2


def load_worker_with_mocked_zmq():
    zmq_mock = MagicMock()
    sub_socket = MagicMock()
    req_socket = MagicMock()
    ctx = MagicMock()
    ctx.socket.side_effect = [sub_socket, req_socket]
    zmq_mock.Context.return_value = ctx
    zmq_mock.SUB = 1
    zmq_mock.REQ = 2
    zmq_mock.POLLIN = 1
    zmq_mock.SUBSCRIBE = 1
    poller = MagicMock()
    poller.poll.return_value = [(sub_socket, zmq_mock.POLLIN)]
    zmq_mock.Poller.return_value = poller

    original_zmq = sys.modules.get("zmq")
    sys.modules["zmq"] = zmq_mock

    import worker

    if original_zmq is not None:
        sys.modules["zmq"] = original_zmq
    else:  # pragma: no cover - no existing zmq module
        del sys.modules["zmq"]

    return worker, poller, sub_socket, req_socket


def test_process_parses_event_and_sends_request():
    worker, poller, sub_socket, req_socket = load_worker_with_mocked_zmq()
    event = data_pb2.UpdateUserEvent(user_id=7, field="name", value="Bob")
    sub_socket.recv_multipart.return_value = [
        b"UpdateUserEvent",
        event.SerializeToString(),
    ]

    result = worker.process(poller, sub_socket, req_socket)

    assert result.user_id == 7
    assert result.field == "name"
    req_socket.send_multipart.assert_called_once()
    sent = req_socket.send_multipart.call_args[0][0]
    assert sent[0] == b"GetUserRequest"
    req = data_pb2.GetUserRequest()
    req.ParseFromString(sent[1])
    assert req.user_id == 7
    assert req.field == "name"


def test_sensor_reading_pub_sub_roundtrip():
    import zmq

    import worker

    ctx = zmq.Context()
    pub_socket, sub_socket = worker.setup_sensor_pubsub(ctx, "inproc://sensor-test")
    reading = data_pb2.SensorReading(sensor_id=7, value=1.23, timestamp=99)

    poller = zmq.Poller()
    poller.register(pub_socket, zmq.POLLOUT)
    if not poller.poll(1000):
        raise TimeoutError("subscriber handshake timed out")

    worker.send_sensor_reading(pub_socket, reading)

    poller = zmq.Poller()
    poller.register(sub_socket, zmq.POLLIN)
    if not poller.poll(1000):
        raise TimeoutError("no sensor reading received")

    received = worker.recv_sensor_reading(sub_socket)
    assert received == reading


def test_main_emits_startup_message(capsys):
    worker, poller, sub_socket, req_socket = load_worker_with_mocked_zmq()
    with (
        patch(
            "worker.setup",
            return_value=(poller, sub_socket, req_socket),
        ),
        patch("worker.process", side_effect=SystemExit),
    ):
        try:
            worker.main()
        except SystemExit:
            pass
    captured = capsys.readouterr()
    assert "Python worker active" in captured.out
