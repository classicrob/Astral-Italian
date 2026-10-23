#!/usr/bin/env python3
"""All-galaxy health sweep in the headless harness (petari_headless + gx_null).

For every galaxy and scenario on the disc, enter the stage exactly as `--stage/--scenario` does, wait for the game's own
play-ready seam, then run a bounded scripted segment (idle, walk, jump, spin, ground pound, camera) through the real
keyboard -> WPAD -> KPAD path. Classifies each run from the exit status, the log and the per-loop trace
(Mario valid/finite/alive/responsive, late loops on the wall clock) and writes results.json, results.csv, table.md.

  python3 native/tools/headless_sweep.py run --headless <petari_headless> --disc <extracted disc> --out <new dir> \
      [--stages A,B] [--scenarios 1-3] [--jobs 4] [--frames 3600] [--script default|<file>] [--rerun]
  python3 native/tools/headless_sweep.py list --disc <extracted disc>
  python3 native/tools/headless_sweep.py report --out <dir>      # re-classify logs + traces with the current parser

Outcomes: PASS, PASS_NOTE (passed with a note: Mario died, heap/alloc line, slow loops, late-frame outliers),
FAIL(<class>) with the first-fault log line.  Classes: CRASH (signal), ABORT (OSPanic/assert/_Exit), HARNESS (harness
refused: non-finite Mario state, startup request failed...), HANG (watchdog), NOT_READY (play-ready never reached),
MARIO_INVALID (player invalid after ready), STUCK (no response to input), HEAP (allocation failure), TIMEOUT.
"""
import argparse
import concurrent.futures
import csv
import json
import math
import os
import re
import shutil
import signal
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import stage_sweep  # noqa: E402  (disc enumeration: scenario data per stage)

LOG_CAP = 400_000  # bytes kept of one run's log (head + tail)

# One scripted segment: frames per line, then the held keys (native/CONTROLS.md names). ~60 s at 3600 loops.
DEFAULT_SCRIPT = """\
60
90 W
30
10 Space
50
3 F
40
90 W D
60 S
10 Space
20
10 LeftShift
30
60 Q
60 E
10 C
60 W
10 Space
40
3 F
30
3 F
60
90 D W
10 Space
60
120 A W
10 Space
60
20 LeftShift
120 S
60 W Space
60
90 E W
90 Q W
60
"""


def random_script(seed, total):
    """Seeded pseudo-random player: steady walking in a random direction with jumps, spins, ground pounds, crouch
    jumps, camera turns and the occasional pause/resume, in 10-70 loop bursts."""
    import random
    rng = random.Random(seed)
    lines, used = [], 0
    while used < total:
        n = min(rng.randint(10, 70), total - used)
        keys = set()
        r = rng.random()
        if r < 0.75:  # walk/run in a direction
            keys.add(rng.choice("WWWASD"))
            if rng.random() < 0.3:
                keys.add(rng.choice("AD"))
        if rng.random() < 0.25:
            keys.add("Space")
        if rng.random() < 0.15:
            keys.add("F")
        if rng.random() < 0.10:
            keys.add("LeftShift")
        if rng.random() < 0.15:
            keys.add(rng.choice(["Q", "E"]))
        if rng.random() < 0.03:
            keys.add("C")
        if rng.random() < 0.01 and n >= 4:  # pause and resume
            lines.append("2 Escape")
            lines.append("20")
            lines.append("2 Escape")
            used += 24
            continue
        lines.append(" ".join([str(n)] + sorted({{"LeftShift": "LeftShift"}.get(k, k) for k in keys})))
        used += n
    return "\n".join(lines) + "\n"


def script_text(total, text):
    """Repeat the base script until it has `total` loops (the last repetition is cut by the harness's own end)."""
    lines = [l for l in text.splitlines() if l.strip()]
    length = sum(int(l.split()[0]) for l in lines)
    reps = max(1, math.ceil(total / length))
    out, used = [], 0
    for _ in range(reps):
        for l in lines:
            n = int(l.split()[0])
            if used + n > total:
                n = total - used
                l = " ".join([str(n)] + l.split()[1:])
            if n > 0:
                out.append(l)
            used += n
            if used >= total:
                return "\n".join(out) + "\n"
    return "\n".join(out) + "\n"


FIRST_FAULT = [
    ("HEAP", re.compile(r"(heap|alloc)[^\n]{0,60}(fail|exhaust|out of memory|no memory|OSAlloc.*null)", re.I)),
    ("ABORT", re.compile(r"OSPanic|OSFatal|Failed assertion|assertion failed|Halt|^\s*Panic|abort\(\)|PETARI_ASSERT", re.I)),
    ("UBSAN", re.compile(r"runtime error:")),
    ("ASAN", re.compile(r"AddressSanitizer|ERROR: ")),
    ("HARNESS", re.compile(r"^headless: (?!rate|completed|scoped|startup target)")),
    ("NATIVE", re.compile(r"native (stub|check|abort)|unsupported|not implemented|PETARI_TRAP|trap undefined", re.I)),
]
WARN_LINES = re.compile(r"\[alloc\]|\[stub\]|missing|not found|failed|cannot open|could not|unknown|invalid|overflow", re.I)
SKIP_LINES = re.compile(r"^(headless: (rate|completed|scoped|startup)|headless GX|headless sweep|\[sweep-stall\]|\[input-script\]|stage launch:)")


def first_lines(text, pattern, limit=1):
    found = []
    for line in text.splitlines():
        if pattern.search(line):
            found.append(line.strip()[:240])
            if len(found) >= limit:
                break
    return found


def trim_log(path):
    size = path.stat().st_size
    if size <= LOG_CAP:
        return
    data = path.read_bytes()
    keep = LOG_CAP // 2
    path.write_bytes(data[:keep] + b"\n[... %d bytes trimmed ...]\n" % (size - 2 * keep) + data[-keep:])


def read_trace(path):
    """Per-loop Mario summary from the trace CSV."""
    if not path.is_file():
        return None
    rows = []
    with path.open(newline="") as f:
        for row in csv.DictReader(f):
            rows.append(row)
    return rows


def f(row, key):
    try:
        return float(row[key])
    except (KeyError, ValueError):
        return float("nan")


def analyze_trace(rows):
    info = {"rows": len(rows)}
    if not rows:
        return info
    valid = [r for r in rows if r.get("player_valid") == "1"]
    info["player_valid_rows"] = len(valid)
    info["scene_not_game_rows"] = sum(1 for r in rows if r.get("scene") != "Game")
    if not valid:
        return info
    nonfinite = [i for i, r in enumerate(valid) if not all(math.isfinite(f(r, k)) for k in ("x", "y", "z", "vx", "vy", "vz"))]
    info["nonfinite_rows"] = len(nonfinite)
    xs = [(f(r, "x"), f(r, "y"), f(r, "z")) for r in valid]
    info["start"] = [round(v, 1) for v in xs[0]]
    info["end"] = [round(v, 1) for v in xs[-1]]
    path = sum(math.dist(a, b) for a, b in zip(xs, xs[1:]) if all(math.isfinite(c) for c in a + b))
    info["path_len"] = round(path, 1)
    info["max_abs_coord"] = round(max(max(abs(c) for c in p if math.isfinite(c)) for p in xs), 1)
    info["min_y"] = round(min(p[1] for p in xs), 1)
    deaths = sum(1 for a, b in zip(valid, valid[1:]) if a.get("dead") != "1" and b.get("dead") == "1")
    info["deaths"] = deaths
    info["final_hp"] = valid[-1].get("hp")
    info["anims"] = len({r.get("anim_name") for r in valid})
    modes = {}
    for r in valid:
        modes[r.get("mode")] = modes.get(r.get("mode"), 0) + 1
    info["modes"] = modes
    states = {r.get("state_ids") for r in valid}
    info["state_sets"] = len(states)
    # A scene reload (Mario died or fell: init_state leaves 6) legitimately has no player; only an invalid player in an
    # established scene (init_state 6) is a fault.
    info["invalid_after_valid"] = sum(1 for r in rows if r.get("player_valid") != "1" and r.get("scene") == "Game" and r.get("init_state") == "6")
    info["reloads"] = sum(1 for a, b in zip(rows, rows[1:]) if a.get("init_state") == "6" and b.get("init_state") != "6")
    info["stage"] = valid[-1].get("stage")
    info["scenario"] = valid[-1].get("scenario")
    return info


PLAYER_MODE_ID = {"invincible": "1", "fire": "2", "ice": "3", "bee": "4", "hopper": "5", "teresa": "6", "foo": "7",
                  "yoshi": "10", "cloud": "12", "rock": "13"}


def stage_phase(text):
    """Last startup-progress line of a log, for a run that never got ready."""
    seen = [l.strip()[:160] for l in text.splitlines() if l.startswith(("stage launch:", "headless: startup target"))]
    return seen[-1] if seen else ""


def classify(res):
    """res: dict with rc, timed_out, log text, trace info, started. Returns (outcome, class, first_fault, notes)."""
    text = res["log"]
    rc = res["rc"]
    notes = []
    started = "[input-script] start" in text
    done = "[input-script] done" in text and "headless: completed" in text
    t = res.get("trace") or {}

    def fault(cls, line=None):
        lines = first_lines(text, FIRST_FAULT_ALL[cls], 1) if cls in FIRST_FAULT_ALL else []
        return "FAIL", cls, (line or (lines[0] if lines else "")), notes

    # warnings & notes from a passing run
    if t.get("deaths"):
        notes.append(f"Mario died {t['deaths']}x")
    if t.get("reloads"):
        notes.append(f"stage restarted {t['reloads']}x (death/fall)")
    allocs = [l for l in text.splitlines() if l.startswith("[alloc]") and "<- MessageData::MessageData <-" not in l]
    if allocs:
        notes.append(f"{len(allocs)} unexpected [alloc] line(s): {allocs[0][:100]}")
    m = re.search(r"headless sweep: (\d+) scripted loops, wall ms p50 ([\d.]+) p99 ([\d.]+) max ([\d.]+), (\d+) over 50 ms", text)
    if m:
        res["wall"] = {"loops": int(m.group(1)), "p50": float(m.group(2)), "p99": float(m.group(3)), "max": float(m.group(4)), "over50": int(m.group(5))}
        if int(m.group(5)):
            notes.append(f"{m.group(5)} loop(s) >50 ms (max {m.group(4)} ms)")
    stalls = re.findall(r"\[sweep-stall\] loop (\d+) took ([\d.]+) ms", text)
    if stalls:
        res["stalls"] = [(int(a), float(b)) for a, b in stalls]

    if res["timed_out"]:
        return "FAIL", "TIMEOUT", (first_lines(text, re.compile(r"\S"), 0) or [""])[0], notes
    if rc is not None and rc < 0:
        sig = signal.Signals(-rc).name
        fl = first_lines(text, re.compile(r"runtime error|AddressSanitizer|ERROR"), 1)
        last = [l.strip()[:200] for l in text.splitlines()[-3:] if l.strip()]
        return "FAIL", f"CRASH({sig})", (fl[0] if fl else " | ".join(last)), notes
    if rc not in (0, None):
        if "play-ready not reached" in text:
            return "FAIL", "NOT_READY", "play-ready not reached within 60000 loops (" + (stage_phase(text) or "no phase info") + ")", notes
        for cls in ("HEAP", "ABORT", "HARNESS"):
            lines = first_lines(text, FIRST_FAULT_ALL[cls], 1)
            if lines:
                return "FAIL", cls, lines[0], notes
        last = [l.strip()[:200] for l in text.splitlines()[-3:] if l.strip()]
        return "FAIL", f"EXIT({rc})", " | ".join(last), notes
    if not started:
        reason = first_lines(text, re.compile(r"startup request|did not|not observed|failed", re.I), 1)
        return "FAIL", "NOT_READY", reason[0] if reason else "no [input-script] start", notes
    if not done:
        return "FAIL", "INCOMPLETE", "exit 0 without completion marker", notes
    if res.get("no_player"):
        return "NON_GAMEPLAY", "", "", notes + ["galaxy-map UI scene: ran %d loops without a crash; no Mario to check" % t.get("rows", 0)]
    if t.get("nonfinite_rows"):
        return "FAIL", "MARIO_NONFINITE", f"{t['nonfinite_rows']} rows with non-finite position/velocity", notes
    if t.get("invalid_after_valid"):
        return "FAIL", "MARIO_INVALID", f"player invalid on {t['invalid_after_valid']} rows after play-ready", notes
    if not t.get("player_valid_rows"):
        return "FAIL", "MARIO_INVALID", "player never valid in trace", notes
    want = PLAYER_MODE_ID.get(res.get("player_mode") or "")
    if want:
        held = (t.get("modes") or {}).get(want, 0)
        if not held:
            return "FAIL", "MODE_NOT_APPLIED", f"--player-mode {res['player_mode']} (mode {want}) never seen in the trace: {t.get('modes')}", notes
        notes.append(f"{res['player_mode']} mode held {held} loops")
    if t.get("path_len", 0) < 20 and not t.get("deaths"):
        return "FAIL", "STUCK", f"Mario moved {t.get('path_len')} units in {t['rows']} loops of scripted input", notes
    if t.get("max_abs_coord", 0) > 5e6 or t.get("min_y", 0) < -1e6:
        return "FAIL", "MARIO_OUT_OF_WORLD", f"coords up to {t['max_abs_coord']}, min y {t['min_y']}", notes
    warn = [l for l in text.splitlines() if WARN_LINES.search(l) and not SKIP_LINES.search(l)
            and not l.startswith("[alloc]")]
    if warn:
        res["warn_lines"] = [w.strip()[:200] for w in warn[:5]]
    for cls in ("HEAP", "UBSAN", "ASAN", "NATIVE"):
        lines = first_lines(text, FIRST_FAULT_ALL[cls], 1)
        if lines:
            notes.append(f"{cls}: {lines[0][:120]}")
    return ("PASS_NOTE" if notes else "PASS"), "", "", notes


FIRST_FAULT_ALL = dict(FIRST_FAULT)


def run_one(args, out, stage, scenario, script_path):
    name = f"{stage}-s{scenario}"
    run = out / "runs" / name
    result_path = run / "result.json"
    if result_path.is_file() and not args.rerun:
        return json.loads(result_path.read_text())
    if run.exists():
        shutil.rmtree(run)
    run.mkdir(parents=True)
    user = run / "user"
    log = run / "app.log"
    trace = run / "trace.csv"
    cmd = [str(args.headless), "--disc", str(args.disc), "--user", str(user), "--stage", stage, "--scenario", str(scenario),
           "--keyboard-script", str(script_path), "--trace", str(trace), "--epoch-ticks", "0"]
    if args.player_mode:
        cmd += ["--player-mode", args.player_mode]
    no_player = bool(re.fullmatch(r"WorldMap\d+Galaxy", stage))  # galaxy-map UI scenes: no Mario to drive
    if no_player:
        cmd += ["--no-player"]
    env = {"PATH": "/usr/bin:/bin", "HOME": os.environ.get("HOME", "/"), "SDL_AUDIODRIVER": "dummy", "PETARI_FAST_BOOT": "1",
           "TMPDIR": str(run), "ASAN_OPTIONS": "detect_leaks=0:abort_on_error=0", "UBSAN_OPTIONS": "print_stacktrace=0"}
    start = time.time()
    timed_out = False
    with log.open("wb") as lf:
        proc = subprocess.Popen(cmd, stdout=lf, stderr=subprocess.STDOUT, env=env, start_new_session=True)
        while True:
            try:
                proc.wait(timeout=2)
                break
            except subprocess.TimeoutExpired:
                if time.time() - start > args.timeout or log.stat().st_size > 400_000_000:
                    timed_out = time.time() - start > args.timeout
                    os.killpg(proc.pid, signal.SIGKILL)
                    proc.wait()
                    break
    elapsed = time.time() - start
    rc = proc.returncode
    trim_log(log)
    shutil.rmtree(user, ignore_errors=True)
    return finish(args, out, stage, scenario, run, rc, timed_out, elapsed)


def finish(args, out, stage, scenario, run, rc, timed_out, elapsed):
    no_player = bool(re.fullmatch(r"WorldMap\d+Galaxy", stage))
    log = run / "app.log"
    text = log.read_text(errors="replace") if log.is_file() else ""
    rows = read_trace(run / "trace.csv")
    info = analyze_trace(rows) if rows is not None else {}
    if rows is None and (run / "result.json").is_file():  # a passing run's bulky trace was dropped: keep its summary
        info = json.loads((run / "result.json").read_text()).get("trace", {})
    res = {"player_mode": args.player_mode, "stage": stage, "scenario": scenario, "rc": rc, "timed_out": timed_out, "elapsed": round(elapsed, 1), "log": text, "trace": info, "no_player": no_player}
    outcome, cls, fault, notes = classify(res)
    res.update({"outcome": outcome, "class": cls, "first_fault": fault, "notes": notes})
    res.pop("log")
    (run / "result.json").write_text(json.dumps(res, indent=1))
    if not args.keep_trace and outcome in ("PASS", "PASS_NOTE") and (run / "trace.csv").is_file():
        (run / "trace.csv").unlink()
        for side in run.glob("trace.csv.*"):
            side.unlink()
    return res


def write_reports(out, results):
    results = sorted(results, key=lambda r: (r["stage"], r["scenario"]))
    (out / "results.json").write_text(json.dumps(results, indent=1))
    with (out / "results.csv").open("w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["stage", "scenario", "outcome", "class", "first_fault", "notes", "elapsed_s", "path_len", "deaths", "wall_max_ms"])
        for r in results:
            t = r.get("trace", {})
            w.writerow([r["stage"], r["scenario"], r["outcome"], r["class"], r["first_fault"], "; ".join(r["notes"]), r["elapsed"],
                        t.get("path_len", ""), t.get("deaths", ""), r.get("wall", {}).get("max", "")])
    by = {}
    for r in results:
        by.setdefault(r["stage"], {})[r["scenario"]] = r
    maxs = max((s for v in by.values() for s in v), default=1)
    counts = {}
    for r in results:
        key = r["outcome"] if r["outcome"] != "FAIL" else f"FAIL({r['class']})"
        counts[key] = counts.get(key, 0) + 1
    lines = ["# Headless health sweep", "", f"{len(results)} runs over {len(by)} galaxies.", ""]
    lines += ["| outcome | runs |", "|---|---:|"] + [f"| {k} | {v} |" for k, v in sorted(counts.items(), key=lambda kv: -kv[1])] + [""]
    lines += ["Legend: `.` PASS, `n` PASS_NOTE, `g` NON_GAMEPLAY (galaxy-map UI, ran without crash), `X` FAIL, blank: scenario absent.", ""]
    lines += ["| galaxy | " + " | ".join(f"s{i}" for i in range(1, maxs + 1)) + " |", "|---|" + "---|" * maxs]
    for stage in sorted(by):
        cells = []
        for i in range(1, maxs + 1):
            r = by[stage].get(i)
            cells.append("" if not r else "." if r["outcome"] == "PASS" else "n" if r["outcome"] == "PASS_NOTE" else "g" if r["outcome"] == "NON_GAMEPLAY" else f"X {r['class']}")
        lines.append(f"| {stage} | " + " | ".join(cells) + " |")
    lines += ["", "## Failures (first fault)", "", "| galaxy | s | class | first fault |", "|---|---:|---|---|"]
    for r in results:
        if r["outcome"] == "FAIL":
            lines.append(f"| {r['stage']} | {r['scenario']} | {r['class']} | `{r['first_fault'][:200].replace('|', '/')}` |")
    lines += ["", "## Notes on passing runs", "", "| galaxy | s | notes |", "|---|---:|---|"]
    for r in results:
        if r["outcome"] == "PASS_NOTE":
            lines.append(f"| {r['stage']} | {r['scenario']} | {'; '.join(r['notes'])[:300].replace('|', '/')} |")
    (out / "table.md").write_text("\n".join(lines) + "\n")
    print("\n".join(lines[:12]))


def selected(args):
    files = args.disc / "files"
    stages = stage_sweep.enumerate_stages(files)
    names = set(args.stages.split(",")) if args.stages else None
    scen = stage_sweep.parse_range(args.scenarios) if args.scenarios else None
    applicable = None
    if args.powerup_map:  # only stages/scenarios that place an object loading this mode's player resource
        applicable = json.loads(args.powerup_map.read_text()).get(args.player_mode, {})
    chosen = []
    for s in stages:
        if names is not None and s["stage"] not in names:
            continue
        if applicable is not None and s["stage"] not in applicable:
            continue
        for sc in s["scenarios"]:
            if applicable is not None and sc["scenario"] not in applicable[s["stage"]]:
                continue
            if scen is None or sc["scenario"] in scen:
                chosen.append((s["stage"], sc["scenario"]))
    return chosen


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("command", choices=["run", "list", "report"])
    ap.add_argument("--headless", type=Path)
    ap.add_argument("--disc", type=Path, default=Path.home() / "MacioGalaxy2-gamedata/SB4E01-extracted")
    ap.add_argument("--out", type=Path)
    ap.add_argument("--stages")
    ap.add_argument("--scenarios")
    ap.add_argument("--jobs", type=int, default=4)
    ap.add_argument("--frames", type=int, default=3600, help="scripted loops after play-ready (3600 = 60 s)")
    ap.add_argument("--script", default="default")
    ap.add_argument("--timeout", type=int, default=600)
    ap.add_argument("--rerun", action="store_true")
    ap.add_argument("--keep-trace", action="store_true")
    ap.add_argument("--powerup-map", type=Path, help="native/tools/powerup_map.py --json output: restrict --player-mode runs to stages that load its resource")
    ap.add_argument("--player-mode", help="power-up applied through MR::changePlayerMode* at script start: bee hopper teresa ice fire yoshi cloud rock foo invincible lifeup")
    args = ap.parse_args()
    if args.command == "list":
        for stage, scenario in selected(args):
            print(stage, scenario)
        return 0
    if args.out is None:
        ap.error("--out is required")
    out = args.out.resolve()
    if args.command == "report":
        results = []
        for rd in sorted((out / "runs").iterdir()):
            old = json.loads((rd / "result.json").read_text())
            results.append(finish(args, out, old["stage"], old["scenario"], rd, old["rc"], old["timed_out"], old["elapsed"]))
        write_reports(out, results)
        return 0
    if args.headless is None:
        ap.error("--headless is required")
    out.mkdir(parents=True, exist_ok=True)
    script_path = out / "keys.txt"
    if args.script.startswith("random:"):
        script_path.write_text(random_script(int(args.script.split(":")[1]), args.frames))
    else:
        base = DEFAULT_SCRIPT if args.script == "default" else Path(args.script).read_text()
        script_path.write_text(script_text(args.frames, base))
    chosen = selected(args)
    results = []
    t0 = time.time()
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {pool.submit(run_one, args, out, st, sc, script_path): (st, sc) for st, sc in chosen}
        for n, fut in enumerate(concurrent.futures.as_completed(futures), 1):
            r = fut.result()
            results.append(r)
            tag = r["outcome"] if r["outcome"] != "FAIL" else f"FAIL({r['class']})"
            print(f"[{n}/{len(chosen)}] {r['stage']} s{r['scenario']}: {tag} {r['elapsed']}s {r['first_fault'][:120]}", flush=True)
    write_reports(out, results)
    print(f"sweep done in {time.time() - t0:.0f}s")
    return 0


if __name__ == "__main__":
    sys.exit(main())
