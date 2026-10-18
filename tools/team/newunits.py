#!/usr/bin/env python3
"""Create units for named SMG2 classes that sit in unsplit (auto_) code, so lanes can see and work them.

About 636 KB of named functions of SMG2-only classes (Wanwan, ItemDrill, Yoshi, BattanKing, ...) live in gaps
between registered .text units. For each gap, this walks the functions in address order and cuts a new unit
whenever the "owning" class changes. Owning class = the class in the mangled name; MR::, templates, free
functions and fn_ symbols attach to the current unit. Each new unit is named after the class with the most bytes
in it and placed in the directory of its header if one exists, else in the directory of the neighbouring units
(the linker order follows the source tree, so neighbours share a directory). Units must hold at least
--min bytes of named class code (default 0x80).

  newunits.py [--write] [--min BYTES] [--only Dir1,Dir2]   (run it for YOUR lane dirs only)
--write appends .text-only blocks to config/SB4E01/splits.txt (sorted by address on the next dtk split) and
NonMatching entries to configure.py (in the matching GameLib). Then: `tools/team/nj.sh`.
"""
import bisect, collections, glob, os, re, sys

ROOT = os.getcwd()
SPLITS = f"{ROOT}/config/SB4E01/splits.txt"
write = "--write" in sys.argv
minb = int(sys.argv[sys.argv.index("--min") + 1], 0) if "--min" in sys.argv else 0x80


def load_fns():
    out = []
    for l in open(f"{ROOT}/config/SB4E01/symbols.txt"):
        m = re.match(r"(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)", l)
        if m:
            out.append((int(m.group(2), 16), int(m.group(3), 16), m.group(1)))
    return sorted(out)


def owner(name):
    """Primary class of a mangled member function, or None for free/MR/template/unnamed."""
    if name.startswith(("fn_", "__sinit", "@")):
        return None
    m = re.search(r"__(?:Q(\d)(\d+)([A-Za-z_]\w*)|(\d+)([A-Za-z_]\w*))", name)
    if not m:
        return None
    if m.group(4):
        L, rest = int(m.group(4)), m.group(5)
    else:
        L, rest = int(m.group(2)), m.group(3)  # outer name of a nested class (Q2 Outer Inner)
    c = rest[:L]
    if c in ("MR", "JGeometry", "JMath", "JMathInlineVEC", "nw4r", "JGadget", "std") or "<" in name.split("__")[0]:
        return None
    return c


def related(c, seg):
    """Same TU if the class is already in it, or is a helper of its classes (FooState/FooNrv/NrvFoo...)."""
    b = c.removeprefix("Nrv")
    return any(b.startswith(k.removeprefix("Nrv")) or k.removeprefix("Nrv").startswith(b) for k in seg["classes"])


SKIP_DIRS = set()
ONLY = set(sys.argv[sys.argv.index("--only") + 1].split(",")) if "--only" in sys.argv else None  # e.g. --only MapObj,Ride
fns = load_fns()
text, cur = [], None
lines_all = open(SPLITS).read().split("\n")
for l in lines_all:
    if l.strip().endswith(":") and not l[:1].isspace():
        cur = l.strip()[:-1]
    m = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", l)
    if m and cur:
        text.append((int(m.group(1), 16), int(m.group(2), 16), cur))
text.sort()
existing = {t[2] for t in text}
hdr = {}
for p in glob.glob(f"{ROOT}/include/Game/**/*.hpp", recursive=True):
    hdr.setdefault(os.path.basename(p)[:-4], p.split("include/")[1].split("/")[1])
fst = [f[0] for f in fns]

new = []  # (start, end, unitpath, named_bytes)
for k in range(len(text) - 1):
    A, B = text[k], text[k + 1]
    if not (A[2].startswith("Game/") or B[2].startswith("Game/")):
        continue
    gap = fns[bisect.bisect_left(fst, A[1]):bisect.bisect_left(fst, B[0])]
    if not gap:
        continue
    neighbour_dir = (A[2] if A[2].startswith("Game/") else B[2]).split("/")[1]
    segs, seg = [], None
    for a, s, n in gap:
        c = owner(n)
        if c and (seg is None or not related(c, seg)):
            # a new class starts a new unit unless it's already in this unit
            if seg and seg["classes"]:
                segs.append(seg)
            seg = {"start": a, "end": a + s, "cls": c, "classes": collections.Counter({c: s})}
        elif seg is None:
            continue  # leading free/unnamed functions stay unsplit
        else:
            seg["end"] = a + s
            if c:
                seg["classes"][c] += s
    if seg and seg["classes"]:
        segs.append(seg)
    for sg in segs:
        named = sum(sg["classes"].values())
        if named < minb:
            continue
        main = sg["classes"].most_common(1)[0][0]
        d = hdr.get(main, neighbour_dir)
        if d in SKIP_DIRS or (ONLY is not None and d not in ONLY):
            continue
        u = f"Game/{d}/{main}.cpp"
        i = 2
        while u in existing:
            u = f"Game/{d}/{main}{i}.cpp"
            i += 1
        existing.add(u)
        new.append((sg["start"], sg["end"], u, named, A[2]))

# end each unit where the next function starts (covers alignment padding), never past the next unit
for idx, (s, e, u, b, after) in enumerate(new):
    j = bisect.bisect_left(fst, e)
    nxt = fns[j][0] if j < len(fns) else e
    bound = text[bisect.bisect_right([t[0] for t in text], s)][0] if bisect.bisect_right([t[0] for t in text], s) < len(text) else nxt
    later = [x[0] for x in new if x[0] > s]
    new[idx] = (s, min(nxt, bound, min(later) if later else nxt), u, b, after)

bydir = collections.Counter(x[2].split("/")[1] for x in new)
print(f"{len(new)} new units, {sum(x[3] for x in new)} bytes of named class code (plus fn_ between them)")
print("by dir:", bydir.most_common())
for s, e, u, b, _ in sorted(new, key=lambda x: -x[3])[:12]:
    print(f"  {u:50s} {s:#x}-{e:#x} named={b}")
if write:
    # insert each new block right after the block of the unit that precedes it (keeps address order)
    after = collections.defaultdict(list)
    for x in sorted(new):
        after[x[4]].append(x)
    out, i, curblk = [], 0, None
    for l in lines_all:
        if l.strip().endswith(":") and not l[:1].isspace():
            if curblk in after:
                for s0, e0, u0, _, _ in after.pop(curblk):
                    out += [f"{u0}:", f"\t.text       start:0x{s0:08X} end:0x{e0:08X}", ""]
            curblk = l.strip()[:-1]
        out.append(l)
    for k0, xs in after.items():  # the preceding unit was the last block
        for s0, e0, u0, _, _ in xs:
            out += ["", f"{u0}:", f"\t.text       start:0x{s0:08X} end:0x{e0:08X}"]
    open(SPLITS, "w").write("\n".join(out))
    cfg = open(f"{ROOT}/configure.py").read()
    for d in sorted(bydir):
        ent = "".join(f'            Object(NonMatching, "{x[2]}"),\n' for x in sorted(new, key=lambda x: x[2]) if x[2].split("/")[1] == d)
        m = re.search(r'    GameLib\(\n        "' + re.escape(d) + r'",\n        \[\n(.*?)\n        \],\n    \),\n', cfg, re.S)
        if m:
            cfg = cfg[:m.end(1) + 1] + ent + cfg[m.end(1) + 1:]
        else:
            ms = list(re.finditer(r'    GameLib\(\n        "[^"]+",\n        \[\n.*?\n        \],\n    \),\n', cfg, re.S))
            pos = ms[-1].end()
            cfg = cfg[:pos] + f'    GameLib(\n        "{d}",\n        [\n{ent}        ],\n    ),\n' + cfg[pos:]
    open(f"{ROOT}/configure.py", "w").write(cfg)
    print("wrote splits.txt and configure.py")
