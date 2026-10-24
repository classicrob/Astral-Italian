#!/usr/bin/env python3
"""Replay a recorded tape headless and report, per scene segment, where (if anywhere) it first diverges.

  replay_tape.py TAPE --headless <petari_headless> --disc <extracted disc> --out <fresh dir>
                 [--allow-incomplete] [--max-frames N] [--no-ftz] [--epoch-ticks T] [--dry-run]

TAPE is the `<x>.tape.json` the GX app writes next to the route CSV (PETARI_TAPE_RECORD=<dir>, or PETARI_ROUTE_RECORD=<x>.csv, or an
ordinary launcher session: native/PLAYTEST.md), or the directory / base path holding it. A tape is a list of SEGMENTS, one per scene
visit that reached play-ready (a galaxy, the Starship, ...): segment i starts at its own play-ready seam (route row start_row) and runs to
the visit's last seam (end_row). The loops in between (teardown, loading, the opening flyover) take a different number of loops in
GX and in headless (they depend on asynchronous loading), so the replay re-anchors at every segment's sync seam, sets the recorded RNG
seed there, and feeds the segment's recorded KPAD reads + mod buttons; between segments it feeds neutral reads. The tool:
  1. writes trimmed, rebased copies of the .kpd / .actions and a segments list into --out (the tape itself is never modified);
  2. runs `petari_headless --stage S --segments ... --sync-play --inputs-from-sync --mod-actions` with PETARI_MODS = the recorded mods
     and the recorded replay_env (PETARI_TEST_RETURN_TO_SHIP, a hook that changes the scene, must fire in the replay too);
  3. compares headless trace row k of segment i with route row start_row_i + k and writes first-divergence.json:
     position (x,y,z, float32 exact) more than 0.01 away, or a stage / scenario / player-valid / dead / grounded change, is THE divergence
     (per segment and overall); the first bit difference and the number of differing rows are reported too (no tolerance hides anything);
  4. reports `reached star`: the route CSV's own power-star count rose (game get) and the replay matched through that row.
Exit status 0: every segment matched to the end. `--allow-incomplete` replays a killed session up to its last checkpoint.
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
DEFAULT_ON = ("OdysseyJumpDive",)  # mods that are on without a mods.txt line (native/input/mods.cpp kDefaults)


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


def parse_actions(text):
    """'frame mod press held' lines -> {(frame, mod): (press, held)}"""
    out = {}
    for line in text.splitlines():
        if not line.strip() or line.startswith("#"):
            continue
        f, m, p, h = (int(x) for x in line.split())
        out[(f, m)] = (p, h)
    return out


def plan_segments(segments, seams, max_frames=None):
    """The sidecar's segments -> [dict(index, start, end, frames, offset, stage, scenario, rng_seed)]. A segment is tape frames
    start+1..end (absolute game-loop numbers of its reads); `frames` = end-start; `offset` = the segment's first global tape frame.
    Segments past the last finished seam are dropped; the last kept one is cut at the last finished loop."""
    plan, offset = [], 0
    for i, g in enumerate(segments):
        start, end = g["start_row"], min(g["end_row"], seams - 1)
        if end <= start:
            continue
        frames = end - start
        if max_frames is not None:
            frames = min(frames, max_frames - offset)
            end = start + frames
        if frames < 1:
            break
        plan.append(dict(index=i, start=start, end=end, frames=frames, offset=offset, stage=g["stage"], scenario=g["scenario"], rng_seed=g["rng_seed"]))
        offset += frames
    if not plan:
        raise ValueError("no complete game loop after a ready seam")
    return plan


def build_tape(records, plan):
    """Reads of segment i (absolute frames start+1..end) rebased to global tape frames offset..offset+frames-1."""
    kept = []
    for seg in plan:
        lo, hi = seg["start"] + 1, seg["end"]
        for r in records:
            if lo <= r["frame"] <= hi:
                kept.append(dict(r, frame=seg["offset"] + r["frame"] - lo))
    return sum(s["frames"] for s in plan), kept


def rebase_actions(actions, plan):
    lines = ["# frame mod press held (global tape frames; segments back to back)"]
    rows = []
    for seg in plan:
        lo, hi = seg["start"] + 1, seg["end"]
        for (f, m), (p, h) in actions.items():
            if lo <= f <= hi:
                rows.append((seg["offset"] + f - lo, m, p, h))
    for f, m, p, h in sorted(rows):
        lines.append(f"{f} {m} {p} {h}")
    return "\n".join(lines) + "\n"


def segments_text(plan):
    return "# stage scenario rng_seed frames\n" + "".join(f"{s['stage']} {s['scenario']} {s['rng_seed']} {s['frames']}\n" for s in plan)


def trim_invalid_tail(route_rows, start, end):
    """The last segment of a session that was quit (or killed) ends with loops where the app was tearing down and the route
    recorder saw no valid player: they are not game state, so the segment ends at its last valid row."""
    end = min(end, len(route_rows) - 1)
    while end > start and route_rows[end]["valid"] != "1":
        end -= 1
    return end


def mods_env(mods):
    """PETARI_MODS value giving exactly the recorded set: the listed mods on, a default-on mod that was off explicitly off."""
    items = list(mods) + [f"-{m}" for m in DEFAULT_ON if m not in mods]
    return ",".join(items) if items else "none"


# --- comparison -----------------------------------------------------------------------------------------------

def f32(text):
    return struct.unpack(">f", struct.pack(">f", float(text)))[0]


def compare(trace_rows, route_rows, route_start):
    """trace row k <-> route row route_start+k. Position is compared as float32 (the route CSV holds %.9g)."""
    n = min(len(trace_rows), len(route_rows) - route_start)
    first_any = first_pos = None
    bit_rows = []
    max_dist = 0.0
    for k in range(n):
        t, r = trace_rows[k], route_rows[route_start + k]
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
                if struct.pack(">f", float(t[key])) != struct.pack(">f", float(r[key])):
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
            if first_any is None:
                first_any = dict(trace_row=k, route_row=route_start + k, diffs=diffs, dist=dist)
        if (dist > POSITION_LIMIT or {"stage", "scenario", "valid", "dead", "grounded"} & diffs.keys()) and first_pos is None:
            first_pos = dict(trace_row=k, route_row=route_start + k, diffs=diffs, dist=dist,
                             trace=[t[c] for c in "xyz"], route=[r[c] for c in "xyz"],
                             inputs={c: r[c] for c in ("move_x", "move_y", "jump", "spin", "crouch", "shoot")})
    return dict(rows_compared=n, first_divergence=first_pos,
                matched_rows=n if first_pos is None else first_pos["trace_row"],
                first_any_difference=first_any, rows_with_any_difference=len(bit_rows),
                last_any_difference_row=bit_rows[-1] if bit_rows else None,
                max_position_error=max_dist, position_limit=POSITION_LIMIT)


def star_events(route_rows):
    """Route rows where the game's own power-star count rose (both rows in a game scene: count >= 0)."""
    events, prev = [], None
    for i, r in enumerate(route_rows):
        try:
            n = int(r["power_stars"])
        except (KeyError, ValueError):
            continue
        if n >= 0:
            if prev is not None and n > prev:
                events.append(dict(route_row=i, stage=r["stage"], stars=n, previous=prev))
            prev = n
    return events


def segment_results(trace_rows, seg_rows, route_rows, plan):
    """seg_rows: [(trace_start_row, rows)] as written by the headless run; per-segment compare + overall verdict."""
    out = []
    for seg, (tstart, rows) in zip(plan, seg_rows):
        res = compare(trace_rows[tstart: tstart + rows], route_rows, seg["start"])
        res.update(segment=seg["index"], stage=seg["stage"], scenario=seg["scenario"], route_start_row=seg["start"],
                   route_end_row=seg["end"], tape_frames=seg["frames"])
        out.append(res)
    return out


def verdict(results, stars):
    first = next(((r["segment"], r["first_divergence"]) for r in results if r["first_divergence"]), None)
    complete = all(r["rows_compared"] == r["tape_frames"] + 1 for r in results)
    reached = None
    for ev in stars:  # the replay reproduced the star if the segment holding that row matched through it
        for r in results:
            if r["route_start_row"] <= ev["route_row"] <= r["route_end_row"]:
                k = ev["route_row"] - r["route_start_row"]
                if r["matched_rows"] > k:
                    reached = ev
                break
        if reached:
            break
    return dict(all_segments_matched=first is None and complete, first_divergence_segment=first[0] if first else None,
                first_divergence=first[1] if first else None, star_in_recording=bool(stars), star_events=stars,
                reached_star="yes" if reached else "no", reached_star_event=reached)


# --- driver ---------------------------------------------------------------------------------------------------

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


def tape_files(sidecar, meta):
    """A copied tape is self-contained; recorder absolute paths are provenance.

    Prefer adjacent files as GX replay does. Older non-directory callers retain
    their metadata paths if the adjacent file is absent.
    """
    base = sidecar.parent / sidecar.name[: -len(".tape.json")]
    def file(key, suffix):
        local = Path(str(base) + suffix)
        return local if local.is_file() else Path(meta.get(key, str(local)))
    return file("kpd", ".kpd"), file("actions", ".actions"), file("route_csv", ".csv")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("tape")
    ap.add_argument("--headless", required=True, type=Path)
    ap.add_argument("--disc", required=True, type=Path)
    ap.add_argument("--out", required=True, type=Path)
    ap.add_argument("--max-frames", type=int, help="replay at most this many tape frames (all segments together)")
    ap.add_argument("--no-ftz", action="store_true")
    ap.add_argument("--epoch-ticks", type=int, default=0)
    ap.add_argument("--anchor", choices=("play", "game"), default="play",
                    help="play (default): re-anchor each scene visit at its play-ready seam; game: ONE segment from the first Game/End seam "
                         "(the opening flyover is part of it; its length differs between GX and headless, so this is a diagnostic, not a gate)")
    ap.add_argument("--allow-incomplete", action="store_true", help="replay a killed session up to its last checkpoint")
    ap.add_argument("--dry-run", action="store_true", help="write the replay inputs, print the command, do not run it")
    ap.add_argument("--require-bit-exact", action="store_true", help="fail on any float32 bit difference, including differences below the legacy distance limit")
    a = ap.parse_args(argv)

    sidecar = locate_sidecar(a.tape)
    meta = json.loads(sidecar.read_text())
    if not meta.get("complete") and not a.allow_incomplete:
        raise SystemExit("replay_tape: the tape was not closed normally (complete=false: the session was killed, not quit). It is valid up to the "
                         f"last checkpoint ({meta['seams']} loops); pass --allow-incomplete to replay that part")
    if meta["unencodable_samples"] or meta["lossy_samples"]:
        raise SystemExit(f"replay_tape: the recorder saw {meta['unencodable_samples']} unencodable / {meta['lossy_samples']} lossy KPAD samples: exact replay is not possible")
    kpd, actions, route = tape_files(sidecar, meta)
    for f in (kpd, actions, route):
        if not f.is_file():
            raise SystemExit(f"replay_tape: missing {f}")
    if a.anchor == "game":
        r = meta["ready_seams"][-1] if not [g for g in meta["ready_seams"] if g["stage"] not in ("FileSelect",)] else [g for g in meta["ready_seams"] if g["stage"] != "FileSelect"][0]
        meta["segments"] = [dict(start_row=r["row"], end_row=meta["seams"] - 1, stage=r["stage"], scenario=r["scenario"], rng_seed=r["rng_seed"])]
    if not meta.get("segments") and meta.get("play_ready_seams"):  # tapes written before segments: the first play-ready to the end
        r = meta["play_ready_seams"][0]
        meta["segments"] = [dict(start_row=r["row"], end_row=meta["seams"] - 1, stage=r["stage"], scenario=r["scenario"], rng_seed=r["rng_seed"])]
    if not meta.get("segments"):
        raise SystemExit("replay_tape: the tape has no segment that reached play-ready (the session never got control of Mario)")
    route_rows = list(csv.DictReader(route.open()))
    meta["segments"][-1]["end_row"] = trim_invalid_tail(route_rows, meta["segments"][-1]["start_row"], min(meta["segments"][-1]["end_row"], meta["seams"] - 1))
    plan = plan_segments(meta["segments"], meta["seams"], a.max_frames)
    if a.out.exists():
        raise SystemExit(f"replay_tape: {a.out} exists; give a fresh --out")
    a.out.mkdir(parents=True)

    _, records = parse_kpd(kpd.read_bytes(), allow_trailing=not meta.get("complete"))
    total, kept = build_tape(records, plan)
    (a.out / "inputs.kpd").write_bytes(pack_kpd(total, kept))
    (a.out / "inputs.actions").write_text(rebase_actions(parse_actions(actions.read_text()), plan))
    (a.out / "segments.txt").write_text(segments_text(plan))
    user = a.out / "user"  # the headless harness wants an empty user dir: the mods go in through PETARI_MODS
    trace = a.out / "trace.csv"
    first = plan[0]
    cmd = [str(a.headless), "--inputs", str(a.out / "inputs.kpd"), "--mod-actions", str(a.out / "inputs.actions"), "--disc", str(a.disc),
           "--user", str(user), "--trace", str(trace), "--epoch-ticks", str(a.epoch_ticks),
           "--stage", first["stage"], "--scenario", str(first["scenario"]),
           "--segments", str(a.out / "segments.txt")] + (["--sync-play"] if a.anchor == "play" else []) + ["--inputs-from-sync"]
    if a.no_ftz:
        cmd.append("--no-ftz")
    env = {k: v for k, v in os.environ.items() if not k.startswith("PETARI_")}
    env["PETARI_MODS"] = mods_env(meta["mods"])
    env.update(meta.get("replay_env") or {})
    (a.out / "command.json").write_text(json.dumps(dict(cmd=cmd, env={k: v for k, v in env.items() if k.startswith("PETARI_")}, segments=plan,
                                                        tape_frames=total, mods=meta["mods"], exe_sha256_recorded=meta.get("exe_sha256")), indent=2) + "\n")
    if a.dry_run:
        print(" ".join(cmd))
        return 0
    log = (a.out / "headless.log").open("wb")
    rc = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, env=env).returncode
    if rc != 0 or not (a.out / "trace.csv.complete").exists():
        raise SystemExit(f"replay_tape: the headless replay failed (exit {rc}); see {a.out / 'headless.log'}")
    seg_rows = [(int(r["trace_start_row"]), int(r["rows"])) for r in csv.DictReader((a.out / "trace.csv.segments.csv").open())]
    if len(seg_rows) != len(plan):
        raise SystemExit(f"replay_tape: the replay ran {len(seg_rows)} segments, the tape has {len(plan)}")
    results = segment_results(list(csv.DictReader(trace.open())), seg_rows, route_rows, plan)
    for i, seg in enumerate(plan):
        sync = json.loads((a.out / ("trace.csv.sync%s.json" % (i or ""))).read_text())
        results[i].update(replay_rng_seed=sync["rng_seed"], replay_natural_rng_seed=sync.get("rng_seed_before"), recorded_rng_seed=seg["rng_seed"],
                          rng_seed_matches=sync["rng_seed"] == seg["rng_seed"])
    summary = verdict(results, star_events(route_rows))
    summary["bit_exact"] = all(r["rows_with_any_difference"] == 0 for r in results)
    summary["bit_exact_required"] = a.require_bit_exact
    if a.require_bit_exact:
        summary["all_segments_matched"] &= summary["bit_exact"]
    summary.update(tape=str(sidecar), tape_frames=total, segments=results)
    (a.out / "first-divergence.json").write_text(json.dumps(summary, indent=2) + "\n")
    brief = {k: summary[k] for k in ("all_segments_matched", "first_divergence_segment", "first_divergence", "reached_star", "star_in_recording")}
    brief["segments"] = [{k: r[k] for k in ("segment", "stage", "tape_frames", "rows_compared", "matched_rows", "first_divergence", "rows_with_any_difference",
                                            "last_any_difference_row", "max_position_error", "rng_seed_matches")} for r in results]
    print(json.dumps(brief, indent=2))
    print(f"reached star: {summary['reached_star']}")
    return 0 if summary["all_segments_matched"] else 1


if __name__ == "__main__":
    sys.exit(main())
