#!/usr/bin/env python3
"""Synthetic, game-data-free tests for the replay comparator."""
import contextlib
import csv
import io
import json
from pathlib import Path
import struct
import tempfile
import unittest

import tracediff as td


class TraceDiffTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.serial = 0

    def trace(self, rows, columns=None, actor_keys=()):
        path = self.path(rows, columns)
        return td.read_trace(path, actor_keys)

    def path(self, rows, columns=None):
        self.serial += 1
        path = self.directory / f"{self.serial}.csv"
        with path.open("w", newline="") as output:
            writer = csv.DictWriter(output, columns or list(rows[0]))
            writer.writeheader()
            writer.writerows(rows)
        return path

    def rows(self, count=4):
        return [dict(frame=str(frame), scene_epoch="1", stage="Fixture", valid="1", x="100", vx="1", state_ids="[1,2]") for frame in range(count)]

    def test_identical(self):
        a = self.trace(self.rows())
        report = td.compare(a, a)
        self.assertEqual(report["exit_code"], 0)
        self.assertIsNone(report["first_divergence"])

    def test_initial_non_rng_state_is_separate_from_seed(self):
        a, b = self.rows(), self.rows()
        for rows, seed in ((a, "10"), (b, "20")):
            for row in rows:
                row.update(rng_seed=seed, rng_valid="1", player_valid="0")
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 1)
        self.assertTrue(report["initial_non_rng_state"]["equal_within_policy"])
        self.assertEqual(report["initial_non_rng_state"]["player_valid_a"], ["0"])
        b[0]["x"] = "103"
        report = td.compare(self.trace(a), self.trace(b))
        self.assertFalse(report["initial_non_rng_state"]["equal_within_policy"])
        self.assertEqual(report["initial_non_rng_state"]["differences"][0]["columns"][0]["column"], "x")

    def test_missing_origin_does_not_claim_initial_equivalence(self):
        report = td.compare(self.trace(self.rows()[1:]), self.trace(self.rows()[1:]))
        self.assertFalse(report["initial_non_rng_state"]["complete"])
        self.assertFalse(report["initial_non_rng_state"]["equal_within_policy"])

    def test_float_jitter_and_bit_difference(self):
        a, b = self.rows(), self.rows()
        for rows, value in ((a, 100.0), (b, 100.00000762939453)):
            bits = struct.pack(">f", value).hex()
            for row in rows:
                row.update(x=format(value, ".9g"), x_bits=bits if rows is a else "0x" + bits)
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 0)
        self.assertEqual(report["first_bit_difference"]["frame"], 0)
        self.assertEqual(report["first_bit_difference"]["ulp"], 1)

    def test_first_divergence_300_and_recovery(self):
        a, b = self.rows(304), self.rows(304)
        for frame in (300, 301):
            b[frame].update(x="101", vx="3", state_ids="[2,1]")
        b[303]["vx"] = "4"
        report = td.compare(self.trace(a), self.trace(b), history=3)
        self.assertEqual(report["exit_code"], 1)
        first = report["first_divergence"]
        self.assertEqual(first["frame"], 300)
        self.assertEqual({d["column"] for d in first["columns"]}, {"x", "vx", "state_ids"})
        self.assertEqual([r["frame"] for r in report["history"]], [297, 298, 299])
        self.assertEqual(report["summary"]["diverging_frames"], 3)
        self.assertEqual(report["summary"]["later_diverging_frames"], 2)
        self.assertEqual(report["summary"]["first_matching_frame_after_divergence"], 302)
        self.assertFalse(report["summary"]["recovered_at_end"])

    def test_matching_invalid_rows_then_validity_mismatch(self):
        a, b = self.rows(), self.rows()
        a[1].update(valid="0", x="", vx="", state_ids="")
        b[1].update(a[1])
        self.assertEqual(td.compare(self.trace(a), self.trace(b))["exit_code"], 0)
        b[1]["valid"] = "1"
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["first_divergence"]["columns"][0]["column"], "valid")

    def test_invalid_does_not_hide_stale_column_difference(self):
        a, b = self.rows(), self.rows()
        a[0]["valid"] = b[0]["valid"] = "0"
        b[0]["state_ids"] = "[7]"
        self.assertEqual(td.compare(self.trace(a), self.trace(b))["exit_code"], 1)

    def test_observer_errors_never_pass_even_when_matching(self):
        rows = self.rows()
        for row in rows:
            row["observer_error"] = "synthetic invalid read"
        a = self.trace(rows)
        self.assertEqual(td.compare(a, a)["exit_code"], 2)

    def test_missing_frame_no_realign(self):
        a, b = self.rows(), self.rows()
        del b[1]
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 2)
        self.assertEqual(report["first_divergence"]["frame"], 1)
        self.assertEqual(report["summary"]["diverging_frames"], 1)
        a = self.trace(b)
        self.assertEqual(td.compare(a, a)["exit_code"], 2)

    def test_truncation(self):
        report = td.compare(self.trace(self.rows()), self.trace(self.rows()[:-1]))
        self.assertEqual(report["exit_code"], 2)
        self.assertEqual(report["first_divergence"]["frame"], 3)

    def test_reordered_columns(self):
        rows = self.rows()
        a, b = self.trace(rows), self.trace(rows, list(reversed(rows[0])))
        self.assertEqual(td.compare(a, b)["exit_code"], 0)

    def test_schema_mismatch_is_not_intersection_comparison(self):
        a, b = self.rows(), self.rows()
        for row in b:
            row["extra"] = "same"
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 2)
        self.assertEqual(report["schema"]["missing_in_a"], ["extra"])

    def test_scene_epoch_and_stage_are_exact(self):
        a, b = self.rows(), self.rows()
        b[2].update(scene_epoch="2", stage="Different")
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["first_divergence"]["frame"], 2)
        self.assertEqual({d["column"] for d in report["first_divergence"]["columns"]}, {"scene_epoch", "stage"})

    def test_no_float_tolerance_for_integers_or_unknown_strings(self):
        a, b = self.rows(), self.rows()
        for row in a + b:
            row.update(rng_seed="4294967294", label="1")
        b[0].update(rng_seed="4294967295", label="1.000000")
        report = td.compare(self.trace(a), self.trace(b), td.Policy(abs_tol=100))
        self.assertEqual({d["column"] for d in report["first_divergence"]["columns"]}, {"rng_seed", "label"})

    def test_nonfinite_and_bad_numeric_never_pass(self):
        for value in ("nan", "inf", "-inf", "nope", "1e100"):
            with self.subTest(value=value):
                rows = self.rows()
                rows[0]["x"] = value
                a = self.trace(rows)
                self.assertEqual(td.compare(a, a)["exit_code"], 2)

    def test_inconsistent_bits_never_pass(self):
        rows = self.rows()
        for row in rows:
            row["x_bits"] = "00000000"
        a = self.trace(rows)
        self.assertEqual(td.compare(a, a)["exit_code"], 2)

    def test_signed_zero_bits_recorded_without_failure(self):
        a, b = self.rows(), self.rows()
        for row in a:
            row.update(x="0", x_bits="00000000")
        for row in b:
            row.update(x="-0", x_bits="80000000")
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 0)
        self.assertEqual(report["first_bit_difference"]["ulp"], 0)

    def test_policy_custom_float_and_exact_override(self):
        a, b = self.rows(), self.rows()
        for row in a + b:
            row["speed"] = "1.0"
        b[0].update(speed="1.00001", x="100.00001")
        a, b = self.trace(a), self.trace(b)
        self.assertEqual(td.compare(a, b)["exit_code"], 1)
        policy = td.Policy(overrides={"speed": {"type": "float", "abs": 0.001}})
        self.assertEqual(td.compare(a, b, policy)["exit_code"], 0)
        policy = td.Policy(overrides={"x": {"type": "exact"}, "speed": {"type": "float"}})
        self.assertEqual(td.compare(a, b, policy)["exit_code"], 1)
        self.assertEqual(td.compare(a, b, td.Policy(overrides={"typo": {}}))["exit_code"], 2)
        for value in (-1, float("nan"), float("inf")):
            with self.assertRaises(td.TraceError):
                td.Policy(abs_tol=value)

    def test_invalid_frame_sequence_and_duplicate_headers(self):
        for frames in ((0, 0), (1, 0), (-1, 0)):
            rows = self.rows(2)
            for row, frame in zip(rows, frames):
                row["frame"] = str(frame)
            with self.assertRaises(td.TraceError):
                self.trace(rows)
        path = self.directory / "bad.csv"
        path.write_text("frame,x,x\n0,1,1\n")
        with self.assertRaises(td.TraceError):
            td.read_trace(path)

    def test_actor_identity_and_first_diverging_actor(self):
        keys = ("actor_name", "actor_id")
        a = [dict(frame=str(f), scene_epoch="0", stage="Fixture", actor_name=name,
                  actor_id=str(i), x="0", nerve="Wait") for f in range(3) for i, name in enumerate(("A", "B"))]
        b = [dict(row) for row in a]
        b[3]["nerve"] = "Move"
        b[0], b[1] = b[1], b[0]
        report = td.compare(self.trace(a, actor_keys=keys), self.trace(b, actor_keys=keys), actor_keys=keys)
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["first_divergence"]["frame"], 1)
        self.assertEqual(report["first_divergence"]["actor_name"], "B")
        self.assertEqual(len(report["history"]), 1)

    def test_actor_missing_is_divergence(self):
        keys = ("actor_name", "actor_id")
        a = [dict(frame="0", actor_name="A", actor_id="1", x="0"),
             dict(frame="0", actor_name="B", actor_id="2", x="0")]
        report = td.compare(self.trace(a, actor_keys=keys), self.trace(a[:1], actor_keys=keys), actor_keys=keys)
        self.assertEqual(report["exit_code"], 1)
        self.assertEqual(report["first_divergence"]["actor_name"], "B")

    def test_bad_values_are_invalid_even_opposite_empty_or_missing_rows(self):
        a, b = self.rows(), self.rows()
        a[1]["x"], b[1]["x"] = "", "nan"
        self.assertEqual(td.compare(self.trace(a), self.trace(b))["exit_code"], 2)
        del a[1]
        self.assertEqual(td.compare(self.trace(a), self.trace(b))["exit_code"], 2)
        a, b = self.rows(), self.rows()
        a[0]["valid"], b[0]["valid"] = "", "2"
        self.assertEqual(td.compare(self.trace(a), self.trace(b))["exit_code"], 2)

    def test_declared_bits_must_be_populated_with_decimal(self):
        rows = self.rows()
        for row in rows:
            row["x_bits"] = ""
        a = self.trace(rows)
        self.assertEqual(td.compare(a, a)["exit_code"], 2)

    def test_tolerance_boundary_and_vector_errors(self):
        a, b = self.rows(), self.rows()
        for row in a + b:
            row.update(x="0", y="0", z="0")
        b[1].update(x="0.01")
        self.assertEqual(td.compare(self.trace(a), self.trace(b), td.Policy(rel_tol=0))["exit_code"], 0)
        b[1].update(x="3", y="4")
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["first_divergence"]["vector_errors"]["position"]["euclidean"], 5)
        self.assertEqual(report["first_divergence"]["vector_errors"]["position"]["max_component"], 4)

    def test_actor_duplicate_identity_and_context_mismatch(self):
        keys = ("actor_name", "actor_id")
        row = dict(frame="0", stage="One", actor_name="A", actor_id="1")
        with self.assertRaisesRegex(td.TraceError, "duplicate"):
            self.trace([row, row], actor_keys=keys)
        with self.assertRaisesRegex(td.TraceError, "inconsistent scene"):
            self.trace([row, dict(row, actor_id="2", stage="Two")], actor_keys=keys)

    def test_recovered_at_end_still_fails(self):
        a, b = self.rows(), self.rows()
        b[1]["x"] = "200"
        report = td.compare(self.trace(a), self.trace(b))
        self.assertEqual(report["exit_code"], 1)
        self.assertTrue(report["summary"]["recovered_at_end"])

    def test_expected_frames_detects_identical_truncation(self):
        trace = self.trace(self.rows())
        self.assertEqual(td.compare(trace, trace, expected_frames=5)["exit_code"], 2)
        self.assertEqual(td.compare(trace, trace, expected_frames=4)["exit_code"], 0)

    def test_empty_declared_validity_never_passes(self):
        rows = self.rows()
        rows[0]["valid"] = ""
        trace = self.trace(rows)
        self.assertEqual(td.compare(trace, trace)["exit_code"], 2)
        rows[0].update(valid="0", x="nan")
        trace = self.trace(rows)
        self.assertEqual(td.compare(trace, trace, td.Policy(overrides={"x": {"type": "exact"}}))["exit_code"], 2)

    def test_native_collector_column_set_is_supported(self):
        from dolphin_collect import COLUMNS
        row = dict.fromkeys(COLUMNS, "")
        row.update(frame="0", scene_epoch="-1", scene_valid="0", player_valid="0", actor_count_valid="0", rng_valid="0", animation_valid="0")
        trace = self.trace([row])
        self.assertEqual(td.compare(trace, trace)["exit_code"], 0)

    def test_cli_json_exit_codes_and_report(self):
        a, b = self.rows(), self.rows()
        ap, bp = self.path(a), self.path(b)
        def invoke(*args):
            out = io.StringIO()
            with contextlib.redirect_stdout(out), contextlib.redirect_stderr(io.StringIO()):
                code = td.main([str(ap), str(bp), *args])
            return code, out.getvalue()
        code, output = invoke("--json", "-")
        self.assertEqual(code, 0)
        self.assertEqual(json.loads(output)["status"], "identical")
        b[1]["x"] = "200"
        bp = self.path(b)
        self.assertEqual(invoke()[0], 1)
        bp.write_text("frame,x\n0,nan\n")
        self.assertEqual(invoke()[0], 2)


if __name__ == "__main__":
    unittest.main()
