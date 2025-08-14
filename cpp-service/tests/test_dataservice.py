import subprocess
from pathlib import Path


def test_dataservice_runs() -> None:
    """Data service should emit startup message."""
    root = Path(__file__).resolve().parents[2]
    binary = root / "cpp-service" / "data_service"
    proc = subprocess.Popen(
        [str(binary)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    try:
        out = proc.communicate(timeout=1)[0].decode()
    except subprocess.TimeoutExpired:
        proc.kill()
        out = proc.communicate()[0].decode()
    assert "Data Service running" in out
