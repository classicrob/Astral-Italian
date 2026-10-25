#!/usr/bin/env python3
"""Pack, expand and verify the pipeline seeds (native/tools/PIPELINE_SEEDING.md, "Repacking the seeds").

The seeds are Aurora pipeline-cache databases (<Stage>.db and __global__.db: GX/Clear render-state configs, no
shaders, textures, geometry or paths). Every stage database is a subset of __global__.db, so the whole set packs
losslessly into a few hundred KB, small enough to track in git as native/data/pipeline-seeds.v<N>.xz (N = the GX
config version). Only database rows are packed: never the .json provenance (it holds disc-file hashes and
local paths).

  seedpack.py pack SEED_DIR OUT.xz           write the pack and OUT.xz.sha256 (deterministic output)
  seedpack.py expand PACK.xz OUT_DIR [--force]   write the databases (and .seedpack.json marker) into OUT_DIR
  seedpack.py verify SEED_DIR_A SEED_DIR_B   every row, blob and load order of A's databases equals B's
  seedpack.py check PACK.xz                  .sha256 matches, the pack parses, and it holds no paths/hashes
  seedpack.py ensure --pack-dir DIR --out OUT_DIR   what the build runs: expand the newest pack into OUT_DIR
                                             when it is missing or still the unmodified expansion of an older
                                             pack; never overwrite local seeds; never fail (warns, exits 0)
"""
import argparse
import glob
import hashlib
import json
import lzma
import os
import re
import shutil
import sqlite3
import struct
import sys
from pathlib import Path
import pipeline_deadlines as deadlines

MAGIC = b"PSD1"
MAGIC_DEADLINES = b"PSD2"
MARKER = ".seedpack.json"
GLOBAL = "__global__"
NEVER = 0xFFFFFFFF
LEGACY_DEADLINE_SUFFIX = ".first-use.bin"  # generated-expansion migration only; never bundled/loaded
SCHEMA = (
    "CREATE TABLE aurora_schema(value INTEGER);"
    "CREATE TABLE pipeline_cache(type INTEGER, hash INTEGER, config_version INTEGER, config_size INTEGER, "
    "config BLOB, first_frame_used INTEGER, PRIMARY KEY(type,hash));"
    "CREATE INDEX pipeline_cache_load_order_idx ON pipeline_cache(first_frame_used);"
)
ROWS = "select type,hash,config_version,config_size,config,first_frame_used from pipeline_cache"


def sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def varint(n):
    out = bytearray()
    while True:
        low = n & 0x7F
        n >>= 7
        if n:
            out.append(low | 0x80)
        else:
            out.append(low)
            return bytes(out)


def read_varint(buf, at):
    n = shift = 0
    while True:
        byte = buf[at]
        at += 1
        n |= (byte & 0x7F) << shift
        shift += 7
        if not byte & 0x80:
            return n, at


def database_rows(path):
    connection = sqlite3.connect(f"file:{path}?mode=ro", uri=True)
    try:
        schema = connection.execute("select value from aurora_schema").fetchall()
        if schema != [(1,)]:
            raise ValueError(f"{path}: aurora_schema is {schema}, expected [(1,)]")
        return connection.execute(ROWS).fetchall()
    finally:
        connection.close()


def stage_paths(seed_dir):
    return sorted(p for p in glob.glob(os.path.join(seed_dir, "*.db")) if os.path.basename(p) != GLOBAL + ".db")


def pack(seed_dir, out):
    global_path = os.path.join(seed_dir, GLOBAL + ".db")
    if not os.path.isfile(global_path):
        raise ValueError(f"{global_path} is missing")
    rows = sorted(database_rows(global_path), key=lambda r: (r[0], r[1]))
    index = {(r[0], r[1]): i for i, r in enumerate(rows)}
    if len(index) != len(rows):
        raise ValueError("duplicate (type, hash) in the global database")
    for r in rows:
        if r[3] != len(r[4]) or r[3] > 0xFFFF or r[2] > 0xFFFF:
            raise ValueError("config size/version out of range or inconsistent")
    if glob.glob(os.path.join(seed_dir, '*' + LEGACY_DEADLINE_SUFFIX)):
        raise ValueError('legacy deadline filenames require a fresh .pfd expansion before packing')
    sidecars = {}
    stages = stage_paths(seed_dir)
    for path in stages:
        name = os.path.basename(path)[:-3]
        sidecar = os.path.join(seed_dir, name + '.pfd')
        if os.path.exists(sidecar):
            sidecars[name] = deadlines.read(sidecar, name, database_rows(path))
    unexpected = set(glob.glob(os.path.join(seed_dir, '*.pfd'))) - {
        os.path.join(seed_dir, os.path.basename(p)[:-3] + '.pfd') for p in stages}
    if unexpected:
        raise ValueError('deadline sidecar has no matching stage: ' + str(sorted(unexpected)))
    buf = bytearray((MAGIC_DEADLINES if sidecars else MAGIC) + struct.pack("<I", len(rows)))
    for r in rows:
        buf += struct.pack("<BqHHI", r[0], r[1], r[2], r[3], r[5])
    for r in rows:
        buf += r[4]
    buf += struct.pack("<I", len(stages))
    for path in stages:
        name = os.path.basename(path)[:-3].encode()
        if not re.fullmatch(rb"[A-Za-z0-9_.-]{1,80}", name):
            raise ValueError(f"unsafe stage name {name!r}")
        stage_rows = database_rows(path)
        buf += struct.pack("<B", len(name)) + name + varint(len(stage_rows))
        for t, h, v, s, c, f in sorted(stage_rows, key=lambda r: (r[5], r[0], r[1])):
            known = index.get((t, h))
            if known is None or rows[known][2] != v or rows[known][4] != c:
                raise ValueError(f"{name.decode()}: row ({t}, {h}) is not in {GLOBAL}.db with the same config")
            buf += varint(known) + varint(0 if f == NEVER else f + 1)
    if sidecars:
        buf += struct.pack('<I', len(sidecars))
        for stage_index, path in enumerate(stages):
            name = os.path.basename(path)[:-3]
            if name not in sidecars:
                continue
            entries = sidecars[name]
            buf += varint(stage_index) + varint(len(entries))
            for key, frame in sorted(entries.items()):
                signed = key - (1 << 64) if key >= (1 << 63) else key
                buf += varint(index[(1, signed)]) + varint(frame)
    data = lzma.compress(bytes(buf), format=lzma.FORMAT_XZ, check=lzma.CHECK_CRC64, preset=9)
    with open(out, "wb") as stream:
        stream.write(data)
    with open(out + ".sha256", "w") as stream:
        stream.write(f"{hashlib.sha256(data).hexdigest()}  {os.path.basename(out)}\n")
    versions = sorted({r[2] for r in rows if r[0] == 1})
    return {"bytes": len(data), "configs": len(rows), "stages": len(stages), "gx_config_versions": versions,
            "deadline_stages": len(sidecars)}


def parse(pack_path):
    return parse_full(pack_path)[:3]


def parse_full(pack_path):
    buf = lzma.decompress(Path(pack_path).read_bytes())
    if buf[:4] not in (MAGIC, MAGIC_DEADLINES):
        raise ValueError("not a seed pack")
    count, = struct.unpack_from("<I", buf, 4)
    if not 0 < count <= 65536:
        raise ValueError('invalid global config count')
    at = 8
    meta = [struct.unpack_from("<BqHHI", buf, at + 17 * i) for i in range(count)]
    at += 17 * count
    blobs = []
    for m in meta:
        blobs.append(buf[at:at + m[3]])
        at += m[3]
    stage_count, = struct.unpack_from("<I", buf, at)
    if not 0 < stage_count <= 4096:
        raise ValueError('invalid stage count')
    at += 4
    stages = []
    for _ in range(stage_count):
        length = buf[at]
        at += 1
        name = buf[at:at + length].decode()
        if not re.fullmatch(r'[A-Za-z0-9_.-]{1,80}', name) or name in {s[0] for s in stages}:
            raise ValueError('invalid/duplicate stage name')
        at += length
        rows, at = read_varint(buf, at)
        if rows > count:
            raise ValueError('too many stage configs')
        selected = []
        seen = set()
        for _ in range(rows):
            i, at = read_varint(buf, at)
            f, at = read_varint(buf, at)
            if i >= count or i in seen or f > NEVER:
                raise ValueError("stage index out of range")
            seen.add(i)
            selected.append((i, NEVER if f == 0 else f - 1))
        stages.append((name, selected))
    sidecars = {}
    if buf[:4] == MAGIC_DEADLINES:
        sidecar_count, = struct.unpack_from('<I', buf, at)
        at += 4
        if not 0 < sidecar_count <= len(stages):
            raise ValueError('invalid deadline stage count')
        for _ in range(sidecar_count):
            stage_index, at = read_varint(buf, at)
            count, at = read_varint(buf, at)
            if stage_index >= len(stages) or not 0 < count <= deadlines.LIMIT:
                raise ValueError('invalid deadline stage/count')
            name, selected = stages[stage_index]
            if name in sidecars:
                raise ValueError('duplicate deadline stage')
            members = {i for i, _ in selected}
            entries = {}
            for _ in range(count):
                index, at = read_varint(buf, at)
                frame, at = read_varint(buf, at)
                if index not in members or meta[index][0] != 1 or meta[index][2] != 13 or frame >= NEVER:
                    raise ValueError('unknown deadline config/version/frame')
                key = meta[index][1] & deadlines.MASK
                if key in entries:
                    raise ValueError('duplicate deadline config')
                entries[key] = frame
            sidecars[name] = entries
    if at != len(buf):
        raise ValueError("trailing bytes in the pack")
    return meta, blobs, stages, sidecars


def write_database(path, meta, blobs, selected):
    if os.path.exists(path):
        os.remove(path)
    db = sqlite3.connect(path)
    db.executescript(SCHEMA)
    db.execute("insert into aurora_schema values(1)")
    db.executemany("insert into pipeline_cache values(?,?,?,?,?,?)",
                   ((meta[i][0], meta[i][1], meta[i][2], meta[i][3], blobs[i], f) for i, f in selected))
    db.commit()
    db.close()


def expand(pack_path, out_dir, pack_hash=None):
    """Writes the databases into out_dir (created; must not hold other files) and the marker."""
    meta, blobs, stages, sidecars = parse_full(pack_path)
    os.makedirs(out_dir, exist_ok=True)
    write_database(os.path.join(out_dir, GLOBAL + ".db"), meta, blobs, [(i, meta[i][4]) for i in range(len(meta))])
    for name, selected in stages:
        write_database(os.path.join(out_dir, name + ".db"), meta, blobs, selected)
        if name in sidecars:
            rows = [(meta[i][0], meta[i][1], meta[i][2], meta[i][3], blobs[i], f) for i, f in selected]
            with open(os.path.join(out_dir, name + '.pfd'), 'wb') as stream:
                stream.write(deadlines.encode(name, rows, sidecars[name]))
    marker = {"pack_sha256": pack_hash or sha256(pack_path), "global_sha256": sha256(os.path.join(out_dir, GLOBAL + ".db")),
              "databases": sorted(os.path.basename(p)[:-3] for p in glob.glob(os.path.join(out_dir, "*.db"))),
              "deadline_sidecars": {os.path.basename(p): sha256(p) for p in glob.glob(os.path.join(out_dir, '*.pfd'))}}
    with open(os.path.join(out_dir, MARKER), "w") as stream:
        json.dump(marker, stream, indent=1)
        stream.write("\n")
    return len(stages)


def verify(a, b):
    bad = 0
    for directory in (a, b):
        if glob.glob(os.path.join(directory, '*' + LEGACY_DEADLINE_SUFFIX)):
            print('legacy deadline filenames cannot be verified as .pfd:', directory)
            bad += 1
    names = sorted(os.path.basename(p) for p in glob.glob(os.path.join(a, "*.db")))
    for name in names:
        other = os.path.join(b, name)
        if not os.path.exists(other):
            print("missing", other)
            bad += 1
            continue
        if sorted(database_rows(os.path.join(a, name))) != sorted(database_rows(other)):
            print("DIFF", name)
            bad += 1
    extra = sorted(set(os.path.basename(p) for p in glob.glob(os.path.join(b, "*.db"))) - set(names))
    for name in extra:
        print("extra", name)
        bad += 1
    sidecars = {os.path.basename(p) for p in glob.glob(os.path.join(a, '*.pfd'))}
    other_sidecars = {os.path.basename(p) for p in glob.glob(os.path.join(b, '*.pfd'))}
    for name in sorted(sidecars | other_sidecars):
        if name not in sidecars or name not in other_sidecars or Path(a, name).read_bytes() != Path(b, name).read_bytes():
            print('DIFF deadline sidecar', name)
            bad += 1
    print(f"verified {len(names)} databases, {bad} problems")
    return bad


FORBIDDEN = [(re.compile(rb"/Users/|/home/|[A-Za-z]:\\\\"), "an absolute path"), (re.compile(rb"[0-9a-f]{64}"), "a 64-hex digest (disc file hash)"),
             (re.compile(rb"\{\s*\""), "JSON provenance")]


def check(pack_path):
    problems = []
    sha_file = pack_path + ".sha256"
    if not os.path.isfile(sha_file):
        problems.append("missing " + sha_file)
    else:
        recorded = Path(sha_file).read_text().split()[0]
        if recorded != sha256(pack_path):
            problems.append("sha256 does not match the pack")
    try:
        meta, blobs, stages = parse(pack_path)
    except Exception as error:  # noqa: BLE001 - any parse failure is a problem to report
        return problems + [f"pack does not parse: {error}"]
    raw = lzma.decompress(Path(pack_path).read_bytes())
    for pattern, what in FORBIDDEN:
        found = pattern.search(raw)
        if found:
            problems.append(f"the pack contains {what}: {found.group(0)[:40]!r}")
    if not meta or not stages:
        problems.append("the pack is empty")
    return problems


def newest_pack(pack_dir):
    def version(path):
        match = re.search(r"\.v(\d+)\.xz$", path)
        return int(match.group(1)) if match else -1
    packs = [p for p in glob.glob(os.path.join(pack_dir, "pipeline-seeds.v*.xz"))]
    return max(packs, key=version) if packs else None


def local_state(out_dir):
    """('absent'|'local'|'expansion'|'modified', marker) for OUT_DIR."""
    if not os.path.isdir(out_dir) or not glob.glob(os.path.join(out_dir, "*.db")):
        return "absent", None
    marker_path = os.path.join(out_dir, MARKER)
    if not os.path.isfile(marker_path):
        return "local", None
    try:
        marker = json.loads(Path(marker_path).read_text())
        current = sorted(os.path.basename(p)[:-3] for p in glob.glob(os.path.join(out_dir, "*.db")))
        sidecars = {os.path.basename(p): sha256(p) for pattern in ('*.pfd', '*' + LEGACY_DEADLINE_SUFFIX)
                    for p in glob.glob(os.path.join(out_dir, pattern))}
        if marker["databases"] == current and marker["global_sha256"] == sha256(os.path.join(out_dir, GLOBAL + ".db")) and marker.get('deadline_sidecars', {}) == sidecars:
            return "expansion", marker
    except (OSError, ValueError, KeyError):
        pass
    return "modified", None


def ensure(pack_dir, out_dir):
    pack_path = newest_pack(pack_dir)
    if pack_path is None:
        print(f"pipeline seeds: no pack in {pack_dir}; nothing to expand")
        return 0
    problems = check(pack_path)
    if problems:
        print(f"pipeline seeds: WARNING: {pack_path} is unusable ({'; '.join(problems)}); not expanding. "
              "The game still runs; shaders compile on first use.")
        return 0
    state, marker = local_state(out_dir)
    pack_hash = sha256(pack_path)
    if state == "local":
        print(f"pipeline seeds: {out_dir} holds locally generated seeds (no {MARKER}); left untouched. "
              f"To use the tracked pack instead: seedpack.py expand --force {os.path.basename(pack_path)} {out_dir}")
        return 0
    if state == "modified":
        print(f"pipeline seeds: WARNING: {out_dir} changed since it was expanded from a pack; left untouched, so a newer "
              f"{os.path.basename(pack_path)} is NOT applied. To replace it: seedpack.py expand --force")
        return 0
    if state == "expansion" and marker["pack_sha256"] == pack_hash and all(name.endswith(".pfd") for name in marker.get("deadline_sidecars", {})):
        print(f"pipeline seeds: {out_dir} is up to date ({os.path.basename(pack_path)})")
        return 0
    try:
        staging = out_dir.rstrip("/") + ".expanding"
        shutil.rmtree(staging, ignore_errors=True)
        count = expand(pack_path, staging, pack_hash)
        shutil.rmtree(out_dir, ignore_errors=True)
        os.replace(staging, out_dir)
        print(f"pipeline seeds: expanded {os.path.basename(pack_path)} into {out_dir} ({count} stages + {GLOBAL})")
    except Exception as error:  # noqa: BLE001 - the build must never fail because of seeds
        shutil.rmtree(out_dir.rstrip("/") + ".expanding", ignore_errors=True)
        print(f"pipeline seeds: WARNING: could not expand {pack_path}: {error}. The game still runs; shaders compile on first use.")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("pack"); p.add_argument("seed_dir"); p.add_argument("out")
    p = sub.add_parser("expand"); p.add_argument("pack"); p.add_argument("out_dir"); p.add_argument("--force", action="store_true")
    p = sub.add_parser("verify"); p.add_argument("a"); p.add_argument("b")
    p = sub.add_parser("check"); p.add_argument("pack")
    p = sub.add_parser("ensure"); p.add_argument("--pack-dir", required=True); p.add_argument("--out", required=True)
    args = parser.parse_args()
    if args.command == "pack":
        print(json.dumps(pack(args.seed_dir, args.out)))
    elif args.command == "expand":
        if os.path.isdir(args.out_dir) and glob.glob(os.path.join(args.out_dir, "*")) and not args.force:
            sys.exit(f"{args.out_dir} is not empty; use --force to replace it")
        if args.force:
            shutil.rmtree(args.out_dir, ignore_errors=True)
        print(f"expanded {expand(args.pack, args.out_dir)} stages")
    elif args.command == "verify":
        sys.exit(1 if verify(args.a, args.b) else 0)
    elif args.command == "check":
        problems = check(args.pack)
        print("\n".join(problems) if problems else "pack ok")
        sys.exit(1 if problems else 0)
    elif args.command == "ensure":
        sys.exit(ensure(args.pack_dir, args.out))


if __name__ == "__main__":
    main()
