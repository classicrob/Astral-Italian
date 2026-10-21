#!/usr/bin/env python3
"""Bound only the separately allocated strict final-two batch to 540 seconds."""
import hashlib
import json
from pathlib import Path
import runpy
import sys

args = sys.argv[1:]
handoff = Path(args[args.index("--handoff") + 1])
h = json.loads(handoff.read_text())
bounded = Path(h["bounded_runner"])
assert hashlib.sha256(bounded.read_bytes()).hexdigest() == h["bounded_runner_sha256"]
runner = runpy.run_path(str(bounded))["run"]
# Existing runner owns termination; allow its five-second stop grace within 540s.
raise SystemExit(runner([sys.executable, str(Path(__file__).with_name("final_two_sweep.py")), *args], seconds=535))
