#!/usr/bin/env python3
"""Headless regression sweep of canonical raw-KPAD gameplay segments.

--headless BIN --disc DIR --tapes DIR --out NEW [--names NAME ...]
Includes the real-player full-star tape by default. This checks the segments the
headless harness supports, not GX menus/arrival/pause-resume RNG checkpoints.
The GX full_star_tape_gate remains the mission acceptance gate. A star-count
transition in a gap between segments is explicitly uncovered, never a PASS.
The caller must serialize builds/replays with the host's heavy->native locks.
"""
import argparse
import hashlib
import json
import subprocess
import sys
from pathlib import Path

import replay_tape

DEFAULT_TAPES = ("tape-normal-54", "tape-user-flipswap-fullstar-541")


def coverage(meta, route):
    import csv
    rows = list(csv.DictReader(route.open()))[:meta["seams"]]
    events = replay_tape.star_events(rows)
    covered = [e for e in events if any(s["start_row"] <= e["route_row"] <= s["end_row"] for s in meta["segments"])]
    return {"recorded_star_events": events, "covered_star_events": covered,
            "uncovered_star_events": [e for e in events if e not in covered],
            "mission_acceptance_supported": False,
            "reason": "Headless segment replay omits GX session boundaries and repeated play-ready/RNG checkpoints; full_star_tape_gate is authoritative."}


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--headless", type=Path, required=True)
    p.add_argument("--disc", type=Path, required=True)
    p.add_argument("--tapes", type=Path, required=True)
    p.add_argument("--out", type=Path, required=True)
    p.add_argument("--names", nargs="+", default=DEFAULT_TAPES)
    a = p.parse_args(argv)
    a.out.mkdir(parents=True, exist_ok=False)
    results = []
    for name in a.names:
        if Path(name).name != name:
            p.error("tape names must be directory names")
        tape = (a.tapes / name).resolve()
        sidecar = replay_tape.locate_sidecar(str(tape))
        meta = json.loads(sidecar.read_text())
        cmd = [sys.executable, "-B", str(Path(replay_tape.__file__)), str(tape), "--headless", str(a.headless.resolve()),
               "--disc", str(a.disc.resolve()), "--out", str((a.out / name).resolve()), "--require-bit-exact"]
        if not meta.get("complete"):
            cmd.append("--allow-incomplete")
        with (a.out / (name + ".log")).open("wb") as log:
            proc = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT)
        report_path = a.out / name / "first-divergence.json"
        report = json.loads(report_path.read_text()) if report_path.exists() else {}
        results.append({"tape": str(tape), "exit": proc.returncode,
                        "gameplay_segments_passed": proc.returncode == 0 and report.get("all_segments_matched") is True
                        and report.get("bit_exact") is True,
                        "scope": coverage(meta, replay_tape.tape_files(sidecar, meta)[2]), "report": str(report_path), "command": cmd})
    result = {"all_gameplay_segments_passed": all(r["gameplay_segments_passed"] for r in results),
              "headless_sha256": hashlib.sha256(a.headless.read_bytes()).hexdigest(), "runs": results,
              "full_star_acceptance": False, "full_star_gate": "gate53/m1gates/full_star_tape_gate.py"}
    (a.out / "RESULT.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))
    return 0 if result["all_gameplay_segments_passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
