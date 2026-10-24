#!/usr/bin/env python3
"""Normal route in GX using recorded KPAD only; no Accessibility or host events.

python3 -B normal_tape_gate.py APP SHA256 SOURCE LABEL [--tape DIR]
  --build-kind server-candidate|dev-m1|frozen54 --mode whole-session|segments

Whole-session acceptance uses a server-built candidate. Segments mode records the
frozen-5.4 baseline and cannot pass the whole-session check. Tape/app hash equality
is evidence, not a gate: a candidate necessarily differs from the recording build.
The unlocked fixture and the supplied app are read-only; the run uses a fresh save
copy, private pipeline cache and an already prepared perf bundle identifier.
"""
import argparse
import csv
import fcntl
import hashlib
import json
import math
import os
import plistlib
import re
import shutil
import signal
import subprocess
import sys
import time
from pathlib import Path

HOME = Path.home()
ROOT = HOME / "MacioGalaxy2-gamedata/port53/m1g"
SEED = HOME / "MacioGalaxy2-gamedata/fixtures/unlocked-242/fixture"
DISC = HOME / "MacioGalaxy2-gamedata/SB4E01-extracted"
INIT_OFFSET_LIMIT = 30
HEAVY = HOME / "MacioGalaxy2-team/slots/m1-heavy.lock"
ANCHOR_POLICY = {
    "id": "scene-identity",
    "authority": "PORT ruling 2026-10-24 superseding interactive-and-play-ready-v1",
    "anchor": "Feed and compare from the first row of each stage/scenario identity, never initialize-End or player-valid.",
    "comparison": "Compare float32 position bits, stage/scenario/alive/grounded on every row valid in both runs; count one-sided validity rows separately. Pointer buffers remain checked every input loop.",
    "init_offset_limit_loops": INIT_OFFSET_LIMIT,
    "play_ready": "Require exact state and natural RNG checkpoint; restore the same seed and verify read-back, without moving the read stream.",
    "normal533_checks": "Unchanged.",
}

CHECKS = ("scene_chain", "destination_300_consecutive_valid", "no_faults",
          "no_invalid_worldmap_request", "existing_unlocked_file1", "seed_unchanged",
          "binary_unchanged", "no_divergence", "whole_session_replay", "replay_completed",
          "lossless_tape", "tape_unchanged", "game_pointer_buffers_compared",
          "anchor_policy_applied", "play_ready_state_checks", "compared_rows_accounted", "init_offsets_within_bound")


def sha(path):
    h = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def tree(root):
    return {str(p.relative_to(root)): sha(p) for p in sorted(root.rglob("*")) if p.is_file()}


def accounted(s):
    keys = ("tape_frames", "rows_aligned", "rows_compared", "both_valid_rows_compared", "both_invalid_rows", "validity_mismatch_rows_skipped")
    return (all(type(s.get(k)) is int and s[k] >= 0 for k in keys)
            and s["rows_aligned"] == s["tape_frames"] + 1
            and s["rows_compared"] + s["validity_mismatch_rows_skipped"] == s["rows_aligned"]
            and s["both_valid_rows_compared"] + s["both_invalid_rows"] == s["rows_compared"])


def init_bounded(s):
    if (not all(k in s for k in ("init_offset_loops", "recorded_player_valid_scene_loop", "replayed_player_valid_scene_loop"))
            or type(s.get("init_offset_limit_loops")) is not int or s["init_offset_limit_loops"] != INIT_OFFSET_LIMIT
            or s.get("init_offset_within_bound") is not True):
        return False
    recorded, replayed = s.get("recorded_player_valid_scene_loop"), s.get("replayed_player_valid_scene_loop")
    if recorded is None and replayed is None:
        return (s.get("init_offset_loops") is None and s.get("both_valid_rows_compared") == 0
                and s.get("validity_mismatch_rows_skipped") == 0)
    return (all(type(v) is int and 0 <= v <= s.get("tape_frames", -1) for v in (recorded, replayed))
            and type(s.get("init_offset_loops")) is int
            and s["init_offset_loops"] == replayed - recorded
            and abs(s["init_offset_loops"]) <= INIT_OFFSET_LIMIT)


def evaluate(rows, log, replay):
    order, run, best = [], 0, 0
    for row in rows:
        stage = row.get("stage", "")
        if stage and (not order or order[-1] != stage):
            order.append(stage)
        try:
            valid = row["valid"] == "1" and row.get("dead", "0") == "0" and all(math.isfinite(float(row[c])) for c in "xyz")
        except (ValueError, KeyError):
            valid = False
        run = run + 1 if stage == "IslandFleetGalaxy" and row.get("scenario") == "1" and valid else 0
        best = max(best, run)
    faults = [line for line in log.splitlines() if re.search(
        r'FATAL|AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|unhandled tcg| in ".*" on line \d+\.', line)]
    segments = replay.get("segments", [])
    complete = bool(segments) and replay.get("finished") is True and all(
        s.get("started") is True and s.get("completed") is True
        and accounted(s) for s in segments)
    divergence = next((s for s in segments if s.get("diverged")), None)
    checks = {
        "scene_chain": order[:4] == ["FileSelect", "MarioFaceShipGalaxy", "WorldMap01Galaxy", "IslandFleetGalaxy"],
        "destination_300_consecutive_valid": best >= 300,
        "no_faults": not faults,
        "no_invalid_worldmap_request": not re.search(r"stage=WorldMap(?!0[1-8]Galaxy)\w*Galaxy", log),
        "existing_unlocked_file1": any(r.get("power_stars") == "242" and r.get("valid") == "1" for r in rows)
        and "FileSelector.Create" not in log and "FileSelector.StartupCreate" not in log,
        "no_divergence": complete and divergence is None and not replay.get("stopped"),
        "whole_session_replay": replay.get("mode") == "whole-session" and "jump cut" not in log,
        "replay_completed": complete,
        "game_pointer_buffers_compared": bool(segments) and replay.get("mode") == "whole-session" and all(
            s.get("pointer_rows_compared") == s.get("tape_frames", -1) for s in segments),
        "anchor_policy_applied": replay.get("anchor_policy") == ANCHOR_POLICY["id"] and bool(segments) and all(
            s.get("anchor") == "scene-identity" and s.get("comparison_anchor") == "scene-identity" for s in segments),
        "compared_rows_accounted": bool(segments) and all(accounted(s) for s in segments),
        "init_offsets_within_bound": bool(segments) and all(init_bounded(s) for s in segments),
        "play_ready_state_checks": bool(segments) and all(s.get("play_ready_required") is False or
            (s.get("play_ready_required") is True and
             all((s.get("play_ready_state_check") or {}).get(k) is True for k in ("checked", "passed", "position_bit_exact")) and
             ((s.get("play_ready_state_check") or {}).get("rng") or {}).get("verified") is True) for s in segments),
    }
    return checks, {"stage_order": order, "destination_max_consecutive_valid": best,
                    "faults": faults[:20], "first_divergence": divergence,
                    "replay_stopped": replay.get("stopped"),
                    "scene_anchors": [{k: s.get(k) for k in ("stage", "scenario", "anchor", "scene_anchor_recorded_frame",
                        "scene_anchor_replay_frame", "scene_anchor_frame_offset", "comparison_anchor",
                        "play_ready_required", "play_ready_state_check", "recorded_player_valid_frame", "replayed_player_valid_frame",
                        "recorded_player_valid_scene_loop", "replayed_player_valid_scene_loop", "init_offset_loops",
                        "init_offset_limit_loops", "init_offset_within_bound", "rows_aligned", "rows_compared",
                        "both_valid_rows_compared", "both_invalid_rows", "validity_mismatch_rows_skipped")} for s in segments],
                    "pointer_evidence": [{k: s.get(k) for k in ("stage", "reads", "pointing_reads", "pointer_rows_compared", "pointer_source",
                        "last_pointer_recorded", "last_pointer_replayed", "last_pointer_valid")} for s in segments]}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("app", type=Path)
    parser.add_argument("sha256")
    parser.add_argument("source")
    parser.add_argument("label")
    parser.add_argument("--tape", type=Path, default=ROOT / "tapes/tape-normal-54")
    parser.add_argument("--build-kind", choices=("server-candidate", "dev-m1", "frozen54"), required=True)
    parser.add_argument("--mode", choices=("whole-session", "segments"), default="whole-session")
    parser.add_argument("--bound", type=float, default=540)
    parser.add_argument("--allow-incomplete", action="store_true", help="use a recording checkpoint; recorded in RESULT")
    a = parser.parse_args(argv)
    if Path(a.label).name != a.label or a.label in (".", ".."):
        parser.error("LABEL must be a single new directory name")
    app, tape = a.app.expanduser().resolve(), a.tape.expanduser().resolve()
    if app.is_relative_to(HOME / "MacioGalaxy2-play") or app.is_relative_to(HOME / "MacioGalaxy2-team/scratch-release"):
        parser.error("prepare an owned perf copy outside frozen builds")
    exe = app / "Contents/MacOS/Petari"
    if sha(exe) != a.sha256:
        parser.error("app executable does not match SHA256")
    with (app / "Contents/Info.plist").open("rb") as stream:
        bundle = plistlib.load(stream).get("CFBundleIdentifier")
    if bundle not in ("dev.petari.Petari.perfm1b", "dev.petari.Petari.perf54r"):
        parser.error("app must have SOL B's perfm1b or tape-exact perf54r identifier")
    meta = json.loads((tape / "route.tape.json").read_text())
    if not meta.get("complete") and not a.allow_incomplete:
        parser.error("recording incomplete; --allow-incomplete explicitly permits its checkpoint")
    if meta.get("lossy_samples", 0) or meta.get("unencodable_samples", 0):
        parser.error("lossy or unencodable tape cannot be an exact replay")
    before = tree(SEED)
    if before != json.loads((SEED.parent / "receipt.json").read_text())["files"]:
        parser.error("unlocked fixture differs from its receipt")
    tape_before = tree(tape)
    case = ROOT / a.label
    case.mkdir(parents=True, exist_ok=False, mode=0o700)
    subprocess.run(["/usr/bin/ditto", str(SEED), str(case / "user")], check=True)
    if tree(case / "user") != before:
        raise RuntimeError("save copy does not match fixture")
    (case / "cache").mkdir()
    env = {k: v for k, v in os.environ.items() if not k.startswith(("PETARI_", "ASTRAL_"))}
    mods = meta.get("mods", [])
    env.update(PETARI_TRACE_BOOT="1", PETARI_MODS=",".join(mods + (["-OdysseyJumpDive"] if "OdysseyJumpDive" not in mods else [])),
               PETARI_CACHE_DIR=str(case / "cache"), PETARI_FRAME_CSV=str(case / "frames.csv"),
               PETARI_ROUTE_RECORD=str(case / "route.csv"), PETARI_TAPE_PLAY_OUT=str(case / "replay"),
               PETARI_TAPE_PLAY_SESSION="1" if a.mode == "whole-session" else "0", PETARI_TAPE_PLAY_QUIT="1",
               PETARI_FRAME_STATS="1", PETARI_AUDIO_DIAG="1", PETARI_SMOKE_REAL_AUDIO="1", LANG="C", LC_ALL="C")
    for key, value in (meta.get("replay_env") or {}).items():
        if key != "PETARI_TEST_RETURN_TO_SHIP":
            parser.error(f"unsupported recorded scene hook: {key}")
        env[key] = str(value)
    cmd = [str(exe), "--disc", str(DISC), "--user", str(case / "user"), "--replay-tape", str(tape)]
    checks = dict.fromkeys(CHECKS, False)
    result = {"passed": False, "checks": checks, "source": a.source, "build_kind": a.build_kind,
              "anchor_policy": ANCHOR_POLICY["id"], "anchor_policy_details": ANCHOR_POLICY,
              "locks": {"heavy": str(HEAVY), "heavy_kind": "lockf -k", "order": ["heavy", "native"]},
              "gate_script_sha256": sha(Path(__file__)),
              "binary_sha256": a.sha256, "tape_exe_sha256": meta.get("exe_sha256"),
              "tape_exe_hash_matches_app": meta.get("exe_sha256") == a.sha256,
              "hash_comparison_gated": False, "tape": str(tape), "case": str(case), "bundle_id": bundle,
              "recording_complete": meta.get("complete"), "mode": a.mode, "host_input_used": False,
              "priority": {"nice": os.getpriority(os.PRIO_PROCESS, 0), "reason": "inherited; KPAD is game-frame driven"}}
    start = None
    child = None
    try:
        with (HOME / "MacioGalaxy2-team/slots/native.lock").open("a") as lock:
            fcntl.lockf(lock, fcntl.LOCK_EX)
            if shutil.disk_usage(case).free < 50 * 1024 ** 3:
                raise RuntimeError("less than 50 GiB free")
            from common import wait_quiet
            wait_quiet(case / "quiet.json")
            start = time.time()
            (case / "command.json").write_text(json.dumps({**result, "command": cmd, "bound_seconds": a.bound,
                "environment": {k: v for k, v in env.items() if k.startswith("PETARI_")},
                "save_seed": str(SEED), "save_before": before, "start_epoch": start}, indent=2) + "\n")
            with (case / "app.log").open("wb") as log:
                child = subprocess.Popen(cmd, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                try:
                    child.wait(timeout=a.bound)
                    result["timed_out"] = False
                except subprocess.TimeoutExpired:
                    result["timed_out"] = True
                finally:
                    if child.poll() is None:
                        os.killpg(child.pid, signal.SIGTERM)
                        try:
                            child.wait(timeout=20)
                        except subprocess.TimeoutExpired:
                            os.killpg(child.pid, signal.SIGKILL)
                            child.wait()
            result["exit"] = child.returncode
            route = case / "route.csv"
            rows = list(csv.DictReader(route.open())) if route.exists() else []
            path = case / "replay/divergence.json"
            replay = json.loads(path.read_text()) if path.exists() else {}
            evaluated, evidence = evaluate(rows, (case / "app.log").read_text(errors="replace"), replay)
            checks.update(evaluated)
            checks.update(seed_unchanged=tree(SEED) == before, binary_unchanged=sha(exe) == a.sha256,
                          lossless_tape=not meta.get("lossy_samples", 0) and not meta.get("unencodable_samples", 0),
                          tape_unchanged=tree(tape) == tape_before)
            result.update(evidence)
            checks["replay_completed"] &= child.returncode == 0 and not result["timed_out"]
            result["passed"] = all(checks.values())
    except Exception as exc:
        result["error"] = f"{type(exc).__name__}: {exc}"
    finally:
        result["elapsed_seconds"] = time.time() - start if start else 0
        result["acceptance_passed"] = result["passed"] and a.build_kind == "server-candidate"
        # Preserve logs and summaries, discard duplicate tape/captures and the private shader cache before another run.
        for name in ("route.csv", "route.kpd", "route.actions", "route.tape.json"):
            (case / "replay" / name).unlink(missing_ok=True)
        for name in ("route.kpd", "route.actions", "route.tape.json"):
            (case / name).unlink(missing_ok=True)
        for path in (case / "cache", case / "user"):
            shutil.rmtree(path, ignore_errors=True)
        (case / "RESULT.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    # Acquire heavy before main takes native.lock. The guard is set only in the lockf child and removed before app launch.
    if os.environ.pop("_M1_TAPE_GATE_HEAVY_HELD", None) != "1":
        HEAVY.parent.mkdir(parents=True, exist_ok=True)
        os.execv("/usr/bin/lockf", ["/usr/bin/lockf", "-k", str(HEAVY), "/usr/bin/env", "_M1_TAPE_GATE_HEAVY_HELD=1",
                                 sys.executable, "-B", str(Path(__file__).resolve()), *sys.argv[1:]])
    sys.exit(main())
