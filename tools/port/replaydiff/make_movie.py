#!/usr/bin/env python3
"""Encode a poll-indexed Wiimote/Nunchuk script for Dolphin 2609a only."""
import argparse
import json
import struct

from dolphin_collect import private_path

REVISION = "409881c34357d0f105fd473167d15ab0bd9c1628"
BUTTONS = {name: 1 << i for i, name in enumerate(("A", "B", "+", "-", "1", "2", "HOME"))}
DPAD = {name: 1 << i for i, name in enumerate(("UP", "DOWN", "LEFT", "RIGHT"))}


def packet(segment):
    unknown = set(segment) - {"polls", "buttons", "stick", "ir", "c", "z"}
    if unknown:
        raise ValueError(f"unknown movie script keys: {sorted(unknown)}")
    names = segment.get("buttons", [])
    if set(names) - (BUTTONS.keys() | DPAD.keys()):
        raise ValueError("unknown button name")
    buttons = sum(BUTTONS.get(name, 0) for name in set(names))
    dpad = sum(DPAD.get(name, 0) for name in set(names))
    has_buttons = bool(buttons or dpad)
    has_ir = "ir" in segment
    data = bytearray([0x10 | int(has_buttons) | (int(has_ir) << 2)])
    if has_buttons:
        data.extend((buttons, dpad))
    if has_ir:
        x, y = segment["ir"]
        # Two stationary sensor-bar lights; remaining camera points are invisible.
        for px in (x - 50, x + 50):
            if not 0 <= px <= 1023 or not 0 <= y <= 767:
                raise ValueError("IR center must fit both lights in the camera")
            data.extend((px & 255, y & 255, 4 | ((px >> 8) << 4) | ((y >> 8) << 6)))
        data.extend(b"\xff" * 6)
    stick = segment.get("stick", [128, 128])
    if len(stick) != 2 or any(not isinstance(v, int) or not 0 <= v <= 255 for v in stick):
        raise ValueError("stick requires two raw u8 values")
    # Nunchuk factory midpoint (512,512,512) plus upright +1g Z (716).
    # Buttons are active low; these are desired, unencrypted extension bytes.
    flags = (0 if segment.get("z") else 1) | (0 if segment.get("c") else 2)
    data.extend((*stick, 128, 128, 179, flags))
    return bytes([len(data)]) + data


def encode(segments):
    count = sum(s["polls"] for s in segments)
    if any(not isinstance(s["polls"], int) or not 1 <= s["polls"] <= 1000000 for s in segments):
        raise ValueError("each segment needs 1..1000000 polls")
    if not 1 <= count <= 1000000:
        raise ValueError("movie must contain 1..1000000 total polls")
    header = bytearray(256)
    header[:4] = b"DTM\x1a"
    header[4:10] = b"SB4E01"
    header[10:13] = bytes((1, 0x10, 0))
    # Upper bound for video frames; input records are polled independently of game loops.
    struct.pack_into("<QQ", header, 13, count, count)
    header[49:49 + len(b"replaydiff script")] = b"replaydiff script"
    struct.pack_into("<Q", header, 129, 946684800)
    # CheckInputEnd also enforces tickCount. This synthetic script uses a
    # conservative upper time bound; the exact input byte count ends playback.
    struct.pack_into("<Q", header, 237, count * 729000000)
    # bSaveConfig remains false: use the explicitly pinned isolated profile.
    header[209:229] = bytes.fromhex(REVISION)
    return bytes(header) + b"".join(packet(s) * s["polls"] for s in segments)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("script", help="JSON array of {polls, buttons?, stick?, ir?, c?, z?}")
    parser.add_argument("output", help="private .dtm path outside repo")
    args = parser.parse_args()
    segments = json.loads(private_path(args.script).read_text())
    result = encode(segments)
    output = private_path(args.output)
    with output.open("xb") as file:
        file.write(result)
    print(f"Wrote {len(result)} bytes; format pinned to Dolphin {REVISION}")


if __name__ == "__main__":
    main()
