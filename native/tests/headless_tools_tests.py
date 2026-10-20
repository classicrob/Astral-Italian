#!/usr/bin/env python3
import copy
import csv
import importlib.util
import io
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("headless", ROOT / "native/tools/headless.py")
headless = importlib.util.module_from_spec(spec)
spec.loader.exec_module(headless)
bounded_spec = importlib.util.spec_from_file_location("bounded_run", ROOT / "native/tools/bounded_run.py")
bounded = importlib.util.module_from_spec(bounded_spec)
bounded_spec.loader.exec_module(bounded)
gx_spec = importlib.util.spec_from_file_location("gx_noops", ROOT / "native/app/headless/gen_gx_noops.py")
gx = importlib.util.module_from_spec(gx_spec)
gx_spec.loader.exec_module(gx)


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
        sample["dev_type"] = 254
        with self.assertRaises(ValueError):
            headless.encode_sample(sample)

    def test_initialization_device_sentinels_have_no_extension_union(self):
        for device in (252, 255):
            sample = headless.idle_sample(False)
            sample.update(dev_type=device, wpad_err=-1, unk_EC=0x12345678)
            raw = headless.encode_sample(sample)
            self.assertEqual(raw[92], device)
            self.assertEqual(raw[93], 255)
            self.assertEqual(raw[96:176], bytes(80))
            self.assertEqual(raw[236:240], bytes.fromhex("12345678"))
            sample["ex_status"] = {"fs": {}}
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

    def test_startup_request_is_forwarded_and_recorded(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "inputs.jsonl"
            source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
            request = 'galaxy "Tall Trunk" scenario 3'
            with patch.object(headless.subprocess, "call", return_value=2) as call:
                status = headless.main(["run", "--inputs", str(source), "--executable", str(root / "petari_headless"),
                                       "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                       "--trace", str(root / "native.csv"), "--epoch-ticks", "0", "--request", request])
            self.assertEqual(status, 2)
            command = call.call_args.args[0]
            self.assertEqual(command[command.index("--request") + 1], request)
            manifest = json.loads((root / "native.csv.manifest.json").read_text())
            self.assertEqual(manifest["startup_request"], request)
            self.assertEqual(manifest["status"], "invalid")
            self.assertFalse((root / "native.csv.complete").exists())

    def test_structured_startup_stage_is_forwarded(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "inputs.jsonl"
            source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
            with patch.object(headless.subprocess, "call", return_value=2) as call:
                status = headless.main(["run", "--inputs", str(source), "--executable", str(root / "petari_headless"),
                                       "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                       "--trace", str(root / "native.csv"), "--epoch-ticks", "0", "--stage", "Tall Trunk", "--scenario", "3"])
            self.assertEqual(status, 2)
            command = call.call_args.args[0]
            self.assertEqual(command[command.index("--stage") + 1], "Tall Trunk")
            self.assertEqual(command[command.index("--scenario") + 1], "3")
            manifest = json.loads((root / "native.csv.manifest.json").read_text())
            self.assertEqual((manifest["startup_stage"], manifest["startup_scenario"]), ("Tall Trunk", 3))

    def test_shared_trace_columns(self):
        schema = json.loads((ROOT / "native/app/headless/trace_schema.json").read_text())
        self.assertEqual(schema["columns"], headless.trace_columns())
        self.assertEqual(len(schema["columns"]), len(set(schema["columns"])))
        for field in ("frame", "scene", "stage", "scenario", "x", "x_bits", "vx", "front_x", "actor_up_x", "gx", "state_ids", "anim_name", "hp", "rng_seed", "actor_living"):
            self.assertIn(field, schema["columns"])

    def test_launcher_requires_and_accepts_completion_marker(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = root / "inputs.jsonl"
            source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
            def completed(command):
                columns = headless.trace_columns()
                row = ["0" if name in ("frame", "scene_epoch", "scene_valid", "player_valid", "actor_count_valid", "rng_valid", "animation_valid") else "" for name in columns]
                with (root / "native.csv").open("w", newline="") as out:
                    writer = csv.writer(out)
                    writer.writerow(columns)
                    writer.writerow(row)
                (root / "native.csv.complete").write_text("1\n")
                (root / "native.csv.timing.csv").write_text("frame,vi_count,ticks\n0,1,123\n")
                return 0
            with patch.object(headless.subprocess, "call", side_effect=completed):
                status = headless.main(["run", "--inputs", str(source), "--executable", str(root / "petari_headless"),
                                       "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                       "--trace", str(root / "native.csv"), "--epoch-ticks", "123"])
            self.assertEqual(status, 0)
            manifest = json.loads((root / "native.csv.manifest.json").read_text())
            self.assertEqual(manifest["status"], "complete")
            self.assertEqual(manifest["epoch_ticks"], 123)

    def test_wrong_schema_or_frame_sequence_is_incomplete(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "trace.csv"
            path.write_text("frame\n0\n")
            self.assertFalse(headless.valid_trace(path, 1))
            columns = headless.trace_columns()
            with path.open("w", newline="") as out:
                writer = csv.writer(out)
                writer.writerow(columns)
                row = ["0" if name in ("frame", "scene_epoch", "scene_valid", "player_valid", "actor_count_valid", "rng_valid", "animation_valid") else "" for name in columns]
                writer.writerow(row)
            self.assertTrue(headless.valid_trace(path, 1))
            with path.open("a", newline="") as out:
                row[0] = "2"
                csv.writer(out).writerow(row)
            self.assertFalse(headless.valid_trace(path, 2))

    def test_sync_completion_keeps_full_input_clock_and_validates_origin(self):
        for corruption in (None, "gated", "missing", "seed", "target", "timing", "row", "outside"):
            with self.subTest(corruption=corruption), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                source, trace = root / "inputs.jsonl", root / "native.csv"
                source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=5)) + "\n")
                def completed(command):
                    tape = Path(command[command.index("--inputs") + 1]).read_bytes()
                    self.assertEqual(struct.unpack_from(">QQ", tape, 8), (5, 20))
                    self.assertEqual(struct.unpack_from(">Q", tape, 24 + 16 * 280)[0], 4)
                    self.assertEqual(command[command.index("--sync-rng-seed") + 1], "0")
                    gated = corruption == "gated"
                    self.assertEqual("--inputs-from-sync" in command, gated)
                    count = 6 if gated else 2
                    origin = dict(schema_version=1, phase="post-endFrame", stage="TestGalaxy", scenario=2,
                                  source_frame=3, source_scene_epoch=7, source_ticks=1000,
                                  source_vi_count=80, rng_valid=True, rng_seed=0, rng_forced=True)
                    if gated: origin["source_frame"] = 999
                    if corruption == "seed": origin["rng_seed"] = 1
                    if corruption == "target": origin["scenario"] = 1
                    if corruption == "timing": origin["source_ticks"] = 999
                    if corruption == "outside": origin["source_frame"] = 5
                    if corruption != "missing":
                        Path(str(trace) + ".sync.json").write_text(json.dumps(origin))
                    with trace.open("w", newline="") as out:
                        writer = csv.DictWriter(out, fieldnames=headless.trace_columns())
                        writer.writeheader()
                        for frame in range(count):
                            writer.writerow(dict(frame=frame, scene_epoch=0, scene_valid=1, scene="Game", stage="TestGalaxy",
                                                 scenario=2, init_state=6, player_valid=0, actor_count_valid=0,
                                                 animation_valid=0, rng_valid=0 if corruption == "row" else 1, rng_seed=frame))
                    Path(str(trace) + ".complete").write_text(str(count) + "\n")
                    Path(str(trace) + ".timing.csv").write_text("frame,vi_count,ticks\n" + "".join(f"{i},{80+i},{1000*(i+1)}\n" for i in range(count)))
                    return 0
                with patch.object(headless.subprocess, "call", side_effect=completed):
                    status = headless.main(["run", "--inputs", str(source), "--executable", str(root / "petari_headless"),
                                           "--frames", "5", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                           "--trace", str(trace), "--epoch-ticks", "123", "--sync-stage", "TestGalaxy",
                                           "--sync-scenario", "2", "--sync-rng-seed", "0x0"] + (["--inputs-from-sync"] if corruption == "gated" else []))
                manifest = json.loads(Path(str(trace) + ".manifest.json").read_text())
                self.assertEqual(status, 0 if corruption in (None, "gated") else 2)
                self.assertEqual(manifest["input_frames"], 5)
                self.assertEqual(manifest["timing_policy"]["audio_output"], "off")
                if corruption in (None, "gated"):
                    gated = corruption == "gated"
                    self.assertEqual(manifest["inputs_from_sync"], gated)
                    if gated: self.assertEqual(manifest["input_source_start"], 1000)
                    self.assertEqual(manifest["trace_frames"], 6 if gated else 2)
                    self.assertEqual(manifest["sync_origin"]["source_frame"], 999 if gated else 3)

    def test_actor_trace_preflight_preserves_existing_sidecars(self):
        for suffix in (".actors.csv", ".actor-frames.csv", ".actors.csv.partial", ".actor-frames.csv.partial"):
            with self.subTest(suffix=suffix), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                trace = root / "trace.csv"
                existing = Path(str(trace) + suffix)
                existing.write_text("existing evidence")
                with patch.object(headless.subprocess, "call") as run:
                    with self.assertRaisesRegex(ValueError, "overwrite"):
                        headless.main(["run", "--inputs", str(root / "inputs.jsonl"), "--executable", str(root / "petari_headless"),
                                       "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                       "--trace", str(trace), "--epoch-ticks", "0", "--actor-trace"])
                    run.assert_not_called()
                self.assertEqual(existing.read_text(), "existing evidence")

    def test_actor_trace_launcher_parses_actual_receipt_and_invalidates_missing_rows(self):
        actor_spec = importlib.util.spec_from_file_location("receipt", ROOT / "native/tools/actor_receipt.py")
        receipt = importlib.util.module_from_spec(actor_spec)
        actor_spec.loader.exec_module(receipt)
        for broken in (False, True):
            with self.subTest(broken=broken), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                source, trace = root / "inputs.jsonl", root / "native.csv"
                source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
                def completed(command):
                    self.assertIn("--actor-trace", command)
                    context = dict(frame="0", scene_epoch="0", scene="Game", stage="SyntheticGalaxy", scenario="1", init_state="6")
                    main = dict(context, scene_valid=1, player_valid=0, actor_count_valid=1, rng_valid=0,
                                animation_valid=0, actor_registered=0, actor_living=0)
                    def write(path, columns, rows):
                        with path.open("w", newline="") as out:
                            writer = csv.DictWriter(out, columns); writer.writeheader(); writer.writerows(rows)
                    write(trace, headless.trace_columns(), [main])
                    write(Path(str(trace) + ".actors.csv"), receipt.ACTOR_COLUMNS, [])
                    census = dict(context, schema_version=1, ready=1, registered=0, living=0, emitted=0,
                                  identity_unresolved=0, identity_ambiguous=0)
                    write(Path(str(trace) + ".actor-frames.csv"), receipt.ACTOR_FRAME_COLUMNS, [] if broken else [census])
                    Path(str(trace) + ".complete").write_text("1\n")
                    Path(str(trace) + ".timing.csv").write_text("frame,vi_count,ticks\n0,80,1000\n")
                    return 0
                with patch.object(headless.subprocess, "call", side_effect=completed):
                    status = headless.main(["run", "--inputs", str(source), "--executable", str(root / "petari_headless"),
                                           "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                           "--trace", str(trace), "--epoch-ticks", "0", "--actor-trace"])
                manifest = json.loads(Path(str(trace) + ".manifest.json").read_text())
                self.assertEqual(status, 2 if broken else 0)
                self.assertEqual(manifest["status"], "invalid" if broken else "complete")
                self.assertTrue(manifest["observer_contract_incomplete"])
                self.assertEqual(manifest["field_availability"], {"pending_state_id": "unavailable (callback-context only)"})
                actors = manifest["actors"]
                self.assertEqual(actors["status"], "incomplete" if broken else "captured")
                if broken:
                    self.assertIsNone(actors["frames"])
                    self.assertEqual(actors["files"], [])
                else:
                    self.assertEqual((actors["frames"], actors["actor_rows"], actors["identity_holds"]), (1, 0, 0))
                    self.assertEqual(actors["qualification"], "sampled-identities-only")
                    self.assertEqual(actors["files"][0]["path"], trace.name + ".actors.csv")
                    self.assertEqual(len(actors["files"][0]["sha256"]), 64)

    def test_sync_flags_require_target_and_unsigned_numbers(self):
        with self.assertRaisesRegex(ValueError, "requires --sync-stage"):
            headless.main(["run", "--executable", "/tmp/no-binary", "--inputs", "/tmp/no-inputs",
                           "--frames", "1", "--disc", "/tmp/disc", "--user", "/tmp/user",
                           "--trace", "/tmp/trace", "--epoch-ticks", "0", "--inputs-from-sync"])
        for value in ("", "-1", "+1", " 1", "0x", "4294967296"):
            with self.subTest(value=value), self.assertRaises(headless.argparse.ArgumentTypeError):
                headless.sync_number(value)
        self.assertEqual(headless.sync_number("012"), 12)
        self.assertEqual(headless.sync_number("0xffffffff"), 0xffffffff)

    def test_wall_deadline_terminates_only_the_spawned_process_group(self):
        with patch.object(bounded.subprocess, "Popen") as spawn, patch.object(bounded.os, "killpg") as kill:
            child = spawn.return_value
            child.pid = 12345
            child.wait.side_effect = [bounded.subprocess.TimeoutExpired("fixture", 540), 0, 0]
            self.assertEqual(bounded.run(["fixture"], 540), 124)
            spawn.assert_called_once_with(["fixture"], start_new_session=True)
            self.assertEqual(kill.call_args_list, [unittest.mock.call(12345, bounded.signal.SIGTERM),
                                                  unittest.mock.call(12345, bounded.signal.SIGKILL)])

    def test_replay_wall_watchdog_accepts_explicit_long_limit(self):
        with patch.object(bounded, "run", return_value=0) as run:
            self.assertEqual(bounded.main(["1800", "fixture"]), 0)
            run.assert_called_once_with(["fixture"], 1800)
        with self.assertRaises(SystemExit): bounded.main(["3601", "fixture"])
        with self.assertRaises(SystemExit): bounded.main(["0", "fixture"])

    def test_custom_wall_timeout_is_recorded_and_passed_to_watchdog(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); source = root / "inputs.jsonl"; trace = root / "trace.csv"
            source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
            def stopped(command):
                watchdog = command.index(str(ROOT / "native/tools/bounded_run.py"))
                self.assertEqual(command[watchdog+1], "1800")
                return 124
            with patch.object(headless.subprocess, "call", side_effect=stopped):
                result = headless.main(["run", "--inputs", str(source), "--executable", str(root / "fixture"),
                    "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                    "--trace", str(trace), "--epoch-ticks", "0", "--wall-timeout", "1800"])
            self.assertEqual(result, 2)
            self.assertEqual(json.loads(Path(str(trace)+".manifest.json").read_text())["wall_timeout_seconds"], 1800)

    def test_complete_placeholder_trace_never_qualifies(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source, trace, executable = root / "inputs.jsonl", root / "native.csv", root / "fixture"
            source.write_text("\n".join(json.dumps(r) for r in headless.generate(frames=1)) + "\n")
            executable.write_text("fixture binary")
            Path(str(executable) + ".diagnostic.json").write_text('{"diagnostic_only":true}')
            def completed(command):
                with trace.open("w", newline="") as out:
                    writer = csv.DictWriter(out, fieldnames=headless.trace_columns())
                    writer.writeheader()
                    writer.writerow(dict(frame=0, scene_epoch=0, scene_valid=0, rng_valid=0,
                                         player_valid=0, actor_count_valid=0, animation_valid=0))
                Path(str(trace) + ".complete").write_text("1\n")
                Path(str(trace) + ".timing.csv").write_text("frame,vi_count,ticks\n0,1,123\n")
                return 0
            with patch.object(headless.subprocess, "call", side_effect=completed):
                result = headless.main(["run", "--inputs", str(source), "--executable", str(executable),
                                       "--frames", "1", "--disc", str(root / "disc"), "--user", str(root / "user"),
                                       "--trace", str(trace), "--epoch-ticks", "123"])
            manifest = json.loads(Path(str(trace) + ".manifest.json").read_text())
            self.assertEqual(result, 2)
            self.assertTrue(manifest["diagnostic_placeholder"])
            self.assertEqual(manifest["status"], "diagnostic-complete")

    def test_timing_sidecar_rejects_gaps_and_backwards_time(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "timing.csv"
            path.write_text("frame,vi_count,ticks\n0,1,1013512\n1,2,2027025\n")
            self.assertTrue(headless.valid_timing(path, 2))
            path.write_text("frame,vi_count,ticks\n0,1,1013512\n1,2,0\n")
            self.assertFalse(headless.valid_timing(path, 2))
            path.write_text("frame,vi_count,ticks\n0,1,1013512\n2,2,2027025\n")
            self.assertFalse(headless.valid_timing(path, 2))

    def test_native_schema_matches_current_dolphin_collector(self):
        path = ROOT / "tools/port/replaydiff/dolphin_collect.py"
        collector_spec = importlib.util.spec_from_file_location("headless_collector_contract", path)
        collector = importlib.util.module_from_spec(collector_spec)
        collector_spec.loader.exec_module(collector)
        self.assertEqual(headless.trace_columns(), collector.COLUMNS)
        self.assertEqual(json.loads((ROOT / "native/app/headless/trace_schema.json").read_text())["schema_version"], 1)

    def test_symlink_cannot_put_artifacts_in_checkout(self):
        with tempfile.TemporaryDirectory() as tmp:
            link = Path(tmp) / "repo-link"
            link.symlink_to(ROOT, target_is_directory=True)
            with self.assertRaises(ValueError):
                headless.outside_repo(link / "build/trace.csv")


class NullGxGeneratorTests(unittest.TestCase):
    def test_real_platform_definition_excludes_query_but_a_call_does_not(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "gx.h").write_text("void GXReadReal(int*);\nvoid GXReadMissing(int*);\n")
            manual, platform, output = root / "manual.cpp", root / "platform.cpp", root / "out.inc"
            manual.write_text("void helper() { GXReadMissing(nullptr); }\n")
            platform.write_text("void GXReadReal(int* out) { *out=0; }\n")
            with patch.object(sys, "argv", ["gen_gx_noops.py", str(root), str(output), "--manual", str(manual), "--manual", str(platform)]):
                gx.main()
            generated = output.read_text()
            self.assertNotIn("GXReadReal", generated)
            self.assertIn('unsupported("GXReadMissing")', generated)

    def test_native_abi_extern_declaration_and_missing_newline(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "gx.h"
            path.write_text("#ifdef GALAXY_NATIVE\nextern void GXSetArray(int, const void*, unsigned, unsigned char, bool);\n#else\nvoid GXSetArray(int, const void*, unsigned char);\n#endif\nvoid GXSetThing(int);\n// no final newline")
            declared = gx.declarations(path.parent)
            self.assertEqual(declared["GXSetArray"][1], "int, const void*, unsigned, unsigned char, bool")
            self.assertIn("GXSetThing", declared)

    def test_queries_panic_instead_of_returning_fake_data(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "gx.h").write_text("extern void GXSetThing(int);\nvoid GXGetThing(int*);\nunsigned GXQueryThing(void);\nvoid GXManual(int);\n")
            manual = root / "manual.cpp"
            manual.write_text("void GXManual(int) {}\n")
            output = root / "noops.inc"
            with patch.object(sys, "argv", ["gen_gx_noops.py", str(root), str(output), "--manual", str(manual)]):
                gx.main()
            generated = output.read_text()
            self.assertIn("GXSetThing(int) { ++gCalls; }", generated)
            self.assertIn('GXGetThing(int*) { unsupported("GXGetThing"); }', generated)
            self.assertIn('GXQueryThing() { unsupported("GXQueryThing"); }', generated)
            self.assertNotIn("GXManual", generated)


if __name__ == "__main__":
    unittest.main()
