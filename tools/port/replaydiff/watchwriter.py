#!/usr/bin/env python3
"""Replay to the first-divergence window and trace writes via Dolphin's RSP."""
from __future__ import annotations

import argparse
from bisect import bisect_right
from collections import Counter
import json
from pathlib import Path
import re
import socket
import struct
import subprocess
import sys
import time

from dolphin_collect import CALLER, Memory, ObserverError, REPO, ROOT, RSP, SEAM, private_path

# Wii offsets, deliberately separate from arm64 object layouts.
FIELDS = {}
for prefix, root, offset in (("", "actor", 0x14), ("sim_", "position", 0xC),
                              ("v", "player", 0x160), ("vel_after_", "player", 0x178),
                              ("actor_v", "actor", 0x38), ("front_", "player", 0x208),
                              ("actor_up_", "actor", 0x618), ("g", "player", 0x1D8),
                              ("actor_gravity_", "actor", 0x44)):
    for i, axis in enumerate("xyz"):
        FIELDS[prefix + axis] = (root, offset + i * 4, 4)
for name, offset in (("movement_lo", 0xC), ("movement_hi", 0x10), ("flags14_lo", 0x14),
                     ("flags14_hi", 0x18), ("draw", 0x1C), ("flags20", 0x20),
                     ("flags24_lo", 0x24), ("flags24_hi", 0x28), ("prev_draw", 0x34)):
    FIELDS[name] = ("player", offset, 4)
FIELDS.update(hp=("actor", 0x6B0, 4), dead=("actor", 0x70, 1), mode=("actor", 0x6F8, 2),
              rng_seed=("objects", 0x34, 4))
for i in range(3):
    FIELDS[f"state_flag{i}"] = ("player", 0xA90 + i, 1)


class Symbols:
    def __init__(self, path):
        self.entries = []
        pattern = re.compile(r"^(.*?) = [^:]+:0x([0-9a-fA-F]+);.*?type:function\b.*?\bsize:0x([0-9a-fA-F]+)")
        for line in Path(path).read_text().splitlines():
            match = pattern.search(line)
            if match:
                name, address, size = match.groups()
                self.entries.append((int(address, 16), int(size, 16), name))
        self.entries.sort()
        self.addresses = [entry[0] for entry in self.entries]
        if not self.entries:
            raise ObserverError("symbol file has no sized function symbols")

    def lookup(self, pc):
        index = bisect_right(self.addresses, pc) - 1
        if index >= 0:
            address, size, name = self.entries[index]
            if address <= pc < address + size:
                return dict(pc=f"0x{pc:08x}", function=name, offset=f"0x{pc - address:x}")
        return dict(pc=f"0x{pc:08x}", function=None, offset=None)


def resolve_field(remote, field):
    if field not in FIELDS:
        raise ObserverError(f"no verified address recipe for {field!r}; use --address and --size")
    root, offset, size = FIELDS[field]
    m = Memory(remote)
    game = m.pointer(ROOT)
    if root == "objects":
        return m.pointer(game + 0x20) + offset, size
    controller = m.pointer(game + 0x24)
    if m.u32(controller + 0x130) != 6 or m.string(controller) != "Game":
        raise ObserverError("field is invalid outside the initialized Game scene")
    scene = m.pointer(controller + 0x12C)
    holder = m.pointer(scene + 0x10)
    mario_holder = m.pointer(holder + 4 * 0x15)
    actor = m.pointer(mario_holder + 0x14)
    if root == "actor":
        return actor + offset, size
    player = m.pointer(actor + 0x584)
    if root == "player":
        return player + offset, size
    return m.pointer(player + 0x138) + offset, size


class FrameRunner:
    def __init__(self, remote, max_stops=10000, deadline=None):
        self.remote = remote
        self.max_stops = max_stops
        self.deadline = deadline
        self.breakpoint = False

    def check_deadline(self):
        if self.deadline is not None and time.monotonic() >= self.deadline:
            raise ObserverError("writer replay wall timeout reached")

    def install(self):
        if not self.breakpoint:
            self.remote.ok(f"Z0,{SEAM:x},4")
            self.breakpoint = True

    def remove(self):
        if self.breakpoint:
            self.remote.ok(f"z0,{SEAM:x},4")
            self.breakpoint = False

    def accepted(self):
        return self.remote.reg(64) == SEAM and self.remote.reg(67) == CALLER

    def step_off(self):
        self.check_deadline()
        self.remove()
        self.remote.stop("s")
        if self.remote.reg(64) != SEAM + 4:
            raise ObserverError("seam single-step failed")
        self.install()

    def advance(self):
        self.install()
        for _ in range(self.max_stops):
            self.check_deadline()
            self.remote.stop("c")
            if self.remote.reg(64) != SEAM:
                raise ObserverError("unexpected stop while replaying prefix")
            if self.accepted():
                return
            self.step_off()
        raise ObserverError("too many non-GameSystem seam callers")


def watch_address(reply):
    match = re.search(r"(?:^|;)(?:watch|awatch|rwatch):([0-9a-fA-F]+);?", reply[3:])
    return int(match[1], 16) if match else None


def overlaps(address, size, target, width):
    return address < target + width and target < address + size


def store_range(remote, instruction):
    """Recognize common Gekko stores for same-value events in step fallback.

    Return None for nonstores/unsupported stores. Byte-change detection still
    observes unsupported writes; paired/string/conditional stores are not proven.
    """
    op, ra, rb, rs = instruction >> 26, (instruction >> 16) & 31, (instruction >> 11) & 31, (instruction >> 21) & 31
    widths = {36: 4, 37: 4, 38: 1, 39: 1, 44: 2, 45: 2, 47: 4 * (32 - rs),
              52: 4, 53: 4, 54: 8, 55: 8}
    if op in widths:
        displacement = instruction & 0xFFFF
        if displacement & 0x8000:
            displacement -= 0x10000
        return (((remote.reg(ra) if ra else 0) + displacement) & 0xFFFFFFFF, widths[op])
    if op == 31:
        xo = (instruction >> 1) & 1023
        widths = {151: 4, 183: 4, 215: 1, 247: 1, 407: 2, 439: 2,
                  663: 4, 695: 4, 727: 8, 759: 8}
        if xo in widths:
            return (((remote.reg(ra) if ra else 0) + remote.reg(rb)) & 0xFFFFFFFF, widths[xo])
        if xo == 1014:
            ea = ((remote.reg(ra) if ra else 0) + remote.reg(rb)) & 0xFFFFFFFF
            return (ea & ~31, 32)
    return None


def trace_window(remote, runner, target, size, symbols, mode="auto", candidates=(), max_steps=2000000, max_stops=100000, result=None):
    """Begin at accepted frame F-1 seam; finish at accepted frame F seam."""
    if result is None:
        result = {}
    result.update(mode=None, writes=[], candidate_hits=[], complete=False, limitations=[])
    previous = remote.read(target, size)
    installed_watch = False
    installed_candidates = []

    def observed_step(pc, step):
        before = remote.read(target, size)
        instruction = struct.unpack(">I", remote.read(pc, 4))[0]
        store = store_range(remote, instruction)
        step()
        after = remote.read(target, size)
        if store and overlaps(*store, target, size) and remote.reg(64) != pc + 4:
            raise ObserverError("watched store did not complete its single step; qualify watchpoint timing or use --mode step")
        if before != after or (store and overlaps(*store, target, size)):
            result["writes"].append(dict(**symbols.lookup(pc), instruction=f"0x{instruction:08x}",
                before=before.hex(), after=after.hex(), changed=before != after,
                evidence="stepped byte change" if before != after else "decoded unconditional store"))
        return after

    try:
        if mode != "step":
            reply = remote.command(f"Z2,{target:x},{size:x}")
            if reply == "OK":
                installed_watch = True
                result["mode"] = "write-watchpoint"
                result["limitations"].append("PC is the raw trap PC. This Dolphin build's before/after-instruction watchpoint timing must be qualified before treating it as the writer instruction.")
                result["limitations"].append("Pinned Dolphin 2609a sends register-only T05 stops without watch addresses. In this isolated session, non-seam/non-candidate register-only traps are presumed memchecks; other stop forms fail closed.")
            else:
                result["watchpoint_reply"] = reply
                if mode == "watch":
                    raise ObserverError(f"write watchpoint unavailable: {reply!r}")
                result["limitations"].append(f"Dolphin rejected Z2 ({reply!r}); falling back to bounded instruction stepping.")
        if not installed_watch:
            result["mode"] = "single-step"
            result["limitations"].append("Reports byte changes plus recognized unconditional store instructions (including same-value stores). Same-value paired, string, or conditional stores may be missed. No claim of a complete writer list in this mode.")
            runner.remove()
            steps = 0
            while steps < max_steps:
                runner.check_deadline()
                pc = remote.reg(64)
                if steps and pc == SEAM and remote.reg(67) == CALLER:
                    result.update(complete=True, steps=steps, final_bytes=previous.hex())
                    return result
                before = previous
                instruction = struct.unpack(">I", remote.read(pc, 4))[0]
                store = store_range(remote, instruction)
                remote.stop("s")
                previous = remote.read(target, size)
                steps += 1
                if previous != before or (store and overlaps(*store, target, size)):
                    result["writes"].append(dict(**symbols.lookup(pc), instruction=f"0x{instruction:08x}",
                        before=before.hex(), after=previous.hex(), changed=before != previous,
                        evidence="byte change" if before != previous else "decoded unconditional store"))
                if pc in candidates:
                    result["candidate_hits"].append(dict(**symbols.lookup(pc), before=before.hex(), after=previous.hex()))
            raise ObserverError(f"single-step limit {max_steps} reached before next accepted game seam")
        for pc in candidates:
            if pc == SEAM:
                raise ObserverError("candidate breakpoint cannot be the game-loop seam")
            remote.ok(f"Z0,{pc:x},4")
            installed_candidates.append(pc)
        previous = observed_step(SEAM, runner.step_off)
        for _ in range(max_stops):
            runner.check_deadline()
            reply = remote.stop("c")
            pc = remote.reg(64)
            hit = watch_address(reply)
            generic = (pc != SEAM and pc not in installed_candidates and
                       re.fullmatch(r"T0540:" + f"{pc:08x}" + r";01:[0-9a-fA-F]{8};", reply) is not None)
            if hit is not None or generic:
                if generic:
                    hit = target
                if not overlaps(hit, 1, target, size):
                    raise ObserverError(f"unexpected watch address {hit:#x}")
                current = remote.read(target, size)
                result["writes"].append(dict(**symbols.lookup(pc), before=previous.hex(), after=current.hex(),
                    changed=current != previous, stop_reply=reply,
                    evidence="register-only SIGTRAP presumed write memcheck" if generic else "write watchpoint trap"))
                previous = current
            elif pc == SEAM:
                if runner.accepted():
                    final = remote.read(target, size)
                    if final != previous:
                        raise ObserverError("field changed without a reported CPU write trap")
                    result.update(complete=True, final_bytes=final.hex())
                    return result
                previous = observed_step(SEAM, runner.step_off)
            elif pc in installed_candidates:
                result["candidate_hits"].append(dict(**symbols.lookup(pc), bytes=remote.read(target, size).hex()))
                remote.ok(f"z0,{pc:x},4")
                installed_candidates.remove(pc)
                previous = observed_step(pc, lambda: remote.stop("s"))
                remote.ok(f"Z0,{pc:x},4")
                installed_candidates.append(pc)
            else:
                raise ObserverError(f"unclassified stop at {pc:#x}: {reply!r}; expected a watch address or pinned Dolphin register-only trap (or use --mode step)")
        raise ObserverError(f"stop limit {max_stops} reached; qualify watchpoint resume semantics or use --mode step")
    finally:
        if installed_watch:
            remote.ok(f"z2,{target:x},{size:x}")
        for pc in installed_candidates:
            remote.ok(f"z0,{pc:x},4")


def connect(endpoint, timeout, stop_timeout, process=None):
    deadline = time.monotonic() + timeout
    while True:
        stream = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        try:
            stream.connect(str(endpoint))
            stream.settimeout(stop_timeout)
            return stream
        except (FileNotFoundError, ConnectionRefusedError):
            stream.close()
            if process and process.poll() is not None:
                raise ObserverError(f"Dolphin exited before connection: {process.returncode}")
            if time.monotonic() >= deadline:
                raise ObserverError("GDB socket unavailable")
            time.sleep(0.2)
        except Exception:
            stream.close()
            raise


def run(args, report):
    endpoint = private_path(args.socket)
    output = private_path(args.output)
    if output.exists():
        raise ObserverError("refusing to overwrite an existing writer report")
    output.parent.mkdir(parents=True, exist_ok=True)
    process, stream, runner = None, None, None
    log = None
    try:
        if args.dolphin:
            if endpoint.exists():
                raise ObserverError("GDB socket already exists; confirm its owner before removing it")
            disc, user, movie = (private_path(p) for p in (args.disc, args.user, args.movie))
            with disc.open("rb") as source:
                if source.read(6) != b"SB4E01":
                    raise ObserverError("disc is not SB4E01")
            if not (user / "Config" / "Dolphin.ini").is_file() or not movie.is_file():
                raise ObserverError("need prepared isolated Dolphin profile and existing deterministic movie")
            command = [str(Path(args.dolphin).expanduser()), "-u", str(user), "-b", "-e", str(disc), "-m", str(movie)]
            report["command"] = command
            log = output.with_suffix(output.suffix + ".log").open("x")
            process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
        stream = connect(endpoint, args.connect_timeout, args.stop_timeout, process)
        remote = RSP(stream)
        remote.stop("?", startup=True)
        if remote.read(CALLER - 4, 4) != bytes.fromhex("480077c1"):
            raise ObserverError("SB4E01 seam call instruction does not match")
        runner = FrameRunner(remote, deadline=time.monotonic() + args.wall_timeout)
        if args.start_frame is not None:
            if not runner.accepted():
                raise ObserverError("--start-frame requires a stop at the accepted GameSystem seam")
            current = args.start_frame
            report["prefix_origin"] = f"caller-asserted existing seam frame {current}"
        else:
            report["prefix_origin"] = "boot stop asserted before the first game-loop seam"
            runner.advance()
            current = 0
        if current > args.frame - 1:
            raise ObserverError("start frame is after the requested F-1 seam")
        while current < args.frame - 1:
            runner.step_off()
            runner.advance()
            current += 1
            if current % 60 == 0:
                print(f"Replayed to game-loop frame {current}", file=sys.stderr, flush=True)
        address, size = (args.address, args.size or 4) if args.address is not None else resolve_field(remote, args.field)
        if args.size is not None:
            size = args.size
        before = remote.read(address, size)
        report.update(address=f"0x{address:08x}", size=size, before_frame=args.frame - 1, initial_bytes=before.hex())
        trace_window(remote, runner, address, size, Symbols(args.symbols), args.mode,
                     args.candidate, args.max_steps, args.max_stops, result=report)
        if args.address is None and resolve_field(remote, args.field)[0] != address:
            raise ObserverError("field object moved/changed during the window; write addresses are no longer comparable")
        counts = Counter(event["pc"] for event in report["writes"])
        report["writer_counts"] = dict(counts)
        report["last_write_event"] = report["writes"][-1] if report["writes"] else None
        report["status"] = "captured; build-specific trap timing qualification required" if report["mode"] == "write-watchpoint" else "captured with single-step limitations"
    except Exception as exc:
        report.update(status="invalid/incomplete", complete=False, error=f"{type(exc).__name__}: {exc}")
        raise
    finally:
        if runner:
            try:
                runner.remove()
            except Exception as exc:
                report.update(status="invalid/incomplete", complete=False, cleanup_error=str(exc))
        if stream:
            stream.close()
        if process:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
        if log:
            log.close()
        output.write_text(json.dumps(report, indent=2) + "\n")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--diff", type=Path, help="tracediff JSON report supplies first-divergence frame/field")
    parser.add_argument("--frame", type=int, help="first diverging completed loop F (F >= 1)")
    target = parser.add_mutually_exclusive_group()
    target.add_argument("--field", help="CSV field with Wii pointer recipe, e.g. vx")
    target.add_argument("--address", type=lambda v: int(v, 0), help="guest address resolved for this replay")
    parser.add_argument("--size", type=int, choices=(1, 2, 4, 8))
    parser.add_argument("--socket", default="~/MacioGalaxy2-team/scratch-port/solc/gdb.sock")
    parser.add_argument("--output", required=True, help="new private JSON artifact outside repo")
    parser.add_argument("--symbols", type=Path, default=REPO / "config/SB4E01/symbols.txt")
    parser.add_argument("--mode", choices=("auto", "watch", "step"), default="auto")
    parser.add_argument("--candidate", type=lambda v: int(v, 0), action="append", default=[], help="candidate writer PC; execution hits are reported separately")
    origin = parser.add_mutually_exclusive_group()
    origin.add_argument("--initial-stop", action="store_true", help="assert attached stub is stopped at boot before first loop seam")
    origin.add_argument("--start-frame", type=int, help="assert attached accepted seam's already-counted frame")
    parser.add_argument("--dolphin", help="launch executable using a fresh isolated profile and existing movie")
    parser.add_argument("--user", default="~/MacioGalaxy2-team/scratch-port/solc/dolphin-user")
    parser.add_argument("--disc", default="~/MacioGalaxy2-team/scratch-port/game-data/SB4E01.iso")
    parser.add_argument("--movie")
    parser.add_argument("--connect-timeout", type=float, default=60)
    parser.add_argument("--stop-timeout", type=float, default=120)
    parser.add_argument("--wall-timeout", type=float, default=900, help="replay/window wall budget after connection")
    parser.add_argument("--max-steps", type=int, default=2000000)
    parser.add_argument("--max-stops", type=int, default=100000)
    parser.add_argument("--lock", default="~/MacioGalaxy2-team/slots/native.lock")
    parser.add_argument("--slot-held", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args(argv)
    try:
        if args.diff:
            diff = json.loads(args.diff.read_text())
            first = diff.get("first_divergence")
            if not first:
                raise ObserverError("diff report has no first divergence")
            if args.frame is None:
                args.frame = first["frame"]
            if args.address is None and not args.field:
                args.field = next((d["column"].removesuffix("_bits") for d in first["columns"]
                                   if d["column"].removesuffix("_bits") in FIELDS), None)
                if not args.field:
                    raise ObserverError("no address recipe for divergent columns; specify --field/--address")
            if diff.get("actor_keys") and args.address is None:
                raise ObserverError("actor-table differences require a resolved --address; player recipes cannot identify arbitrary actors")
        if args.frame is None or args.frame < 1:
            raise ObserverError("--frame F >= 1 is required (frame 0 has no previous seam)")
        if args.address is None and args.field not in FIELDS:
            raise ObserverError("supply --field with a known recipe or --address")
        if args.start_frame is not None and (args.start_frame < 0 or args.start_frame >= args.frame):
            raise ObserverError("--start-frame must be between 0 and F-1")
        if args.dolphin and (not args.movie or args.start_frame is not None):
            raise ObserverError("launch requires --movie and cannot use --start-frame")
        if not args.dolphin and not args.initial_stop and args.start_frame is None:
            raise ObserverError("attach requires --initial-stop or --start-frame; frame numbering is never guessed")
        if args.max_steps < 1 or args.max_stops < 1 or any(not 0 < v < float("inf") for v in (args.connect_timeout, args.stop_timeout, args.wall_timeout)):
            raise ObserverError("limits/timeouts must be finite and positive")
        if any(pc & 3 or not 0x80000000 <= pc < 0x94000000 for pc in args.candidate):
            raise ObserverError("candidate PCs must be aligned guest code addresses")
        if args.dolphin and not args.slot_held:
            command = ["lockf", "-k", str(Path(args.lock).expanduser()), "nice", "-n", "10", sys.executable,
                       str(Path(__file__).resolve()), *(sys.argv[1:] if argv is None else argv), "--slot-held"]
            return subprocess.call(command)
        report = dict(status="incomplete", complete=False, frame=args.frame, field=args.field)
        run(args, report)
        print(json.dumps({k: report[k] for k in ("status", "frame", "address", "mode", "writer_counts", "last_write_event")}))
        return 0 if report["complete"] else 2
    except (ObserverError, OSError, ValueError, KeyError, TypeError) as exc:
        print(f"Writer trace failed: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
