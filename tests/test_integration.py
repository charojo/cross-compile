import threading
import time
import zmq

from proto import data_pb2
from python_worker import worker


class DataServiceStub(threading.Thread):
    """Minimal stand-in for the C++ data service.

    It echoes update requests, publishes an event, and records get requests
    sent by the worker.
    """

    def __init__(self):
        super().__init__()
        self._stop_event = threading.Event()
        self.received_get_requests = []
        self.context = zmq.Context()
        self.rep = self.context.socket(zmq.REP)
        self.rep.bind("tcp://127.0.0.1:5555")
        self.pub = self.context.socket(zmq.PUB)
        self.pub.bind("tcp://127.0.0.1:5556")

    def run(self):
        poller = zmq.Poller()
        poller.register(self.rep, zmq.POLLIN)
        while not self._stop_event.is_set():
            socks = dict(poller.poll(100))
            if self.rep in socks:
                mtype, msg = self.rep.recv_multipart()
                if mtype == b"UpdateUserRequest":
                    update = data_pb2.UpdateUserRequest()
                    update.ParseFromString(msg)
                    resp = data_pb2.UpdateUserResponse(success=True, message="ok")
                    self.rep.send_multipart(
                        [b"UpdateUserResponse", resp.SerializeToString()]
                    )
                    event = data_pb2.UpdateUserEvent(
                        user_id=update.user_id,
                        field=update.field,
                        value=update.value,
                    )
                    self.pub.send_multipart(
                        [b"UpdateUserEvent", event.SerializeToString()]
                    )
                elif mtype == b"GetUserRequest":
                    get_req = data_pb2.GetUserRequest()
                    get_req.ParseFromString(msg)
                    self.received_get_requests.append(get_req)
                    resp = data_pb2.GetUserResponse(found=True, value="")
                    self.rep.send_multipart(
                        [b"GetUserResponse", resp.SerializeToString()]
                    )
                else:  # pragma: no cover - unknown message type
                    pass

    def stop(self):
        self._stop_event.set()
        self.join()
        self.rep.close(0)
        self.pub.close(0)
        self.context.term()


def test_end_to_end_message_flow():
    """Ensure that an update results in a worker query via ZeroMQ."""

    service = DataServiceStub()
    service.start()
    time.sleep(0.2)
    poller, sub_socket, req_socket = worker.setup()
    time.sleep(0.2)

    ctx = zmq.Context()
    client = ctx.socket(zmq.REQ)
    client.connect("tcp://127.0.0.1:5555")
    update = data_pb2.UpdateUserRequest(user_id=1, field="name", value="Alice")
    client.send_multipart(
        [
            b"UpdateUserRequest",
            update.SerializeToString(),
        ]
    )
    _, raw = client.recv_multipart()
    resp = data_pb2.UpdateUserResponse()
    resp.ParseFromString(raw)
    assert resp.success

    event = worker.process(poller, sub_socket, req_socket)
    assert event is not None
    assert event.user_id == 1
    assert event.field == "name"
    time.sleep(0.1)
    assert service.received_get_requests
    assert service.received_get_requests[0].user_id == 1
    assert service.received_get_requests[0].field == "name"

    client.close(0)
    ctx.term()
    service.stop()
    sub_socket.close(0)
    req_socket.close(0)
