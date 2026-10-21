#!/usr/bin/env python3
"""tools/port/replay_tape.py: tape file handling and the divergence logic (no game needed)."""
import importlib.util
import struct
import sys
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

    def test_trim_rebases_after_the_ready_seam_and_drops_the_unfinished_loop(self):
        # ready seam = route row 4; seams written = 10 (rows 0..9): loops 5..9 finished, loop 10 (frame 10) did not.
        records = [rec(f, 0) for f in range(12)]
        total, kept = rt.trim_kpd(records, 4, 10)
        self.assertEqual(total, 5)
        self.assertEqual([r["frame"] for r in kept], [0, 1, 2, 3, 4])
        self.assertEqual(kept[0]["samples"], records[5]["samples"])  # absolute frame 5 = tape frame 0
        total, kept = rt.trim_kpd(records, 4, 10, max_frames=2)
        self.assertEqual((total, len(kept)), (2, 2))
        with self.assertRaises(ValueError):
            rt.trim_kpd(records, 9, 10)

    def test_actions_rebase(self):
        actions = rt.parse_actions("# c\n5 3 1 1\n6 3 0 1\n2 3 1 0\n")
        text = rt.rebase_actions(actions, 4, 5)
        self.assertIn("0 3 1 1\n", text)
        self.assertIn("1 3 0 1\n", text)
        self.assertNotIn("2 3 1 0", text)  # before the ready seam


class Mods(unittest.TestCase):
    def test_env_gives_exactly_the_recorded_set(self):
        self.assertEqual(rt.mods_env(["OdysseyMovement", "Cappy", "OdysseyJumpDive"]), "OdysseyMovement,Cappy,OdysseyJumpDive")
        self.assertEqual(rt.mods_env(["OdysseyMovement"]), "OdysseyMovement,-OdysseyJumpDive")  # default-on mod that was off
        self.assertEqual(rt.mods_env([]), "-OdysseyJumpDive")


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
