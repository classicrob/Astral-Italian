#!/usr/bin/env python3
"""Synthetic native producer receipt controls; no game data or game process."""
import csv
import hashlib
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import actor_receipt as receipt
from headless import trace_columns


class ReceiptTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="petari-actor-receipt-synthetic-")
        self.addCleanup(self.temp.cleanup)
        self.trace = Path(self.temp.name) / "synthetic.csv"
        self.marker = Path(str(self.trace) + ".complete")
        self.actors = Path(str(self.trace) + ".actors.csv")
        self.frames = Path(str(self.trace) + ".actor-frames.csv")
        self.main = dict.fromkeys(trace_columns(), "")
        self.main.update(frame="0", scene_epoch="0", scene_valid="1", scene="Game", stage="SyntheticGalaxy",
                         scenario="2", init_state="6", player_valid="1", actor_count_valid="1",
                         actor_registered="1", actor_living="1", dead="0", x_bits="0x80000000", y_bits="0x00000001", z_bits="0x00000000")
        self.census = dict(schema_version="1", frame="0", scene_epoch="0", scene="Game", stage="SyntheticGalaxy",
                           scenario="2", init_state="6", ready="1", registered="1", living="1", emitted="1", identity_unresolved="0", identity_ambiguous="0")
        self.actor = {key: self.census[key] for key in receipt.CONTEXT}
        self.actor.update(producer_handle="1", actor_id="player", identity_kind="player", identity_status="source-key",
                          name_valid="1", name_hex="82a0ff", link_valid="0", link_id="-1", link_zone="-1", link_context="-1", dead="0")
        for key in receipt.VECTORS:
            self.actor[key], self.actor[key + "_bits"] = "0", "0x00000000"
        self.actor.update(x="-0", x_bits="0x80000000", y="1.40129846e-45", y_bits="0x00000001")
        self.save()

    @staticmethod
    def write(path, columns, rows):
        with path.open("w", newline="") as output:
            writer = csv.DictWriter(output, columns); writer.writeheader(); writer.writerows(rows)

    def save(self, actors=None, frames=None, mains=None):
        self.write(self.trace, trace_columns(), [self.main] if mains is None else mains)
        self.write(self.actors, receipt.ACTOR_COLUMNS, [self.actor] if actors is None else actors)
        self.write(self.frames, receipt.ACTOR_FRAME_COLUMNS, [self.census] if frames is None else frames)
        self.marker.write_text("1\n")

    def collect(self):
        return receipt.collect_actor_receipt(self.trace, trace_columns())

    def reject(self):
        with self.assertRaises((ValueError, OSError, csv.Error)):
            self.collect()

    def test_schema_and_actual_hashes(self):
        result = self.collect()
        self.assertEqual((len(receipt.ACTOR_COLUMNS), len(receipt.ACTOR_FRAME_COLUMNS)), (36, 13))
        self.assertEqual({key: result[key] for key in ("schema_version", "status", "frames", "actor_rows", "identity_holds")},
                         dict(schema_version=1, status="captured", frames=1, actor_rows=1, identity_holds=0))
        self.assertEqual(result["qualification"], "sampled-identities-only")
        for item, path in zip(result["files"], [self.actors, self.frames]):
            self.assertEqual(item, dict(path=path.name, sha256=hashlib.sha256(path.read_bytes()).hexdigest()))

    def test_missing_complete(self):
        self.marker.unlink(); self.reject()

    def test_complete_count_not_inferred(self):
        self.marker.write_text("2\n"); self.reject()

    def test_partial_blocks_even_with_final(self):
        Path(str(self.actors) + ".partial").touch(); self.reject()

    def test_symlink_not_capture(self):
        self.actors.rename(self.actors.with_suffix(".saved")); self.actors.symlink_to(self.actors.with_suffix(".saved")); self.reject()

    def test_changed_header(self):
        self.write(self.actors, receipt.ACTOR_COLUMNS[::-1], [self.actor]); self.reject()

    def test_missing_census_frame(self):
        self.save(frames=[]); self.reject()

    def test_extra_actor_frame(self):
        other = dict(self.actor, frame="1"); self.save(actors=[self.actor, other]); self.reject()

    def test_context_epoch_disagreement(self):
        self.actor["scene_epoch"] = "1"; self.save(); self.reject()

    def test_registered_living_totals(self):
        self.census["living"] = "0"; self.save(); self.reject()

    def test_duplicate_handle(self):
        self.census.update(registered="2", living="2", emitted="2"); self.main.update(actor_registered="2", actor_living="2")
        self.save(actors=[self.actor, self.actor]); self.reject()

    def test_main_player_bits_must_match(self):
        self.main["x_bits"] = "0x00000000"; self.save(); self.reject()

    def test_no_name_inferred_player(self):
        self.actor.update(identity_kind="unresolved", identity_status="unresolved", actor_id="")
        self.census["identity_unresolved"] = "1"; self.save(); self.reject()

    def test_raw_signed_link_and_candidate(self):
        self.main["player_valid"] = "0"
        self.actor.update(identity_kind="link", actor_id="link:2:1:3:82a0ff", link_valid="1", link_id="3", link_zone="2", link_context="1")
        self.save(); self.assertEqual(self.collect()["identity_holds"], 0)
        self.actor["link_context"] = "-1"; self.save(); self.reject()

    def test_hold_counts_are_parsed_not_defaulted(self):
        self.actor.update(identity_status="key-reuse", actor_id="")
        self.census["identity_unresolved"] = "1"; self.save()
        result = self.collect(); self.assertEqual(result["identity_holds"], 1); self.assertEqual(result["qualification"], "identity-hold")
        self.census["identity_unresolved"] = "0"; self.save(); self.reject()

    def test_float_decimal_bits_mismatch(self):
        self.actor["x"] = "0"; self.save(); self.reject()

    def test_nonfinite_rejected(self):
        self.actor.update(x="inf", x_bits="0x7f800000"); self.save(); self.reject()

    def test_name_extent_and_null_distinction(self):
        self.actor.update(name_valid="0", name_hex=""); self.save(); self.assertEqual(self.collect()["actor_rows"], 1)
        self.actor.update(name_hex="00"); self.save(); self.reject()

    def test_unready_vs_empty(self):
        self.main.update(actor_count_valid="0", actor_registered="", actor_living="", player_valid="0")
        self.census.update(ready="0", registered="", living="", emitted="0")
        self.save(actors=[]); self.assertEqual(self.collect()["actor_rows"], 0)
        self.census.update(registered="0", living="0"); self.save(actors=[]); self.reject()

    def test_ready_empty_valid(self):
        self.main.update(actor_registered="0", actor_living="0", player_valid="0")
        self.census.update(registered="0", living="0", emitted="0"); self.save(actors=[])
        self.assertEqual(self.collect()["frames"], 1)

    def test_changed_files_during_read_fail(self):
        digest = receipt._digest; calls = 0
        def changed(path):
            nonlocal calls
            calls += 1
            return digest(path) if calls <= 2 else "0" * 64
        with patch.object(receipt, "_digest", side_effect=changed):
            self.reject()

    def test_missing_actor_rows_fail(self):
        self.save(actors=[]); self.reject()


if __name__ == "__main__":
    unittest.main()
