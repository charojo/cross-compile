#!/usr/bin/env python3
"""Benchmark harness for cross-compile demo.

Builds the C++ data service with various optimization profiles and
measures runtime and maximum resident set size using QEMU. Results are
written to CSV and HTML files.
"""

from __future__ import annotations

import csv
import subprocess
from pathlib import Path
from dataclasses import dataclass

ROOT = Path(__file__).resolve().parent.parent
CPP_SRC = ROOT / "cpp-service" / "DataService.cpp"
BUILD_DIR = ROOT / "bench-build"
RESULT_CSV = ROOT / "bench_results.csv"
RESULT_HTML = ROOT / "bench_results.html"

@dataclass
class Result:
    profile: str
    runtime: float
    rss_kb: int

def run_cmd(cmd: list[str], **kwargs) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, check=True, text=True, **kwargs)

def compile_service(flags: list[str], output: Path) -> None:
    cmd = [
        "aarch64-linux-gnu-g++",
        str(CPP_SRC),
        "-static",
        "-o",
        str(output),
    ] + flags
    run_cmd(cmd)

def run_service(binary: Path) -> tuple[float, int]:
    cmd = [
        "/usr/bin/time",
        "-f",
        "%e,%M",
        "qemu-aarch64",
        "-L",
        "/usr/aarch64-linux-gnu",
        str(binary),
    ]
    proc = run_cmd(cmd, capture_output=True)
    time_str, rss_str = proc.stderr.strip().split(",")
    return float(time_str), int(rss_str)

def bench_profile(profile: str, flags: list[str]) -> Result:
    out_dir = BUILD_DIR / profile
    out_dir.mkdir(parents=True, exist_ok=True)
    bin_path = out_dir / "data_service"

    if profile == "PGO":
        # Generate profile data
        compile_service(flags + ["-fprofile-generate"], bin_path)
        run_service(bin_path)
        # Use generated profile
        compile_service(flags + ["-fprofile-use"], bin_path)
    else:
        compile_service(flags, bin_path)

    runtime, rss = run_service(bin_path)
    return Result(profile, runtime, rss)

def main() -> None:
    BUILD_DIR.mkdir(exist_ok=True)
    profiles = {
        "O3": ["-O3"],
        "Os": ["-Os"],
        "LTO": ["-O3", "-flto"],
        "PGO": ["-O3"],
    }

    results: list[Result] = []
    for name, flags in profiles.items():
        print(f"Building and benchmarking {name} profile...")
        results.append(bench_profile(name, flags))

    with RESULT_CSV.open("w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["profile", "runtime_sec", "rss_kb"])
        for r in results:
            writer.writerow([r.profile, f"{r.runtime:.3f}", r.rss_kb])

    with RESULT_HTML.open("w") as f:
        f.write("<html><body><table border='1'>\n")
        f.write("<tr><th>Profile</th><th>Runtime (s)</th><th>RSS (KB)</th></tr>\n")
        for r in results:
            f.write(
                f"<tr><td>{r.profile}</td><td>{r.runtime:.3f}</td><td>{r.rss_kb}</td></tr>\n"
            )
        f.write("</table></body></html>\n")

    print(f"Results written to {RESULT_CSV} and {RESULT_HTML}")

if __name__ == "__main__":
    main()
