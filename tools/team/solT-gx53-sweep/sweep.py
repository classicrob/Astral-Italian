#!/usr/bin/env python3
"""Bounded production GX direct-entry idle observations. Never reads app logs."""
import argparse
import csv
import datetime
import fcntl
import hashlib
import json
import math
import os
import re
from pathlib import Path
import shutil
import sqlite3
import statistics
import subprocess
import sys
import tempfile
import time

SOURCE = "33e71ee880b426c0c7da03a724377200e874c417"
MANIFEST_SHA = "b25e906dd6454964506421989c241eb17fe9f1e159f1ec26b91ea48b1869459a"
SERVER = "host_amemqempmf"
INITIAL = ["RedBlueExGalaxy", "MokumokuValleyGalaxy", "YosshiHomeGalaxy"]
FLOOR = 50 * 1024**3
SECONDS = 90
PROVIDERS = ["native/app/frame_seam.cpp", "native/app/frame_stats.cpp",
             "native/app/smoke_game.cpp", "native/app/route_record_writer.hpp",
             "native/gx/present/present_aurora.cpp", "native/tools/bounded_run.py"]


def digest(path):
    h = hashlib.sha256()
    with Path(path).open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False) + "\n")


def files(root):
    """Only for owned cache/user inputs; never used on app logs or old outputs."""
    root = Path(root)
    result = {}
    for p in sorted(root.rglob("*")):
        if p.is_symlink():
            raise ValueError("cache/user symlink is not an independent copy")
        if p.is_file():
            result[str(p.relative_to(root))] = {"bytes": p.stat().st_size, "sha256": digest(p)}
    return result


def finite(value):
    try:
        x = float(value)
        return x if math.isfinite(x) else None
    except (ValueError, TypeError):
        return None


def cache_metadata(root):
    result = {}
    for p in sorted(Path(root).glob("*.db")):
        key = re.search(r"\.([a-f0-9]{64})\.db$", p.name)
        row = {"cache_key": key[1] if key else "NOT_MEASURED", "query_mode": "read-only"}
        try:
            db = sqlite3.connect(p.resolve().as_uri() + "?mode=ro", uri=True, timeout=2)
            try:
                db.execute("PRAGMA query_only=ON")
                names = [r[0] for r in db.execute("SELECT name FROM sqlite_schema WHERE type='table'")]
                row["table_rows"] = {name: db.execute('SELECT COUNT(*) FROM "' + name.replace('"', '""') + '"').fetchone()[0] for name in names}
            finally:
                db.close()
        except sqlite3.Error:
            row["table_rows"] = "NOT_MEASURED: read-only cache metadata unavailable"
        result[p.name] = row
    return result


def summarize_metrics(case, stage):
    result = {"route": "NOT_MEASURED", "frame_timing": "NOT_MEASURED",
              "gpu_time": "NOT_MEASURED", "scanout_latency": "NOT_MEASURED",
              "visual_dolphin_diff": "NOT_MEASURED: no qualified paired reference",
              "crash_firstcause": "UNKNOWN: app logs deliberately unread"}
    route = case / "route.csv"
    if route.is_file():
        count = valid = longest = streak = 0
        previous = None
        for row in csv.DictReader(route.open()):
            count += 1
            frame = finite(row.get("frame"))
            good = (row.get("stage") == stage and row.get("scenario") == "1"
                    and row.get("valid") == "1" and row.get("dead") == "0"
                    and all(finite(row.get(k)) is not None for k in ("x", "y", "z")))
            if good:
                valid += 1
                streak = streak + 1 if previous is not None and frame == previous + 1 else 1
                longest = max(longest, streak)
            else:
                streak = 0
            previous = frame
        result["route"] = {"rows": count, "valid_target_alive_rows": valid,
                           "longest_consecutive_valid_target_alive": longest,
                           "provider": "native/app/route_record_writer.hpp",
                           "sha256": digest(route)}
    frame_csv = case / "frames.csv"
    if frame_csv.is_file():
        values, phases, unfocused, rows = [], {}, 0, 0
        pipeline = 0
        pipeline_measured = False
        for row in csv.DictReader(frame_csv.open()):
            rows += 1
            phase = row.get("phase", "unknown")
            phases[phase] = phases.get(phase, 0) + 1
            unfocused += row.get("unfocused") == "1"
            value = finite(row.get("interval_ms"))
            if value is not None and value >= 0:
                values.append(value)
            n = finite(row.get("pipeline_resolves"))
            if n is not None and n >= 0:
                pipeline += n
                pipeline_measured = True
        values.sort()
        percentile = lambda q: values[min(len(values) - 1, round((len(values) - 1) * q))] if values else None
        result["frame_timing"] = {"rows": rows, "finite_intervals": len(values),
            "phase_rows": phases, "unfocused_rows": unfocused,
            "mean_ms": statistics.fmean(values) if values else None,
            "p50_ms": percentile(.5), "p95_ms": percentile(.95), "p99_ms": percentile(.99),
            "max_ms": max(values) if values else None,
            "pipeline_resolves": pipeline if pipeline_measured else "NOT_MEASURED",
            "provider": "native/app/frame_stats.cpp", "sha256": digest(frame_csv),
            "limits": "All recorded phases; no route-frame/presentation-frame equality claim"}
    return result


def verify(handoff_path, manifest_path, selection_path=None):
    h = json.loads(handoff_path.read_text())
    assert h["source_sha"] == SOURCE and h["host_id"] == SERVER
    assert h["production_only"] is True and h["extra_game_fixes"] == []
    assert h["explicit_L_artifact_and_lock_release"] and h["artifact_key"]
    assert digest(h["artifact_receipt"]) == h["artifact_receipt_sha256"]
    assert digest(h["executable"]) == h["executable_sha256"]
    assert digest(h["bounded_runner"]) == h["bounded_runner_sha256"]
    assert digest(manifest_path) == MANIFEST_SHA
    m = json.loads(manifest_path.read_text())
    assert m["supported_count"] == len(m["supported_destinations"]) == 60
    assert m["selected_scenario"] == 1
    assert all(s in m["supported_destinations"] for s in INITIAL)
    selection, stages, allocation = None, INITIAL, 1200
    if selection_path:
        selection = json.loads(selection_path.read_text())
        assert selection["source_sha"] == SOURCE and selection["manifest_sha256"] == MANIFEST_SHA
        stages = [row["stage"] for row in selection["stages"]]
        assert len(stages) == len(set(stages)) == 6
        assert all(row["scenario"] == 1 and row["scenario1_metadata_present"] for row in selection["stages"])
        assert all(s in m["supported_destinations"] and not s.startswith("WorldMap") for s in stages)
        assert digest(h["prior_receipt"]) == selection["prior_batch_receipt_sha256"]
        prior = json.loads(Path(h["prior_receipt"]).read_text())
        assert prior["source_sha"] == SOURCE and prior["artifact_sha256"] == h["executable_sha256"]
        assert set(stages).isdisjoint(prior["observed_fresh_and_warm_galaxies"])
        allocation = selection["max_runtime_seconds"]
        assert allocation == 1500
    source = Path(h["source_root"])
    assert subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=source, text=True).strip() == SOURCE
    assert not subprocess.check_output(["git", "diff", "HEAD", "--"], cwd=source)
    for provider in PROVIDERS:
        expected = subprocess.check_output(["git", "show", SOURCE + ":" + provider], cwd=source)
        assert hashlib.sha256(expected).hexdigest() == digest(source / provider)
    assert files(h["private_seed"]) == h["private_seed_files"]
    assert Path(h["disc"]).is_dir()
    return h, m, stages, allocation, selection


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--handoff", type=Path, required=True)
    p.add_argument("--manifest", type=Path, required=True)
    p.add_argument("--output-parent", type=Path, required=True)
    p.add_argument("--state", type=Path, required=True)
    p.add_argument("--cleanup-success-caches", action="store_true")
    p.add_argument("--batch-id", help="preannounced new private directory name")
    p.add_argument("--selection", type=Path)
    a = p.parse_args()
    h, manifest, stages, allocation, selection = verify(a.handoff, a.manifest, a.selection)
    assert "MacioGalaxy2-gamedata" in a.output_parent.parts
    a.output_parent.mkdir(parents=True, mode=0o700, exist_ok=True)
    if a.batch_id:
        assert re.fullmatch(r"gx53-batch[0-9]+-[A-Za-z0-9_-]+", a.batch_id)
        out = a.output_parent / a.batch_id
        out.mkdir(mode=0o700)
    else:
        out = Path(tempfile.mkdtemp(prefix="gx53-batch1-", dir=a.output_parent))
    os.chmod(out, 0o700)
    a.state.parent.mkdir(parents=True, exist_ok=True)
    def state(phase, child="none"):
        a.state.write_text(f"UTC={datetime.datetime.now(datetime.timezone.utc).isoformat()} phase={phase} "
            f"inputs={SOURCE};manifest={MANIFEST_SHA};artifact={h['executable_sha256']} "
            f"output={out} controllerPID={os.getpid()} childPID={child} resume=inspect-status-json-only;logs-UNREAD\n")
    state("PRE-LOCK")
    lock = Path(h["native_lock"]).open("a")
    fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    os.setpriority(os.PRIO_PROCESS, 0, 19)
    started = time.monotonic()
    rows = []
    source_providers = {name: digest(Path(h["source_root"]) / name) for name in PROVIDERS}
    write(out / "batch.json", {"source_sha": SOURCE, "handoff": h,
        "handoff_sha256": digest(a.handoff), "manifest_sha256": MANIFEST_SHA,
        "runner_sha256": digest(__file__), "source_providers": source_providers,
        "coverage_inventory": [{"stage": s, "status": "PLANNED_BATCH" if s in stages else "NOT_RUN_WORLDMAP_OUT_OF_SCOPE" if s.startswith("WorldMap") else "NOT_RUN"} for s in manifest["supported_destinations"]],
        "planned_stages": stages, "selection": selection,
        "selection_sha256": digest(a.selection) if a.selection else None,
        "case_wall_seconds": SECONDS, "max_runtime_seconds": allocation,
        "input": "neutral idle; no SMOKE driver or injected input",
        "cache_scope": "fresh/warm app cache only; OS/driver cache uncontrolled",
        "coverage": "direct entry only; normal path and visual comparison separate"})
    print(json.dumps({"batch": str(out), "controller_pid": os.getpid()}), flush=True)
    try:
        for stage in stages:
            if (out / "STOP-AFTER-CURRENT").exists():
                break
            fresh = None
            successful = []
            for mode in ("fresh", "warm"):
                if (out / "STOP-AFTER-CURRENT").exists():
                    break
                if mode == "warm" and fresh is None:
                    rows.append({"stage": stage, "mode": mode, "status": "NOT_RUN: fresh case failed or cache unavailable"})
                    continue
                assert time.monotonic() - started + SECONDS + 20 <= allocation, "batch allocation exhausted"
                free = shutil.disk_usage(out).free
                assert free >= FLOOR, "50 GiB floor; no global pruning permitted"
                assert digest(h["executable"]) == h["executable_sha256"]
                case = out / (stage + "-" + mode)
                case.mkdir(mode=0o700)
                if h.get("seed_mode") == "empty":
                    assert h["private_seed_files"] == {}
                    (case / "user").mkdir(mode=0o700)
                else:
                    shutil.copytree(h["private_seed"], case / "user")
                assert files(case / "user") == h["private_seed_files"]
                if mode == "fresh":
                    (case / "cache").mkdir(mode=0o700)
                    initial_cache = {}
                else:
                    shutil.copytree(fresh / "cache", case / "cache")
                    initial_cache = files(case / "cache")
                    assert initial_cache and initial_cache == files(fresh / "cache")
                initial_cache_metadata = cache_metadata(case / "cache")
                for name in ("tmp", "surface", "xfb"):
                    (case / name).mkdir(mode=0o700)
                env = {k: v for k, v in os.environ.items() if not k.startswith(("PETARI_", "ASTRAL_"))}
                env.update(PETARI_MODS="none,-OdysseyJumpDive", PETARI_CACHE_DIR=str(case / "cache"),
                    PETARI_FRAME_CSV=str(case / "frames.csv"), PETARI_FRAME_CSV_FRAMES="12000",
                    PETARI_ROUTE_RECORD=str(case / "route.csv"), TMPDIR=str(case / "tmp") + "/", LANG="C", LC_ALL="C")
                for kind in ("SURFACE", "XFB"):
                    env["PETARI_" + kind + "_DUMP"] = str(case / kind.lower())
                    env["PETARI_" + kind + "_DUMP_LABELS"] = "boot"
                    env["PETARI_" + kind + "_DUMP_EVERY"] = "1800"
                    env["PETARI_" + kind + "_DUMP_SPAN"] = "3601"
                cmd = [sys.executable, h["bounded_runner"], str(SECONDS), h["executable"],
                    "--disc", h["disc"], "--user", str(case / "user"), "--stage", stage, "--scenario", "1"]
                write(case / "command.json", {"argv": cmd, "environment": {k: v for k, v in env.items() if k.startswith("PETARI_")},
                    "cache_mode": mode, "initial_cache": initial_cache, "initial_cache_metadata": initial_cache_metadata,
                    "warm_source": str(fresh) if mode == "warm" else None,
                    "seed_files": h["private_seed_files"], "disk_free_before_bytes": free})
                state("PRE-SPAWN-" + stage + "-" + mode)
                begin = time.monotonic()
                with (case / "app-UNREAD.log").open("xb") as log:
                    child = subprocess.Popen(cmd, cwd=h["source_root"], env=env, stdout=log, stderr=subprocess.STDOUT)
                    state("RUNNING-" + stage + "-" + mode, child.pid)
                    rc = child.wait()
                elapsed = time.monotonic() - begin
                state("COMPLETED-" + stage + "-" + mode)
                final_cache_metadata = cache_metadata(case / "cache")
                result = {"label": "SERVER GX5.3 direct-entry idle", "source_sha": SOURCE,
                    "artifact_sha256": h["executable_sha256"], "stage": stage, "scenario": 1, "mode": mode, "exit_code": rc,
                    "elapsed_seconds": round(elapsed, 3), "log": "app-UNREAD.log",
                    "log_bytes": (case / "app-UNREAD.log").stat().st_size, "logs_read": False,
                    "firstcause": "UNKNOWN; no log/core analysis permitted", "metrics": "NOT_MEASURED",
                    "status": "BROKEN_OR_UNKNOWN", "cache_final": files(case / "cache"), "cache_metadata": final_cache_metadata,
                    "captures": {k: [{"name": f.name, "bytes": f.stat().st_size} for f in sorted((case / k).glob("*.png"))] for k in ("surface", "xfb")}}
                if rc in (0, 124) and elapsed >= SECONDS - 1:
                    try:
                        result["metrics"] = summarize_metrics(case, stage)
                        route = result["metrics"]["route"]
                        timing = result["metrics"]["frame_timing"]
                        if isinstance(route, dict) and route["longest_consecutive_valid_target_alive"] >= 300 and isinstance(timing, dict) and timing["finite_intervals"] >= 300:
                            result["status"] = "BOUNDED_IDLE_OBSERVED"
                    except (OSError, ValueError, csv.Error, KeyError, TypeError):
                        result["metrics"] = "NOT_MEASURED: incomplete/invalid metrics channel"
                # No log/fault text parsing, retries, core reads, or failure diagnosis.
                result["binary_unchanged"] = digest(h["executable"]) == h["executable_sha256"]
                if not result["binary_unchanged"]:
                    result["status"] = "BROKEN_OR_UNKNOWN"
                write(case / "result.json", result)
                rows.append({"stage": stage, "mode": mode, "status": result["status"], "case": str(case),
                    "exit_code": rc, "elapsed_seconds": result["elapsed_seconds"]})
                write(out / "results.json", rows)
                print(json.dumps(rows[-1]), flush=True)
                if result["status"] == "BOUNDED_IDLE_OBSERVED":
                    successful.append(case)
                    if mode == "fresh" and result["cache_final"]:
                        fresh = case
                else:
                    if mode == "fresh":
                        rows.append({"stage": stage, "mode": "warm", "status": "NOT_RUN: fresh case failed; no retry"})
                        write(out / "results.json", rows)
                    break
            if len(successful) == 2 and a.cleanup_success_caches:
                cleanup = []
                for case in successful:
                    inventory = files(case / "cache")
                    write(case / "cache-retention-manifest.json", inventory)
                    cleanup.append({"case": str(case), "bytes_freed": sum(x["bytes"] for x in inventory.values())})
                    shutil.rmtree(case / "cache")
                write(out / (stage + "-cleanup.json"), cleanup)
        assert files(h["private_seed"]) == h["private_seed_files"], "source seed changed"
        write(out / "closed.json", {"status": "BATCH_CLOSED", "rows": rows,
            "elapsed_seconds": round(time.monotonic() - started, 3), "disk_free_after_bytes": shutil.disk_usage(out).free,
            "no_log_analysis": True, "no_worldmap_cases": True, "promotion": False})
    finally:
        fcntl.flock(lock, fcntl.LOCK_UN)
        lock.close()
        state("EXPLICIT-RELEASED")


if __name__ == "__main__":
    main()
