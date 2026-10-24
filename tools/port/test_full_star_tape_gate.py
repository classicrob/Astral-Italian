#!/usr/bin/env python3
"""Fail-closed coverage of resumed play-ready RNG and real star-count evidence."""
import copy
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "gate53/m1gates"))
import full_star_tape_gate as gate


class FullStarTests(unittest.TestCase):
    def setUp(self):
        self.rows = [dict(frame=str(i), stage=gate.STAGE, scenario="1", valid="1", grounded="1",
                          dead="0", x="0", y="1.25", z="-2", power_stars="0") for i in range(6)]
        self.points = [dict(row=i, stage=gate.STAGE, scenario=1, rng_seed=i+99) for i in (1, 3, 5)]
        self.meta = dict(play_ready_seams=self.points)
        self.actual = copy.deepcopy(self.meta)
        self.report = dict(segments=[dict(stage=gate.STAGE, scenario=1, play_ready_required=True,
            play_ready_state_check=dict(frame_offset=0, recorded_frame=1, checked=True, passed=True,
                                        position_bit_exact=True, rng=dict(verified=True)))])

    def check(self):
        return gate.checkpoints(self.rows, self.rows, self.meta, self.actual, self.report)[0]

    def test_all_resumes_require_natural_rng_match(self):
        self.assertTrue(self.check())
        self.actual["play_ready_seams"][1]["rng_seed"] += 1
        self.assertFalse(self.check())

    def test_missing_or_extra_checkpoint_fails(self):
        for points in (self.points[:-1], self.points + [self.points[-1]]):
            self.actual["play_ready_seams"] = points
            self.assertFalse(self.check())

    def test_shifted_resume_fails(self):
        self.actual["play_ready_seams"][1]["row"] += 1
        self.assertFalse(self.check())

    def test_position_bits_and_native_rng_proof_required(self):
        rows = copy.deepcopy(self.rows)
        rows[3]["x"] = "-0"
        self.assertFalse(gate.checkpoints(self.rows, rows, self.meta, self.actual, self.report)[0])
        self.report["segments"][0]["play_ready_state_check"]["rng"]["verified"] = False
        self.assertFalse(self.check())

    def test_star_count_must_actually_rise(self):
        self.assertEqual(gate.stars(self.rows), [])
        self.rows[3]["power_stars"] = "1"
        self.rows[4]["power_stars"] = self.rows[5]["power_stars"] = "1"
        self.assertEqual(gate.stars(self.rows), [dict(row=3, stage=gate.STAGE, scenario=1, previous=0, power_stars=1)])
        self.rows[3]["valid"] = "0"
        self.assertEqual(gate.stars(self.rows)[0]["row"], 4)


if __name__ == "__main__":
    unittest.main()
