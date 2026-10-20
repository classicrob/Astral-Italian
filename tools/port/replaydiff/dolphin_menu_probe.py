#!/usr/bin/env python3
"""Read-only FileSelector telemetry for private Dolphin routing experiments."""
import argparse
import json
from pathlib import Path
import re
import sys
import struct

import dolphin_collect as collector


def menu_state(memory):
    game = memory.pointer(collector.ROOT, nullable=True)
    if not game:
        return {}
    control = memory.pointer(game + 0x24, nullable=True)
    if not control:
        return {}
    scene = memory.pointer(control + 0x12C, nullable=True)
    if not scene:
        return {}
    holder = memory.pointer(scene + 0x10)
    group = memory.pointer(holder + 4 * 0x39, nullable=True)
    if not group:
        return {}
    capacity, count = struct.unpack(">ii", memory.read(group + 0x14, 8))
    if not 0 <= count <= capacity <= 32768:
        raise collector.ObserverError("invalid menu actor group")
    if not count:
        return {}
    array = memory.pointer(group + 0x1C)
    for actor in memory.pointers(array, count):
        if memory.u32(actor) != 0x806974C0:
            continue
        spine = memory.pointer(actor + 0x58)
        result = dict(nerve=memory.u32(spine + 4), step=memory.u32(spine + 0xC))
        for name, offset in (("selected_file", 0xB0), ("pointed_file", 0xB8), ("candidate_file", 0xBC)):
            item = memory.pointer(actor + offset, nullable=True)
            result[name] = memory.u32(item + 0x144) if item else None
        button = memory.pointer(actor + 0xA0, nullable=True)
        result["selected_button"] = memory.u32(button + 0x5C) if button else None
        return result
    return {}


def main():
    parser = argparse.ArgumentParser(description=__doc__, add_help=False)
    parser.add_argument("--menu-log", required=True)
    args, remaining = parser.parse_known_args()
    path = collector.game_private_path(args.menu_log)
    path.parent.mkdir(parents=True, exist_ok=True)
    symbols = (collector.REPO / "config/SB4E01/symbols.txt").read_text()
    nerves = {int(address, 16): name for name, address in re.findall(
        r"sInstance__Q215NrvFileSelector\d+FileSelectorNrv(\w+) = \.sbss:0x([0-9A-Fa-f]+);", symbols)}
    base = collector.Snapshot
    with path.open("x", buffering=1) as out:
        class Snapshot(base):
            def take(self, frame):
                row = super().take(frame)
                if row["stage"] == "FileSelect" and row["init_state"] == 6 and frame % 30 == 0:
                    state = menu_state(self.m)
                    state["nerve_name"] = nerves.get(state.get("nerve"))
                    out.write(json.dumps(dict(frame=frame, **state)) + "\n")
                return row
        collector.Snapshot = Snapshot
        sys.argv = [sys.argv[0], *remaining]
        return collector.main()


if __name__ == "__main__":
    sys.exit(main())
