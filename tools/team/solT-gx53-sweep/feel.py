#!/usr/bin/env python3
"""Source-bound closed-case frame pacing summary. No logs or image reads."""
import csv
import hashlib
import json
import math
from pathlib import Path
import statistics
import sys

root = Path(sys.argv[1]).resolve()
assert "solT-gx53-direct" in root.parts and root.name.startswith("gx53-batch")
batch = json.loads((root / "batch.json").read_text())
assert batch["source_sha"] == "33e71ee880b426c0c7da03a724377200e874c417"
closed = json.loads((root / "closed.json").read_text())


def numeric(value):
    try:
        value = float(value)
        return value if math.isfinite(value) and value >= 0 else None
    except (ValueError, TypeError):
        return None


def stats(values):
    if not values:
        return "NOT_MEASURED"
    values = sorted(values)
    q = lambda p: values[min(len(values) - 1, round((len(values) - 1) * p))]
    return {"rows": len(values), "mean_ms": statistics.fmean(values), "p50_ms": q(.5),
            "p95_ms": q(.95), "p99_ms": q(.99), "max_ms": values[-1],
            "over_20ms": sum(x > 20 for x in values),
            "over_25ms": sum(x > 25 for x in values),
            "over_33_333ms": sum(x > 1000 / 30 for x in values),
            "over_50ms": sum(x > 50 for x in values), "over_100ms": sum(x > 100 for x in values)}


cases = []
for entry in closed["rows"]:
    if entry.get("status") != "BOUNDED_IDLE_OBSERVED":
        continue
    case = Path(entry["case"]).resolve()
    assert case.parent == root
    path = case / "frames.csv"
    if not path.is_file():
        cases.append({"stage": entry["stage"], "mode": entry["mode"], "feel": "NOT_MEASURED"})
        continue
    frame, present, states = [], [], {}
    excluded, efb_rows = 0, 0
    with path.open() as handle:
        for row in csv.DictReader(handle):
            if row.get("phase") != "gameplay" or row.get("unfocused") != "0":
                excluded += 1
                continue
            n = numeric(row.get("interval_ms"))
            if n is not None:
                frame.append(n)
            n = numeric(row.get("present_ms"))
            if n is not None:
                present.append(n)
            state = row.get("present_state", "NOT_MEASURED")
            states[state] = states.get(state, 0) + 1
            efb_rows += (numeric(row.get("efb_captures")) or 0) > 0
    cases.append({"stage": entry["stage"], "mode": entry["mode"],
        "frame_csv_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        "focused_source_gameplay_intervals": stats(frame),
        "presented_drawable_intervals": stats(present), "present_state_rows": states,
        "non_gameplay_or_unfocused_rows_excluded": excluded, "included_efb_capture_rows": efb_rows})
result = {"summary_schema_version": 2,
    "threshold_semantics": "Strictly greater than each threshold, using recorded CSV values; exactly 25.000ms is excluded from over_25ms. Source timing columns have three decimal places.",
    "summary_tool_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    "source_sha": batch["source_sha"], "artifact_sha256": batch["handoff"]["executable_sha256"],
    "renderer_key": batch["handoff"]["artifact_key"], "cases": cases,
    "providers": {name: batch["source_providers"][name] for name in ("native/app/frame_seam.cpp", "native/app/frame_stats.cpp")},
    "present_semantics": "native/include/petari/present_stats.hpp: interval between presented drawable timestamps, not GPU duration or input-to-screen latency",
    "gpu_time": "NOT_MEASURED", "input_to_screen_latency": "NOT_MEASURED", "visual_dolphin_diff": "NOT_MEASURED",
    "qualification": "Neutral direct-entry idle, focused rows labelled gameplay by existing source. Includes capture overhead and uncontrolled OS/driver cache. No performance/GPU/visual PASS. Logs unread."}
with (root / "FRAME-FEEL-25MS.json").open("x") as handle:
    json.dump(result, handle, indent=2, allow_nan=False)
    handle.write("\n")
print(json.dumps(result, indent=2, allow_nan=False))
