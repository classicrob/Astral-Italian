#!/usr/bin/env python3
"""tools/port/replay_tape.py: tape file handling and the divergence logic (no game needed)."""
import importlib.util
import struct
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("replay_tape", ROOT / "tools/port/replay_tape.py")
rt = importlib.util.module_from_spec(spec)
spec.loader.exec_module(rt)


def rec(frame, ordinal, channel=0, samples=1, returned=None):
    return dict(frame=frame, ordinal=ordinal, channel=channel, capacity=120, returned=samples if returned is None else returned,
                error=0, probe=0, device=1, samples=[bytes([frame % 250 + 1]) * 240 for _ in range(samples)])


def trace_row(**kw):
    row = dict(stage="S", scenario="1", player_valid="1", x="1.5", y="2", z="3", movement_lo=str(1 << 30), dead="0")
    row.update({k: str(v) for k, v in kw.items()})
    return row


def route_row(**kw):
    row = dict(stage="S", scenario="1", valid="1", x="1.5", y="2", z="3", grounded="1", dead="0",
               move_x="0", move_y="0", jump="0", spin="0", crouch="0", shoot="0")
    row.update({k: str(v) for k, v in kw.items()})
    return row


class TapeFiles(unittest.TestCase):
    def test_copied_tape_uses_adjacent_files_before_recording_host_paths(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory) / "route"
            for suffix in (".csv", ".kpd", ".actions"):
                Path(str(base) + suffix).touch()
            paths = rt.tape_files(Path(str(base) + ".tape.json"),
                                  dict(route_csv="/recording-host/route.csv", kpd="/recording-host/route.kpd",
                                       actions="/recording-host/route.actions"))
            self.assertEqual(paths, tuple(Path(str(base) + s) for s in (".kpd", ".actions", ".csv")))

    def test_kpd_round_trip(self):
        records = [rec(0, 0), rec(0, 1, 1, 1, 0), rec(1, 0, samples=3)]
        data = rt.pack_kpd(2, records)
        frames, back = rt.parse_kpd(data)
        self.assertEqual(frames, 2)
        self.assertEqual(back, records)
        with self.assertRaises(ValueError):
            rt.parse_kpd(data + b"x")
        with self.assertRaises(ValueError):
            rt.parse_kpd(b"NOTATAPE" + data[8:])

    SEGMENTS = [dict(start_row=4, end_row=9, stage="A", scenario=1, rng_seed=11), dict(start_row=14, end_row=18, stage="B", scenario=2, rng_seed=22)]

    def test_plan_segments_cuts_at_the_last_finished_loop(self):
        # seams written = 17 (rows 0..16): segment 1 (rows 14..18) is cut at row 16; segment 0 is whole
        plan = rt.plan_segments(self.SEGMENTS, 17)
        self.assertEqual([(p["start"], p["end"], p["frames"], p["offset"]) for p in plan], [(4, 9, 5, 0), (14, 16, 2, 5)])
        self.assertEqual(len(rt.plan_segments(self.SEGMENTS, 12)), 1)  # segment 1 never started
        self.assertEqual(rt.plan_segments(self.SEGMENTS, 20, max_frames=7)[1]["frames"], 2)
        with self.assertRaises(ValueError):
            rt.plan_segments(self.SEGMENTS, 4)

    def test_build_tape_rebases_each_segment_and_drops_the_gap(self):
        records = [rec(f, 0) for f in range(20)]
        plan = rt.plan_segments(self.SEGMENTS, 20)
        total, kept = rt.build_tape(records, plan)
        self.assertEqual(total, 9)
        # segment 0: absolute frames 5..9 -> 0..4; gap 10..14 dropped; segment 1: 15..18 -> 5..8
        self.assertEqual([r["frame"] for r in kept], [0, 1, 2, 3, 4, 5, 6, 7, 8])
        self.assertEqual(kept[0]["samples"], records[5]["samples"])
        self.assertEqual(kept[5]["samples"], records[15]["samples"])
        self.assertEqual(rt.segments_text(plan), "# stage scenario rng_seed frames\nA 1 11 5\nB 2 22 4\n")

    def test_actions_rebase(self):
        actions = rt.parse_actions("# c\n5 3 1 1\n6 3 0 1\n2 3 1 0\n12 3 1 1\n16 3 1 0\n")
        text = rt.rebase_actions(actions, rt.plan_segments(self.SEGMENTS, 20))
        self.assertIn("0 3 1 1\n", text)
        self.assertIn("1 3 0 1\n", text)
        self.assertIn("6 3 1 0\n", text)  # absolute 16 = segment 1 frame 1 -> global 5 + 1
        self.assertNotIn("2 3 1 0", text)  # before the first segment
        self.assertNotIn("12 3", text)     # in the gap


class Tail(unittest.TestCase):
    def test_invalid_teardown_tail_is_trimmed(self):
        rows = [route_row(valid=1) for _ in range(6)] + [route_row(valid=0) for _ in range(3)]
        self.assertEqual(rt.trim_invalid_tail(rows, 1, 8), 5)
        self.assertEqual(rt.trim_invalid_tail(rows, 1, 4), 4)
        self.assertEqual(rt.trim_invalid_tail([route_row(valid=0)] * 4, 1, 3), 1)


class Mods(unittest.TestCase):
    def test_env_gives_exactly_the_recorded_set(self):
        self.assertEqual(rt.mods_env(["OdysseyMovement", "Cappy", "OdysseyJumpDive"]), "OdysseyMovement,Cappy,OdysseyJumpDive")
        self.assertEqual(rt.mods_env(["OdysseyMovement"]), "OdysseyMovement,-OdysseyJumpDive")  # default-on mod that was off
        self.assertEqual(rt.mods_env([]), "-OdysseyJumpDive")


class Verdict(unittest.TestCase):
    def test_star_events_and_reached_star(self):
        rows = [route_row(power_stars=-1), route_row(power_stars=0), route_row(power_stars=0), route_row(power_stars=1), route_row(power_stars=1)]
        stars = rt.star_events(rows)
        self.assertEqual([(e["route_row"], e["stars"]) for e in stars], [(3, 1)])
        plan = [dict(index=0, start=1, end=4, frames=3)]
        good = [dict(segment=0, route_start_row=1, route_end_row=4, tape_frames=3, rows_compared=4, matched_rows=4, first_divergence=None)]
        v = rt.verdict(good, stars)
        self.assertEqual((v["all_segments_matched"], v["reached_star"]), (True, "yes"))
        early = [dict(good[0], matched_rows=1, first_divergence=dict(trace_row=1))]  # diverged before the star row (k = 2)
        v = rt.verdict(early, stars)
        self.assertEqual((v["all_segments_matched"], v["reached_star"], v["first_divergence_segment"]), (False, "no", 0))
        self.assertEqual(rt.verdict(good, [])["reached_star"], "no")


class Compare(unittest.TestCase):
    def test_identical_rows_match(self):
        route = [route_row() for _ in range(10)]
        r = rt.compare([trace_row() for _ in range(6)], route, 4)
        self.assertEqual((r["rows_compared"], r["matched_rows"], r["first_divergence"], r["first_any_difference"]), (6, 6, None, None))

    def test_tiny_difference_is_reported_but_is_not_the_divergence(self):
        route = [route_row() for _ in range(10)]
        trace = [trace_row() for _ in range(6)]
        trace[3]["x"] = "1.50000012"  # 1 ULP of float32
        r = rt.compare(trace, route, 4)
        self.assertEqual(r["first_any_difference"]["trace_row"], 3)
        self.assertIsNone(r["first_divergence"])
        self.assertLess(r["max_position_error"], 1e-6)

    def test_position_over_limit_and_discrete_fields(self):
        route = [route_row() for _ in range(10)]
        trace = [trace_row() for _ in range(6)]
        trace[2]["z"] = "3.02"
        r = rt.compare(trace, route, 4)
        self.assertEqual(r["first_divergence"]["trace_row"], 2)
        self.assertEqual(r["first_divergence"]["route_row"], 6)
        self.assertEqual(r["matched_rows"], 2)
        for field, trace_kw, route_kw in (("stage", dict(stage="T"), {}), ("scenario", dict(scenario=2), {}), ("valid", dict(player_valid=0), {}),
                                          ("dead", dict(dead=1), {}), ("grounded", dict(movement_lo=0), {})):
            t = [trace_row() for _ in range(3)]
            t[1].update({k: str(v) for k, v in trace_kw.items()})
            r = rt.compare(t, [route_row() for _ in range(8)], 4)
            self.assertIn(field, r["first_divergence"]["diffs"], field)

    def test_route_text_is_compared_as_float32(self):
        # the route CSV prints %.9g of a float32: it must equal the trace's float exactly
        value = struct.unpack(">f", struct.pack(">f", -4758.43))[0]
        route = [route_row(x="%.9g" % value) for _ in range(6)]
        trace = [trace_row(x=repr(value)) for _ in range(2)]
        self.assertIsNone(rt.compare(trace, route, 4)["first_any_difference"])


if __name__ == "__main__":
    unittest.main()
