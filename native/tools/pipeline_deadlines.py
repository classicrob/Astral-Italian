#!/usr/bin/env python3
"""PFD1 frame-deadline sidecars: config rows and frame integers, never capture provenance."""
from pathlib import Path
import re
import struct

MAGIC = b'PFD1'
HEADER = struct.Struct('<4sHHHHQI')
ENTRY = struct.Struct('<QI')
LIMIT = 65536
MASK = (1 << 64) - 1


def fingerprint(rows):
    """FNV-1a of canonical unsigned-hash row order; a stale-data guard, not authentication."""
    value = 14695981039346656037
    for t, h, version, size, blob, first in sorted(rows, key=lambda r: (r[0], r[1] & MASK)):
        if t not in (0, 1) or size != len(blob) or not 0 <= first <= 0xFFFFFFFF:
            raise ValueError('invalid canonical manifest row')
        data = struct.pack('<BQHHI', t, h & MASK, version, size, first) + blob
        for byte in data:
            value = ((value ^ byte) * 1099511628211) & MASK
    return value


def encode(stage, rows, deadlines):
    if not re.fullmatch(r'[A-Za-z0-9_-]{1,80}', stage):
        raise ValueError('invalid deadline stage name')
    if not 0 < len(deadlines) <= min(LIMIT, sum(r[0] == 1 for r in rows)):
        raise ValueError('invalid deadline entry count')
    known = {h & MASK for t, h, v, s, b, f in rows if t == 1 and v == 13}
    name = stage.encode('ascii')
    data = bytearray(HEADER.pack(MAGIC, 1, 13, len(name), 0, fingerprint(rows), len(deadlines)) + name)
    for key, frame in sorted(deadlines.items()):
        if type(key) is not int or key not in known or type(frame) is not int or not 0 <= frame < 0xFFFFFFFF:
            raise ValueError('unknown config or invalid explicit frame')
        data += ENTRY.pack(key, frame)
    return bytes(data)


def decode(data, stage, rows):
    if len(data) < HEADER.size or len(data) > HEADER.size + 80 + ENTRY.size * LIMIT:
        raise ValueError('invalid deadline file size')
    magic, version, gx, length, reserved, stamp, count = HEADER.unpack_from(data)
    if (magic, version, gx, reserved) != (MAGIC, 1, 13, 0) or not 1 <= length <= 80:
        raise ValueError('invalid deadline header/version')
    if len(data) != HEADER.size + length + ENTRY.size * count or not 0 < count <= LIMIT:
        raise ValueError('deadline truncation/trailing bytes/count')
    if data[HEADER.size:HEADER.size + length] != stage.encode('ascii') or stamp != fingerprint(rows):
        raise ValueError('deadline stage/manifest binding mismatch')
    keys = {h & MASK for t, h, v, s, b, f in rows if t == 1 and v == 13}
    result, previous = {}, -1
    for key, frame in ENTRY.iter_unpack(data[HEADER.size + length:]):
        if key <= previous or key not in keys or frame == 0xFFFFFFFF:
            raise ValueError('deadline duplicate/order/unknown config/frame')
        result[key] = frame; previous = key
    return result


def read(path, stage, rows):
    return decode(Path(path).read_bytes(), stage, rows)
