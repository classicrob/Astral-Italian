#!/usr/bin/env python3
"""Use the unchanged production bounded runner for the entire next batch."""
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
# Existing runner owns termination; allow its five-second stop grace within 1500s.
raise SystemExit(runner([sys.executable, str(Path(__file__).with_name("sweep.py")), *args], seconds=1490))
