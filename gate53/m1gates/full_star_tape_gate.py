#!/usr/bin/env python3
"""Replay a real player's entire Flip-Swap Star session on an exact GX app.

APP SHA256 SOURCE LABEL --build-kind server-candidate --allow-incomplete
Uses a fresh private user, the recorded direct-stage launch and raw KPAD only.
Checks every available declared row, every play-ready checkpoint (also resumes),
and the game's own zero-to-one power-star transition. Tape hash is informational.
"""
import csv
import json
import os
import struct
import sys
from pathlib import Path

import normal_tape_gate as normal

NAME = "full_star_tape_gate"
TAPE = normal.ROOT / "tapes/tape-user-flipswap-fullstar-541"
BOUND = 360
STAGE, SCENARIO = "RedBlueExGalaxy", 1
BASE_CHECKS = tuple(k for k in normal.CHECKS if k not in (
    "destination_300_consecutive_valid", "existing_unlocked_file1", "seed_unchanged"))
CHECKS = BASE_CHECKS + ("app_unchanged", "fresh_user_zero_stars", "no_deaths", "star_get",
                       "star_row_aligned", "star_state_bit_exact", "zero_interactive_position_error",
                       "all_play_ready_state_rng_checks", "actual_recording_complete", "route_rows_contiguous")


def chain(rows):
    order = []
    for row in rows:
        stage = row.get("stage")
        if stage and (not order or order[-1] != stage):
            order.append(stage)
    return order


def bits(row):
    return [struct.unpack(">I", struct.pack(">f", float(row[c])))[0] for c in "xyz"]


def state(row):
    return {**{k: row[k] for k in ("stage", "scenario", "valid", "grounded", "dead")},
            "position_bits": bits(row)}


def stars(rows):
    previous, events = None, []
    for row in rows:
        if row.get("valid") != "1":
            continue
        count = int(row["power_stars"])
        if count < 0:
            continue
        if previous is not None and count > previous:
            events.append({"row": int(row["frame"]), "stage": row["stage"],
                           "scenario": int(row["scenario"]), "previous": previous, "power_stars": count})
        previous = count
    return events


def checkpoints(recorded, replayed, meta, actual, report):
    """Actual recorder reads the game RNG immediately after TapePlay at the seam.

    First scene checkpoints retain native seed restore + readback proof. Resumes
    inside a scene require the naturally evolved seed, with NO extra restoration.
    Native strict comparison covers all intervening interactive rows.
    """
    expected = meta.get("play_ready_seams", [])
    observed = actual.get("play_ready_seams", [])
    evidence = []
    for index, point in enumerate(expected):
        item = {"recorded": point, "passed": False}
        try:
            seen = observed[index]
            item["replayed"] = seen
            row, actual_row = int(point["row"]), int(seen["row"])
            assert (seen["stage"], seen["scenario"]) == (point["stage"], point["scenario"])
            part = next(s for s in report["segments"] if s["stage"] == point["stage"]
                        and s["scenario"] == point["scenario"] and s["play_ready_required"])
            anchor = part["play_ready_state_check"]
            offset = anchor["frame_offset"]
            item.update(frame_offset=actual_row-row, expected_frame_offset=offset,
                        expected=state(recorded[row]), actual=state(replayed[actual_row]),
                        rng={"recorded": point["rng_seed"], "readback": seen["rng_seed"],
                             "source": "TapeRecord::seam actual game RNG after TapePlay::seam",
                             "restored_at_this_checkpoint": row == anchor["recorded_frame"]})
            assert actual_row-row == offset
            assert item["expected"] == item["actual"] and recorded[row]["valid"] == "1"
            assert seen["rng_seed"] == point["rng_seed"]
            if row == anchor["recorded_frame"]:
                assert all(anchor.get(k) is True for k in ("checked", "passed", "position_bit_exact"))
                assert anchor["rng"]["verified"] is True
            item["position_bit_exact"] = True
            item["rng"]["verified"] = True
            item["passed"] = True
        except (AssertionError, KeyError, IndexError, StopIteration, ValueError, TypeError) as exc:
            item["error"] = type(exc).__name__ + ": " + str(exc)
        evidence.append(item)
    passed = bool(expected) and len(observed) == len(expected) and all(c["passed"] for c in evidence)
    return passed, evidence


def evaluate(rows, log, replay, meta, case, tape):
    base, details = normal.evaluate(rows, log, replay)
    checks = {k: v for k, v in base.items() if k in BASE_CHECKS}
    source = list(csv.DictReader((tape / "route.csv").open()))[:meta["seams"]]
    # Old _Exit recordings can have a final buffered CSV block missing. Use the
    # native loader's existing min(declared seams, complete CSV rows), report it.
    source = [r for r in source if r.get("frame") is not None and r.get("pointer_y") is not None]
    actual_path = case / "route.tape.json"
    actual = json.loads(actual_path.read_text()) if actual_path.exists() else {}
    exact, proof = checkpoints(source, rows, meta, actual, replay)
    expected_stars, actual_stars = stars(source), stars(rows)
    expected = next((s for s in expected_stars if s["previous"] == 0 and s["power_stars"] == 1), None)
    observed = next((s for s in actual_stars if s["previous"] == 0 and s["power_stars"] == 1), None)
    star_offset = None
    if expected:
        part = next((s for s in replay.get("segments", []) if s["stage"] == expected["stage"]
                     and s["scenario"] == expected["scenario"]), {})
        if expected["row"] >= part.get("route_start_row", sys.maxsize):
            cp = part.get("play_ready_state_check") or {}
            star_offset = cp.get("frame_offset") if part.get("play_ready_required") else part.get("scene_anchor_frame_offset")
        else:
            star_offset = part.get("scene_anchor_frame_offset")
    star_exact = False
    if expected and observed:
        try:
            star_exact = state(source[expected["row"]]) == state(rows[observed["row"]])
        except (IndexError, KeyError, ValueError):
            pass
    checks.update(
        scene_chain=chain(rows) == chain(source) == ["FileSelect", STAGE, "MarioFaceShipGalaxy"],
        fresh_user_zero_stars=bool(rows) and next((r["power_stars"] for r in rows if r.get("valid") == "1"), None) == "0",
        no_deaths=all(r.get("dead") == "0" for r in rows if r.get("valid") == "1"),
        star_get=expected is not None and observed is not None,
        star_row_aligned=expected is not None and observed is not None and star_offset is not None
        and observed["row"] == expected["row"] + star_offset
        and (observed["stage"], observed["scenario"]) == (expected["stage"], expected["scenario"]),
        all_play_ready_state_rng_checks=exact,
        actual_recording_complete=actual.get("complete") is True,
        star_state_bit_exact=star_exact,
        zero_interactive_position_error=bool(replay.get("segments")) and all(
            s.get("max_position_error") == 0 for s in replay.get("segments", [])),
        route_rows_contiguous=bool(source) and bool(rows) and all(
            str(i) == r.get("frame") for route in (source, rows) for i, r in enumerate(route)),
    )
    details.update(recorded_scene_chain=chain(source), scene_chain=chain(rows),
                   recorded_star_event=expected, star_event=observed, star_anchor_offset=star_offset,
                   recorded_play_ready_count=len(meta.get("play_ready_seams", [])),
                   actual_play_ready_count=len(actual.get("play_ready_seams", [])),
                   all_play_ready_checkpoints=proof,
                   tape_prefix={"declared_seams": meta["seams"], "available_complete_csv_rows": len(source),
                                "last_available_recorded_row": len(source)-1,
                                "policy": "unchanged native min(declared seams, complete CSV rows); no input tape mutation"},
                   replay=replay)
    return checks, details


def main(argv=None):
    if any(a == "--mode" or a.startswith("--mode=") for a in (argv or sys.argv[1:])):
        raise SystemExit("full-star gate requires whole-session replay")
    return normal.main(argv, profile=sys.modules[__name__])


if __name__ == "__main__":
    if os.environ.pop("_M1_TAPE_GATE_HEAVY_HELD", None) != "1":
        normal.HEAVY.parent.mkdir(parents=True, exist_ok=True)
        os.execv("/usr/bin/lockf", ["/usr/bin/lockf", "-k", str(normal.HEAVY), "/usr/bin/env",
                                 "_M1_TAPE_GATE_HEAVY_HELD=1", sys.executable, "-B", str(Path(__file__).resolve()), *sys.argv[1:]])
    sys.exit(main())
