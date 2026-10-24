#!/usr/bin/env python3
"""Receipt safety checks for the GX normal-route tape gate (no game needed)."""
import copy
import importlib.util
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("normal_tape_gate", ROOT / "gate53/m1gates/normal_tape_gate.py")
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)


class VerdictTests(unittest.TestCase):
    def setUp(self):
        self.rows = [dict(stage=s, scenario="1", valid="1", dead="0", x="0", y="0", z="0", power_stars="242")
                     for s in ["FileSelect", "MarioFaceShipGalaxy", "WorldMap01Galaxy"] + ["IslandFleetGalaxy"] * 300]
        self.replay = dict(mode="whole-session", anchor_policy=gate.ANCHOR_POLICY["id"], finished=True, stopped="", segments=[dict(
            started=True, completed=True, rows_compared=301, tape_frames=300, pointer_rows_compared=300, diverged=False,
            play_ready_required=False)])

    def evaluate(self, log="", report=None):
        return gate.evaluate(self.rows, log, self.replay if report is None else report)[0]

    def test_complete_route_with_verified_replay_passes(self):
        self.assertTrue(all(self.evaluate().values()))

    def test_incomplete_missing_stopped_and_legacy_reports_fail(self):
        for patch in [dict(finished=False), dict(mode="segments"), dict(stopped="KPAD mismatch"), dict(segments=[])]:
            with self.subTest(patch=patch):
                report = dict(self.replay, **patch)
                self.assertFalse(all(self.evaluate(report=report).values()))
        for key, value in [("started", False), ("completed", False), ("rows_compared", 300), ("diverged", True)]:
            report = copy.deepcopy(self.replay)
            report["segments"][0][key] = value
            self.assertFalse(self.evaluate(report=report)["no_divergence"])

    def test_input_delivery_without_game_pointer_comparison_cannot_pass(self):
        for count in [None, 0, 299]:
            self.replay["segments"][0]["pointer_rows_compared"] = count
            self.assertFalse(self.evaluate()["game_pointer_buffers_compared"])

    def test_play_ready_requires_exact_state_and_rng_readback(self):
        segment = self.replay["segments"][0]
        segment["play_ready_required"] = True
        segment["play_ready_state_check"] = dict(checked=True, passed=True, position_bit_exact=True, rng=dict(verified=True))
        self.assertTrue(all(self.evaluate().values()))
        for key in ("checked", "passed", "position_bit_exact"):
            segment["play_ready_state_check"][key] = False
            self.assertFalse(self.evaluate()["play_ready_state_checks"])
            segment["play_ready_state_check"][key] = True
        segment["play_ready_state_check"]["rng"]["verified"] = False
        self.assertFalse(self.evaluate()["play_ready_state_checks"])
        self.replay["anchor_policy"] = "old-strict-scene-ready"
        self.assertFalse(self.evaluate()["anchor_policy_applied"])

    def test_withdrawn_scene_identity_policy_cannot_pass_final_gate(self):
        self.replay["anchor_policy"] = "scene-identity"
        self.assertFalse(self.evaluate()["anchor_policy_applied"])

    def test_destination_requires_300_finite_alive_consecutive_rows(self):
        for key, value in [("dead", "1"), ("valid", "0"), ("x", "nan"), ("scenario", "2")]:
            rows = copy.deepcopy(self.rows)
            rows[150][key] = value
            checks, _ = gate.evaluate(rows, "", self.replay)
            self.assertFalse(checks["destination_300_consecutive_valid"])

    def test_crash_bad_map_and_fresh_save_are_failures(self):
        for log, check in [("FATAL: crash", "no_faults"), ("stage=WorldMap99Galaxy", "no_invalid_worldmap_request"),
                           ("FileSelector.StartupCreate", "existing_unlocked_file1")]:
            self.assertFalse(self.evaluate(log)[check])


if __name__ == "__main__":
    unittest.main()
