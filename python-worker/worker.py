import argparse
import logging

LOG_FORMAT = "%""%(asctime)s %(levelname)s [%(message_id)s] [trace=%(trace_id)s] %(message)s"""

def configure_logging(level: str) -> None:
    logging.basicConfig(level=level.upper(), format=LOG_FORMAT)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log-level", default="INFO")
    parser.add_argument("--trace-id", type=int, default=0)
    args = parser.parse_args()

    configure_logging(args.log_level)
    logger = logging.LoggerAdapter(logging.getLogger(__name__), {
        "trace_id": args.trace_id,
        "message_id": "PY1001",
    })
    logger.info("Python worker active")

if __name__ == "__main__":
    main()
