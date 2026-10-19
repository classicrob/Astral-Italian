#!/usr/bin/env python3
"""Compare game-loop CSV traces without shifting frames after a mismatch."""
from __future__ import annotations

import argparse
from collections import Counter
import csv
import hashlib
from dataclasses import dataclass
import json
import math
from pathlib import Path
import re
import struct
import sys

POSITION = {prefix + axis for prefix in ("", "sim_") for axis in "xyz"}
VELOCITY = {prefix + axis for prefix in ("v", "vel_after_", "actor_v") for axis in "xyz"}
DIRECTION = {prefix + axis for prefix in ("front_", "up_", "actor_up_", "gravity_up_", "g", "actor_gravity_") for axis in "xyz"}
FLOATS = POSITION | VELOCITY | DIRECTION
INTS = set("schema_version frame scene_epoch scenario init_state system_valid scene_valid rng_valid valid player_valid actors_valid actor_count_valid animation_valid rng_seed actors_registered actors_living actor_registered actor_living hp dead mode pending_state_id movement_lo movement_hi flags14_lo flags14_hi draw flags20 flags24_lo flags24_hi prev_draw state_flag0 state_flag1 state_flag2 stage_play_time vi_count ticks".split())
INTEGER = re.compile(r"[+-]?(?:[0-9]+|0[xX][0-9a-fA-F]+)\Z")


class TraceError(ValueError):
    pass


@dataclass
class Trace:
    columns: list[str]
    rows: dict[tuple, dict[str, str]]
    errors: list[str]


def integer(value):
    if not INTEGER.fullmatch(value):
        raise TraceError(f"invalid integer {value!r}")
    return int(value, 16 if value.lstrip("+-").lower().startswith("0x") else 10)


def read_trace(path, actor_keys=()):
    rows, errors, frames, frame_contexts = {}, [], set(), {}
    previous = -1
    with Path(path).open(newline="", encoding="utf-8-sig") as source:
        reader = csv.DictReader(source, strict=True)
        columns = reader.fieldnames
        if not columns or any(not c for c in columns) or len(set(columns)) != len(columns):
            raise TraceError(f"{path}: missing/empty/duplicate column names")
        for column in ("frame", *actor_keys):
            if column not in columns:
                raise TraceError(f"{path}: missing key column {column}")
        for line, row in enumerate(reader, 2):
            if None in row or None in row.values():
                raise TraceError(f"{path}:{line}: wrong CSV cell count")
            frame = integer(row["frame"])
            if frame < 0 or frame < previous or (not actor_keys and frame == previous):
                raise TraceError(f"{path}:{line}: frame sequence is negative, repeated, or out of order")
            previous = frame
            if any(not row[k] for k in actor_keys):
                raise TraceError(f"{path}:{line}: empty actor identity")
            context = tuple(row.get(k, "") for k in ("scene_epoch", "scene", "stage", "scenario"))
            if frame in frame_contexts and context != frame_contexts[frame]:
                raise TraceError(f"{path}:{line}: inconsistent scene context within actor frame")
            frame_contexts[frame] = context
            frames.add(frame)
            key = (frame, *(row[k] for k in actor_keys))
            if key in rows:
                raise TraceError(f"{path}:{line}: duplicate frame/actor identity {key}")
            rows[key] = row
            if row.get("observer_error"):
                errors.append(f"{path}:{line}: observer_error: {row['observer_error']}")
        if not rows:
            errors.append(f"{path}: empty trace")
        if frames:
            expected = 0
            for frame in sorted(frames):
                if frame != expected:
                    errors.append(f"{path}: missing frame range {expected}..{frame - 1}")
                expected = frame + 1
    return Trace(columns, rows, errors)


class Policy:
    def __init__(self, abs_tol=None, rel_tol=1e-6, overrides=None):
        for value in (abs_tol, rel_tol):
            if value is not None and (not math.isfinite(value) or value < 0):
                raise TraceError("tolerances must be finite and nonnegative")
        self.abs_tol, self.rel_tol = abs_tol, rel_tol
        self.overrides = overrides or {}
        for name, rule in self.overrides.items():
            if not isinstance(rule, dict) or rule.get("type", "float") not in ("float", "int", "exact"):
                raise TraceError(f"invalid policy for {name}")
            if set(rule) - {"type", "abs", "rel"}:
                raise TraceError(f"unknown policy option for {name}")
            for key in ("abs", "rel"):
                if key in rule and (not isinstance(rule[key], (int, float)) or not math.isfinite(rule[key]) or rule[key] < 0):
                    raise TraceError(f"invalid {key} tolerance for {name}")

    def rule(self, column):
        default = 0.01 if column in POSITION else 0.001 if column in VELOCITY else 0.0001
        rule = dict(type="float" if column in FLOATS else "int" if column in INTS or column.endswith("_valid") else "exact",
                    abs=default if self.abs_tol is None else self.abs_tol, rel=self.rel_tol)
        rule.update(self.overrides.get(column, {}))
        return rule


def float_bits(value):
    try:
        return struct.unpack(">I", struct.pack(">f", value))[0]
    except (OverflowError, struct.error) as exc:
        raise TraceError("float outside float32 range") from exc


def bits_value(text):
    if not re.fullmatch(r"(?:0[xX])?[0-9a-fA-F]{8}", text):
        raise TraceError(f"invalid float32 bits {text!r}")
    return int(text, 16)


def ulps(a, b):
    def ordered(bits):
        return 0x80000000 - (bits & 0x7fffffff) if bits & 0x80000000 else 0x80000000 + bits
    return abs(ordered(a) - ordered(b))


def validate_cell(column, value, policy):
    if value == "":
        if column == "valid" or column.endswith("_valid"):
            raise TraceError(f"{column}: declared validity bit is unavailable")
        return
    rule = policy.rule(column)
    if column in FLOATS or rule["type"] == "float":
        try:
            number = float(value)
        except ValueError as exc:
            raise TraceError(f"{column}: nonnumeric float {value!r}") from exc
        if not math.isfinite(number):
            raise TraceError(f"{column}: nonfinite float {value!r}")
        float_bits(number)
    elif rule["type"] == "int":
        number = integer(value)
        if column.endswith("_valid") or column in ("valid", "dead", "state_flag0", "state_flag1", "state_flag2"):
            if number not in (0, 1):
                raise TraceError(f"{column}: validity/bool must be 0 or 1")


def validate_row(row, policy):
    invalid = {}
    for column, value in row.items():
        try:
            validate_cell(column, value, policy)
            base = column.removesuffix("_bits")
            if column.endswith("_bits") and base in row and policy.rule(base)["type"] == "float":
                if bool(value) != bool(row[base]):
                    raise TraceError("decimal and float32 bits must be available together")
                if value and float_bits(float(row[base])) != bits_value(value):
                    raise TraceError("decimal/float32 bits disagree")
        except (TraceError, ValueError, OverflowError) as exc:
            invalid[column] = str(exc)
    return invalid


def cell_difference(column, a, b, policy):
    result = dict(column=column, a=a, b=b)
    rule = policy.rule(column)
    if a == "" or b == "":
        return None if a == b else result
    if rule["type"] == "float":
        av, bv = float(a), float(b)
        delta = bv - av
        tolerance = rule["abs"] + rule["rel"] * max(abs(av), abs(bv))
        result.update(delta=delta, absolute_error=abs(delta), tolerance=tolerance,
                      ulp=ulps(float_bits(av), float_bits(bv)))
        return result if abs(delta) > tolerance else None
    if rule["type"] == "int":
        av, bv = integer(a), integer(b)
        result["delta"] = bv - av
        return result if av != bv else None
    return result if a != b else None


def vector_errors(ar, br, policy):
    result = {}
    for prefix in ("", "sim_", "v", "vel_after_", "actor_v", "front_", "up_", "actor_up_", "gravity_up_", "g", "actor_gravity_"):
        names = [prefix + axis for axis in "xyz"]
        if all(ar.get(c) and br.get(c) and policy.rule(c)["type"] == "float" for c in names):
            try:
                for name in names:
                    validate_cell(name, ar[name], policy)
                    validate_cell(name, br[name], policy)
                deltas = [float(br[c]) - float(ar[c]) for c in names]
                result[prefix or "position"] = dict(columns=names, euclidean=math.hypot(*deltas), max_component=max(abs(d) for d in deltas))
            except (ValueError, OverflowError):
                continue
    return result


def describe_key(key, actor_keys):
    return dict(frame=key[0], **dict(zip(actor_keys, key[1:])))


def compare(a, b, policy=None, history=120, actor_keys=(), expected_frames=None):
    policy = policy or Policy()
    errors = a.errors + b.errors
    if expected_frames is not None:
        for side, trace in (("a", a), ("b", b)):
            count = len({key[0] for key in trace.rows})
            if count != expected_frames:
                errors.append(f"trace {side}: expected {expected_frames} frames, found {count}")
    missing_a = sorted(set(b.columns) - set(a.columns))
    missing_b = sorted(set(a.columns) - set(b.columns))
    report = dict(status="identical", exit_code=0, schema=dict(missing_in_a=missing_a, missing_in_b=missing_b),
                  invalid_errors=errors, first_divergence=None, first_bit_difference=None,
                  history=[], summary={})
    if missing_a or missing_b:
        errors.append("column sets differ; no columns are silently discarded")
    unknown = set(policy.overrides) - (set(a.columns) | set(b.columns))
    if unknown:
        errors.append("policy names unknown columns: " + ", ".join(sorted(unknown)))
    columns = sorted(set(a.columns) | set(b.columns))
    differences = {}
    all_keys = sorted(set(a.rows) | set(b.rows))
    for key in all_keys:
        ar, br = a.rows.get(key), b.rows.get(key)
        diffs = []
        invalid_columns = set()
        for side, row in (("a", ar), ("b", br)):
            for column, error in validate_row(row or {}, policy).items():
                errors.append(f"frame {key[0]} {side}.{column}: {error}")
                if column not in invalid_columns:
                    diffs.append(dict(column=column, a=(ar or {}).get(column), b=(br or {}).get(column), error=error))
                invalid_columns.add(column)
        if ar is None or br is None:
            diffs.append(dict(column="<row>", a="missing" if ar is None else "present", b="missing" if br is None else "present"))
            if not actor_keys:
                errors.append(f"frame {key[0]} missing in {'a' if ar is None else 'b'}")
        else:
            for column in columns:
                if column in invalid_columns:
                    continue
                av, bv = ar.get(column), br.get(column)
                if av is None or bv is None:
                    diffs.append(dict(column=column, a=av, b=bv))
                    continue
                base = column.removesuffix("_bits")
                try:
                    if column.endswith("_bits") and base in columns and policy.rule(base)["type"] == "float":
                        if not av or not bv:
                            diff = None if av == bv else dict(column=column, a=av, b=bv)
                            if diff:
                                diffs.append(diff)
                            continue
                        ab, bb = bits_value(av), bits_value(bv)
                        if ab != bb and report["first_bit_difference"] is None:
                            report["first_bit_difference"] = dict(**describe_key(key, actor_keys), column=column, a=av, b=bv, ulp=ulps(ab, bb))
                        continue
                    diff = cell_difference(column, av, bv, policy)
                    if av and bv and policy.rule(column)["type"] == "float" and report["first_bit_difference"] is None:
                        ab, bb = float_bits(float(av)), float_bits(float(bv))
                        if ab != bb:
                            report["first_bit_difference"] = dict(**describe_key(key, actor_keys), column=column + "_bits",
                                a=ar.get(column + "_bits", f"0x{ab:08x}"), b=br.get(column + "_bits", f"0x{bb:08x}"),
                                ulp=ulps(ab, bb), source="CSV bits" if column + "_bits" in ar and column + "_bits" in br else "reconstructed float32")
                    if diff:
                        if policy.rule(column)["type"] == "float":
                            diff.update(a_bits=ar.get(column + "_bits"), b_bits=br.get(column + "_bits"))
                        diffs.append(diff)
                except (TraceError, ValueError, OverflowError) as exc:
                    errors.append(f"frame {key[0]} {column}: {exc}")
                    diffs.append(dict(column=column, a=av, b=bv, error=str(exc)))
        if diffs:
            differences[key] = diffs
    if differences:
        first = next(iter(differences))
        ar, br = a.rows.get(first, {}), b.rows.get(first, {})
        report["first_divergence"] = dict(**describe_key(first, actor_keys),
            context_a={k: ar.get(k) for k in ("scene_epoch", "scene", "stage", "scenario", "state_ids", "rng_seed")},
            context_b={k: br.get(k) for k in ("scene_epoch", "scene", "stage", "scenario", "state_ids", "rng_seed")},
            columns=differences[first], vector_errors=vector_errors(ar, br, policy))
        first_columns = [d["column"] for d in differences[first]]
        for key in all_keys:
            if first[0] - history <= key[0] < first[0] and key[1:] == first[1:]:
                report["history"].append(dict(**describe_key(key, actor_keys),
                    a={c: a.rows.get(key, {}).get(c) for c in first_columns},
                    b={c: b.rows.get(key, {}).get(c) for c in first_columns}))
        bad_frames = {key[0] for key in differences}
        all_frames = sorted({key[0] for key in all_keys})
        recovered = next((f for f in all_frames if f > first[0] and f not in bad_frames), None)
        counts = Counter(d["column"] for ds in differences.values() for d in ds)
        later_columns = {d["column"] for key, ds in differences.items() if key[0] > first[0] for d in ds}
        report["summary"] = dict(compared_frames=len(all_frames), compared_rows=len(all_keys),
            diverging_frames=len(bad_frames), later_diverging_frames=len(bad_frames - {first[0]}),
            diverging_rows=len(differences), diverging_columns=len(counts), later_diverging_columns=len(later_columns), column_occurrences=dict(counts),
            first_matching_frame_after_divergence=recovered, recovered_at_end=all_frames[-1] not in bad_frames,
            last_diverging_frame=max(bad_frames))
    else:
        report["summary"] = dict(compared_frames=len({k[0] for k in all_keys}), compared_rows=len(all_keys), diverging_frames=0, diverging_columns=0)
    report["exit_code"] = 2 if errors else 1 if differences else 0
    report["status"] = ("identical", "diverged", "invalid/incomplete")[report["exit_code"]]
    return report


def format_report(report):
    lines = [f"Trace comparison: {report['status']}"]
    for error in report["invalid_errors"]:
        lines.append("ERROR: " + error)
    first = report["first_divergence"]
    if first:
        identity = {k: v for k, v in first.items() if k not in ("columns", "context_a", "context_b", "vector_errors")}
        lines.append("First divergence: " + json.dumps(identity))
        lines.append("Context A/B: " + json.dumps([first["context_a"], first["context_b"]]))
        for diff in first["columns"]:
            lines.append("  " + json.dumps(diff))
        if first["vector_errors"]:
            lines.append("Vector errors: " + json.dumps(first["vector_errors"]))
        lines.append("Previous frames of first-diverging columns:")
        lines.extend("  " + json.dumps(row) for row in report["history"])
    if report["first_bit_difference"]:
        lines.append("First float bit difference (may be within tolerance): " + json.dumps(report["first_bit_difference"]))
    lines.append("Summary: " + json.dumps(report["summary"]))
    return "\n".join(lines)


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("a", type=Path)
    parser.add_argument("b", type=Path)
    parser.add_argument("--abs", dest="abs_tol", type=float, help="override default absolute budget for every float")
    parser.add_argument("--rel", dest="rel_tol", type=float, default=1e-6)
    parser.add_argument("--policy", type=Path, help='JSON map: {"x":{"type":"float","abs":0.01,"rel":1e-6}}')
    parser.add_argument("--history", type=int, default=120)
    parser.add_argument("--expected-frames", type=int, help="require a declared run length; detects identical truncation")
    parser.add_argument("--actors", action="store_true", help="one row per actor/frame; identity defaults to actor_name + actor_id")
    parser.add_argument("--actor-key", action="append", help="repeat for stable identity columns in actor mode")
    parser.add_argument("--json", type=str, metavar="PATH", help="write machine report; '-' prints only JSON")
    args = parser.parse_args(argv)
    if args.history < 0 or (args.actor_key and not args.actors) or (args.expected_frames is not None and args.expected_frames < 1):
        parser.error("history must be nonnegative; expected frames must be positive; --actor-key requires --actors")
    try:
        overrides = json.loads(args.policy.read_text()) if args.policy else {}
        if not isinstance(overrides, dict):
            raise TraceError("policy must be an object")
        policy = Policy(args.abs_tol, args.rel_tol, overrides)
        actor_keys = tuple(args.actor_key or ("actor_name", "actor_id")) if args.actors else ()
        report = compare(read_trace(args.a, actor_keys), read_trace(args.b, actor_keys), policy, args.history, actor_keys, args.expected_frames)
        report.update(trace_a=str(args.a), trace_b=str(args.b), actor_keys=list(actor_keys),
                      trace_a_sha256=sha256(args.a), trace_b_sha256=sha256(args.b),
                      policy=dict(abs_override=args.abs_tol, relative=args.rel_tol, columns=overrides), expected_frames=args.expected_frames)
        if args.json:
            output = json.dumps(report, indent=2, allow_nan=False) + "\n"
            if args.json == "-":
                print(output, end="")
            else:
                Path(args.json).write_text(output)
        if args.json != "-":
            print(format_report(report))
        return report["exit_code"]
    except (OSError, ValueError, csv.Error) as exc:
        print(f"Invalid trace: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
