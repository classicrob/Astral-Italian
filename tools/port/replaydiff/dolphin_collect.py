#!/usr/bin/env python3
"""Read SB4E01 state at the GameSystem waitForRetrace seam over Dolphin RSP."""
from __future__ import annotations

import argparse
import configparser
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import plistlib
import socket
import struct
import subprocess
import sys
import time

SEAM = 0x804BE280
CALLER = 0x804B6AC4
ROOT = 0x807D0DA4
SIGNATURES = {
    0x804B64C0: "480005b1",  # main calls GameSystem::frameLoop
    CALLER - 4: "480077c1",
    0x803B8568: "38850618",  # actual actor up offset
    0x803B9008: "8085058438840208",  # actor -> Mario -> front
    0x8003B404: "8084002080640034",  # GameSystem -> ObjHolder -> RNG
    0x803DCD7C: "93fd0114",  # MarioAnimator cached current BCK
}
REPO = Path(__file__).resolve().parents[3]
VECTORS = ("", "sim_", "v", "vel_after_", "actor_v", "front_", "actor_up_",
           "g", "actor_gravity_")
FLOATS = [prefix + axis for prefix in VECTORS for axis in "xyz"]
COLUMNS = ["frame", "scene_epoch", "scene_valid", "player_valid", "actor_count_valid",
           "rng_valid", "animation_valid", "scene", "stage", "scenario", "init_state"]
COLUMNS += [column for field in FLOATS for column in (field, field + "_bits")]
COLUMNS += ["movement_lo", "movement_hi", "flags14_lo", "flags14_hi", "draw", "flags20",
            "flags24_lo", "flags24_hi", "prev_draw", "state_flags3", "state_ids",
            "pending_state_id", "mode", "anim_name", "hp", "dead", "rng_seed",
            "actor_registered", "actor_living", "vi_count", "ticks"]


class ObserverError(RuntimeError):
    pass


class RSP:
    """ACK-mode RSP; never issue guest-memory or register writes."""

    def __init__(self, stream: socket.socket):
        self.stream = stream
        self.last = b""

    def byte(self):
        result = self.stream.recv(1)
        if not result:
            raise ObserverError("GDB connection closed")
        return result

    def send(self, payload):
        data = payload.encode("ascii")
        self.last = b"$" + data + b"#" + f"{sum(data) & 255:02x}".encode()
        self.stream.sendall(self.last)

    def receive(self):
        for _ in range(100):
            start = self.byte()
            if start == b"-":
                self.stream.sendall(self.last)
                continue
            if start == b"+":
                continue
            if start != b"$":
                raise ObserverError(f"unexpected RSP prefix {start!r}")
            data = bytearray()
            while True:
                char = self.byte()
                if char == b"#":
                    break
                data += char
                if len(data) > 65536:
                    raise ObserverError("oversized RSP reply")
            check = self.byte() + self.byte()
            if sum(data) & 255 != int(check, 16):
                self.stream.sendall(b"-")
                continue
            self.stream.sendall(b"+")
            # Dolphin's m/p/stop replies are ASCII hex, without binary escaping.
            result = data.decode("ascii")
            if result.startswith("O") and len(result) > 2 and result != "OK":
                continue
            return result
        raise ObserverError("too many RSP retries/console packets")

    def command(self, payload):
        self.send(payload)
        return self.receive()

    def ok(self, payload):
        reply = self.command(payload)
        if reply != "OK":
            raise ObserverError(f"{payload}: expected OK, got {reply!r}")

    def stop(self, payload, startup=False):
        reply = self.command(payload)
        accepted = ("T05", "S05", "T0f", "S0f") if startup else ("T05", "S05")
        if not reply.startswith(accepted):
            raise ObserverError(f"{payload}: expected SIGTRAP, got {reply!r}")
        return reply

    def reg(self, number):
        reply = self.command(f"p{number:x}")
        if len(reply) != 8:
            raise ObserverError(f"bad PPC register {number}: {reply!r}")
        return int(reply, 16)

    def read(self, address, size):
        if not 0 <= size <= 65536:
            raise ObserverError(f"invalid read size {size}")
        if not any(lo <= address and address + size <= hi for lo, hi in
                   ((0x80000000, 0x81800000), (0x90000000, 0x94000000))):
            raise ObserverError(f"unmapped guest read {address:#x}+{size:#x}")
        result = bytearray()
        for offset in range(0, size, 512):
            length = min(512, size - offset)
            reply = self.command(f"m{address + offset:x},{length:x}")
            if len(reply) != length * 2:
                code = reply if reply.startswith("E") and len(reply) <= 8 else "malformed/short reply"
                raise ObserverError(f"memory read {address + offset:#x}, wanted {length} bytes: {code} ({len(reply)} chars)")
            try:
                result += bytes.fromhex(reply)
            except ValueError as exc:
                raise ObserverError("nonhex memory response") from exc
        return bytes(result)


class Memory:
    def __init__(self, remote):
        self.remote = remote

    def u32(self, address):
        return struct.unpack(">I", self.remote.read(address, 4))[0]

    @staticmethod
    def checked_pointer(value, address, nullable=False, alignment=4):
        if value == 0 and nullable:
            return 0
        if value % alignment or not (0x80000000 <= value < 0x81800000 or
                             0x90000000 <= value < 0x94000000):
            raise ObserverError(f"bad guest pointer at {address:#x}: {value:#x}")
        return value

    def pointer(self, address, nullable=False, alignment=4):
        return self.checked_pointer(self.u32(address), address, nullable, alignment)

    def pointers(self, address, count):
        for first in range(0, count, 1024):
            length = min(1024, count - first)
            block = self.remote.read(address + first * 4, length * 4)
            for index, (value,) in enumerate(struct.iter_unpack(">I", block)):
                yield self.checked_pointer(value, address + (first + index) * 4)

    def string(self, address, limit=64):
        data = self.remote.read(address, limit)
        end = data.find(b"\0")
        if end < 0:
            raise ObserverError(f"unterminated guest string {address:#x}")
        return data[:end].decode("ascii", errors="strict")

    def boolean(self, address):
        value = self.remote.read(address, 1)[0]
        if value not in (0, 1):
            raise ObserverError(f"nonboolean at {address:#x}: {value}")
        return value

    def vector(self, row, prefix, address):
        data = self.remote.read(address, 12)
        for i, axis in enumerate("xyz"):
            raw = data[i * 4:i * 4 + 4]
            value = struct.unpack(">f", raw)[0]
            if not math.isfinite(value):
                raise ObserverError(f"nonfinite {prefix}{axis} at {address:#x}")
            row[prefix + axis] = format(value, ".9g")
            row[prefix + axis + "_bits"] = "0x" + raw.hex()


class Snapshot:
    def __init__(self, remote):
        self.m = Memory(remote)
        self.epoch = 0
        self.identity = None

    def take(self, frame):
        m = self.m
        row = dict.fromkeys(COLUMNS, "")
        row.update(frame=frame, scene_valid=0, rng_valid=0, player_valid=0,
                   actor_count_valid=0, animation_valid=0, observer_error="")
        try:
            game = m.pointer(ROOT, nullable=True)
            if not game:
                row["scene_epoch"] = self.epoch
                return row
            objects = m.pointer(game + 0x20, nullable=True)
            if objects:
                row.update(rng_seed=m.u32(objects + 0x34), rng_valid=1)
            controller = m.pointer(game + 0x24, nullable=True)
            if not controller:
                row["scene_epoch"] = self.epoch
                return row
            control = m.remote.read(controller, 0x134)
            for name, start in (("scene", 0), ("stage", 0x40)):
                data = control[start:start + 64]
                end = data.find(b"\0")
                if end < 0:
                    raise ObserverError(f"unterminated {name}")
                row[name] = data[:end].decode("ascii")
            row["scenario"] = struct.unpack_from(">i", control, 0x80)[0]
            row["init_state"] = struct.unpack_from(">I", control, 0x130)[0]
            if row["init_state"] > 6:
                raise ObserverError(f"invalid scene initialization enum {row['init_state']}")
            identity = (control[0x12C:0x130], row["scene"], row["stage"], row["scenario"])
            if identity != self.identity:
                self.epoch += 1
                self.identity = identity
            row.update(scene_valid=1, scene_epoch=self.epoch)
            if row["scene"] != "Game" or row["init_state"] != 6:
                return row
            scene = m.pointer(controller + 0x12C)
            holder = m.pointer(scene + 0x10)
            all_actors = m.pointer(holder + 4 * 0x39, nullable=True)
            if all_actors:
                capacity, count = struct.unpack(">ii", m.remote.read(all_actors + 0x14, 8))
                if not 0 <= count <= capacity <= 32768:
                    raise ObserverError(f"invalid actor count/capacity {count}/{capacity}")
                living = 0
                if count:
                    array = m.pointer(all_actors + 0x1C)
                    for actor in m.pointers(array, count):
                        living += 1 - m.boolean(actor + 0x70)
                row.update(actor_count_valid=1, actor_registered=count, actor_living=living)
            mario_holder = m.pointer(holder + 4 * 0x15, nullable=True)
            if not mario_holder:
                return row
            actor = m.pointer(mario_holder + 0x14, nullable=True)
            if not actor:
                return row
            player = m.pointer(actor + 0x584)
            position = m.pointer(player + 0x138)
            for prefix, address in (("", actor + 0x14), ("sim_", position + 0xC),
                                    ("v", player + 0x160), ("vel_after_", player + 0x178),
                                    ("actor_v", actor + 0x38), ("front_", player + 0x208),
                                    ("actor_up_", actor + 0x618), ("g", player + 0x1D8),
                                    ("actor_gravity_", actor + 0x44)):
                m.vector(row, prefix, address)
            row.update(hp=m.u32(actor + 0x6B0), dead=m.boolean(actor + 0x70),
                       mode=struct.unpack(">H", m.remote.read(actor + 0x6F8, 2))[0])
            for key, offset in (("movement_lo", 0xC), ("movement_hi", 0x10),
                                ("flags14_lo", 0x14), ("flags14_hi", 0x18),
                                ("draw", 0x1C), ("flags20", 0x20),
                                ("flags24_lo", 0x24), ("flags24_hi", 0x28),
                                ("prev_draw", 0x34)):
                row[key] = m.u32(player + offset)
            row["state_flags3"] = ";".join(str(m.boolean(player + 0xA90 + i)) for i in range(3))
            node = m.pointer(player + 0x980, nullable=True)
            seen, ids = set(), []
            while node:
                if node in seen or len(ids) >= 64:
                    raise ObserverError("cyclic/oversized Mario state stack")
                seen.add(node)
                ids.append(m.u32(node + 0xC))
                node = m.pointer(node + 8, nullable=True)
            row["state_ids"] = json.dumps(ids, separators=(",", ":"))
            pending = m.pointer(player + 0x984, nullable=True)
            if pending:
                row["pending_state_id"] = m.u32(pending + 0xC)
            animator = m.pointer(actor + 0x588, nullable=True)
            if animator:
                name = m.pointer(animator + 0x114, nullable=True, alignment=1)
                if name:
                    row["anim_name"] = m.string(name, 1024)
                    row["animation_valid"] = 1
            row["player_valid"] = 1
        except (ObserverError, UnicodeError, struct.error) as exc:
            row["observer_error"] = str(exc)
            row["player_valid"] = 0
        return row


def private_path(value):
    path = Path(value).expanduser().resolve()
    if path == REPO or REPO in path.parents:
        raise ObserverError(f"private artifact must be outside the repository: {path}")
    return path


def game_private_path(value):
    path = private_path(value)
    team = Path("~/MacioGalaxy2-team").expanduser().resolve()
    if path == team or team in path.parents:
        raise ObserverError(f"game data/profile must be outside the team upload tree: {path}")
    return path


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def prepare(args):
    user = game_private_path(args.user)
    if user.exists() and any(user.iterdir()):
        raise ObserverError("prepare requires an empty isolated user directory")
    config = user / "Config"
    config.mkdir(parents=True)
    endpoint = private_path(args.socket)
    if len(os.fsencode(endpoint)) >= 100:
        raise ObserverError("Unix socket path too long")
    (config / "Dolphin.ini").write_text(
        "[General]\nGDBSocket = " + str(endpoint) + "\nGDBPort = -1\n"
        "[Core]\nCPUThread = False\nCPUCore = " + str(args.cpu_core) + "\n"
        "EnableCheats = False\nEnableCustomRTC = True\nCustomRTCValue = 946684800\n"
        "OverrideRegionSettings = True\nSelectedLanguage = 1\n"
        "DSPHLE = True\nEmulationSpeed = 1.0\nWiiSDCard = False\n"
        "SIDevice0 = 0\nSIDevice1 = 0\nSIDevice2 = 0\nSIDevice3 = 0\n"
        "[Interface]\nConfirmStop = False\nDebugModeEnabled = True\n"
        "[Analytics]\nEnabled = False\nPermissionAsked = True\n"
        "[DSP]\nBackend = No Audio Output\n", encoding="utf-8")
    (config / "WiimoteNew.ini").write_text(
        "[Wiimote1]\nSource = 1\nExtension = Nunchuk\n"
        "[Wiimote2]\nSource = 0\n[Wiimote3]\nSource = 0\n[Wiimote4]\nSource = 0\n"
        "[BalanceBoard]\nSource = 0\n", encoding="utf-8")
    print(f"Prepared fresh isolated profile: {user}")


def collect(args):
    output = private_path(args.output)
    output.mkdir(parents=True, exist_ok=False)
    endpoint = private_path(args.socket)
    manifest = dict(schema_version=1, status="incomplete", frames=0,
                    requested_frames=args.frames, scene_spans=[],
                    frame_seam=f"{SEAM:08x}", caller_lr=f"{CALLER:08x}",
                    resume_method="temporary breakpoint" if args.gui else "single instruction",
                    input_method="Dolphin movie" if args.movie else "none; idle disconnected host controls")
    process = None
    stream = None
    installed = False
    step_installed = False
    remote = None
    inputs = None
    log = (output / "run.log").open("w", buffering=1)
    try:
        manifest.update(collector_sha256=sha256(__file__), python_version=sys.version.split()[0],
                        columns_sha256=hashlib.sha256(",".join(COLUMNS).encode()).hexdigest())
        if args.input_script:
            if args.movie or args.gui:
                raise ObserverError("scripted KPAD input requires batch collection without a movie")
            sys.modules.setdefault("dolphin_collect", sys.modules[__name__])
            from kpad_script import ScriptedInput
            script = Path(args.input_script).expanduser().resolve()
            inputs = ScriptedInput(script, args.frames, output / "inputs.jsonl", args.live_input_script)
            manifest.update(input_method="scripted filtered KPAD injection", input_script_sha256=sha256(script),
                            input_adapter_sha256=sha256(Path(__file__).with_name("kpad_script.py")),
                            input_contract_sha256=sha256(REPO / "native/tools/headless.py"))
            (output / "input-script.json").write_bytes(script.read_bytes())
        if args.dolphin:
            user = game_private_path(args.user)
            disc = game_private_path(args.disc)
            dolphin = Path(args.dolphin).expanduser().resolve()
            with disc.open("rb") as source:
                game_id = source.read(6)
            if game_id != b"SB4E01":
                raise ObserverError("disc header is not SB4E01")
            config = configparser.ConfigParser()
            config.read(user / "Config/Dolphin.ini")
            if (config.getboolean("Core", "CPUThread", fallback=True) or
                not config.getboolean("Core", "EnableCustomRTC", fallback=False) or
                config.getboolean("Core", "EnableCheats", fallback=True) or
                not config.getboolean("Interface", "DebugModeEnabled", fallback=False) or
                config.get("General", "GDBSocket", fallback="") != str(endpoint) or
                config.getint("General", "GDBPort", fallback=-1) != -1):
                raise ObserverError("profile must enable debugging, disable dual core/cheats, fix RTC, and use only the private GDB socket")
            manifest["cpu_core"] = config.getint("Core", "CPUCore")
            manifest["rtc_epoch"] = config.getint("Core", "CustomRTCValue")
            manifest["game_save_present_at_start"] = (user / "Wii/title/00010000/53423445/data").exists()
            manifest.update(disc_sha256=sha256(disc), dolphin_sha256=sha256(dolphin))
            bundle_info = dolphin.parent.parent / "Info.plist"
            if bundle_info.exists():
                with bundle_info.open("rb") as file:
                    info = plistlib.load(file)
                manifest.update(dolphin_version=info.get("CFBundleShortVersionString"),
                                dolphin_revision=info.get("CFBundleLongVersionString"))
            for name in ("Dolphin.ini", "WiimoteNew.ini", "GFX.ini"):
                path = user / "Config" / name
                if path.exists():
                    (output / name).write_bytes(path.read_bytes())
            command = [str(dolphin), "-u", str(user), "-e", str(disc)]
            if not args.gui:
                command.append("-b")
            if args.movie:
                movie = private_path(args.movie)
                manifest["movie_sha256"] = sha256(movie)
                command += ["-m", str(movie)]
            manifest["command"] = command
            if endpoint.exists():
                raise ObserverError("stale/existing GDB socket: remove only after confirming no emulator owns it")
            log.write("Shared emulator/build slot held; launching " + repr(command) + "\n")
            process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
        deadline = time.monotonic() + args.connect_timeout
        while True:
            stream = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            try:
                stream.connect(str(endpoint))
                break
            except (FileNotFoundError, ConnectionRefusedError):
                stream.close()
                if process and process.poll() is not None:
                    raise ObserverError(f"Dolphin exited before GDB connection: {process.returncode}")
                if time.monotonic() >= deadline:
                    raise ObserverError("GDB socket unavailable (or shared slot still occupied)")
                time.sleep(0.2)
        stream.settimeout(args.stop_timeout)
        stream.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
        remote = RSP(stream)
        remote.stop("?", startup=True)
        for address, encoded in SIGNATURES.items():
            expected = bytes.fromhex(encoded)
            if remote.read(address, len(expected)) != expected:
                raise ObserverError(f"SB4E01 instruction signature mismatch at {address:#x}")
        if inputs:
            inputs.install(remote)
        remote.ok(f"Z0,{SEAM:x},4")
        installed = True
        snapshot = Snapshot(remote)
        rejected = 0
        started = time.monotonic()
        with (output / "oracle.csv.partial").open("w", newline="") as file:
            writer = csv.DictWriter(file, COLUMNS, extrasaction="ignore")
            writer.writeheader()
            while manifest["frames"] < args.frames:
                if time.monotonic() - started > args.wall_timeout:
                    raise ObserverError("capture wall timeout")
                remote.stop("c")
                pc, lr = remote.reg(64), remote.reg(67)
                if inputs and inputs.handle(remote, pc, manifest["frames"]):
                    continue
                if pc != SEAM:
                    raise ObserverError(f"unexpected stop PC {pc:#x}, LR {lr:#x}")
                if lr == CALLER:
                    row = snapshot.take(manifest["frames"])
                    writer.writerow(row)
                    file.flush()
                    manifest["frames"] += 1
                    spans = manifest["scene_spans"]
                    context = [row[k] for k in ("scene", "stage", "scenario", "init_state")]
                    changed = not spans or spans[-1]["context"] != context
                    if changed:
                        spans.append(dict(first_frame=row["frame"], context=context))
                    if changed or manifest["frames"] % 60 == 0 or row["observer_error"]:
                        log.write(json.dumps({k: row[k] for k in ("frame", "scene", "stage", "scenario", "init_state", "player_valid", "observer_error")}) + "\n")
                    if row["observer_error"]:
                        raise ObserverError(row["observer_error"])
                    if inputs:
                        inputs.end_frame()
                else:
                    rejected += 1
                    manifest["rejected_stops"] = rejected
                    if rejected <= 10 or rejected % 1000 == 0:
                        log.write(f"Non-GameSystem seam caller LR={lr:08x}; rejected={rejected}\n")
                    if rejected > 10000:
                        raise ObserverError("too many non-GameSystem seam callers")
                if args.gui:
                    # Avoid the GUI's competing single-step state.
                    remote.ok(f"Z0,{SEAM + 4:x},4")
                    step_installed = True
                remote.ok(f"z0,{SEAM:x},4")
                installed = False
                remote.stop("c" if args.gui else "s")
                next_pc = remote.reg(64)
                if next_pc != SEAM + 4:
                    raise ObserverError(f"step breakpoint expected {SEAM + 4:#x}, got {next_pc:#x}")
                if step_installed:
                    remote.ok(f"z0,{SEAM + 4:x},4")
                    step_installed = False
                remote.ok(f"Z0,{SEAM:x},4")
                installed = True
        if not any(span["context"][0] for span in manifest["scene_spans"]):
            raise ObserverError("capture contains no initialized scene-control strings")
        (output / "oracle.csv.partial").rename(output / "oracle.csv")
        manifest.update(status="captured; repeat qualification required", rejected_stops=rejected)
    except Exception as exc:
        manifest["error"] = f"{type(exc).__name__}: {exc}"
        log.write(manifest["error"] + "\n")
        raise
    finally:
        if inputs:
            manifest["input_reads"] = inputs.reads
            manifest["live_input_revisions"] = inputs.revisions
            (output / "input-script-final.json").write_text(json.dumps(inputs.script, indent=2) + "\n")
            inputs.close(remote, manifest["status"] == "captured; repeat qualification required")
        if remote and step_installed:
            try:
                remote.ok(f"z0,{SEAM + 4:x},4")
            except Exception:
                pass
        if remote and installed:
            try:
                remote.ok(f"z0,{SEAM:x},4")
            except Exception:
                pass
        if stream:
            stream.close()
        if process:
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
                log.write("Dolphin required SIGKILL after SIGTERM timeout.\n")
            if stream and endpoint.is_socket():
                endpoint.unlink()
        (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
        log.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    fresh = sub.add_parser("prepare", help="write a fresh isolated deterministic profile")
    fresh.add_argument("--cpu-core", type=int, choices=(0, 1, 4), default=0,
                       help="0 interpreter, 1 x64 JIT, 4 arm64 JIT (qualify separately)")
    capture = sub.add_parser("collect", help="capture stopped guest state; artifacts must be outside repo")
    for cmd in (fresh, capture):
        cmd.add_argument("--user", default="~/MacioGalaxy2-gamedata/dolphin-user")
        cmd.add_argument("--socket", default="~/MacioGalaxy2-team/scratch-port/solc/gdb.sock")
    capture.add_argument("--dolphin", help="executable; omit to attach to an already locked isolated run")
    capture.add_argument("--disc", default="~/MacioGalaxy2-gamedata/SB4E01.iso")
    capture.add_argument("--movie")
    capture.add_argument("--input-script", help="explicit filtered KPAD script; writes SDK output buffers/register r3")
    capture.add_argument("--live-input-script", action="store_true", help="exploration only: reload future-only script edits")
    capture.add_argument("--gui", action="store_true", help="show the main UI for TAS recording/navigation")
    capture.add_argument("--output", required=True)
    capture.add_argument("--frames", type=int, default=600)
    capture.add_argument("--connect-timeout", type=float, default=600)
    capture.add_argument("--stop-timeout", type=float, default=120)
    capture.add_argument("--wall-timeout", type=float, default=900)
    capture.add_argument("--lock", default="~/MacioGalaxy2-team/slots/native.lock")
    capture.add_argument("--slot-held", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.action == "collect" and args.frames < 1:
        parser.error("--frames must be positive")
    if args.action == "collect" and args.dolphin and not args.slot_held:
        command = ["lockf", "-k", str(Path(args.lock).expanduser()), "nice", "-n", "10",
                   sys.executable, str(Path(__file__).resolve()), *sys.argv[1:], "--slot-held"]
        print("Waiting for shared emulator/build slot", flush=True)
        return subprocess.call(command)
    try:
        (prepare if args.action == "prepare" else collect)(args)
    except (ObserverError, OSError, ValueError) as exc:
        print(str(exc), file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
