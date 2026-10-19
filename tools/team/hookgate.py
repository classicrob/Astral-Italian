#!/usr/bin/env python3
"""Compile gate for tools/team/hooktransfer.py: apply the transfer, force-rebuild everything it touched with
MWCC (nj.sh -k 0), and exclude each transferred file that an MWCC error points at; repeat until the Wii build
is clean. Then run tools/team/hookproof.sh for the object-identity proof.

A hook can be Wii-neutral in isolation yet break the Wii build in SMG2: its use of a macro or typedef is
neutral, but the definition lives in a hunk that did not transfer, or in a header the SMG2 file does not
include. Only a real compile finds these.
Usage: tools/team/hookgate.py --out DIR [hooktransfer args...]   (run from the worktree root, clean src tree)
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

TOPS = ["src", "include", "libs"]


def sh(cmd, **kw):
    return subprocess.run(cmd, text=True, capture_output=True, **kw)


def changed_files():
    mod = sh(["git", "diff", "--name-only", "--"] + TOPS).stdout.split()
    new = sh(["git", "ls-files", "--others", "--exclude-standard", "--"] + TOPS).stdout.split()
    return mod, new


def restore():
    sh(["git", "checkout", "--"] + TOPS)
    for f in changed_files()[1]:
        Path(f).unlink()


def error_locations(log):
    """(location, translation unit) of every MWCC error block."""
    out = []
    for block in log.split("### mwcceppc.exe Compiler:"):
        if "Error:" not in block:
            continue
        where = re.search(r"^#\s+(?:In|File):\s*(\S+)", block, re.M)
        tu = re.search(r"^#\s+(?:From|File):\s*(\S+)", block, re.M)
        if where:
            out.append((where.group(1).replace("\\", "/"), tu.group(1).replace("\\", "/") if tu else None))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--max-rounds", type=int, default=8)
    args, rest = ap.parse_known_args()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    exclude = out / "excluded.txt"
    excluded = set(exclude.read_text().split()) if exclude.exists() else set()
    if sh(["git", "status", "--porcelain", "--untracked-files=no", "--"] + TOPS).stdout.strip():
        sys.exit("src/include/libs must be clean (commit or revert first)")
    for rnd in range(1, args.max_rounds + 1):
        restore()
        exclude.write_text("".join(f + "\n" for f in sorted(excluded)))
        r = sh([sys.executable, "tools/team/hooktransfer.py", "--out", str(out), "--apply",
                "--exclude-file", str(exclude)] + rest)
        if r.returncode:
            sys.exit(r.stderr[-2000:])
        mod, new = changed_files()
        touched = set(mod) | set(new)
        for f in touched:
            Path(f).touch()
        b = sh(["nice", "-n", "10", "sh", "tools/team/nj.sh", "-k", "0"])
        print(f"round {rnd}: {len(touched)} files, {len(excluded)} excluded, build rc={b.returncode}", flush=True)
        if b.returncode == 0:
            print(f"GATE OK: {len(touched)} files compile; excluded {len(excluded)}: {out / 'excluded.txt'}")
            return 0
        log = Path("build/ninja-error.log").read_text(errors="replace")
        (out / f"round{rnd}-errors.log").write_text(log)
        locs = error_locations(log)
        bad = {w for w, tu in locs if w in touched} | {tu for w, tu in locs if w not in touched and tu in touched}
        for w, tu in locs:
            if w in touched or (tu and tu in touched):
                continue
            # An untouched TU broken by a transferred header it includes: exclude those headers.
            obj = str(Path("build/SB4E01") / Path(tu or w).with_suffix(".o"))
            root = str(Path.cwd()) + "/"
            deps = {l.strip().replace(root, "") for l in sh(["ninja", "-t", "deps", obj]).stdout.splitlines()[1:]}
            bad |= {d for d in deps if d in touched}
        if not bad:
            print("errors outside the transferred files:", sorted({w for w, _ in locs})[:20])
            return 1
        print("  excluding:", " ".join(sorted(bad)), flush=True)
        excluded |= bad
    print("gate did not converge")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
