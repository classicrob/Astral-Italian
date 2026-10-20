#!/usr/bin/env python3
"""Native (clang arm64) syntax check of src/ files against the headless compile DB.

The CMake build compiles Shift-JIS-encoded copies (build/<dir>/legacy-sjis/src/...),
so this re-encodes the CURRENT worktree into a private dir first, then runs each
file's exact DB command with -fsyntax-only (never touches the build dir).

  python3 tools/team/nativecheck.py src/Game/System/GameSystem.cpp [...]
  python3 tools/team/nativecheck.py --db build/macos-link-check --all-errors FILE
"""
import argparse, json, os, shlex, subprocess, sys, tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("files", nargs="+")
    ap.add_argument("--db", default="build/macos-arm64-headless")
    ap.add_argument("--all-errors", action="store_true")
    ap.add_argument("-I", dest="extra_inc", action="append", default=[])
    a = ap.parse_args()
    dbdir = (ROOT / a.db).resolve()
    db = json.load(open(dbdir / "compile_commands.json"))
    enc = Path(tempfile.mkdtemp(prefix="nativecheck-"))
    subprocess.run([sys.executable, str(ROOT / "native/tools/encode_legacy_sources.py"),
                    "--root", str(ROOT), "--output", str(enc)], check=True, capture_output=True)
    legacy = str(dbdir / "legacy-sjis")
    bad = 0
    for f in a.files:
        rel = os.path.relpath(Path(f).resolve(), ROOT)
        entries = [e for e in db if e["file"].endswith("/" + rel)]
        new_file = not entries
        if new_file:  # new TU: borrow a same-directory neighbour's command
            d = os.path.dirname(rel)
            entries = [e for e in db if os.path.dirname(e["file"]).endswith("/" + d)]
            if not entries:
                print(f"{rel}: NOT IN DB"); bad += 1; continue
        e = entries[0]
        args = e["arguments"] if "arguments" in e else shlex.split(e["command"])
        if new_file:
            args = [str(enc / rel) if t == e["file"] else t for t in args]
        out, skip = [], False
        for t in args:
            if skip:
                skip = False; continue
            if t == "-o":
                skip = True; continue
            out.append(t.replace(legacy, str(enc)))
        out += ["-fsyntax-only"] + [x for i in a.extra_inc for x in ("-I", i)]
        r = subprocess.run(out, cwd=e["directory"], capture_output=True, text=True)
        errs = [l for l in r.stderr.splitlines() if "error:" in l]
        print(f"{rel}: {'OK' if r.returncode == 0 else 'FAIL'} ({len(errs)} errors)")
        for l in errs if a.all_errors else errs[:8]:
            print("   " + l.replace(str(enc) + "/", "").replace(str(ROOT) + "/", ""))
        bad += r.returncode != 0
    subprocess.run(["rm", "-rf", str(enc)])
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
