#!/usr/bin/env python3
"""Synthetic RSP sessions: no Dolphin executable or game data needed."""
import contextlib
import io
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

from dolphin_collect import CALLER, ObserverError, SEAM
import watchwriter as ww

TARGET = 0x80010000
PC = 0x80008000


class FakeRSP:
    def __init__(self, watch=True):
        self.watch = watch
        self.pc = SEAM
        self.lr = CALLER
        self.value = b"\0\0\0\0"
        self.calls = []
        self.events = []
        self.instructions = {}
        self.registers = {3: TARGET, 4: 0}

    def command(self, command):
        self.calls.append(command)
        return "OK" if self.watch else ""

    def ok(self, command):
        self.calls.append(command)

    def reg(self, number):
        if number == 64:
            return self.pc
        if number == 67:
            return self.lr
        return self.registers.get(number, 0)

    def read(self, address, size):
        if address == TARGET:
            return self.value[:size]
        return struct.pack(">I", self.instructions.get(address, 0x60000000))[:size]

    def stop(self, command, startup=False):
        self.calls.append(command)
        if command == "?":
            return "S05"
        if command == "s" and self.pc == SEAM:
            self.pc += 4
            return "S05"
        if not self.events:
            raise AssertionError("fake event queue exhausted")
        self.pc, self.lr, self.value, reply = self.events.pop(0)
        return reply


class WriterTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        path = Path(self.temp.name) / "symbols.txt"
        path.write_text("writer = .text:0x80008000; // type:function size:0x20 scope:global\n"
                        "data = .data:0x80008040; // type:object size:0x100\n"
                        "next = .text:0x80008100; // type:function size:0x10\n")
        self.symbol_path = path
        self.symbols = ww.Symbols(path)

    def test_symbols_size_boundaries(self):
        self.assertEqual(self.symbols.lookup(PC + 4)["function"], "writer")
        self.assertEqual(self.symbols.lookup(PC + 4)["offset"], "0x4")
        self.assertIsNone(self.symbols.lookup(PC + 0x20)["function"])
        self.assertIsNone(self.symbols.lookup(PC - 4)["function"])
        self.assertIsNone(self.symbols.lookup(PC + 0x40)["function"])

    def test_watchpoint_records_and_removes_watch(self):
        remote = FakeRSP()
        remote.events = [(PC, 0, bytes.fromhex("3f800000"), f"T05watch:{TARGET:x};"),
                         (PC + 4, 0, bytes.fromhex("40000000"), f"T05watch:{TARGET:x};"),
                         (SEAM, CALLER, bytes.fromhex("40000000"), "S05")]
        runner = ww.FrameRunner(remote)
        runner.install()
        result = ww.trace_window(remote, runner, TARGET, 4, self.symbols)
        self.assertTrue(result["complete"])
        self.assertEqual(result["mode"], "write-watchpoint")
        self.assertEqual([w["offset"] for w in result["writes"]], ["0x0", "0x4"])
        self.assertIn(f"z2,{TARGET:x},4", remote.calls)
        self.assertEqual(result["final_bytes"], "40000000")

    def test_unclassified_stop_fails_closed_and_cleans_up(self):
        remote = FakeRSP()
        remote.events = [(PC, 0, bytes(4), "S05")]
        with self.assertRaisesRegex(ObserverError, "unclassified stop"):
            ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols)
        self.assertIn(f"z2,{TARGET:x},4", remote.calls)

    def test_no_watchpoint_step_fallback(self):
        remote = FakeRSP(watch=False)
        # Seam step -> body -> a real writer -> an unchanged-value writer -> seam.
        remote.events = [(PC, 0, bytes(4), "S05"),
                         (PC + 4, 0, bytes.fromhex("3f800000"), "S05"),
                         (PC + 8, 0, bytes.fromhex("3f800000"), "S05"),
                         (SEAM, CALLER, bytes.fromhex("3f800000"), "S05")]
        remote.instructions[PC] = (36 << 26) | (3 << 16)
        remote.instructions[PC + 4] = (36 << 26) | (3 << 16)
        runner = ww.FrameRunner(remote)
        runner.install()
        result = ww.trace_window(remote, runner, TARGET, 4, self.symbols)
        self.assertTrue(result["complete"])
        self.assertEqual(result["mode"], "single-step")
        self.assertEqual(len(result["writes"]), 2)
        self.assertTrue(result["writes"][0]["changed"])
        self.assertFalse(result["writes"][1]["changed"])
        self.assertTrue(any("falling back" in s for s in result["limitations"]))

    def test_step_limit_is_an_error(self):
        remote = FakeRSP(watch=False)
        with self.assertRaisesRegex(ObserverError, "single-step limit"):
            ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols, max_steps=1)

    def test_watch_only_rejects_unsupported(self):
        remote = FakeRSP(watch=False)
        with self.assertRaisesRegex(ObserverError, "unavailable"):
            ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols, mode="watch")

    def test_watch_stop_address_parsing(self):
        for reply in ("T05watch:80010000;", "T05thread:1;awatch:80010000;"):
            self.assertEqual(ww.watch_address(reply), TARGET)
        self.assertIsNone(ww.watch_address("S05"))

    def test_store_decoder(self):
        remote = FakeRSP()
        self.assertEqual(ww.store_range(remote, (36 << 26) | (3 << 16) | 0xFFFC), (TARGET - 4, 4))
        remote.registers[4] = 8
        self.assertEqual(ww.store_range(remote, (31 << 26) | (3 << 16) | (4 << 11) | (151 << 1)), (TARGET + 8, 4))
        self.assertIsNone(ww.store_range(remote, 0x60000000))

    def test_frame_runner_rejects_wrong_callers(self):
        remote = FakeRSP()
        remote.events = [(SEAM, 0x80001234, bytes(4), "S05"), (SEAM, CALLER, bytes(4), "S05")]
        runner = ww.FrameRunner(remote)
        runner.advance()
        self.assertTrue(runner.accepted())
        self.assertIn(f"z0,{SEAM:x},4", remote.calls)

    def test_pinned_dolphin_register_only_watch_trap(self):
        remote = FakeRSP()
        reply = f"T0540:{PC:08x};01:80002000;"
        remote.events = [(PC, 0, bytes.fromhex("3f800000"), reply),
                         (SEAM, CALLER, bytes.fromhex("3f800000"), "S05")]
        result = ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols)
        self.assertTrue(result["complete"])
        self.assertIn("presumed", result["writes"][0]["evidence"])

    def test_partial_events_survive_failure(self):
        remote = FakeRSP()
        remote.events = [(PC, 0, bytes.fromhex("3f800000"), f"T05watch:{TARGET:x};"),
                         (PC + 4, 0, bytes.fromhex("3f800000"), "S05")]
        result = {}
        with self.assertRaises(ObserverError):
            ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols, result=result)
        self.assertEqual(len(result["writes"]), 1)
        self.assertFalse(result["complete"])

    def test_wrong_watch_address_rejected(self):
        remote = FakeRSP()
        remote.events = [(PC, 0, bytes(4), "T05watch:80020000;")]
        with self.assertRaisesRegex(ObserverError, "unexpected watch address"):
            ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols)

    def test_candidate_breakpoint_is_not_reported_as_a_write(self):
        remote = FakeRSP()
        remote.events = [(PC, 0, bytes(4), "S05"), (PC + 4, 0, bytes(4), "S05"),
                         (SEAM, CALLER, bytes(4), "S05")]
        result = ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols, candidates=[PC])
        self.assertEqual(len(result["candidate_hits"]), 1)
        self.assertEqual(result["writes"], [])
        self.assertIn(f"z0,{PC:x},4", remote.calls)

    def test_candidate_store_is_observed_during_step(self):
        remote = FakeRSP()
        remote.instructions[PC] = (36 << 26) | (3 << 16)
        remote.events = [(PC, 0, bytes(4), "S05"),
                         (PC + 4, 0, bytes.fromhex("3f800000"), "S05"),
                         (SEAM, CALLER, bytes.fromhex("3f800000"), "S05")]
        result = ww.trace_window(remote, ww.FrameRunner(remote), TARGET, 4, self.symbols, candidates=[PC])
        self.assertEqual(result["writes"][0]["pc"], f"0x{PC:08x}")
        self.assertTrue(result["writes"][0]["changed"])

    def test_wall_timeout(self):
        remote = FakeRSP(watch=False)
        runner = ww.FrameRunner(remote, deadline=0)
        with self.assertRaisesRegex(ObserverError, "wall timeout"):
            ww.trace_window(remote, runner, TARGET, 4, self.symbols)

    def test_pointer_chain_resolution_and_scene_guard(self):
        class RAM:
            def __init__(self):
                self.words = {ww.ROOT: 0x80001000, 0x80001020: 0x80002000,
                    0x80001024: 0x80003000, 0x80003130: 6, 0x8000312C: 0x80004000,
                    0x80004010: 0x80004100, 0x80004154: 0x80004500, 0x80004514: 0x80005000,
                    0x80005584: 0x80006000, 0x80006138: 0x80008000}
            def read(self, address, size):
                if address == 0x80003000 and size == 64:
                    return b"Game" + bytes(60)
                return struct.pack(">I", self.words.get(address, 0))
        ram = RAM()
        self.assertEqual(ww.resolve_field(ram, "vx"), (0x80006160, 4))
        self.assertEqual(ww.resolve_field(ram, "x"), (0x80005014, 4))
        self.assertEqual(ww.resolve_field(ram, "sim_x"), (0x8000800C, 4))
        self.assertEqual(ww.resolve_field(ram, "rng_seed"), (0x80002034, 4))
        ram.words[0x80003130] = 5
        with self.assertRaisesRegex(ObserverError, "outside the initialized Game"):
            ww.resolve_field(ram, "vx")
        self.assertEqual(ww.resolve_field(ram, "rng_seed"), (0x80002034, 4))

    def test_cli_attach_writes_private_artifact_without_launching(self):
        remote = FakeRSP()
        remote.events = [(PC, 0, bytes.fromhex("3f800000"), f"T05watch:{TARGET:x};"),
                         (SEAM, CALLER, bytes.fromhex("3f800000"), "S05")]
        original_read = remote.read
        def read(address, size):
            return bytes.fromhex("480077c1") if address == CALLER - 4 else original_read(address, size)
        remote.read = read
        output = Path(self.temp.name) / "writer.json"
        args = ["--start-frame", "0", "--frame", "1", "--address", hex(TARGET),
                "--symbols", str(self.symbol_path), "--output", str(output)]
        with mock.patch.object(ww, "connect", return_value=mock.Mock()), mock.patch.object(ww, "RSP", return_value=remote), contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(ww.main(args), 0)
        report = json.loads(output.read_text())
        self.assertTrue(report["complete"])
        self.assertEqual(report["writer_counts"], {f"0x{PC:08x}": 1})
        self.assertEqual(report["last_write_event"]["function"], "writer")

    def test_recipes_and_unknown_field(self):
        self.assertEqual(ww.FIELDS["vx"], ("player", 0x160, 4))
        self.assertEqual(ww.FIELDS["mode"], ("actor", 0x6F8, 2))
        with self.assertRaisesRegex(ObserverError, "no verified address recipe"):
            ww.resolve_field(FakeRSP(), "state_ids")


if __name__ == "__main__":
    unittest.main()
