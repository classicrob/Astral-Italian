#!/usr/bin/env python3
"""Shared strict SMG2 KPAD JSONL/sample encoder; no renderer or launcher imports."""
import json
import math
import struct
import tempfile
from pathlib import Path
MAGIC = b"SMG2KPD1"
RECORD = struct.Struct(">QIIIiiiII")

def integer(value, low, high, name):
    if type(value) is not int or not low <= value <= high:
        raise ValueError(f"{name}: expected integer {low}..{high}")
    return value


def fp(value, width=32):
    fmt = ">f" if width == 32 else ">d"
    if not isinstance(value, dict) or set(value) != {"value", "bits"}:
        raise ValueError("floating fields require {value, bits}")
    if type(value["value"]) not in (float, int) or not math.isfinite(value["value"]):
        raise ValueError("nonfinite floating sample")
    bits = value["bits"]
    if not isinstance(bits, str) or len(bits) != width // 4 + 2 or not bits.startswith("0x"):
        raise ValueError("invalid floating bit pattern")
    raw = bytes.fromhex(bits[2:])
    if not math.isfinite(struct.unpack(fmt, raw)[0]) or struct.pack(fmt, value["value"]) != raw:
        raise ValueError("decimal and floating bits disagree")
    return raw


def floating(value=0.0, width=32):
    fmt = ">f" if width == 32 else ">d"
    raw = struct.pack(fmt, value)
    return {"value": struct.unpack(fmt, raw)[0], "bits": "0x" + raw.hex()}


def encode_sample(sample):
    out = bytearray(240)

    def u32(offset, value):
        struct.pack_into(">I", out, offset, integer(value, 0, 0xffffffff, "sample word"))

    def real(offset, value):
        out[offset:offset + 4] = fp(value)

    def vector(offset, value, names=("x", "y", "z")):
        if set(value) != set(names):
            raise ValueError("invalid sample vector")
        for index, name in enumerate(names):
            real(offset + 4 * index, value[name])

    required = {"hold", "trig", "release", "acc", "acc_value", "acc_speed", "pos", "vec", "speed",
                "horizon", "hori_vec", "hori_speed", "dist", "dist_vec", "dist_speed", "acc_vertical",
                "dev_type", "wpad_err", "dpd_valid_fg", "data_format", "ex_status", "mpls", "unk_EC"}
    if set(sample) != required:
        raise ValueError(f"sample field mismatch: missing {required - set(sample)}, extra {set(sample) - required}")
    for offset, name in [(0, "hold"), (4, "trig"), (8, "release"), (236, "unk_EC")]:
        u32(offset, sample[name])
    vector(12, sample["acc"])
    for offset, name in [(24, "acc_value"), (28, "acc_speed"), (48, "speed"), (68, "hori_speed"),
                         (72, "dist"), (76, "dist_vec"), (80, "dist_speed")]:
        real(offset, sample[name])
    for offset, name in [(32, "pos"), (40, "vec"), (52, "horizon"), (60, "hori_vec"), (84, "acc_vertical")]:
        vector(offset, sample[name], ("x", "y"))
    device = integer(sample["dev_type"], 0, 255, "dev_type")
    struct.pack_into(">BbbB", out, 92, device,
                     integer(sample["wpad_err"], -128, 127, "wpad_err"),
                     integer(sample["dpd_valid_fg"], -128, 127, "dpd_valid_fg"),
                     integer(sample["data_format"], 0, 255, "data_format"))
    ex = sample["ex_status"]
    if device in (1, 6):
        if set(ex) != {"fs"} or set(ex["fs"]) != {"stick", "acc", "acc_value", "acc_speed"}:
            raise ValueError("Nunchuk sample requires its complete named union")
        fs = ex["fs"]
        vector(96, fs["stick"], ("x", "y"))
        vector(104, fs["acc"])
        real(116, fs["acc_value"])
        real(120, fs["acc_speed"])
    elif device in (2, 7):
        if set(ex) != {"cl"} or set(ex["cl"]) != {"hold", "trig", "release", "lstick", "rstick", "ltrigger", "rtrigger"}:
            raise ValueError("Classic sample requires its complete named union")
        cl = ex["cl"]
        for offset, name in [(96, "hold"), (100, "trig"), (104, "release")]:
            u32(offset, cl[name])
        vector(108, cl["lstick"], ("x", "y"))
        vector(116, cl["rstick"], ("x", "y"))
        real(124, cl["ltrigger"])
        real(128, cl["rtrigger"])
    elif device in (0, 5, 252, 253, 255):
        if ex != {}:
            raise ValueError("core/disconnected sample must have an empty extension union")
    else:
        raise ValueError("unsupported extension union; no implicit raw-byte reinterpretation")
    mpls = sample["mpls"]
    if set(mpls) != {"angularVelocity", "angle", "direction"} or set(mpls["direction"]) != {"X", "Y", "Z"}:
        raise ValueError("MotionPlus sample field mismatch")
    vector(176, mpls["angularVelocity"])
    vector(188, mpls["angle"])
    for offset, name in [(200, "X"), (212, "Y"), (224, "Z")]:
        vector(offset, mpls["direction"][name])
    return out


def encode_rows(rows, frames, out):
    integer(frames, 1, 10000000, "frame count")
    records = tempfile.TemporaryFile()
    count = 0
    previous = None
    connections = {}
    try:
        for row in rows:
            required = {"frame", "read_ordinal", "channel", "requested_length", "returned_count", "error", "connection", "samples"}
            if set(row) != required:
                raise ValueError("input row fields must match the KPAD v1 schema")
            frame = integer(row["frame"], 0, frames - 1, "frame")
            ordinal = integer(row["read_ordinal"], 0, 0xffffffff, "read_ordinal")
            channel = integer(row["channel"], 0, 3, "channel")
            capacity = integer(row["requested_length"], 1, 4096, "requested_length")
            returned = integer(row["returned_count"], -128, capacity, "returned_count")
            error = integer(row["error"], -0x80000000, 0x7fffffff, "error")
            if previous is None or frame != previous[0]:
                if ordinal != 0 or (previous and frame < previous[0]):
                    raise ValueError("input frames/ordinals are out of order")
                connections = {}
            elif ordinal != previous[1] + 1:
                raise ValueError("read ordinals must be contiguous within each frame")
            connection = row["connection"]
            if set(connection) != {"probe_error", "device_type"}:
                raise ValueError("invalid connection state")
            probe = integer(connection["probe_error"], -128, 127, "probe_error")
            device = integer(connection["device_type"], 0, 255, "device_type")
            if channel in connections and connections[channel] != (probe, device):
                raise ValueError("within-frame connection transitions require timestamped raw replay")
            connections[channel] = (probe, device)
            samples = row["samples"]
            if not isinstance(samples, list) or len(samples) != max(returned, 1):
                raise ValueError("samples must include every returned sample, or slot 0 for zero/error returns")
            records.write(RECORD.pack(frame, ordinal, channel, capacity, returned, error, probe, device, len(samples)))
            for sample in samples:
                records.write(encode_sample(sample))
            previous = (frame, ordinal)
            count += 1
            if count > 40000000:
                raise ValueError("too many KPAD reads")
        out.write(MAGIC + struct.pack(">QQ", frames, count))
        records.seek(0)
        while chunk := records.read(1024 * 1024):
            out.write(chunk)
    finally:
        records.close()


def rows_from_jsonl(path):
    def no_duplicates(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError(f"duplicate JSON key: {key}")
            result[key] = value
        return result
    with path.open() as source:
        for number, line in enumerate(source, 1):
            if not line.strip():
                raise ValueError(f"empty input record at line {number}")
            try:
                yield json.loads(line, object_pairs_hook=no_duplicates)
            except (ValueError, TypeError) as exc:
                raise ValueError(f"input line {number}: {exc}") from exc


def idle_sample(connected=True):
    def vec(*values):
        return {name: floating(value) for name, value in zip(("x", "y", "z"), values)}
    sample = {name: floating() for name in ("acc_value", "acc_speed", "speed", "hori_speed", "dist", "dist_vec", "dist_speed")}
    sample.update(hold=0, trig=0, release=0, acc=vec(0, -1, 0), pos=vec(0, 0), vec=vec(0, 0),
                  horizon=vec(1, 0), hori_vec=vec(0, 0), acc_vertical=vec(1, 0), dev_type=1 if connected else 253,
                  wpad_err=0 if connected else -1, dpd_valid_fg=0, data_format=5 if connected else 0,
                  ex_status={"fs": {"stick": vec(0, 0), "acc": vec(0, -1, 0), "acc_value": floating(1), "acc_speed": floating()}} if connected else {},
                  mpls={"angularVelocity": vec(0, 0, 0), "angle": vec(0, 0, 0), "direction": {key: vec(0, 0, 0) for key in ("X", "Y", "Z")}}, unk_EC=0)
    sample["acc_value"] = floating(1)
    return sample


