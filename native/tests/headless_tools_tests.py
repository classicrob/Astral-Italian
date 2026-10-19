#!/usr/bin/env python3
import copy
import importlib.util
import io
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("headless", ROOT / "native/tools/headless.py")
headless = importlib.util.module_from_spec(spec)
spec.loader.exec_module(headless)


class InterchangeTests(unittest.TestCase):
    def test_idle_then_jump_and_release(self):
        rows = list(headless.generate())
        self.assertEqual(len(rows), 602 * 4)
        for row in rows[:600 * 4]:
            self.assertEqual(row["samples"][0]["hold"], 0)
        self.assertEqual(rows[600 * 4]["samples"][0]["trig"], 0x0800)
        self.assertEqual(rows[601 * 4]["samples"][0]["release"], 0x0800)
        self.assertEqual(rows[1]["returned_count"], 0)
        self.assertEqual(rows[1]["samples"][0]["dev_type"], 253)
        self.assertEqual({r["channel"] for r in rows}, {0, 1, 2, 3})

    def test_wire_preserves_bits_and_slot_zero(self):
        rows = list(headless.generate(frames=1))
        rows[0]["samples"][0]["acc"]["x"] = headless.floating(-0.0)
        rows[1]["samples"][0]["unk_EC"] = 0xabcdef12
        out = io.BytesIO()
        headless.encode_rows(rows, 1, out)
        data = out.getvalue()
        self.assertEqual(data[:24], headless.MAGIC + struct.pack(">QQ", 1, 4))
        self.assertEqual(len(data), 24 + 4 * (40 + 240))
        self.assertEqual(data[24 + 40 + 12:24 + 40 + 16], b"\x80\x00\x00\x00")
        second = 24 + 280
        self.assertEqual(headless.RECORD.unpack_from(data, second)[4:7], (0, -2, -1))
        self.assertEqual(data[second + 40 + 236:second + 40 + 240], b"\xab\xcd\xef\x12")

    def test_rejects_lossy_and_nonfinite_floats(self):
        for value in [{"value": 1.0, "bits": "0x3f800001"}, {"value": float("nan"), "bits": "0x7fc00000"},
                      {"value": 0.0, "bits": "0x80000000"}, 1.0]:
            with self.subTest(value=value), self.assertRaises(ValueError):
                headless.fp(value)

    def test_requires_all_fields(self):
        sample = headless.idle_sample()
        del sample["unk_EC"]
        with self.assertRaises(ValueError):
            headless.encode_sample(sample)

    def test_rejects_order_capacity_and_missing_slot(self):
        base = list(headless.generate(frames=1))
        mutations = [("read_ordinal", 2), ("requested_length", 0), ("returned_count", 121),
                     ("channel", 4), ("samples", [])]
        for key, value in mutations:
            rows = copy.deepcopy(base)
            rows[0][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                headless.encode_rows(rows, 1, io.BytesIO())

    def test_repeated_channel_reads_and_error_slot(self):
        rows = list(headless.generate(frames=1))[:1]
        repeat = copy.deepcopy(rows[0])
        repeat.update(read_ordinal=1, returned_count=0, error=-1)
        repeat["samples"][0]["unk_EC"] = 123
        rows.append(repeat)
        out = io.BytesIO()
        headless.encode_rows(rows, 1, out)
        self.assertEqual(len(out.getvalue()), 24 + 2 * 280)
        repeat["connection"]["device_type"] = 0
        with self.assertRaises(ValueError):
            headless.encode_rows(rows, 1, io.BytesIO())

    def test_unknown_extension_fails(self):
        sample = headless.idle_sample()
        sample["dev_type"] = 255
        with self.assertRaises(ValueError):
            headless.encode_sample(sample)

    def test_classic_union_and_motionplus(self):
        sample = headless.idle_sample()
        sample["dev_type"] = 2
        v = {"x": headless.floating(0.5), "y": headless.floating(-0.5)}
        sample["ex_status"] = {"cl": {"hold": 0x1234, "trig": 2, "release": 4, "lstick": v, "rstick": v,
                                        "ltrigger": headless.floating(), "rtrigger": headless.floating(1)}}
        sample["mpls"]["angle"]["z"] = headless.floating(0.125)
        raw = headless.encode_sample(sample)
        self.assertEqual(struct.unpack_from(">I", raw, 96)[0], 0x1234)
        self.assertEqual(struct.unpack_from(">f", raw, 108)[0], 0.5)
        self.assertEqual(struct.unpack_from(">f", raw, 196)[0], 0.125)

    def test_duplicate_json_keys_fail(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "input.jsonl"
            path.write_text('{"frame":0,"frame":1}\n')
            with self.assertRaisesRegex(ValueError, "duplicate JSON key"):
                list(headless.rows_from_jsonl(path))

    def test_artifact_path_and_existing_empty_file(self):
        with self.assertRaises(ValueError):
            headless.outside_repo(ROOT / "build/not-allowed.csv")
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "existing.kpd"
            source = Path(tmp) / "input.jsonl"
            output.touch()
            source.write_text("{}\n")
            with self.assertRaises(FileExistsError):
                headless.main(["pack", str(source), str(output), "--frames", "1"])
            self.assertTrue(output.exists())

    def test_invalid_pack_removes_only_its_new_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "new.kpd"
            source = Path(tmp) / "input.jsonl"
            source.write_text("{}\n")
            with self.assertRaises(ValueError):
                headless.main(["pack", str(source), str(output), "--frames", "1"])
            self.assertFalse(output.exists())

    def test_run_uses_team_lock_and_no_emulator(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "inputs.jsonl"
            source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
            with patch.object(headless.subprocess, "call", return_value=0) as call:
                status = headless.main(["run", "--inputs", str(source), "--executable", str(root / "petari_headless"),
                                       "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                       "--trace", str(root / "native.csv"), "--epoch-ticks", "0"])
            self.assertEqual(status, 2)
            self.assertEqual(call.call_args.args[0][:6], ["lockf", "-k", str(Path.home() / "MacioGalaxy2-team/slots/native.lock"), "nice", "-n", "10"])
            self.assertFalse((root / "native.csv").exists())
            self.assertEqual(json.loads((root / "native.csv.manifest.json").read_text())["status"], "invalid")

    def test_shared_trace_columns(self):
        schema = json.loads((ROOT / "native/app/headless/trace_schema.json").read_text())
        self.assertEqual(schema["columns"], headless.trace_columns())
        self.assertEqual(len(schema["columns"]), len(set(schema["columns"])))
        for field in ("frame", "scene", "stage", "scenario", "x", "x_bits", "vx", "front_x", "actor_up_x", "gx", "state_ids", "anim_name", "hp", "rng_seed", "actor_living"):
            self.assertIn(field, schema["columns"])


if __name__ == "__main__":
    unittest.main()
