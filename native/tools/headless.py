#!/usr/bin/env python3
"""Strict KPAD JSONL interchange, synthetic input generation, and headless launcher."""
import argparse
import csv
import hashlib
import importlib.util
import json
import math
import re
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

REPO = Path(__file__).resolve().parents[2]


def actor_capture_receipt(trace):
    spec = importlib.util.spec_from_file_location("headless_actor_receipt", REPO / "native/tools/actor_receipt.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.collect_actor_receipt(trace, trace_columns())


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


def sync_number(value):
    if not re.fullmatch(r"(?:[0-9]+|0[xX][0-9a-fA-F]+)", value):
        raise argparse.ArgumentTypeError("expected decimal or 0x unsigned 32-bit integer")
    number = int(value, 16 if value.lower().startswith("0x") else 10)
    if number > 0xffffffff:
        raise argparse.ArgumentTypeError("stage-sync integer exceeds u32")
    return number


def sync_completion(path, args):
    origin = json.loads(path.read_text())
    if type(origin.get("schema_version")) is not int or origin["schema_version"] != 1 or origin.get("phase") != "post-endFrame":
        raise ValueError("invalid stage-sync schema/phase")
    if origin.get("stage") != args.sync_stage or type(origin.get("scenario")) is not int or origin["scenario"] != args.sync_scenario:
        raise ValueError("stage-sync target mismatch")
    for key in ("source_frame", "source_scene_epoch", "source_ticks", "source_vi_count"):
        integer(origin[key], 0, 0xffffffffffffffff, key)
    integer(origin["rng_seed"], 0, 0xffffffff, "stage-sync seed")
    if origin.get("rng_valid") is not True or type(origin.get("rng_forced")) is not bool or origin["rng_forced"] != (args.sync_rng_seed is not None):
        raise ValueError("invalid stage-sync RNG provenance")
    if args.sync_rng_seed is not None and origin["rng_seed"] != args.sync_rng_seed:
        raise ValueError("stage-sync forced seed mismatch")
    frames = args.frames + 1 if args.inputs_from_sync else args.frames - origin["source_frame"]
    if frames <= 0:
        raise ValueError("stage-sync origin outside input tape")
    with args.trace.open(newline="") as source:
        first = next(csv.DictReader(source), {})
    expected = dict(frame="0", scene_epoch="0", scene_valid="1", scene="Game", init_state="6",
                    stage=args.sync_stage, scenario=str(args.sync_scenario), rng_valid="1", rng_seed=str(origin["rng_seed"]))
    if any(first.get(key) != value for key, value in expected.items()):
        raise ValueError("stage-sync origin disagrees with row zero")
    with Path(str(args.trace) + ".timing.csv").open(newline="") as source:
        first_time = next(csv.DictReader(source), {})
    if first_time != dict(frame="0", vi_count=str(origin["source_vi_count"]), ticks=str(origin["source_ticks"])):
        raise ValueError("stage-sync origin disagrees with timing sidecar")
    return frames, origin


# Preserve the headless module's public helper API for existing converters and
# tests, while keeping exactly one renderer-independent encoding implementation.
_kpad_spec = importlib.util.spec_from_file_location("petari_kpad_tape", Path(__file__).with_name("kpad_tape.py"))
_kpad = importlib.util.module_from_spec(_kpad_spec)
_kpad_spec.loader.exec_module(_kpad)
for _name in ("MAGIC", "RECORD", "integer", "fp", "floating", "encode_sample", "encode_rows", "rows_from_jsonl", "idle_sample"):
    globals()[_name] = getattr(_kpad, _name)


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
    run.add_argument("--existing-unlocked-user", action="store_true",
                     help="reuse only the separate AstralItalian-unlocked NAND for save reload validation")
    run.add_argument("--trace", type=Path, required=True)
    run.add_argument("--epoch-ticks", type=int, required=True)
    run.add_argument("--rate-numerator", type=int, default=60000)
    run.add_argument("--rate-denominator", type=int, default=1001)
    run.add_argument("--no-ftz", action="store_true")
    run.add_argument("--request", help='shared startup command: galaxy "Name" scenario N')
    run.add_argument("--stage", help="shared catalog internal name, alias, or English galaxy label")
    run.add_argument("--scenario", type=sync_number, help="actual selected ScenarioNo; defaults to 1 with --stage")
    run.add_argument("--actor-trace", action="store_true", help="record and validate complete actor census sidecars")
    run.add_argument("--wall-timeout", type=int, default=540, help="wall watchdog seconds; never advances simulation time")
    run.add_argument("--inputs-from-sync", action="store_true", help="hold a neutral connected Nunchuk on channel 0 until the ready seam, then consume tape frame 0; trace includes the anchor")
    run.add_argument("--sync-stage", help="canonical target stage at the first ready post-endFrame seam")
    run.add_argument("--sync-scenario", type=sync_number, help="placed ScenarioNo, not selected hidden/green ID")
    run.add_argument("--sync-rng-seed", type=sync_number, help="one-shot test override of the game RNG at that seam")
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
    if not 1 <= args.wall_timeout <= 3600:
        raise ValueError("wall timeout must be in 1..3600 seconds")
    if args.inputs_from_sync and args.sync_stage is None:
        raise ValueError("--inputs-from-sync requires --sync-stage")
    if args.sync_stage is not None:
        if not re.fullmatch(r"[A-Za-z0-9_]{1,127}", args.sync_stage) or not args.sync_scenario or args.sync_scenario > 0x7fffffff:
            raise ValueError("stage sync requires an internal stage name and positive placed scenario")
    elif args.sync_scenario is not None or args.sync_rng_seed is not None:
        raise ValueError("stage-sync scenario/seed requires --sync-stage")
    if args.request is not None and args.stage is not None:
        raise ValueError("choose --request or --stage")
    if args.scenario is not None and args.stage is None:
        raise ValueError("--scenario requires --stage")
    if args.stage is not None:
        if not args.stage.strip():
            raise ValueError("startup stage must not be empty")
        args.scenario = 1 if args.scenario is None else args.scenario
        integer(args.scenario, 1, 0x7fffffff, "selected scenario")
    if args.request is not None and not args.request.strip():
        raise ValueError("startup request must not be empty")
    disc, user, trace, source = (outside_repo(p) for p in (args.disc, args.user, args.trace, args.inputs))
    args.trace = trace
    sync_path = Path(str(trace) + ".sync.json")
    manifest_path = Path(str(trace) + ".manifest.json")
    marker = Path(str(trace) + ".complete")
    timing = Path(str(trace) + ".timing.csv")
    artifacts = [trace, manifest_path, marker, timing, sync_path]
    if args.actor_trace:
        for suffix in (".actors.csv", ".actor-frames.csv"):
            artifacts.extend([Path(str(trace) + suffix), Path(str(trace) + suffix + ".partial")])
    if any(path.exists() or path.is_symlink() for path in artifacts):
        raise ValueError("refusing to overwrite an existing trace")
    user.mkdir(parents=True, exist_ok=True)
    if args.existing_unlocked_user:
        allowed = (Path.home() / "Library/Application Support/AstralItalian-unlocked").absolute()
        if allowed.resolve() != allowed or not user.is_relative_to(allowed):
            raise ValueError("--existing-unlocked-user requires the separate AstralItalian-unlocked directory")
    elif any(user.iterdir()):
        raise ValueError("headless --user must be a fresh isolated directory")
    with tempfile.TemporaryDirectory(prefix="smg2-kpad-", dir=user.parent) as temp:
        tape = Path(temp) / "inputs.kpd"
        with tape.open("xb") as out:
            encode_rows(rows_from_jsonl(source), args.frames, out)
        command = [str(args.executable.expanduser().resolve()), "--inputs", str(tape), "--disc", str(disc),
                   "--user", str(user), "--trace", str(trace), "--epoch-ticks", str(args.epoch_ticks),
                   "--rate-numerator", str(args.rate_numerator), "--rate-denominator", str(args.rate_denominator)]
        if args.actor_trace:
            command.append("--actor-trace")
        if args.inputs_from_sync:
            command.append("--inputs-from-sync")
        if args.no_ftz:
            command.append("--no-ftz")
        if args.existing_unlocked_user:
            command.append("--existing-unlocked-user")
        if args.request is not None:
            command.extend(["--request", args.request])
        if args.stage is not None:
            command.extend(["--stage", args.stage, "--scenario", str(args.scenario)])
        if args.sync_stage is not None:
            command.extend(["--sync-stage", args.sync_stage, "--sync-scenario", str(args.sync_scenario)])
            if args.sync_rng_seed is not None:
                command.extend(["--sync-rng-seed", str(args.sync_rng_seed)])
        lock = Path.home() / "MacioGalaxy2-team/slots/native.lock"
        def digest(path):
            result = hashlib.sha256()
            with path.open("rb") as contents:
                while chunk := contents.read(1024 * 1024):
                    result.update(chunk)
            return result.hexdigest()
        timing_policy = dict(tick_hz=60750000, rate_numerator=args.rate_numerator,
                             rate_denominator=args.rate_denominator, epoch_ticks=args.epoch_ticks,
                             fpcr_fz=not args.no_ftz, audio_output="off", audio_processing="virtual-ai-dsp",
                             tick_hz_basis="native SDK virtual timebase", fp_policy="ARM FPCR.FZ on game/device/OS threads")
        diagnostic = Path(command[0] + ".diagnostic.json").is_file()
        manifest = {"schema_version": 1, "frames": args.frames, "input_frames": args.frames, "timing_policy": timing_policy, "wall_timeout_seconds": args.wall_timeout, "epoch_ticks": args.epoch_ticks,
                    "rate_numerator": args.rate_numerator, "rate_denominator": args.rate_denominator,
                    "fpcr_fz": not args.no_ftz, "input_sha256": digest(source), "inputs_from_sync": args.inputs_from_sync,
                    "input_origin": "first loop after ready anchor" if args.inputs_from_sync else "boot", "columns": trace_columns(),
                    "disc_root": str(disc), "startup_request": args.request, "startup_stage": args.stage, "startup_scenario": args.scenario, "status": "prepared", "command": command, "timing_csv": str(timing),
                    "sync_json": str(sync_path) if args.sync_stage is not None else None,
                    "executable_sha256": digest(args.executable) if args.executable.is_file() else None}
        manifest["field_availability"] = {"pending_state_id": "unavailable (callback-context only)"}
        manifest["observer_contract_incomplete"] = True
        manifest["diagnostic_placeholder"] = diagnostic
        if args.actor_trace:
            manifest["actors"] = dict(schema_version=1, status="incomplete", frames=None,
                                      actor_rows=None, identity_holds=None, qualification="unverified", files=[])
        with manifest_path.open("x") as out:
            json.dump(manifest, out, indent=2)
            out.write("\n")
        status = subprocess.call(["lockf", "-k", str(lock), "nice", "-n", "10", sys.executable, str(REPO / "native/tools/bounded_run.py"), str(args.wall_timeout), *command])
        trace_frames, origin, validation_error = args.frames, None, None
        try:
            if args.sync_stage is not None:
                trace_frames, origin = sync_completion(sync_path, args)
            elif sync_path.exists():
                raise ValueError("unexpected sync sidecar in an unsynchronized run")
            complete = status == 0 and trace.is_file() and marker.is_file() and marker.read_text().strip() == str(trace_frames) and valid_trace(trace, trace_frames) and valid_timing(timing, trace_frames)
        except (OSError, ValueError, KeyError, TypeError, csv.Error) as exc:
            complete, validation_error = False, str(exc)
        if args.actor_trace:
            try:
                if not complete:
                    raise ValueError("main capture did not complete validation")
                manifest["actors"] = actor_capture_receipt(trace)
                if manifest["actors"]["frames"] != trace_frames:
                    raise ValueError("actor receipt frame count differs from main trace")
            except (OSError, ValueError, KeyError, TypeError, csv.Error) as exc:
                complete, validation_error = False, "actor sidecar receipt: " + str(exc)
                manifest["actors"] = dict(schema_version=1, status="incomplete", frames=None,
                                          actor_rows=None, identity_holds=None, qualification="unverified", files=[],
                                          error=str(exc))
        if args.inputs_from_sync and origin is not None:
            manifest["input_source_start"] = origin["source_frame"] + 1
        manifest.update(status=("diagnostic-complete" if diagnostic else "complete") if complete else "invalid", exit_status=status,
                        frames=trace_frames, trace_frames=trace_frames, sync_origin=origin, validation_error=validation_error)
        manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")
        return 0 if complete and not diagnostic else 2


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, TypeError, KeyError, OSError, OverflowError, struct.error) as exc:
        print(f"headless: {exc}", file=sys.stderr)
        sys.exit(2)
