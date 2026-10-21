#!/usr/bin/env python3
"""Seal closed new-sweep JSON receipts; never opens app logs or images."""
import hashlib
import json
from pathlib import Path
import sys

root = Path(sys.argv[1]).resolve()
assert "solT-gx53-direct" in root.parts and root.name.startswith("gx53-batch1-")
load = lambda p: json.loads(Path(p).read_text())
batch = load(root / "batch.json")
closed = load(root / "closed.json")
assert batch["source_sha"] == "33e71ee880b426c0c7da03a724377200e874c417"
assert closed["status"] == "BATCH_CLOSED" and closed["no_log_analysis"]
table = []
for entry in closed["rows"]:
    if "case" not in entry:
        table.append(entry)
        continue
    case = Path(entry["case"]).resolve()
    assert case.parent == root
    result, command = load(case / "result.json"), load(case / "command.json")
    metrics = result["metrics"]
    route = metrics.get("route") if isinstance(metrics, dict) else None
    timing = metrics.get("frame_timing") if isinstance(metrics, dict) else None
    row = {k: result[k] for k in ("stage", "mode", "status", "exit_code", "elapsed_seconds")}
    row.update(valid_alive_streak=route.get("longest_consecutive_valid_target_alive") if isinstance(route, dict) else "NOT_MEASURED",
               frame_rows=timing.get("rows") if isinstance(timing, dict) else "NOT_MEASURED",
               capture_counts={k: len(v) for k, v in result["captures"].items()},
               cache_metadata=result["cache_metadata"], logs_unread=not result["logs_read"],
               result_sha256=hashlib.sha256((case / "result.json").read_bytes()).hexdigest())
    if result["mode"] == "warm":
        source = Path(command["warm_source"]).resolve()
        assert source.parent == root
        fresh = load(source / "result.json")
        row["warm_initial_cache_matches_fresh_final"] = command["initial_cache"] == fresh["cache_final"]
        row["warm_initial_rows_match_fresh_final"] = command["initial_cache_metadata"] == fresh["cache_metadata"]
    table.append(row)
observed = [s for s in batch["planned_stages"] if all(any(r.get("stage") == s and r.get("mode") == mode and r.get("status") == "BOUNDED_IDLE_OBSERVED" for r in table) for mode in ("fresh", "warm"))]
freed = sum(item["bytes_freed"] for path in root.glob("*-cleanup.json") for item in load(path))
summary = {"status": "INITIAL_BATCH_CLOSED", "source_sha": batch["source_sha"],
    "artifact_sha256": batch["handoff"]["executable_sha256"], "renderer_key": batch["handoff"]["artifact_key"],
    "manifest_sha256": batch["manifest_sha256"], "runner_sha256": batch["runner_sha256"],
    "case_receipts": table, "observed_fresh_and_warm_galaxies": observed,
    "supported_manifest_count": len(batch["coverage_inventory"]),
    "full_manifest_coverage": False, "worldmap_not_run": 7,
    "normal_path": "NOT_MEASURED", "qualified_dolphin_visual_diff": "NOT_MEASURED",
    "gpu_time": "NOT_MEASURED", "scanout_latency": "NOT_MEASURED",
    "elapsed_seconds": closed["elapsed_seconds"], "disk_free_after_bytes": closed["disk_free_after_bytes"],
    "owned_success_cache_bytes_freed": freed, "logs_all_unread": all(r.get("logs_unread", True) for r in table),
    "firstcause": "UNKNOWN; no crash/core/log inspection", "promotion": False,
    "limits": "Expected bounded exit124 is distinct from clean exit; selected captures are not exact game-frame pairs; normal bundled seed may populate fresh app caches; OS/driver caches uncontrolled."}
with (root / "BATCH-RECEIPT.json").open("x") as f:
    json.dump(summary, f, indent=2, allow_nan=False)
    f.write("\n")
print(json.dumps(summary, indent=2, allow_nan=False))
