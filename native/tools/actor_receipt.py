"""Validate native actor sidecars and bind the actual capture to launcher metadata.

This receipt certifies file/census consistency, not cross-producer equivalence or
actor lifetime. The launcher must additionally validate its main trace/timing,
process exit status, and diagnostic-placeholder policy before accepting a run.
"""
import csv
import hashlib
import itertools
import math
from pathlib import Path
import re
import struct

CONTEXT = ["schema_version", "frame", "scene_epoch", "scene", "stage", "scenario", "init_state"]
VECTORS = ["x", "y", "z", "rot_x", "rot_y", "rot_z", "scale_x", "scale_y", "scale_z"]
ACTOR_COLUMNS = CONTEXT + [
    "producer_handle", "actor_id", "identity_kind", "identity_status", "name_valid", "name_hex",
    "link_valid", "link_id", "link_zone", "link_context", "dead",
] + [column for value in VECTORS for column in (value, value + "_bits")]
ACTOR_FRAME_COLUMNS = CONTEXT + [
    "ready", "registered", "living", "emitted", "identity_unresolved", "identity_ambiguous",
]


def _uint(value):
    if not isinstance(value, str) or not re.fullmatch(r"0|[1-9][0-9]*", value):
        raise ValueError("noncanonical actor receipt integer")
    result = int(value)
    if result > 0xffffffffffffffff:
        raise ValueError("actor receipt integer exceeds u64")
    return result


def _flag(value):
    if value not in ("0", "1"):
        raise ValueError("noncanonical actor receipt flag")
    return value == "1"


def _rows(path, columns):
    with path.open(encoding="utf-8", newline="") as source:
        rows = csv.reader(source, strict=True)
        if next(rows, None) != columns:
            raise ValueError("actor receipt CSV header mismatch: " + str(path))
        for row in rows:
            if len(row) != len(columns):
                raise ValueError("actor receipt CSV width mismatch")
            yield dict(zip(columns, row))


def _digest(path):
    result = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            result.update(block)
    return result.hexdigest()


def _actor(row):
    if _uint(row["producer_handle"]) == 0:
        raise ValueError("zero actor handle")
    name_valid, linked = _flag(row["name_valid"]), _flag(row["link_valid"])
    name = row["name_hex"]
    if not re.fullmatch(r"(?:[0-9a-f]{2}){0,1023}", name) or (not name_valid and name):
        raise ValueError("invalid actor name encoding/extent")
    if b"\0" in bytes.fromhex(name):
        raise ValueError("embedded actor name NUL")
    links = []
    for key in ("link_id", "link_zone", "link_context"):
        value = int(row[key])
        if str(value) != row[key] or not -32768 <= value <= 32767:
            raise ValueError("invalid signed actor link")
        links.append(value)
    if linked != all(value >= 0 for value in links):
        raise ValueError("actor link predicate mismatch")
    kind, status = row["identity_kind"], row["identity_status"]
    if kind == "player":
        candidate = "player"
    elif kind == "link" and linked and name_valid:
        candidate = f"link:{links[1]}:{links[2]}:{links[0]}:{name}"
    elif kind == "unresolved" and not (linked and name_valid):
        candidate = ""
    else:
        raise ValueError("actor identity kind inconsistent with fields")
    if status not in ("source-key", "unresolved", "ambiguous", "key-change", "key-reuse"):
        raise ValueError("unknown actor identity status")
    if status == "source-key":
        if not candidate or row["actor_id"] != candidate:
            raise ValueError("source key does not match actor fields")
    elif row["actor_id"] or (status in ("ambiguous", "key-reuse") and not candidate) or (status == "unresolved" and candidate):
        raise ValueError("identity HOLD fields inconsistent")
    _flag(row["dead"])
    for field in VECTORS:
        bits = row[field + "_bits"]
        if not re.fullmatch(r"0x[0-9a-f]{8}", bits):
            raise ValueError("invalid f32 bit encoding")
        value = float(row[field])
        try:
            encoded = struct.pack(">f", value)
        except OverflowError as exc:
            raise ValueError("actor decimal exceeds f32") from exc
        if not math.isfinite(value) or encoded.hex() != bits[2:]:
            raise ValueError("actor f32 decimal/bit mismatch")
    return candidate


def collect_actor_receipt(trace_path, main_columns):
    """Return manifest.actors only for a complete, reconciled native capture.

    On failure raise (ValueError, OSError, csv.Error); the launcher must mark the
    run invalid/incomplete, never fill unavailable counts with invented zeros.
    ``main_columns`` must be the launcher's canonical 86-column trace schema.
    """
    trace = Path(trace_path)
    marker = Path(str(trace) + ".complete")
    paths = [Path(str(trace) + ".actors.csv"), Path(str(trace) + ".actor-frames.csv")]
    if len(main_columns) != 86 or len(set(main_columns)) != 86:
        raise ValueError("canonical main trace schema required")
    for path in [trace, marker] + paths:
        if not path.is_file() or path.is_symlink():
            raise ValueError("missing/nonregular actor capture artifact")
    if any(Path(str(path) + ".partial").exists() or Path(str(path) + ".partial").is_symlink() for path in paths):
        raise ValueError("actor capture still has partial files")
    marker_text = marker.read_text(encoding="ascii")
    if not re.fullmatch(r"[1-9][0-9]*\n", marker_text):
        raise ValueError("invalid main completion count")
    expected = _uint(marker_text.rstrip("\n"))
    before = [_digest(path) for path in paths]
    actors = _rows(paths[0], ACTOR_COLUMNS)
    pending = next(actors, None)
    frames = actor_rows = holds = 0
    previous_epoch = -1
    for main, census in itertools.zip_longest(_rows(trace, main_columns), _rows(paths[1], ACTOR_FRAME_COLUMNS)):
        if main is None or census is None or main["frame"] != str(frames) or census["frame"] != str(frames):
            raise ValueError("main/census frames are not dense and equal")
        if census["schema_version"] != "1" or any(census[key] != main[key] for key in CONTEXT[1:]):
            raise ValueError("actor census/main context mismatch")
        epoch = _uint(census["scene_epoch"])
        if epoch < previous_epoch:
            raise ValueError("actor epoch moved backwards")
        previous_epoch = epoch
        ready = _flag(census["ready"])
        if ready != (_flag(main["scene_valid"]) and main["scene"] == "Game" and main["init_state"] == "6" and _flag(main["actor_count_valid"])):
            raise ValueError("actor/main readiness mismatch")
        emitted, living, unresolved, ambiguous = 0, 0, 0, 0
        handles, identities, players = set(), set(), []
        while pending is not None and _uint(pending["frame"]) == frames:
            if any(pending[key] != census[key] for key in CONTEXT):
                raise ValueError("actor row/census context mismatch")
            _actor(pending)
            handle, identity = pending["producer_handle"], pending["actor_id"]
            if handle in handles or (identity and identity in identities):
                raise ValueError("duplicate actor handle/qualified identity")
            handles.add(handle)
            if identity:
                identities.add(identity)
            if pending["identity_kind"] == "player":
                players.append(pending)
            status = pending["identity_status"]
            ambiguous += status == "ambiguous"
            unresolved += status not in ("source-key", "ambiguous")
            living += not _flag(pending["dead"])
            emitted += 1
            pending = next(actors, None)
        if pending is not None and _uint(pending["frame"]) < frames:
            raise ValueError("actor rows out of frame order")
        if emitted > 32768 or any(_uint(census[key]) != count for key, count in (
                ("emitted", emitted), ("identity_unresolved", unresolved), ("identity_ambiguous", ambiguous))):
            raise ValueError("actor census totals mismatch")
        if ready:
            if _uint(census["registered"]) != emitted or _uint(census["living"]) != living or main["actor_registered"] != str(emitted) or main["actor_living"] != str(living):
                raise ValueError("registered/living counts disagree")
            if len(players) > 1:
                raise ValueError("multiple typed player actors")
            if _flag(main["player_valid"]):
                if len(players) != 1 or any(players[0][key] != main[key] for key in ("x_bits", "y_bits", "z_bits", "dead")):
                    raise ValueError("typed player does not match main observation")
        elif emitted or census["registered"] or census["living"]:
            raise ValueError("unready census claims actor membership")
        actor_rows += emitted
        holds += unresolved + ambiguous
        frames += 1
    if pending is not None or frames != expected:
        raise ValueError("extra actor rows or completion frame mismatch")
    after = [_digest(path) for path in paths]
    if before != after or marker.read_text(encoding="ascii") != marker_text:
        raise ValueError("actor artifacts changed during receipt")
    return dict(schema_version=1, status="captured", frames=frames, actor_rows=actor_rows,
                identity_holds=holds, qualification="identity-hold" if holds else "sampled-identities-only",
                files=[dict(path=path.name, sha256=digest) for path, digest in zip(paths, after)])
