#!/usr/bin/env python3
"""Approved retail stage-request entry at the native file-loaded override seam."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import traceback

import dolphin_collect as collector
import kpad_script
from dolphin_menu_probe import menu_state

ENTRY = 0x804D62FC
EPILOGUE = 0x804D63DC
STAGE_STRING = 0x806694F8
STAGE_STRINGS = {'RedBlueExGalaxy': STAGE_STRING, 'YosshiHomeGalaxy': 0x806B88D8}
FUNCTIONS = {"after_file_select": (0x804D62C0, 0x130), "reset": (0x804C8D70, 0x120),
             "request": (0x804D8410, 0x48), "set_in_stage": (0x804B3F00, 0x50)}


def dol_function(path, address, size):
    data = Path(path).read_bytes()
    for index in range(18):
        offset = struct.unpack_from(">I", data, index * 4)[0]
        base = struct.unpack_from(">I", data, 0x48 + index * 4)[0]
        length = struct.unpack_from(">I", data, 0x90 + index * 4)[0]
        if base <= address and address + size <= base + length:
            return data[offset + address - base:offset + address - base + size]
    raise collector.ObserverError("stage-entry function outside DOL sections")


def select_stage(stage, scenario):
    if stage not in STAGE_STRINGS or type(scenario) is not int or scenario != 1:
        raise collector.ObserverError('entry supports only RedBlueExGalaxy or YosshiHomeGalaxy scenario 1')
    return STAGE_STRINGS[stage]


def validate_sync_target(stage, scenario, sync_stage, sync_scenario):
    select_stage(stage, scenario)
    if sync_stage != stage or sync_scenario != scenario:
        raise collector.ObserverError('sync stage/scenario must exactly match selected entry')


class StageEntry:
    def __init__(self, dol, stage='RedBlueExGalaxy', scenario=1):
        self.stage, self.scenario = stage, scenario
        self.stage_string = select_stage(stage, scenario)
        self.stage_bytes = stage.encode('ascii') + b'\0'
        if dol_function(dol, self.stage_string, len(self.stage_bytes)) != self.stage_bytes:
            raise collector.ObserverError('immutable DOL stage string mismatch')
        self.expected = {name: dol_function(dol, address, size) for name, (address, size) in FUNCTIONS.items()}
        self.receipt = dict(schema_version=1, status="not-reached", approval="port lead A thr_re66v9zuv4",
                            method="original guest functions via GDB registers; no instruction or progress-flag patch",
                            entry_tool_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                            stage=stage, placed_scenario=scenario, selected_scenario=scenario,
                            entry_pc=f"0x{ENTRY:08x}", return_epilogue=f"0x{EPILOGUE:08x}",
                            stage_string=f"0x{self.stage_string:08x}",
                            stage_string_sha256=hashlib.sha256(self.stage_bytes).hexdigest(),
                            selector_approval='bounded YosshiHomeGalaxy/scenario1 selector authorized by port lead thr_dqaxhnywic',
                            calls=[], destination_branch_skipped=False,
                            functions={name: dict(address=f"0x{FUNCTIONS[name][0]:08x}",
                                                 sha256=hashlib.sha256(code).hexdigest())
                                       for name, code in self.expected.items()})

    @staticmethod
    def register(remote, number, value):
        remote.ok(f"P{number:x}={value:08x}")
        if remote.reg(number) != value:
            raise collector.ObserverError("stage-entry register readback mismatch")

    @staticmethod
    def save_registers(remote):
        saved = {}
        for number in range(71):
            value = remote.command(f"p{number:x}")
            expected = 16 if 32 <= number < 64 else 8
            if len(value) != expected or any(ch not in "0123456789abcdefABCDEF" for ch in value):
                raise collector.ObserverError(f"cannot preserve PPC register {number}")
            saved[number] = value
        return saved

    @staticmethod
    def restore_registers(remote, saved):
        for number, value in saved.items():
            remote.ok(f"P{number:x}={value}")
        if StageEntry.save_registers(remote) != saved:
            raise collector.ObserverError("individual PPC register restore mismatch")

    def enter(self, remote, frame):
        if self.receipt["status"] != "not-reached":
            raise collector.ObserverError("stage entry is one-shot")
        self.receipt.update(status="incomplete", source_frame=frame)
        for name, (address, size) in FUNCTIONS.items():
            if remote.read(address, size) != self.expected[name]:
                raise collector.ObserverError(f"stage-entry retail signature mismatch: {name}")
        if remote.read(self.stage_string, len(self.stage_bytes)) != self.stage_bytes:
            raise collector.ObserverError("stage string mismatch")
        memory = collector.Memory(remote)
        game = memory.pointer(collector.ROOT)
        director = memory.pointer(game + 0xC)
        sequence = memory.pointer(director)
        in_game = memory.pointer(sequence + 0x10)
        in_stage = memory.pointer(in_game + 0x18)
        holder = memory.pointer(in_stage + 0x10)
        save_sequence = memory.pointer(director + 8)
        user = memory.pointer(save_sequence + 0xC)
        game_data = memory.pointer(user)
        if memory.string(game_data + 0x1C, 16) != "user1" or memory.boolean(user + 8):
            raise collector.ObserverError("stage entry requires successfully loaded file 1")
        stack = remote.reg(1)
        caller = memory.u32(stack + 20)
        if not 0x80251E50 <= caller < 0x80251F24:
            raise collector.ObserverError("stage entry did not originate in FileSelector::exeDemo")
        saved = self.save_registers(remote)
        self.receipt.update(guest_fpscr_entry_bits="0x" + saved[70], guest_msr_entry_bits="0x" + saved[65],
                            fp_observation_phase="file-loaded entry, before stage initialization; not a host FPCR/FZ equivalence claim")
        self.restore_registers(remote, saved)
        calls = [("reset", {3: holder}),
                 ("request", {3: self.stage_string, 4: self.scenario, 5: self.scenario, 6: 0}),
                 ("set_in_stage", {3: in_game, 4: self.stage_string})]
        for name, registers in calls:
            self.restore_registers(remote, saved)
            for number, value in registers.items():
                self.register(remote, number, value)
            self.register(remote, 67, ENTRY)
            self.register(remote, 64, FUNCTIONS[name][0])
            remote.stop("c")
            if remote.reg(64) != ENTRY or remote.reg(1) != stack:
                raise collector.ObserverError(f"stage-entry call did not return cleanly: {name}")
            self.receipt["calls"].append(dict(function=name, address=f"0x{FUNCTIONS[name][0]:08x}",
                                              registers={f"r{k}": f"0x{v:08x}" for k, v in registers.items()}))
        self.restore_registers(remote, saved)
        self.register(remote, 64, EPILOGUE)
        self.receipt.update(status="requested", loaded_file=1, destination_branch_skipped=True,
                            register_restore="individual p/P: GPR0..31, FPR PS0 lanes, PC/MSR/CR/LR/CTR/XER/FPSCR; original stack retained",
                            paired_single_ps1="not exposed by pinned Dolphin GDB stub; original callees retain ABI responsibility",
                            direct_guest_memory_writes=False)


def main():
    parser = argparse.ArgumentParser(description=__doc__, add_help=False)
    parser.add_argument("--entry-dol", default="~/MacioGalaxy2-gamedata/SB4E01-extracted/sys/main.dol")
    parser.add_argument("--pointer-diagnostics", action="store_true")
    parser.add_argument('--entry-stage', default='RedBlueExGalaxy')
    parser.add_argument('--entry-scenario', type=int, default=1)
    args, remaining = parser.parse_known_args()
    sync_parser = argparse.ArgumentParser(add_help=False)
    sync_parser.add_argument('--sync-stage')
    sync_parser.add_argument('--sync-scenario', type=int)
    sync_args, _ = sync_parser.parse_known_args(remaining)
    try:
        validate_sync_target(args.entry_stage, args.entry_scenario, sync_args.sync_stage, sync_args.sync_scenario)
    except collector.ObserverError as exc:
        parser.error(str(exc))
    if "--slot-held" not in remaining or "--input-script" not in remaining or "--dolphin" not in remaining:
        parser.error("use the shared lock externally and pass --slot-held, --dolphin and --input-script")
    output = collector.game_private_path(remaining[remaining.index("--output") + 1])
    if args.pointer_diagnostics:
        checked = collector.Memory.checked_pointer
        def checked_pointer(value, address, nullable=False, alignment=4):
            try:
                return checked(value, address, nullable, alignment)
            except collector.ObserverError:
                if output.is_dir():
                    (output / "pointer-error-context.json").write_text(json.dumps(
                        dict(address=address, value=value, stack=traceback.format_stack()), indent=2) + "\n")
                raise
        collector.Memory.checked_pointer = staticmethod(checked_pointer)
    entry = StageEntry(collector.game_private_path(args.entry_dol), args.entry_stage, args.entry_scenario)
    original = kpad_script.ScriptedInput
    class Input(original):
        def install(self, remote):
            super().install(remote)
            remote.ok(f"Z0,{ENTRY:x},4")
            self.breakpoints.add(ENTRY)

        def handle(self, remote, pc, frame):
            if pc != ENTRY:
                return super().handle(remote, pc, frame)
            self.deferred_stop = False
            entry.enter(remote, frame)
            remote.ok(f"z0,{ENTRY:x},4")
            self.breakpoints.discard(ENTRY)
            return True
    kpad_script.ScriptedInput = Input
    base = collector.Snapshot
    class Snapshot(base):
        def take(self, frame):
            row = super().take(frame)
            if row["stage"] == "FileSelect" and row["init_state"] == 6 and frame % 30 == 0:
                state = dict(frame=frame, **menu_state(self.m))
                with (output / "menu-state.jsonl").open("a") as stream:
                    stream.write(json.dumps(state) + "\n")
            return row
    collector.Snapshot = Snapshot
    sys.argv = [sys.argv[0], *remaining]
    try:
        return collector.main()
    finally:
        if output.is_dir():
            (output / "stage-entry.json").write_text(json.dumps(entry.receipt, indent=2) + "\n")
            manifest = output / "manifest.json"
            if manifest.exists():
                data = json.loads(manifest.read_text())
                data["stage_entry"] = entry.receipt
                manifest.write_text(json.dumps(data, indent=2) + "\n")


if __name__ == "__main__":
    sys.exit(main())
