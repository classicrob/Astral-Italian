#!/usr/bin/env python3
"""Extend a unit's data splits so it owns the literals its code references.

The team registration gave most units only .text/.data/.sbss ranges, so float
literals (.sdata2), doubles/tables (.rodata), strings (.data before the vtables)
and .sdata land in other objects and the unit can never reach 100%.

  splitfix.py UNIT [UNIT...] [--write]

For each unit: read the target object's relocations (build/SB4E01/obj/<unit>.o),
keep referenced symbols in .rodata/.data/.sdata/.sdata2 that NO other object
references (index of all build/SB4E01/obj/**/*.o, cached), cluster them next to
the unit's existing range, and propose [min, max+size) per section. Refuses
ranges overlapping another unit (including ones claimed earlier in the same run).
With --write it rewrites config/SB4E01/splits.txt; then run `ninja` to re-split.
Only one agent per worktree should run --write (it is the lane lead's job).
"""
import json, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

ROOT = os.getcwd()
VER = "SB4E01"
SPLITS = f"{ROOT}/config/{VER}/splits.txt"
SYMS = f"{ROOT}/config/{VER}/symbols.txt"
OBJDIR = f"{ROOT}/build/{VER}/obj"
OBJDUMP = f"{ROOT}/build/binutils/powerpc-eabi-objdump"
DATA_SECS = [".rodata", ".data", ".sdata", ".sdata2"]
SEC_ORDER = [".text", ".ctors", ".dtors", ".rodata", ".data", ".bss", ".sdata", ".sbss", ".sdata2", ".sbss2"]
GAP = {".rodata": 0x100, ".data": 0x100, ".sdata": 0x20, ".sdata2": 0x20}


def norm(u):
    u = u.removeprefix("main/").removeprefix("src/")
    return re.sub(r"\.(cpp|c)$", "", u)


def load_syms():
    d = {}
    for l in open(SYMS):
        m = re.match(r"(\S+) = (\.\w+):0x([0-9A-Fa-f]+);", l)
        if m:
            z = re.search(r"size:0x([0-9A-Fa-f]+)", l)
            d[m.group(1)] = (m.group(2), int(m.group(3), 16), int(z.group(1), 16) if z else 0)
    return d


def parse_splits(lines=None):
    lines = lines if lines is not None else open(SPLITS).read().split("\n")
    units, cur = {}, None
    for i, l in enumerate(lines):
        if l and not l[0].isspace() and l.rstrip().endswith(":") and l != "Sections:":
            cur = l.rstrip()[:-1]
            units[cur] = {"line": i, "secs": {}}
        elif cur and l.strip() == "":
            cur = None
        elif cur:
            m = re.match(r"\s+(\S+)\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)(.*)", l)
            if m:
                units[cur]["secs"][m.group(1)] = [int(m.group(2), 16), int(m.group(3), 16), i, m.group(4)]
    return lines, units


def relocs(obj):
    out = subprocess.run([OBJDUMP, "-r", obj], capture_output=True, text=True).stdout
    s = set()
    for l in out.splitlines():
        p = l.split()
        if len(p) >= 3 and p[1].startswith("R_PPC"):
            s.add(p[2].split("+")[0])
    return s


def ref_index():
    """symbol -> set of objects referencing it (cached on obj count + newest mtime)."""
    objs = []
    for dp, _, fs in os.walk(OBJDIR):
        objs += [os.path.join(dp, f) for f in fs if f.endswith(".o")]
    cache = f"{ROOT}/build/.splitfix_refs.json"
    stamp = max(os.path.getmtime(o) for o in objs)
    if os.path.exists(cache):
        c = json.load(open(cache))
        if c.get("stamp") == stamp and c.get("n") == len(objs):
            return {k: set(v) for k, v in c["idx"].items()}
    idx = {}
    with ThreadPoolExecutor(16) as ex:
        for o, rs in zip(objs, ex.map(relocs, objs)):
            for r in rs:
                idx.setdefault(r, set()).add(os.path.relpath(o, OBJDIR))
    json.dump({"stamp": stamp, "n": len(objs), "idx": {k: sorted(v) for k, v in idx.items()}}, open(cache, "w"))
    return idx


def cluster(items, sec, existing):
    groups, cur = [], []
    for a, b in items:
        if cur and a - cur[-1][1] > GAP[sec]:
            groups.append(cur); cur = []
        cur.append((a, b))
    if cur:
        groups.append(cur)
    if existing:
        ea, eb = existing
        return [g for g in groups if g[0][0] - GAP[sec] <= eb and ea <= g[-1][1] + GAP[sec]]
    return [max(groups, key=len)] if groups else []


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    write = "--write" in sys.argv
    syms = load_syms()
    idx = ref_index()
    lines, units = parse_splits()
    changed = False
    for arg in args:
        u = norm(arg)
        key = next((k for k in units if norm(k) == u), None)
        if not key:
            print(f"{u}: not in splits.txt"); continue
        mine = f"{u}.o"
        if not os.path.exists(f"{OBJDIR}/{mine}"):
            print(f"{u}: no target object (run ninja once)"); continue
        want = {}
        for s in relocs(f"{OBJDIR}/{mine}"):
            if s not in syms:
                continue
            sec, addr, size = syms[s]
            if sec not in DATA_SECS or idx.get(s, {mine}) != {mine}:
                continue
            want.setdefault(sec, []).append((addr, addr + max(size, 1)))
        secs = units[key]["secs"]
        occupied = [(x, y, n, s) for n, uu in units.items() if n != key for s, (x, y, _, _) in uu["secs"].items()]
        for sec in sorted(want, key=SEC_ORDER.index):
            old = secs.get(sec)
            g = cluster(sorted(want[sec]), sec, (old[0], old[1]) if old else None)
            if not g:
                continue
            a = min(x[0][0] for x in g)
            b = max(x[-1][1] for x in g)
            if old:
                a, b = min(a, old[0]), max(b, old[1])
            b = (b + 3) & ~3
            bad = sorted({n for (x, y, n, s) in occupied if s == sec and x < b and a < y})
            if bad:
                print(f"{u}: {sec} 0x{a:08X}-0x{b:08X} overlaps {bad}; skipped"); continue
            if old and old[0] == a and old[1] == b:
                continue
            print(f"{u}: {sec} {'0x%08X-0x%08X' % (old[0], old[1]) if old else 'none'} -> 0x{a:08X}-0x{b:08X}")
            if old:
                old[0], old[1] = a, b
            else:
                secs[sec] = [a, b, None, ""]
            changed = True
        start = units[key]["line"]
        end = start + 1
        while end < len(lines) and lines[end].strip():
            end += 1
        block = [lines[start]]
        for sec in sorted(secs, key=lambda s: SEC_ORDER.index(s) if s in SEC_ORDER else 99):
            a, b, _, rest = secs[sec]
            block.append(f"\t{sec:<11} start:0x{a:08X} end:0x{b:08X}{rest}")
        lines[start:end] = block
        lines, units = parse_splits(lines)
    if changed and write:
        open(SPLITS, "w").write("\n".join(lines))
    elif changed:
        print("(dry run; pass --write)")


if __name__ == "__main__":
    main()
