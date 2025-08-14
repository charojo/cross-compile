import sys
from pathlib import Path

sys.path.append(str(Path(__file__).resolve().parents[1]))
from worker import main


def test_main(capsys):
    main()
    assert "Python worker active" in capsys.readouterr().out
