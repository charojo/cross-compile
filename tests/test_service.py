import ctypes
import pathlib
import sqlite3

from proto import data_pb2

LIB_PATH = (
    pathlib.Path(__file__).resolve().parents[1]
    / "build"
    / "cpp-service"
    / "libsensor_service.so"
)
lib = ctypes.CDLL(str(LIB_PATH))

lib.service_init.argtypes = [ctypes.c_char_p]
lib.service_init.restype = ctypes.c_void_p
lib.service_close.argtypes = [ctypes.c_void_p]
lib.service_handle_reading.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
lib.service_handle_reading.restype = ctypes.c_int
lib.service_process_command.argtypes = [ctypes.c_char_p, ctypes.c_int]
lib.service_get_rate.restype = ctypes.c_ulonglong
lib.service_get_target.restype = ctypes.c_char_p


def test_sensor_reading_written(tmp_path):
    db_path = tmp_path / "readings.db"
    db = lib.service_init(str(db_path).encode())
    reading = data_pb2.SensorReading(sensor_id=2, value=3.14, timestamp=111)
    payload = reading.SerializeToString()
    assert lib.service_handle_reading(db, payload, len(payload)) == 1
    lib.service_close(db)
    conn = sqlite3.connect(db_path)
    row = conn.execute(
        "SELECT sensor_id, value, timestamp FROM sensor_readings"
    ).fetchone()
    assert row == (2, 3.14, 111)
    conn.close()


def test_control_command_updates_state():
    cmd = data_pb2.ControlCommand(new_rate=7, target="pump")
    payload = cmd.SerializeToString()
    lib.service_process_command(payload, len(payload))
    assert lib.service_get_rate() == 7
    assert lib.service_get_target().decode() == "pump"
