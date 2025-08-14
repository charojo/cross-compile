"""Integration test that exercises the real C++ data service."""

from __future__ import annotations

import threading
import time
import contextlib
import pathlib
import subprocess

import zmq

from proto import data_pb2
from python_worker import worker


@contextlib.contextmanager
def _launch_service() -> subprocess.Popen[bytes]:
    """Build and launch the ARM data service under QEMU."""

    import shutil, pytest

    if not shutil.which("qemu-aarch64"):
        pytest.skip("qemu-aarch64 not available")

    root = pathlib.Path(__file__).resolve().parents[1]
    subprocess.run(["make", "build"], cwd=root, check=True)
    binary = (root / "cpp-service" / "data_service").resolve()
    proc = subprocess.Popen(
        ["qemu-aarch64", "-L", "/usr/aarch64-linux-gnu", str(binary)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    # Give the service time to bind to its sockets
    time.sleep(0.5)
    try:
        yield proc
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:  # pragma: no cover - best effort
            proc.kill()


def test_end_to_end_message_flow() -> None:
    """Ensure that an update flows through the service and worker."""

    with _launch_service():
        poller, sub_socket, req_socket = worker.setup()
        time.sleep(0.2)

        ctx = zmq.Context()
        client = ctx.socket(zmq.REQ)
        client.setsockopt(zmq.RCVTIMEO, 2000)
        client.connect("tcp://127.0.0.1:5555")
        update = data_pb2.UpdateUserRequest(user_id=1, field="name", value="Alice")
        client.send_multipart([b"UpdateUserRequest", update.SerializeToString()])
        mtype, raw = client.recv_multipart()
        assert mtype == b"UpdateUserResponse"
        resp = data_pb2.UpdateUserResponse()
        resp.ParseFromString(raw)
        assert resp.success

        result: list[data_pb2.UpdateUserEvent | None] = []
        thread = threading.Thread(
            target=lambda: result.append(worker.process(poller, sub_socket, req_socket))
        )
        thread.start()
        thread.join(timeout=5)
        assert not thread.is_alive(), "worker.process timed out"
        event = result[0]
        assert event is not None
        assert event.user_id == 1
        assert event.field == "name"

        client.close(0)
        ctx.term()
        sub_socket.close(0)
        req_socket.close(0)
