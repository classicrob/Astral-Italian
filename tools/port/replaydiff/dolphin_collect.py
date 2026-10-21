#!/usr/bin/env python3
"""Read SB4E01 state at the GameSystem waitForRetrace seam over Dolphin RSP."""
from __future__ import annotations

import argparse
from collections import Counter
import configparser
import csv
import hashlib
import json
import math
import os
from pathlib import Path
import plistlib
import signal
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
# Additional original-DOL layout guards, read only when --actors is requested.
ACTOR_SIGNATURES = {
    0x80339760: "908300044e800020",  # NameObj::setName: pointer at +4.
    0x80339770: "3863000c4bd2e24c",  # NameObj::setLinkedInfo: member at +0xC.
    0x80067950: "3800ffffb0030000b0030002b00300044e800020",  # signed -1 defaults.
    0x80067A90: ("a8030000388000002c00000041800020a80300022c00000041800014"
                 "a80300042c00000041800008388000017c8323784e800020"),
    0x8000F568: ("c004003490010014c0240014d0010008c0440018c064001c"
                 "c0840020c0a40024c0c40028c0e4002cc1040030"),  # actor pos/rot/scale.
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
ACTOR_CONTEXT = ["frame", "scene_epoch", "scene", "stage", "scenario", "init_state"]
ACTOR_FLOATS = [prefix + axis for prefix in ("", "rot_", "scale_") for axis in "xyz"]
ACTOR_COLUMNS = ["schema_version", *ACTOR_CONTEXT, "producer_handle", "actor_id",
                 "identity_kind", "identity_status", "name_valid", "name_hex",
                 "link_valid", "link_id", "link_zone", "link_context", "dead"]
ACTOR_COLUMNS += [column for field in ACTOR_FLOATS for column in (field, field + "_bits")]
ACTOR_FRAME_COLUMNS = ["schema_version", *ACTOR_CONTEXT, "ready", "registered", "living",
                       "emitted", "identity_unresolved", "identity_ambiguous"]


class ObserverError(RuntimeError):
    pass


class RSP:
    """Buffered ACK-mode transport for Dolphin PPC RSP packets."""

    def __init__(self, stream: socket.socket):
        self.stream = stream
        self.last = b""
        self.buffer = b""
        self.cursor = 0

    def byte(self):
        if self.cursor == len(self.buffer):
            self.buffer = self.stream.recv(65536)
            self.cursor = 0
            if not self.buffer:
                raise ObserverError("GDB connection closed")
        result = self.buffer[self.cursor:self.cursor + 1]
        self.cursor += 1
        return result

    def send(self, payload):
        data = payload.encode("ascii")
        self.last = b"$" + data + b"#" + f"{sum(data) & 255:02x}".encode()
        self.stream.sendall(self.last)

    def until(self, delimiter, limit):
        parts, size = [], 0
        while True:
            if self.cursor == len(self.buffer):
                self.buffer = self.stream.recv(65536)
                self.cursor = 0
                if not self.buffer:
                    raise ObserverError("GDB connection closed")
            end = self.buffer.find(delimiter, self.cursor)
            stop = end if end >= 0 else len(self.buffer)
            part = self.buffer[self.cursor:stop]
            size += len(part)
            if size > limit:
                raise ObserverError("oversized RSP reply")
            parts.append(part)
            self.cursor = stop + (end >= 0)
            if end >= 0:
                return b"".join(parts)

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
            data = self.until(b"#", 65536)
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
        for offset in range(0, size, 4096):
            length = min(4096, size - offset)
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
    def __init__(self, remote, cache_pages=False):
        self.remote = remote
        self.cache_pages = cache_pages
        self.pages = {}

    def read(self, address, size):
        if not self.cache_pages:
            return self.remote.read(address, size)
        result = bytearray()
        while size:
            page = address & ~4095
            if page not in self.pages:
                self.pages[page] = self.remote.read(page, 4096)
            offset = address - page
            length = min(size, 4096 - offset)
            result += self.pages[page][offset:offset + length]
            address += length
            size -= length
        return bytes(result)

    def u32(self, address):
        return struct.unpack(">I", self.read(address, 4))[0]

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
            block = self.read(address + first * 4, length * 4)
            for index, (value,) in enumerate(struct.iter_unpack(">I", block)):
                yield self.checked_pointer(value, address + (first + index) * 4)

    def string(self, address, limit=64):
        return self.string_bytes(address, limit).decode("ascii", errors="strict")

    def string_bytes(self, address, limit=64):
        data = self.read(address, limit)
        end = data.find(b"\0")
        if end < 0:
            raise ObserverError(f"unterminated guest string {address:#x}")
        return data[:end]

    def boolean(self, address):
        value = self.read(address, 1)[0]
        if value not in (0, 1):
            raise ObserverError(f"nonboolean at {address:#x}: {value}")
        return value

    def vector(self, row, prefix, address):
        data = self.read(address, 12)
        for i, axis in enumerate("xyz"):
            raw = data[i * 4:i * 4 + 4]
            value = struct.unpack(">f", raw)[0]
            if not math.isfinite(value):
                raise ObserverError(f"nonfinite {prefix}{axis} at {address:#x}")
            row[prefix + axis] = format(value, ".9g")
            row[prefix + axis + "_bits"] = "0x" + raw.hex()


class ActorCensus:
    """Sampled continuity and candidate source keys, never address-based cross-run IDs."""

    def __init__(self):
        self.epoch = None
        self.next_handle = 1
        self.active = {}
        self.key_owners = {}

    def take(self, memory, row, members, player_actor):
        if row["scene_epoch"] != self.epoch:
            self.epoch = row["scene_epoch"]
            self.active.clear()
            self.key_owners.clear()
        if not actor_ready(row):
            self.active.clear()
            return []
        if len({address for address, _ in members}) != len(members):
            raise ObserverError("duplicate registered actor pointer")
        result, candidates, current = [], [], {}
        for address, dead in members:
            actor = dict(schema_version=1, **{key: row[key] for key in ACTOR_CONTEXT}, dead=dead)
            name = memory.pointer(address + 4, nullable=True, alignment=1)
            actor.update(name_valid=int(bool(name)),
                         name_hex=memory.string_bytes(name, 1024).hex() if name else "")
            follow, zone, context = struct.unpack(">hhh", memory.read(address + 0xC, 6))
            valid = min(follow, zone, context) >= 0  # JMapLinkInfo::isValid, signed s16.
            actor.update(link_valid=int(valid), link_id=follow, link_zone=zone, link_context=context)
            for prefix, offset in (("", 0x14), ("rot_", 0x20), ("scale_", 0x2C)):
                memory.vector(actor, prefix, address + offset)
            if address == player_actor:
                kind, key = "player", "player"
            elif valid and name:
                kind, key = "link", f"link:{zone}:{context}:{follow}:{actor['name_hex']}"
            else:
                kind, key = "unresolved", ""
            previous = self.active.get(address)
            known_handle = previous is not None
            if previous is None:
                previous = dict(handle=self.next_handle, key="", hold="")
                self.next_handle += 1
            state = previous.copy()
            if known_handle and key != state["key"]:
                state["hold"] = "key-change"
            state["key"] = key
            current[address] = state
            actor.update(producer_handle=state["handle"], actor_id="", identity_kind=kind)
            result.append(actor)
            candidates.append((key, state))
        counts = Counter(key for key, _ in candidates if key)
        # Retain every observed owner, including ambiguous candidates. Losing an
        # ambiguous member later cannot upgrade the remaining member's identity.
        for key, state in candidates:
            if key:
                self.key_owners.setdefault(key, set()).add(state["handle"])
        for actor, (key, state) in zip(result, candidates):
            if key and counts[key] > 1:
                status = "ambiguous"
            else:
                if key and self.key_owners[key] != {state["handle"]}:
                    state["hold"] = "key-reuse"
                status = state["hold"] or ("source-key" if key else "unresolved")
                if key and status == "source-key":
                    actor["actor_id"] = key
            actor["identity_status"] = status
        self.active = current
        return result


def actor_ready(row):
    return (row["scene_valid"] == 1 and row["scene"] == "Game" and
            row["init_state"] == 6 and row["actor_count_valid"] == 1)


class ActorSidecars:
    """One complete census receipt per emitted primary frame, including zero actors."""

    def __init__(self, output):
        self.paths = [output / ("oracle.csv" + suffix) for suffix in (".actors.csv", ".actor-frames.csv")]
        self.files = []
        self.frames = self.rows = self.holds = 0
        self.complete = False
        try:
            for path in self.paths:
                self.files.append(Path(str(path) + ".partial").open("x", newline=""))
            self.actors = csv.DictWriter(self.files[0], ACTOR_COLUMNS)
            self.census = csv.DictWriter(self.files[1], ACTOR_FRAME_COLUMNS)
            self.actors.writeheader()
            self.census.writeheader()
        except Exception:
            for file in self.files:
                file.close()
            raise

    def write(self, row, actors):
        if row["frame"] != self.frames:
            raise ObserverError("actor census frame is not contiguous with primary trace")
        ready = actor_ready(row)
        if (not ready and actors) or (ready and len(actors) != row["actor_registered"]):
            raise ObserverError("actor census does not cover every registered entry")
        if ready and sum(1 - actor["dead"] for actor in actors) != row["actor_living"]:
            raise ObserverError("actor census living count disagrees with primary trace")
        context = {key: row[key] for key in ACTOR_CONTEXT}
        unresolved = ambiguous = 0
        for actor in actors:
            if any(actor[key] != value for key, value in context.items()):
                raise ObserverError("actor row context disagrees with primary trace")
            ambiguous += actor["identity_status"] == "ambiguous"
            unresolved += actor["identity_status"] not in ("source-key", "ambiguous")
        # Commit the receipt last: a partial actor batch never claims complete frame coverage.
        self.actors.writerows(actors)
        self.files[0].flush()
        self.census.writerow(dict(schema_version=1, **context, ready=int(ready),
                                 registered=row["actor_registered"] if ready else "",
                                 living=row["actor_living"] if ready else "", emitted=len(actors),
                                 identity_unresolved=unresolved, identity_ambiguous=ambiguous))
        self.files[1].flush()
        self.frames += 1
        self.rows += len(actors)
        self.holds += unresolved + ambiguous

    def close(self, complete):
        for file in self.files:
            file.close()
        if complete:
            renamed = []
            try:
                for path in self.paths:
                    Path(str(path) + ".partial").rename(path)
                    renamed.append(path)
            except OSError:
                # Preserve diagnostic partials when only one rename succeeded.
                for path in renamed:
                    path.rename(Path(str(path) + ".partial"))
                raise
            self.complete = True

    def manifest(self):
        return dict(schema_version=1, status="captured" if self.complete else "incomplete",
                    frames=self.frames, actor_rows=self.rows, identity_holds=self.holds,
                    qualification="Candidate keys only; sampled pointer continuity is not a lifetime proof",
                    files=[dict(path=path.name, sha256=sha256(path)) for path in self.paths] if self.complete else [])


class Snapshot:
    def __init__(self, remote, actors=False):
        self.m = Memory(remote, cache_pages=True)
        self.epoch = 0
        self.identity = None
        self.actor_census = ActorCensus() if actors else None
        self.actor_rows = []

    def take(self, frame):
        self.members = []
        self.player_actor = 0
        row = self._take(frame)
        self.actor_rows = []
        if self.actor_census and not row["observer_error"]:
            try:
                self.actor_rows = self.actor_census.take(self.m, row, self.members, self.player_actor)
            except (ObserverError, UnicodeError, struct.error) as exc:
                row["observer_error"] = str(exc)
        return row

    def _take(self, frame):
        m = self.m
        m.pages.clear()
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
            control = m.read(controller, 0x134)
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
                capacity, count = struct.unpack(">ii", m.read(all_actors + 0x14, 8))
                if not 0 <= count <= capacity <= 32768:
                    raise ObserverError(f"invalid actor count/capacity {count}/{capacity}")
                living = 0
                if count:
                    array = m.pointer(all_actors + 0x1C)
                    for actor in m.pointers(array, count):
                        dead = m.boolean(actor + 0x70)
                        living += 1 - dead
                        if self.actor_census:
                            self.members.append((actor, dead))
                row.update(actor_count_valid=1, actor_registered=count, actor_living=living)
            mario_holder = m.pointer(holder + 4 * 0x15, nullable=True)
            if not mario_holder:
                return row
            actor = m.pointer(mario_holder + 0x14, nullable=True)
            self.player_actor = actor
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
                       mode=struct.unpack(">H", m.read(actor + 0x6F8, 2))[0])
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
            # +0x984 is noticed-state callback scratch, not a live pointer at this seam.
            row["pending_state_id"] = ""
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


class StageSync:
    """Choose the first ready stage seam without changing observed guest state."""

    def __init__(self, stage, scenario):
        self.stage = stage
        self.scenario = scenario
        self.origin = None

    def select(self, row):
        if self.origin is None:
            if not (row["scene_valid"] == 1 and row["scene"] == "Game" and
                    row["init_state"] == 6 and row["stage"] == self.stage and
                    row["scenario"] == self.scenario):
                return None
            if row["rng_valid"] != 1:
                raise ObserverError("first stage-sync seam has no valid RNG seed")
            self.origin = dict(schema_version=1, phase="post-endFrame", stage=self.stage,
                               scenario=self.scenario, source_frame=row["frame"],
                               source_scene_epoch=row["scene_epoch"], source_ticks=row["ticks"] or None,
                               source_vi_count=row["vi_count"] or None, rng_valid=True,
                               rng_seed=row["rng_seed"], rng_forced=False)
        result = row.copy()
        result["frame"] -= self.origin["source_frame"]
        result["scene_epoch"] -= self.origin["source_scene_epoch"]
        return result


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


def input_export_receipt(output, read_count):
    """Bind the actual closed exporter file, never an intent script or a fixture."""
    path = output / "inputs.jsonl"
    digest = hashlib.sha256()
    rows = size = 0
    with path.open("rb") as file:
        for line in file:
            if not line.endswith(b"\n") or not line.strip():
                raise ObserverError("input export has an incomplete/empty JSONL record")
            digest.update(line)
            size += len(line)
            rows += 1
    if rows != read_count:
        raise ObserverError("input export record count disagrees with observed reads")
    return dict(schema_version=1, status="captured", path=path.name,
                sha256=digest.hexdigest(), bytes=size, read_count=rows)


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
        "EnableCheats = False\nEnableSaveStates = True\nEnableCustomRTC = True\nCustomRTCValue = 946684800\n"
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
                    requested_frames=args.frames, source_frames=0, scene_spans=[],
                    frame_seam=f"{SEAM:08x}", caller_lr=f"{CALLER:08x}",
                    resume_method="temporary breakpoint" if args.gui or args.input_script else "single instruction",
                    input_method="Dolphin movie" if args.movie else "none; idle disconnected host controls")
    manifest['producer_host'] = socket.gethostname()
    manifest['collection_started_utc'] = time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())
    manifest["field_availability"] = {"pending_state_id": "unavailable (callback-context only)"}
    manifest["observer_contract_incomplete"] = True
    manifest['runtime_budget'] = dict(default_seconds=480, effective_seconds=args.wall_timeout,
                                    extended=bool(getattr(args, 'extended_wall_budget', False)))
    if getattr(args, 'extended_wall_budget', False):
        manifest['runtime_budget']['approval'] = ('team lead thr_qzkz8x9vp8 via port lead thr_dqaxhnywic, '
                                                  '2026-10-20: ONE continuous human-tape run <=1500s; no stitching')
    if getattr(args, 'yoshi_replay_budget', False):
        manifest['runtime_budget'].update(
            extended=True, scope='Yoshi continuous replay only',
            approval='port lead thr_brg5rhzvas: unattended full Yoshi replay on M1; no stitching',
            engineering_cap_seconds=10800)
    process = None
    collection_started = time.monotonic()
    sampling_started = None
    suffix_started = None
    stream = None
    installed = False
    step_installed = False
    remote = None
    inputs = None
    actor_output = None
    if getattr(args, "actors", False):
        manifest["actors"] = dict(schema_version=1, status="incomplete")
    handlers = {}
    sync = StageSync(args.sync_stage, args.sync_scenario) if getattr(args, "sync_stage", None) else None
    input_frames = args.frames
    inputs_installed = False
    suffix = None
    raw_suffix = None
    def interrupted(number, _frame):
        raise ObserverError(f"capture interrupted by {signal.Signals(number).name}")
    log = (output / "run.log").open("w", buffering=1)
    try:
        for number in (signal.SIGTERM, signal.SIGINT, signal.SIGALRM):
            handlers[number] = signal.signal(number, interrupted)
        signal.setitimer(signal.ITIMER_REAL, max(540, args.wall_timeout + 60) if (getattr(args, 'extended_wall_budget', False) or getattr(args, 'yoshi_replay_budget', False)) else 540)
        manifest.update(collector_sha256=sha256(__file__), python_version=sys.version.split()[0],
                        columns_sha256=hashlib.sha256(",".join(COLUMNS).encode()).hexdigest())
        if args.input_script:
            if args.movie:
                raise ObserverError("scripted KPAD input cannot be combined with a movie")
            sys.modules.setdefault("dolphin_collect", sys.modules[__name__])
            from kpad_script import ScriptedInput, load_script
            script = Path(args.input_script).expanduser().resolve()
            if sync:
                input_frames = json.loads(script.read_text())["frames"]
                if type(input_frames) is not int or input_frames < 1:
                    raise ObserverError("input script must declare a positive source-frame count")
            if getattr(args, "sync_input_script", None):
                suffix_path = Path(args.sync_input_script).expanduser().resolve()
                suffix_frames = json.loads(suffix_path.read_text())["frames"]
                suffix = load_script(suffix_path, suffix_frames)
                if type(suffix_frames) is not int or suffix_frames < 1 or args.frames != suffix_frames + 1:
                    raise ObserverError("--frames must equal sync suffix frames + 1 for the anchor")
                manifest["sync_input_script_sha256"] = sha256(suffix_path)
                (output / "sync-input-script.json").write_bytes(suffix_path.read_bytes())
            if getattr(args, 'sync_input_records', None):
                from kpad_records import RawSuffix
                from kpad_script import contract
                raw_suffix = RawSuffix(game_private_path(args.sync_input_records), args.sync_input_records_sha256,
                                       args.frames - 1, output, contract, getattr(args, 'sync_observed_connection', False))
                manifest['raw_input_suffix'] = raw_suffix.receipt
            inputs = ScriptedInput(script, input_frames, output / "inputs.jsonl", args.live_input_script)
            manifest.update(input_method="scripted filtered KPAD injection", input_frames=input_frames,
                            input_script_sha256=sha256(script),
                            input_adapter_sha256=sha256(Path(__file__).with_name("kpad_script.py")),
                            input_contract_sha256=sha256(REPO / "native/tools/headless.py"))
            (output / "input-script.json").write_bytes(script.read_bytes())
        if getattr(args, "export_inputs", False):
            if args.input_script:
                raise ObserverError("--export-inputs is read-only and cannot be combined with --input-script")
            sys.modules.setdefault("dolphin_collect", sys.modules[__name__])
            from kpad_observe import ObservedInput
            inputs = ObservedInput(output / "inputs.jsonl")
            manifest.update(input_method="read-only original KPADReadEx and verified WPADProbe-equivalent snapshot",
                            input_adapter_sha256=sha256(Path(__file__).with_name("kpad_observe.py")),
                            input_contract_sha256=sha256(REPO / "native/tools/headless.py"))
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
            manifest["timing_policy"] = dict(
                tick_hz=60750000, tick_hz_basis="guest PPC time-base ticks: Dolphin Wii 729000000 / TIMER_RATIO 12; not measured",
                rate_numerator=None, rate_denominator=None, epoch_ticks=None,
                rate_basis="guest VI configuration; this collector does not observe VI registers",
                fpcr_fz=None, fp_policy="Gekko semantics under selected Dolphin CPU core; host FPCR not observed",
                audio_output="off" if config.get("DSP", "Backend", fallback="") == "No Audio Output" else "configured backend",
                audio_processing="Dolphin DSP HLE with virtual AI/DSP timing" if config.getboolean("Core", "DSPHLE", fallback=False) else "Dolphin DSP LLE",
                dsp_hle=config.getboolean("Core", "DSPHLE", fallback=False))
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
            if getattr(args, "save_state", None):
                state = game_private_path(args.save_state)
                manifest["save_state_sha256"] = sha256(state)
                manifest["input_anchor"] = "KPAD injection starts after first restored GameSystem seam"
                command += ["-s", str(state)]
            if args.movie:
                movie = private_path(args.movie)
                manifest["movie_sha256"] = sha256(movie)
                command += ["-m", str(movie)]
            manifest["command"] = command
            if endpoint.exists():
                raise ObserverError("stale/existing GDB socket: remove only after confirming no emulator owns it")
            log.write("Shared emulator/build slot held; launching " + repr(command) + "\n")
            process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
            manifest['ui_mode'] = 'visible'
            manifest.update(collector_pid=os.getpid(), dolphin_pid=process.pid)
        (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
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
        if getattr(args, "actors", False):
            for address, encoded in ACTOR_SIGNATURES.items():
                expected = bytes.fromhex(encoded)
                if remote.read(address, len(expected)) != expected:
                    raise ObserverError(f"SB4E01 actor layout signature mismatch at {address:#x}")
        if inputs and not getattr(args, "save_state", None):
            inputs.install(remote)
            inputs_installed = True
        remote.ok(f"Z0,{SEAM:x},4")
        installed = True
        snapshot = Snapshot(remote, actors=True) if getattr(args, "actors", False) else Snapshot(remote)
        if getattr(args, "actors", False):
            actor_output = ActorSidecars(output)
        rejected = 0
        started = time.monotonic()
        sampling_started = started
        stop_pending = False
        with (output / "oracle.csv.partial").open("w", newline="") as file:
            writer = csv.DictWriter(file, COLUMNS, extrasaction="ignore")
            writer.writeheader()
            while manifest["frames"] < args.frames:
                if time.monotonic() - started > args.wall_timeout:
                    raise ObserverError("capture wall timeout")
                if not stop_pending:
                    remote.stop("c")
                stop_pending = False
                pc, lr = remote.reg(64), remote.reg(67)
                if inputs_installed and inputs.handle(remote, pc, manifest["source_frames"]):
                    stop_pending = getattr(inputs, "deferred_stop", False)
                    continue
                if pc != SEAM:
                    raise ObserverError(f"unexpected stop PC {pc:#x}, LR {lr:#x}")
                if lr == CALLER:
                    source_row = snapshot.take(manifest["source_frames"])
                    if source_row["observer_error"]:
                        raise ObserverError(source_row["observer_error"])
                    row = sync.select(source_row) if sync else source_row
                    if inputs:
                        if raw_suffix:
                            inputs.end_frame(manifest['source_frames'])
                        else:
                            inputs.end_frame()
                        if not inputs_installed:
                            inputs.install(remote)
                            inputs_installed = True
                    manifest["source_frames"] += 1
                    if row is not None:
                        if sync and row["frame"] == 0:
                            suffix_started = time.monotonic()
                            manifest["sync"] = sync.origin
                            if suffix:
                                inputs.start_suffix(suffix, manifest["source_frames"])
                                input_frames = inputs.script["frames"]
                                manifest.update(input_frames=input_frames,
                                                sync_input_source_start=manifest["source_frames"])
                            if raw_suffix:
                                inputs.start_record_suffix(raw_suffix, manifest['source_frames'], remote)
                                input_frames = manifest['source_frames'] + raw_suffix.frames
                                manifest.update(sync_input_source_start=manifest['source_frames'], input_frames=input_frames)
                            (output / "oracle.csv.sync.json").write_text(json.dumps(sync.origin, indent=2) + "\n")
                        writer.writerow(row)
                        file.flush()
                        if actor_output:
                            actor_output.write(row, [dict(actor, frame=row["frame"],
                                                          scene_epoch=row["scene_epoch"])
                                                     for actor in snapshot.actor_rows])
                        manifest["frames"] += 1
                        spans = manifest["scene_spans"]
                        context = [row[k] for k in ("scene", "stage", "scenario", "init_state")]
                        changed = not spans or spans[-1]["context"] != context
                        if changed:
                            spans.append(dict(first_frame=row["frame"], context=context))
                    else:
                        changed = False
                    if changed or manifest["source_frames"] % 60 == 0:
                        log.write(json.dumps({k: source_row[k] for k in ("frame", "scene", "stage", "scenario", "init_state", "player_valid", "observer_error")}) + "\n")
                        (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
                    if args.input_script and manifest["source_frames"] >= input_frames and manifest["frames"] < args.frames:
                        raise ObserverError("full source input tape exhausted before requested trace completed")
                else:
                    rejected += 1
                    manifest["rejected_stops"] = rejected
                    if rejected <= 10 or rejected % 1000 == 0:
                        log.write(f"Non-GameSystem seam caller LR={lr:08x}; rejected={rejected}\n")
                    if rejected > 10000:
                        raise ObserverError("too many non-GameSystem seam callers")
                if args.gui or inputs:
                    # Avoid the GUI's competing single-step state.
                    remote.ok(f"Z0,{SEAM + 4:x},4")
                    step_installed = True
                remote.ok(f"z0,{SEAM:x},4")
                installed = False
                remote.stop("c" if args.gui or inputs else "s")
                next_pc = remote.reg(64)
                if next_pc != SEAM + 4:
                    if not inputs or next_pc not in inputs.breakpoints:
                        raise ObserverError(f"step breakpoint expected {SEAM + 4:#x}, got {next_pc:#x}")
                    stop_pending = True
                    manifest.setdefault("seam_resume_diversions", []).append(
                        dict(source_frame=manifest["source_frames"], stopped_pc=f"{next_pc:08x}"))
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
        stopped = time.monotonic()
        manifest["collection_elapsed_seconds"] = stopped - collection_started
        manifest["sampling_elapsed_seconds"] = stopped - sampling_started if sampling_started is not None else None
        manifest["suffix_elapsed_seconds"] = stopped - suffix_started if suffix_started is not None else None
        signal.setitimer(signal.ITIMER_REAL, 0)
        if stream:
            stream.settimeout(1)
        cleanup_error = None
        if inputs:
            try:
                manifest["input_export"] = dict(schema_version=1, status="incomplete", path="inputs.jsonl")
                manifest["input_reads"] = inputs.reads
                if hasattr(inputs, "probe_counts"):
                    manifest["connection_observation"] = dict(
                        method=inputs.connection_method,
                        probe_counts=inputs.probe_counts, maximum_age_source_loops=inputs.max_probe_age,
                        function_sha256=inputs.probe_sha256,
                        within_read_transitions=inputs.connection_transitions,
                        connection_phase="post-KPADReadEx; intra-call timing is not represented by the native tape")
                manifest["live_input_revisions"] = inputs.revisions
                manifest["input_resume_diversions"] = getattr(inputs, "resume_diversions", [])
                if inputs.script is not None:
                    (output / "input-script-final.json").write_text(json.dumps(inputs.script, indent=2) + "\n")
                inputs.close(remote, manifest["status"] == "captured; repeat qualification required")
                if manifest["status"] == "captured; repeat qualification required":
                    manifest["input_export"] = input_export_receipt(output, inputs.reads)
            except Exception as exc:
                cleanup_error = f"input cleanup failed: {type(exc).__name__}"
                manifest.update(status="incomplete", cleanup_error=cleanup_error)
        if raw_suffix:
            try:
                raw_suffix.close(manifest['status'] == 'captured; repeat qualification required')
            except Exception as exc:
                manifest.update(status='incomplete', raw_input_error=str(exc))
            manifest['raw_input_suffix'] = raw_suffix.receipt
        if actor_output:
            try:
                actor_output.close(manifest["status"] == "captured; repeat qualification required")
                manifest["actors"] = actor_output.manifest()
            except Exception as exc:
                cleanup_error = f"actor finalization failed: {type(exc).__name__}: {exc}"
                manifest.update(status="incomplete", cleanup_error=cleanup_error)
                manifest["actors"] = dict(schema_version=1, status="incomplete")
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
        for number, handler in handlers.items():
            signal.signal(number, handler)
        if cleanup_error:
            raise ObserverError(cleanup_error)


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
    capture.add_argument("--export-inputs", action="store_true", help="read original KPADReadEx samples and WPADProbe results without injecting input")
    capture.add_argument("--actors", action="store_true", help="capture complete registered-actor and per-frame census sidecars")
    capture.add_argument("--save-state", help="private Dolphin state to restore; KPAD injection starts after first seam")
    capture.add_argument("--sync-stage", help="canonical internal stage; first matching ready end-of-loop seam becomes row 0")
    capture.add_argument("--sync-scenario", type=int, help="placed ScenarioNo; required with --sync-stage")
    capture.add_argument("--sync-input-script", help="freeze this shared suffix immediately after sync row 0")
    capture.add_argument('--sync-input-records', help='private strict native KPAD JSONL suffix after ready row 0')
    capture.add_argument('--sync-input-records-sha256')
    capture.add_argument('--sync-observed-connection', action='store_true', help='export actual read-only WPADProbe connection; keep all source sample bits/history unchanged')
    capture.add_argument('--yoshi-replay-budget', action='store_true', help='bounded full Yoshi replay, up to 10800 seconds; default remains 480')
    capture.add_argument("--input-script", help="explicit filtered KPAD script; writes SDK output buffers/register r3")
    capture.add_argument("--live-input-script", action="store_true", help="exploration only: reload future-only script edits")
    capture.add_argument("--gui", action="store_true", help="show the main UI for TAS recording/navigation")
    capture.add_argument("--output", required=True)
    capture.add_argument("--frames", type=int, default=600)
    capture.add_argument("--connect-timeout", type=float, default=60)
    capture.add_argument("--stop-timeout", type=float, default=30)
    capture.add_argument("--wall-timeout", type=float, default=480)
    capture.add_argument('--extended-wall-budget', action='store_true', help='explicit team-approved ONE continuous human-tape run, maximum 1500 seconds; default remains 480')
    capture.add_argument("--lock-timeout", type=int, help="optional maximum seconds waiting for the shared slot")
    capture.add_argument("--lock", default="~/MacioGalaxy2-team/slots/native.lock")
    capture.add_argument("--slot-held", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.action == "collect" and args.export_inputs and args.input_script:
        parser.error("--export-inputs cannot be combined with --input-script")
    if args.action == "collect" and args.sync_input_script and not (args.sync_stage and args.input_script):
        parser.error("--sync-input-script requires --sync-stage and --input-script")
    if args.action == "collect" and (bool(args.sync_stage) != (args.sync_scenario is not None)):
        parser.error("--sync-stage and --sync-scenario must be supplied together")
    if args.action == "collect" and args.sync_scenario is not None and args.sync_scenario <= 0:
        parser.error("--sync-scenario must be a positive placed ScenarioNo")
    if args.action == "collect" and args.sync_stage and not all(
            char.isascii() and (char.isalnum() or char == "_") for char in args.sync_stage):
        parser.error("--sync-stage must be a canonical internal stage identifier")
    if args.action == "collect" and args.save_state and (not args.dolphin or args.movie):
        parser.error("--save-state requires --dolphin and cannot be combined with --movie")
    if args.action == "collect" and args.lock_timeout is not None and args.lock_timeout < 0:
        parser.error("--lock-timeout must be nonnegative")
    if args.action == "collect" and args.frames < 1:
        parser.error("--frames must be positive")
    if args.action == 'collect' and args.sync_observed_connection and not args.sync_input_records:
        parser.error('--sync-observed-connection requires a raw suffix')
    if args.action == 'collect' and args.sync_input_records:
        if not (args.sync_stage and args.input_script and args.sync_input_records_sha256) or args.sync_input_script or args.frames < 2:
            parser.error('raw suffix requires sync, boot script, SHA256 and N+1 frames; cannot combine suffix modes')
    if args.action == 'collect' and args.yoshi_replay_budget:
        if not (args.sync_stage == 'YosshiHomeGalaxy' and args.sync_scenario == 1 and args.sync_input_records and 2 <= args.frames <= 30001) or args.extended_wall_budget:
            parser.error('Yoshi budget requires bounded Yoshi/scenario1 raw suffix, exclusive of old extended budget')
    if args.action == "collect" and not 0 < args.wall_timeout <= (10800 if args.yoshi_replay_budget else 1500 if args.extended_wall_budget else 480):
        parser.error('--wall-timeout must be within 0..480, or 0..1500 only with explicit --extended-wall-budget approval')
    if args.action == "collect" and args.dolphin and not args.slot_held:
        command = ["lockf", "-k"]
        if args.lock_timeout is not None:
            command += ["-t", str(args.lock_timeout)]
        command += [str(Path(args.lock).expanduser()), "nice", "-n", "10",
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
