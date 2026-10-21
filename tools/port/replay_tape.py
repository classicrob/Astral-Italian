#!/usr/bin/env python3
"""Replay a recorded tape headless from its ready seam and report where (if anywhere) it first diverges.

  replay_tape.py TAPE --headless <petari_headless> --disc <extracted disc> --out <fresh dir>
                 [--ready N] [--max-frames N] [--no-ftz] [--epoch-ticks T] [--dry-run]

TAPE is the `<x>.tape.json` the GX app writes next to the route CSV (PETARI_TAPE_RECORD=<dir> or
PETARI_ROUTE_RECORD=<x>.csv; native/PLAYTEST.md), or the directory / base path holding it. The tool:
  1. takes the N-th ready seam of the sidecar (default 0): the first Game/SceneInitializeState_End seam of a stage visit,
     the same seam `petari_headless --sync-stage` selects; reads after it are tape frame 0..T-1;
  2. writes a trimmed, rebased copy of the .kpd / .actions into --out (the tape itself is never modified),
     a fresh user dir with the recorded mods.txt, and runs the headless replay (--stage/--sync-stage/--inputs-from-sync);
  3. compares the headless trace row k with route row R+k and writes first-divergence.json:
     position (x,y,z, float32 exact) > 0.01 is THE divergence; any bit difference, a stage/scenario/player-valid/
     dead/grounded change is reported too (`first_any_difference`). `matched_rows` = rows 0..N-1 identical in the gate sense.
No tolerance hides anything: both numbers (first bit difference, first > 0.01) are printed.
"""
import argparse
import csv
import json
import math
import os
import struct
import subprocess
import sys
from pathlib import Path

MAGIC = b"SMG2KPD1"
RECORD = struct.Struct(">QIIIiiiII")  # frame, ordinal, channel, capacity, returned, error, probe, device, nsamples
SAMPLE = 240
POSITION_LIMIT = 0.01


# --- tape files -----------------------------------------------------------------------------------------------

def parse_kpd(data, allow_trailing=False):
    """-> (frames, [dict(frame, ordinal, channel, capacity, returned, error, probe, device, samples=[bytes])])"""
    if data[:8] != MAGIC:
        raise ValueError("bad KPAD tape magic")
    frames, count = struct.unpack_from(">QQ", data, 8)
    pos, records = 24, []
    for _ in range(count):
        f, o, ch, cap, ret, err, probe, dev, n = RECORD.unpack_from(data, pos)
        pos += RECORD.size
        samples = [data[pos + i * SAMPLE: pos + (i + 1) * SAMPLE] for i in range(n)]
        if any(len(s) != SAMPLE for s in samples):
            raise ValueError("truncated KPAD tape")
        pos += n * SAMPLE
        records.append(dict(frame=f, ordinal=o, channel=ch, capacity=cap, returned=ret, error=err, probe=probe, device=dev, samples=samples))
    if pos != len(data) and not allow_trailing:
        raise ValueError("trailing KPAD tape data")
    return frames, records


def pack_kpd(frames, records):
    out = bytearray(MAGIC + struct.pack(">QQ", frames, len(records)))
    for r in records:
        out += RECORD.pack(r["frame"], r["ordinal"], r["channel"], r["capacity"], r["returned"], r["error"], r["probe"], r["device"], len(r["samples"]))
        for s in r["samples"]:
            out += s
    return bytes(out)


def trim_kpd(records, ready_row, seams, max_frames=None):
    """Reads after the ready seam (row R) are tape frames 0.. (absolute frame R+1 -> 0). Only loops that finished (their seam
    was written: absolute frame <= seams-1) count. Returns (tape_frames, rebased records)."""
    last = seams - 1
    total = last - ready_row
    if max_frames is not None:
        total = min(total, max_frames)
    if total < 1:
        raise ValueError("no complete game loop after the ready seam")
    kept = []
    for r in records:
        t = r["frame"] - (ready_row + 1)
        if 0 <= t < total:
            kept.append(dict(r, frame=t))
    return total, kept


def parse_actions(text):
    """'frame mod press held' lines -> {(frame, mod): (press, held)}"""
    out = {}
    for line in text.splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        f, m, p, h = (int(x) for x in line.split())
        out[(f, m)] = (p, h)
    return out


def rebase_actions(actions, ready_row, total):
    lines = ["# frame mod press held (tape frames after the ready seam)"]
    for (f, m), (p, h) in sorted(actions.items()):
        t = f - (ready_row + 1)
        if 0 <= t < total:
            lines.append(f"{t} {m} {p} {h}")
    return "\n".join(lines) + "\n"


# --- comparison -----------------------------------------------------------------------------------------------

def f32(text):
    return struct.unpack(">f", struct.pack(">f", float(text)))[0]


def compare(trace_rows, route_rows, ready_row):
    """trace row k <-> route row ready_row+k. Position is compared as float32 (the route CSV holds %.9g)."""
    n = min(len(trace_rows), len(route_rows) - ready_row)
    first_any = first_pos = None
    bit_rows = []
    max_dist = 0.0
    for k in range(n):
        t, r = trace_rows[k], route_rows[ready_row + k]
        diffs = {}
        if t["stage"] != r["stage"]:
            diffs["stage"] = [t["stage"], r["stage"]]
        if t["scenario"] != r["scenario"]:
            diffs["scenario"] = [t["scenario"], r["scenario"]]
        if t["player_valid"] != r["valid"]:
            diffs["valid"] = [t["player_valid"], r["valid"]]
        dist = 0.0
        if t["player_valid"] == "1" and r["valid"] == "1":
            for key in ("x", "y", "z"):
                if f32(t[key]) != f32(r[key]):
                    diffs[key] = [t[key], r[key]]
            dist = math.dist([f32(t[c]) for c in "xyz"], [f32(r[c]) for c in "xyz"])
            grounded = str((int(t["movement_lo"]) >> 30) & 1)
            if grounded != r["grounded"]:
                diffs["grounded"] = [grounded, r["grounded"]]
            if t["dead"] != r["dead"]:
                diffs["dead"] = [t["dead"], r["dead"]]
        max_dist = max(max_dist, dist)
        if diffs:
            bit_rows.append(k)
        if diffs and first_any is None:
            first_any = dict(trace_row=k, route_row=ready_row + k, diffs=diffs, dist=dist)
        if (dist > POSITION_LIMIT or {"stage", "scenario", "valid", "dead", "grounded"} & diffs.keys()) and first_pos is None:
            first_pos = dict(trace_row=k, route_row=ready_row + k, diffs=diffs, dist=dist,
                             trace=[t[c] for c in "xyz"], route=[r[c] for c in "xyz"],
                             inputs={c: r[c] for c in ("move_x", "move_y", "jump", "spin", "crouch", "shoot")})
    return dict(rows_compared=n, first_divergence=first_pos,
                matched_rows=n if first_pos is None else first_pos["trace_row"],
                first_any_difference=first_any, rows_with_any_difference=len(bit_rows), last_any_difference_row=bit_rows[-1] if bit_rows else None,
                max_position_error=max_dist, position_limit=POSITION_LIMIT)


# --- driver ---------------------------------------------------------------------------------------------------

DEFAULT_ON = ("OdysseyJumpDive",)  # mods that are on without a mods.txt line (native/input/mods.cpp kDefaults)


def mods_env(mods):
    """PETARI_MODS value giving exactly the recorded set: the listed mods on, a default-on mod that was off explicitly off."""
    items = list(mods) + [f"-{m}" for m in DEFAULT_ON if m not in mods]
    return ",".join(items) if items else "none"


def locate_sidecar(arg):
    p = Path(arg).expanduser()
    if p.is_dir():
        found = sorted(p.glob("*.tape.json"))
        if len(found) != 1:
            raise SystemExit(f"replay_tape: need exactly one *.tape.json in {p}")
        return found[0]
    if p.suffix == ".json":
        return p
    return Path(str(p) + ".tape.json")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("tape")
    ap.add_argument("--headless", required=True, type=Path)
    ap.add_argument("--disc", required=True, type=Path)
    ap.add_argument("--out", required=True, type=Path)
    ap.add_argument("--ready", type=int, default=0, help="which anchor seam of the sidecar (default the first)")
    ap.add_argument("--anchor", choices=("play", "game"), default="play", help="play: the game's ready-for-play seam (default); game: first Game/End seam")
    ap.add_argument("--max-frames", type=int)
    ap.add_argument("--no-ftz", action="store_true")
    ap.add_argument("--epoch-ticks", type=int, default=0)
    ap.add_argument("--allow-incomplete", action="store_true", help="replay a killed session up to its last checkpoint")
    ap.add_argument("--dry-run", action="store_true", help="write the replay inputs, print the command, do not run it")
    a = ap.parse_args(argv)

    sidecar = locate_sidecar(a.tape)
    meta = json.loads(sidecar.read_text())
    if not meta.get("complete") and not a.allow_incomplete:
        raise SystemExit("replay_tape: the tape was not closed normally (complete=false: the session was killed, not quit). It is valid up to the "
                         f"last checkpoint ({meta['seams']} loops); pass --allow-incomplete to replay that part")
    if meta["unencodable_samples"] or meta["lossy_samples"]:
        raise SystemExit(f"replay_tape: the recorder saw {meta['unencodable_samples']} unencodable / {meta['lossy_samples']} lossy KPAD samples: exact replay is not possible")
    base = sidecar.parent / sidecar.name[: -len(".tape.json")]
    kpd, actions, route = Path(meta.get("kpd", str(base) + ".kpd")), Path(meta.get("actions", str(base) + ".actions")), Path(meta["route_csv"])
    for f in (kpd, actions, route):
        if not f.is_file():
            raise SystemExit(f"replay_tape: missing {f}")
    # Anchor: the game's play-ready seam by default (the opening flyover's length depends on asynchronous loading, so the
    # Game/End seam is a different game state in GX and headless); --anchor game selects the Game/End seam.
    seams_key = "play_ready_seams" if a.anchor == "play" else "ready_seams"
    ready = meta[seams_key][a.ready]
    if a.out.exists():
        raise SystemExit(f"replay_tape: {a.out} exists; give a fresh --out")
    a.out.mkdir(parents=True)

    _, records = parse_kpd(kpd.read_bytes(), allow_trailing=not meta.get("complete"))
    total, kept = trim_kpd(records, ready["row"], meta["seams"], a.max_frames)
    (a.out / "inputs.kpd").write_bytes(pack_kpd(total, kept))
    (a.out / "inputs.actions").write_text(rebase_actions(parse_actions(actions.read_text()), ready["row"], total))
    user = a.out / "user"  # the headless harness wants an empty user dir: the mods go in through PETARI_MODS
    trace = a.out / "trace.csv"
    cmd = [str(a.headless), "--inputs", str(a.out / "inputs.kpd"), "--mod-actions", str(a.out / "inputs.actions"), "--disc", str(a.disc),
           "--user", str(user), "--trace", str(trace), "--epoch-ticks", str(a.epoch_ticks),
           "--stage", ready["stage"], "--scenario", str(ready["scenario"]),
           "--sync-stage", ready["stage"], "--sync-scenario", str(ready["scenario"]), "--sync-rng-seed", str(ready["rng_seed"]),
           "--inputs-from-sync"] + (["--sync-play"] if a.anchor == "play" else [])
    if a.no_ftz:
        cmd.append("--no-ftz")
    (a.out / "command.json").write_text(json.dumps(dict(cmd=cmd, PETARI_MODS=mods_env(meta["mods"]), ready=ready, tape_frames=total, mods=meta["mods"], exe_sha256_recorded=meta.get("exe_sha256")), indent=2) + "\n")
    if a.dry_run:
        print(" ".join(cmd))
        return 0
    env = {k: v for k, v in os.environ.items() if not k.startswith("PETARI_")}
    env["PETARI_MODS"] = mods_env(meta["mods"])
    log = (a.out / "headless.log").open("wb")
    rc = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, env=env).returncode
    if rc != 0 or not (a.out / "trace.csv.complete").exists():
        raise SystemExit(f"replay_tape: the headless replay failed (exit {rc}); see {a.out / 'headless.log'}")
    sync = json.loads((a.out / "trace.csv.sync.json").read_text())  # rng_seed = the seed after the one-shot set (the recorded one); rng_seed_before = the natural one
    result = compare(list(csv.DictReader(trace.open())), list(csv.DictReader(route.open())), ready["row"])
    result.update(tape=str(sidecar), ready=ready, tape_frames=total, replay_rng_seed=sync["rng_seed"], replay_natural_rng_seed=sync.get("rng_seed_before"), recorded_rng_seed=ready["rng_seed"],
                  rng_seed_matches=sync["rng_seed"] == ready["rng_seed"])
    (a.out / "first-divergence.json").write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({k: result[k] for k in ("rows_compared", "matched_rows", "first_divergence", "first_any_difference", "rows_with_any_difference", "last_any_difference_row", "max_position_error", "rng_seed_matches")}, indent=2))
    return 0 if result["first_divergence"] is None else 1


if __name__ == "__main__":
    sys.exit(main())
