import csv
import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import dolphin_collect as dc
from test_dolphin_collect import FakeRAM


class ActorTests(unittest.TestCase):
    def setUp(self):
        self.ram = FakeRAM()
        r = self.ram
        r.word(0x80001020, 0x80002000)
        r.word(0x80001024, 0x80003000)
        r.put(0x80003000, b"Game\0")
        r.put(0x80003040, b"TestGalaxy\0")
        r.word(0x80003080, 1)
        r.word(0x80003130, 6)
        r.word(0x8000312C, 0x80004000)
        r.word(0x80004010, 0x80004100)
        r.word(0x800041E4, 0x8000A000)  # AllLiveActorGroup, scene slot 0x39.
        r.word(0x8000A014, 2)
        self.members(0x8000C000, 0x8000D000)
        self.actor(0x8000C000, 0x8000E000, b"\x83\x8f\x83\x93", 1)
        self.actor(0x8000D000, 0x8000E100, b"Other", 2, dead=1)
        self.snapshot = dc.Snapshot(r, actors=True)

    def members(self, *addresses):
        self.ram.word(0x8000A018, len(addresses))
        self.ram.word(0x8000A01C, 0x8000B000)
        for index, address in enumerate(addresses):
            self.ram.word(0x8000B000 + 4 * index, address)

    def actor(self, address, name_address, name, follow, dead=0):
        r = self.ram
        r.word(address + 4, name_address)
        r.put(name_address, name + b"\0")
        r.put(address + 0xC, struct.pack(">hhh", follow, 2, 0))
        r.put(address + 0x14, struct.pack(">9I", 0x80000000, 1, 0x3f800000,
                                        0x40000000, 0x40400000, 0x40800000,
                                        0x3f800000, 0x3f800000, 0x3f800000))
        r.put(address + 0x70, bytes([dead]))

    def take(self, frame=0):
        row = self.snapshot.take(frame)
        self.assertEqual(row["observer_error"], "")
        return row, self.snapshot.actor_rows

    def test_complete_census_lossless_names_dead_and_nine_float_bits(self):
        row, actors = self.take()
        self.assertEqual((row["actor_registered"], row["actor_living"]), (2, 1))
        self.assertEqual(len(actors), 2)
        self.assertEqual(set(actors[0]), set(dc.ACTOR_COLUMNS))
        self.assertEqual(actors[0]["name_hex"], "838f8393")
        self.assertEqual(actors[0]["actor_id"], "link:2:0:1:838f8393")
        self.assertEqual((actors[0]["x"], actors[0]["x_bits"]), ("-0", "0x80000000"))
        self.assertEqual(actors[0]["y_bits"], "0x00000001")  # Do not flush stored subnormals.
        self.assertEqual(actors[0]["rot_x_bits"], "0x40000000")
        self.assertEqual(actors[0]["scale_z_bits"], "0x3f800000")
        self.assertEqual([actor["dead"] for actor in actors], [0, 1])
        self.assertNotIn("address", actors[0])

    def test_order_is_not_identity_and_handles_only_track_sampled_continuity(self):
        _, before = self.take()
        self.members(0x8000D000, 0x8000C000)
        _, after = self.take(1)
        self.assertEqual({a["actor_id"]: a["producer_handle"] for a in before},
                         {a["actor_id"]: a["producer_handle"] for a in after})

    def test_signed_invalid_link_is_unresolved_not_an_unsigned_key(self):
        self.ram.put(0x8000C00C, struct.pack(">hhh", 1, -32768, 0))
        _, actors = self.take()
        self.assertEqual((actors[0]["link_zone"], actors[0]["link_valid"]), (-32768, 0))
        self.assertEqual((actors[0]["actor_id"], actors[0]["identity_status"]), ("", "unresolved"))

    def test_duplicate_keys_retain_every_actor_and_hold_both(self):
        self.actor(0x8000D000, 0x8000E100, b"\x83\x8f\x83\x93", 1)
        _, actors = self.take()
        self.assertEqual(len(actors), 2)
        self.assertEqual([a["identity_status"] for a in actors], ["ambiguous", "ambiguous"])
        self.assertEqual([a["actor_id"] for a in actors], ["", ""])

    def test_disappearing_ambiguous_member_cannot_upgrade_remaining_identity(self):
        self.actor(0x8000D000, 0x8000E100, b"\x83\x8f\x83\x93", 1)
        self.take()
        self.members(0x8000C000)
        _, actors = self.take(1)
        self.assertEqual(len(actors), 1)
        self.assertEqual((actors[0]["identity_status"], actors[0]["actor_id"]), ("key-reuse", ""))

    def test_unresolved_to_valid_is_a_sticky_key_change_not_an_upgrade(self):
        self.ram.put(0x8000C00C, struct.pack(">hhh", -1, -1, -1))
        _, actors = self.take()
        self.assertEqual(actors[0]["identity_status"], "unresolved")
        self.ram.put(0x8000C00C, struct.pack(">hhh", 1, 2, 0))
        for frame in (1, 2):
            _, actors = self.take(frame)
            self.assertEqual((actors[0]["identity_status"], actors[0]["actor_id"]), ("key-change", ""))

    def test_null_and_empty_names_are_distinct(self):
        self.ram.word(0x8000C004, 0)
        self.ram.put(0x8000E100, b"\0")
        _, actors = self.take()
        self.assertEqual([(a["name_valid"], a["name_hex"]) for a in actors], [(0, ""), (1, "")])
        self.assertEqual(actors[0]["identity_kind"], "unresolved")
        self.assertEqual(actors[1]["actor_id"], "link:2:0:2:")

    def test_player_key_comes_from_holder_not_name(self):
        r = self.ram
        r.word(0x80004154, 0x80004500)
        r.word(0x80004514, 0x8000C000)
        r.word(0x8000C584, 0x80006000)
        r.word(0x80006138, 0x80008000)
        _, actors = self.take()
        self.assertEqual((actors[0]["identity_kind"], actors[0]["actor_id"]), ("player", "player"))
        self.assertEqual(actors[1]["identity_kind"], "link")

    def test_key_changes_are_sticky_holds(self):
        self.take()
        self.ram.put(0x8000E000, b"Changed\0")
        for frame in (1, 2):
            _, actors = self.take(frame)
            self.assertEqual((actors[0]["identity_status"], actors[0]["actor_id"]), ("key-change", ""))

    def test_observed_absence_then_key_reuse_is_a_new_handle_and_hold(self):
        _, initial = self.take()
        self.members(0x8000D000)
        self.take(1)
        self.members(0x8000C000, 0x8000D000)
        _, actors = self.take(2)
        self.assertNotEqual(initial[0]["producer_handle"], actors[0]["producer_handle"])
        self.assertEqual((actors[0]["identity_status"], actors[0]["actor_id"]), ("key-reuse", ""))

    def test_epoch_change_resets_history_but_not_monotonic_handle(self):
        _, initial = self.take()
        self.ram.put(0x80003040, b"OtherGalaxy\0")
        _, actors = self.take(1)
        self.assertGreater(actors[0]["producer_handle"], initial[0]["producer_handle"])
        self.assertEqual(actors[0]["identity_status"], "source-key")

    def test_nonready_gap_does_not_claim_old_pointer_continuity(self):
        _, initial = self.take()
        self.ram.word(0x80003130, 5)
        self.take(1)
        self.ram.word(0x80003130, 6)
        _, actors = self.take(2)
        self.assertNotEqual(initial[0]["producer_handle"], actors[0]["producer_handle"])
        self.assertEqual(actors[0]["identity_status"], "key-reuse")

    def test_bad_actor_data_fails_capture_instead_of_omitting_it(self):
        for address, data, message in ((0x8000C070, b"\x02", "nonboolean"),
                                       (0x8000C020, bytes.fromhex("7fc00000"), "nonfinite"),
                                       (0x8000E000, b"x" * 1024, "unterminated")):
            with self.subTest(message=message):
                original = bytes(self.ram.data)
                self.ram.put(address, data)
                self.assertIn(message, self.snapshot.take(0)["observer_error"])
                self.assertEqual(self.snapshot.actor_rows, [])
                self.ram.data[:] = original

    def test_duplicate_pointer_is_error_not_silent_deduplication(self):
        self.members(0x8000C000, 0x8000C000)
        self.assertIn("duplicate registered", self.snapshot.take(0)["observer_error"])

    def test_disabled_actor_capture_does_not_read_names_or_link_data(self):
        self.ram.word(0x8000C004, 1)
        row = dc.Snapshot(self.ram).take(0)
        self.assertEqual(row["observer_error"], "")
        self.assertIn("bad guest pointer", self.snapshot.take(0)["observer_error"])

    def test_census_covers_nonready_missing_group_and_valid_empty_frames(self):
        with tempfile.TemporaryDirectory() as directory:
            writer = dc.ActorSidecars(Path(directory))
            self.ram.word(0x80003130, 5)
            row, actors = self.take(0)
            writer.write(row, actors)
            self.ram.word(0x80003130, 6)
            self.ram.word(0x800041E4, 0)
            row, actors = self.take(1)
            writer.write(row, actors)
            self.ram.word(0x800041E4, 0x8000A000)
            self.members()
            row, actors = self.take(2)
            writer.write(row, actors)
            writer.close(True)
            with writer.paths[1].open() as stream:
                rows = list(csv.DictReader(stream))
            self.assertEqual([r["ready"] for r in rows], ["0", "0", "1"])
            self.assertEqual([r["registered"] for r in rows], ["", "", "0"])
            self.assertEqual([r["emitted"] for r in rows], ["0", "0", "0"])
            self.assertEqual(writer.manifest()["frames"], 3)

    def test_writer_checks_frame_origin_counts_context_and_binds_success_hashes(self):
        row, actors = self.take()
        with tempfile.TemporaryDirectory() as directory:
            writer = dc.ActorSidecars(Path(directory))
            with self.assertRaisesRegex(dc.ObserverError, "contiguous"):
                writer.write(dict(row, frame=2), actors)
            with self.assertRaisesRegex(dc.ObserverError, "every registered"):
                writer.write(row, actors[:1])
            with self.assertRaisesRegex(dc.ObserverError, "context"):
                writer.write(row, [dict(actors[0], scene_epoch=99), actors[1]])
            writer.write(row, actors)
            writer.close(True)
            manifest = writer.manifest()
            self.assertEqual((manifest["frames"], manifest["actor_rows"]), (1, 2))
            for path, entry in zip(writer.paths, manifest["files"]):
                self.assertEqual(entry["sha256"], hashlib.sha256(path.read_bytes()).hexdigest())
                self.assertFalse(Path(str(path) + ".partial").exists())

    def test_ambiguous_and_unresolved_counts_are_not_hidden(self):
        self.actor(0x8000D000, 0x8000E100, b"\x83\x8f\x83\x93", 1)
        row, actors = self.take()
        with tempfile.TemporaryDirectory() as directory:
            writer = dc.ActorSidecars(Path(directory))
            writer.write(row, actors)
            self.ram.put(0x8000C00C, struct.pack(">hhh", -1, -1, -1))
            row, actors = self.take(1)
            writer.write(row, actors)
            writer.close(True)
            with writer.paths[1].open() as stream:
                rows = list(csv.DictReader(stream))
            self.assertEqual((rows[0]['emitted'], rows[0]['identity_ambiguous']), ('2', '2'))
            self.assertEqual((rows[1]['emitted'], rows[1]['identity_unresolved']), ('2', '2'))
            self.assertEqual(writer.manifest()['identity_holds'], 4)

    def test_incomplete_writer_keeps_partial_files_and_no_capture_claim(self):
        row, actors = self.take()
        with tempfile.TemporaryDirectory() as directory:
            writer = dc.ActorSidecars(Path(directory))
            writer.write(row, actors)
            writer.close(False)
            self.assertEqual(writer.manifest()["status"], "incomplete")
            self.assertEqual(writer.manifest()["files"], [])
            self.assertTrue(all(Path(str(p) + ".partial").exists() for p in writer.paths))
            self.assertTrue(all(not p.exists() for p in writer.paths))

    def test_second_rename_failure_rolls_back_first_and_never_claims_complete(self):
        row, actors = self.take()
        with tempfile.TemporaryDirectory() as directory:
            writer = dc.ActorSidecars(Path(directory))
            writer.write(row, actors)
            rename = Path.rename
            def fail_census(path, target):
                if str(path).endswith('.actor-frames.csv.partial'):
                    raise OSError('synthetic second rename failure')
                return rename(path, target)
            with patch.object(Path, 'rename', fail_census):
                with self.assertRaisesRegex(OSError, 'second rename'):
                    writer.close(True)
            self.assertFalse(writer.complete)
            self.assertTrue(all(Path(str(p) + '.partial').exists() for p in writer.paths))
            self.assertTrue(all(not p.exists() for p in writer.paths))


class InputExportReceiptTests(unittest.TestCase):
    def test_actual_closed_file_hash_byte_length_and_read_count(self):
        data = b'{"frame":0,"read_ordinal":0}\n{"frame":0,"read_ordinal":1}\n'
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'inputs.jsonl').write_bytes(data)
            receipt = dc.input_export_receipt(root, 2)
            self.assertEqual(receipt, dict(schema_version=1, status='captured', path='inputs.jsonl',
                                          sha256=hashlib.sha256(data).hexdigest(), bytes=len(data), read_count=2))

    def test_missing_partial_incomplete_empty_record_and_count_mismatch_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'inputs.jsonl.partial').write_text('{}\n')
            with self.assertRaises(OSError):
                dc.input_export_receipt(root, 1)
            for data, count in ((b'{}', 1), (b'\n', 1), (b'{}\n', 2)):
                (root / 'inputs.jsonl').write_bytes(data)
                with self.subTest(data=data, count=count), self.assertRaises(dc.ObserverError):
                    dc.input_export_receipt(root, count)


if __name__ == "__main__":
    unittest.main()
