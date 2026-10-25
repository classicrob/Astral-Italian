#!/usr/bin/env python3
"""Release gate: fail if an app bundle (or a zip of one) could contain game data, or is not self-contained.

    python3 -I scan_bundle.py [--max-total-mb N] [--min-macos X.Y] PATH.app | PATH.zip

Checks, each a hard failure:
  * the file type allowlist: only Mach-O code, property lists, SQLite shader seeds, icons, text and signatures;
  * disc-content names and extensions (arc, szs, bcsv, dol, iso, ...) and `files/`, `sys/`, `DATA/` directories;
  * disc-format magic numbers at the start of any non-Mach-O file (Yaz0, RARC, U.8, Wii/GameCube disc header,
    WBFS, RVZ/WIA, CISO, THP, ...), and the game ID inside non-Mach-O files;
  * size limits (a single non-Mach-O file, the whole bundle);
  * Mach-O files: every segment is an ordinary one (a blob linked into an odd segment is flagged), the main
    binary and libraries load nothing from /opt/homebrew, /usr/local or the build machine, and the deployment
    target (minos) does not exceed --min-macos.
It is a screen, not a proof: the real guarantee is that the release script never copies anything but the
frozen build products. It exists so that an accident is caught before upload.
"""
import argparse
import json
import os
import plistlib
import re
import struct
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

DENY_EXTENSIONS = {
    ".arc", ".szs", ".rarc", ".bcsv", ".bti", ".brres", ".brlyt", ".brlan", ".brctr", ".bnr", ".dol", ".iso",
    ".wbfs", ".rvz", ".wia", ".gcm", ".ciso", ".nfs", ".thp", ".brstm", ".bfstm", ".brsar", ".brwsd", ".bwav",
    ".msbt", ".msbf", ".kcl", ".pa", ".tpl", ".bdl", ".bmd", ".bck", ".btk", ".btp", ".brk", ".bva", ".bpk",
    ".blo", ".jpc", ".jpa", ".ymd", ".bin", ".img", ".pak", ".zip", ".gz", ".xz", ".7z", ".rar",
}
DENY_DIRS = {"files", "sys", "data", "gamedata", "game data", "disc", "stagedata", "objectdata", "audioress"}
ALLOW_EXTENSIONS = {
    "", ".dylib", ".plist", ".db", ".json", ".txt", ".md", ".icns", ".png", ".strings", ".lproj", ".car",
    ".html", ".command", ".sh", ".entitlements", ".license", ".pfd",
}
# Text mentions of the game ID are fine in a README; binary files are checked for it separately.
TEXT_EXTENSIONS = {".txt", ".md", ".json", ".plist", ".strings", ".html", ".command", ".sh", ".entitlements", ".license"}
MAGIC_PREFIXES = {
    b"Yaz0": "Yaz0-compressed game data",
    b"RARC": "RARC archive",
    b"U\xaa8-": "U8 archive",
    b"WBFS": "WBFS disc container",
    b"RVZ\x01": "RVZ disc image",
    b"WIA\x01": "WIA disc image",
    b"CISO": "CISO disc image",
    b"THP\x00": "THP movie",
    b"J3D2": "J3D model",
    b"bres": "BRRES archive",
    b"RSAR": "BRSAR sound archive",
    b"SARC": "SARC archive",
    b"\x00 \xaf0": "TPL texture library",
}
GAME_ID = re.compile(rb"SB4[A-Z]01|SUPER MARIO GALAXY")
MACHO_MAGICS = {0xFEEDFACF, 0xCFFAEDFE, 0xCAFEBABE, 0xBEBAFECA}
OK_SEGMENTS = {"__PAGEZERO", "__TEXT", "__DATA", "__DATA_CONST", "__LINKEDIT", "__OBJC", "__AUTH", "__AUTH_CONST", "__UNICODE"}
BAD_LOAD_PREFIXES = ("/opt/homebrew", "/usr/local", "/Users/", "/private/", "/tmp/", "/var/")
MAX_PLAIN_FILE = 64 * 1024 * 1024


def is_macho(path):
    try:
        with open(path, "rb") as f:
            head = f.read(4)
    except OSError:
        return False
    return len(head) == 4 and struct.unpack(">I", head)[0] in MACHO_MAGICS


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, check=False)


def macho_info(path):
    """(segments, load_paths, rpaths, minos) from otool."""
    segs, loads, rpaths, minos = [], [], [], None
    out = run("otool", "-l", str(path)).stdout
    lines = out.splitlines()
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith("segname "):
            segs.append(s.split(None, 1)[1])
        elif s.startswith("cmd LC_RPATH"):
            for follow in lines[i + 1:i + 4]:
                m = re.match(r"\s*path (.+) \(offset", follow)
                if m:
                    rpaths.append(m.group(1))
        elif s.startswith("cmd LC_BUILD_VERSION"):
            for follow in lines[i + 1:i + 8]:
                m = re.match(r"\s*minos (\S+)", follow)
                if m:
                    minos = m.group(1)
        elif s.startswith("cmd LC_VERSION_MIN_MACOSX"):
            for follow in lines[i + 1:i + 4]:
                m = re.match(r"\s*version (\S+)", follow)
                if m:
                    minos = m.group(1)
    for line in run("otool", "-L", str(path)).stdout.splitlines()[1:]:
        m = re.match(r"\s+(\S+) \(", line)
        if m:
            loads.append(m.group(1))
    return segs, loads, rpaths, minos


def version_tuple(text):
    return tuple(int(p) for p in re.findall(r"\d+", text))


def scan(root, min_macos=None, max_total_mb=700):
    problems, notes = [], []
    root = Path(root)
    total = 0
    for dirpath, dirnames, filenames in os.walk(root):
        rel_dir = Path(dirpath).relative_to(root)
        for d in dirnames:
            if d.lower() in DENY_DIRS:
                problems.append(f"{rel_dir / d}: directory name used for disc data")
        for name in filenames:
            path = Path(dirpath) / name
            rel = path.relative_to(root)
            if path.is_symlink():
                target = os.readlink(path)
                resolved = (path.parent / target).resolve()
                try:
                    resolved.relative_to(root.resolve())
                except ValueError:
                    problems.append(f"{rel}: symlink leaves the bundle ({target})")
                continue
            size = path.stat().st_size
            total += size
            ext = path.suffix.lower()
            in_signature = "_CodeSignature" in rel.parts
            if ext in DENY_EXTENSIONS:
                problems.append(f"{rel}: extension {ext} is game/disc data")
                continue
            macho = is_macho(path)
            if macho:
                check_macho(rel, path, min_macos, problems, notes)
                continue
            if in_signature:
                continue
            if ext not in ALLOW_EXTENSIONS:
                problems.append(f"{rel}: file type {ext or '(none)'} is not on the allowlist")
                continue
            if size > MAX_PLAIN_FILE:
                problems.append(f"{rel}: {size // 1048576} MiB is too large for a non-code file")
            with open(path, "rb") as f:
                head = f.read(0x40)
                if len(head) >= 0x1C and head[0x18:0x1C] == b"\x5d\x1c\x9e\xa3":
                    problems.append(f"{rel}: Wii disc header")
                if len(head) >= 0x20 and head[0x1C:0x20] == b"\xc2\x33\x9f\x3d":
                    problems.append(f"{rel}: GameCube disc header")
                for magic, what in MAGIC_PREFIXES.items():
                    if head.startswith(magic):
                        problems.append(f"{rel}: starts like {what}")
                if size <= 16 * 1024 * 1024 and ext not in TEXT_EXTENSIONS:
                    f.seek(0)
                    if GAME_ID.search(f.read()):
                        problems.append(f"{rel}: contains the game ID/title (disc-derived data?)")
            if ext == ".db":
                with open(path, "rb") as f:
                    if f.read(16) != b"SQLite format 3\x00":
                        problems.append(f"{rel}: .db that is not SQLite")
    if total > max_total_mb * 1048576:
        problems.append(f"bundle is {total // 1048576} MiB, over the {max_total_mb} MiB limit")
    notes.append(f"scanned {total / 1048576:.1f} MiB")
    return problems, notes


def check_macho(rel, path, min_macos, problems, notes):
    segs, loads, rpaths, minos = macho_info(path)
    for seg in segs:
        if seg not in OK_SEGMENTS:
            problems.append(f"{rel}: unusual Mach-O segment {seg}")
    for dep in loads:
        if dep.startswith(BAD_LOAD_PREFIXES):
            problems.append(f"{rel}: loads {dep} (not available on a clean Mac)")
        elif dep.startswith("@rpath/"):
            contents = next((q for q in Path(path).parents if q.name == "Contents"), None)
            if dep[7:] != path.name and not (contents and (contents / "Frameworks" / dep[7:]).exists()):
                problems.append(f"{rel}: {dep} is not in Contents/Frameworks")
        elif not dep.startswith(("/usr/lib/", "/System/", "@executable_path/", "@loader_path/")):
            problems.append(f"{rel}: unexpected load path {dep}")
    for rp in rpaths:
        if rp.startswith(BAD_LOAD_PREFIXES):
            problems.append(f"{rel}: rpath {rp} points outside the bundle")
    if min_macos and minos and version_tuple(minos) > version_tuple(min_macos):
        problems.append(f"{rel}: minos {minos} is above the target {min_macos}")
    if minos is None:
        notes.append(f"{rel}: no minos found")


def scan_target(target, **kw):
    target = Path(target)
    if target.suffix.lower() == ".zip":
        with tempfile.TemporaryDirectory(prefix="scan-bundle-") as tmp:
            # ditto preserves symlinks and modes the way macOS unzip would
            r = run("ditto", "-x", "-k", str(target), tmp)
            if r.returncode != 0:
                return [f"cannot unpack {target}: {r.stderr.strip()}"], []
            return scan(tmp, **kw)
    return scan(target, **kw)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("target", help="an .app directory, a directory containing one, or a .zip")
    ap.add_argument("--min-macos", help="fail when any Mach-O minos is above this (e.g. 14.0)")
    ap.add_argument("--max-total-mb", type=int, default=700)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()
    problems, notes = scan_target(args.target, min_macos=args.min_macos, max_total_mb=args.max_total_mb)
    if args.json:
        print(json.dumps({"ok": not problems, "problems": problems, "notes": notes}, indent=1))
    else:
        for n in notes:
            print("note:", n)
        for p in problems:
            print("FAIL:", p)
        print("scan_bundle:", "FAILED" if problems else "clean")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
