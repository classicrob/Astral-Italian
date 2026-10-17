#!/usr/bin/env python3
"""Extend a unit's data splits so it owns the literals its code references.

The team registration gave most units only .text/.data/.sbss ranges, so float
literals (.sdata2), doubles/tables (.rodata), strings (.data before the vtables)
and .sdata/.bss land in other objects and the unit can never reach 100%.

  splitfix.py UNIT [UNIT...] [--write]

Reads the target object's relocations (build/SB4E01/obj/<unit>.o), looks up each
referenced lbl_/@ symbol in symbols.txt, and proposes per-section ranges
[min, max+size) merged with the unit's existing ranges. Refuses ranges that
overlap another unit. With --write it rewrites config/SB4E01/splits.txt; run
`ninja` afterwards to re-split the target objects.
"""
import re, subprocess, sys, os

ROOT = os.getcwd()
VER = "SB4E01"
SPLITS = f"{ROOT}/config/{VER}/splits.txt"
SYMS = f"{ROOT}/config/{VER}/symbols.txt"
OBJDUMP = f"{ROOT}/build/binutils/powerpc-eabi-objdump"
DATA_SECS = [".rodata", ".data", ".bss", ".sdata", ".sbss", ".sdata2", ".sbss2"]
SEC_ORDER = [".text", ".ctors", ".dtors", ".rodata", ".data", ".bss", ".sdata", ".sbss", ".sdata2", ".sbss2"]


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


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    write = "--write" in sys.argv
    syms = load_syms()
    lines, units = parse_splits()
    occupied = {}
    for name, u in units.items():
        for s, (a, b, _, _) in u["secs"].items():
            occupied.setdefault(s, []).append((a, b, name))
    changed = False
    for arg in args:
        u = norm(arg)
        key = None
        for k in units:
            if norm(k) == u:
                key = k
        if not key:
            print(f"{u}: not in splits.txt"); continue
        obj = f"{ROOT}/build/{VER}/obj/{u}.o"
        out = subprocess.run([OBJDUMP, "-r", obj], capture_output=True, text=True).stdout
        want = {}
        for l in out.splitlines():
            p = l.split()
            if len(p) >= 3 and p[2] in syms:
                sec, addr, size = syms[p[2].split("+")[0]]
                if sec in DATA_SECS:
                    a, b = want.get(sec, (addr, addr + max(size, 1)))
                    want[sec] = (min(a, addr), max(b, addr + max(size, 1)))
        secs = units[key]["secs"]
        for sec, (a, b) in sorted(want.items(), key=lambda x: SEC_ORDER.index(x[0])):
            if sec in secs:
                a, b = min(a, secs[sec][0]), max(b, secs[sec][1])
            b = (b + 3) & ~3
            bad = [n for (x, y, n) in occupied.get(sec, []) if n != key and x < b and a < y]
            if bad:
                print(f"{u}: {sec} 0x{a:08X}-0x{b:08X} overlaps {bad}; skipped"); continue
            old = secs.get(sec)
            if old and old[0] == a and old[1] == b:
                continue
            print(f"{u}: {sec} {'0x%08X-0x%08X' % (old[0], old[1]) if old else 'none'} -> 0x{a:08X}-0x{b:08X}")
            if old:
                lines[old[2]] = f"\t{sec:<11} start:0x{a:08X} end:0x{b:08X}{old[3]}"
            else:
                secs[sec] = [a, b, None, ""]
            secs[sec][0], secs[sec][1] = a, b
            changed = True
        # rebuild this unit's block in section order
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
    if changed and not write:
        print("(dry run; pass --write)")


if __name__ == "__main__":
    main()
