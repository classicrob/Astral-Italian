#!/usr/bin/env python3
"""Strict KPAD JSONL interchange, synthetic input generation, and headless launcher."""
import argparse
import csv
import hashlib
import json
import math
import re
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

MAGIC = b"SMG2KPD1"
RECORD = struct.Struct(">QIIIiiiII")
REPO = Path(__file__).resolve().parents[2]


def trace_columns():
    columns = []
    fields = REPO / "native/app/headless/trace_fields.inc"
    for kind, name in re.findall(r"^(TEXT|FLOAT)\((\w+)\)$", fields.read_text(), re.M):
        columns.append(name)
        if kind == "FLOAT":
            columns.append(name + "_bits")
    return columns


def valid_trace(path, frames):
    try:
        with path.open(newline="") as source:
            rows = csv.reader(source, strict=True)
            columns = trace_columns()
            if next(rows, None) != columns:
                return False
            validity = [columns.index(key) for key in ("scene_valid", "player_valid", "actor_count_valid", "rng_valid", "animation_valid")]
            reserved = [columns.index(key) for key in ("ticks", "vi_count")]
            count = 0
            for row in rows:
                if len(row) != len(columns) or row[0] != str(count) or any(row[index] not in ("0", "1") for index in validity) or any(row[index] for index in reserved):
                    return False
                count += 1
            return count == frames
    except (OSError, ValueError, csv.Error):
        return False


def valid_timing(path, frames):
    try:
        with path.open(newline="") as source:
            rows = csv.reader(source, strict=True)
            if next(rows, None) != ["frame", "vi_count", "ticks"]:
                return False
            count, previous_tick = 0, -1
            for row in rows:
                if len(row) != 3 or row[0] != str(count) or any(str(int(value)) != value or int(value) < 0 for value in row):
                    return False
                if int(row[2]) < previous_tick:
                    return False
                previous_tick = int(row[2])
                count += 1
            return count == frames
    except (OSError, ValueError, csv.Error):
        return False


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
    elif device in (0, 5, 253):
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


def generate(frames=602, idle=600):
    for frame in range(frames):
        for channel in range(4):
            sample = idle_sample(channel == 0)
            if channel == 0 and frame == idle:
                sample.update(hold=0x0800, trig=0x0800)
            elif channel == 0 and frame == idle + 1:
                sample["release"] = 0x0800
            yield {"frame": frame, "read_ordinal": channel, "channel": channel, "requested_length": 120,
                   "returned_count": 1 if channel == 0 else 0, "error": 0 if channel == 0 else -2,
                   "connection": {"probe_error": 0 if channel == 0 else -1, "device_type": 1 if channel == 0 else 253},
                   "samples": [sample]}


def outside_repo(path):
    path = path.expanduser().resolve()
    if path == REPO or REPO in path.parents:
        raise ValueError(f"game data and replay artifacts must stay outside the checkout: {path}")
    return path


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("schema", help="print the shared strict trace column contract")
    gen = sub.add_parser("generate", help="synthetic four-channel fixture, not a Dolphin recording")
    gen.add_argument("output", type=Path)
    gen.add_argument("--idle", type=int, default=600)
    gen.add_argument("--frames", type=int, default=602)
    pack = sub.add_parser("pack", help="validate named JSONL and create the native tape")
    pack.add_argument("input", type=Path)
    pack.add_argument("output", type=Path)
    pack.add_argument("--frames", type=int, required=True)
    run = sub.add_parser("run")
    run.add_argument("--executable", type=Path, required=True)
    run.add_argument("--inputs", type=Path, required=True)
    run.add_argument("--frames", type=int, required=True)
    run.add_argument("--disc", type=Path, required=True)
    run.add_argument("--user", type=Path, required=True)
    run.add_argument("--trace", type=Path, required=True)
    run.add_argument("--epoch-ticks", type=int, required=True)
    run.add_argument("--rate-numerator", type=int, default=60000)
    run.add_argument("--rate-denominator", type=int, default=1001)
    run.add_argument("--no-ftz", action="store_true")
    args = parser.parse_args(argv)
    if args.command == "schema":
        print(json.dumps({"schema_version": 1, "columns": trace_columns()}, indent=2))
        return 0
    if args.command == "generate":
        if args.idle < 0 or args.frames < args.idle + 2:
            raise ValueError("fixture must include idle frames, jump, and release")
        output = outside_repo(args.output)
        output.parent.mkdir(parents=True, exist_ok=True)
        with output.open("x") as out:
            for row in generate(args.frames, args.idle):
                out.write(json.dumps(row, separators=(",", ":")) + "\n")
        return 0
    if args.command == "pack":
        source, output = outside_repo(args.input), outside_repo(args.output)
        output.parent.mkdir(parents=True, exist_ok=True)
        created = False
        try:
            with output.open("xb") as out:
                created = True
                encode_rows(rows_from_jsonl(source), args.frames, out)
        except Exception:
            if created:
                output.unlink()
            raise
        return 0
    disc, user, trace, source = (outside_repo(p) for p in (args.disc, args.user, args.trace, args.inputs))
    manifest_path = Path(str(trace) + ".manifest.json")
    marker = Path(str(trace) + ".complete")
    timing = Path(str(trace) + ".timing.csv")
    if any(path.exists() or path.is_symlink() for path in (trace, manifest_path, marker, timing)):
        raise ValueError("refusing to overwrite an existing trace")
    user.mkdir(parents=True, exist_ok=True)
    if any(user.iterdir()):
        raise ValueError("headless --user must be a fresh isolated directory")
    with tempfile.TemporaryDirectory(prefix="smg2-kpad-", dir=user.parent) as temp:
        tape = Path(temp) / "inputs.kpd"
        with tape.open("xb") as out:
            encode_rows(rows_from_jsonl(source), args.frames, out)
        command = [str(args.executable.expanduser().resolve()), "--inputs", str(tape), "--disc", str(disc),
                   "--user", str(user), "--trace", str(trace), "--epoch-ticks", str(args.epoch_ticks),
                   "--rate-numerator", str(args.rate_numerator), "--rate-denominator", str(args.rate_denominator)]
        if args.no_ftz:
            command.append("--no-ftz")
        lock = Path.home() / "MacioGalaxy2-team/slots/native.lock"
        def digest(path):
            result = hashlib.sha256()
            with path.open("rb") as contents:
                while chunk := contents.read(1024 * 1024):
                    result.update(chunk)
            return result.hexdigest()
        manifest = {"schema_version": 1, "frames": args.frames, "epoch_ticks": args.epoch_ticks,
                    "rate_numerator": args.rate_numerator, "rate_denominator": args.rate_denominator,
                    "fpcr_fz": not args.no_ftz, "input_sha256": digest(source), "columns": trace_columns(),
                    "disc_root": str(disc), "status": "prepared", "command": command, "timing_csv": str(timing)}
        with manifest_path.open("x") as out:
            json.dump(manifest, out, indent=2)
            out.write("\n")
        status = subprocess.call(["lockf", "-k", str(lock), "nice", "-n", "10", *command])
        complete = status == 0 and trace.is_file() and marker.is_file() and marker.read_text().strip() == str(args.frames) and valid_trace(trace, args.frames) and valid_timing(timing, args.frames)
        manifest.update(status="complete" if complete else "invalid", exit_status=status)
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")
        return 0 if complete else 2


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, TypeError, KeyError, OSError, OverflowError, struct.error) as exc:
        print(f"headless: {exc}", file=sys.stderr)
        sys.exit(2)
