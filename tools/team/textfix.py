#!/usr/bin/env python3
"""Widen units' .text ranges to absorb Petari-known functions stranded in the auto_ gaps next to them.

Garigari's splits often stop a unit's .text early (or start it late), leaving functions of the same class in
auto-generated gap objects. For each gap between two consecutive .text units A and B, a function belongs to A
if Petari places it in the Petari unit A corresponds to (same for B). A grows forward over the gap while the
functions are A's or unknown (fn_ / not in Petari) and an A-owned function still lies ahead; B grows backward
the same way. Functions owned by a third unit stop the growth.

  textfix.py [PREFIX] [--write]      (default prefix "Game/"; dry-run unless --write)
Run from a worktree root, then `python3 configure.py && ninja` to re-split.
"""
import os, re, sys, collections, bisect

ROOT = os.getcwd()
SPLITS = f"{ROOT}/config/SB4E01/splits.txt"
P = os.path.expanduser("~/MacioGalaxy2-team/Petari")
prefix = next((a for a in sys.argv[1:] if not a.startswith("--")), "Game/")
write = "--write" in sys.argv


def load_syms(path):
    d = {}
    for l in open(path):
        m = re.match(r"(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)", l)
        if m:
            d[m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16))
    return d


lines = open(SPLITS).read().split("\n")
# .text ranges with their line index
text = []  # (start, end, unit, line_idx)
cur = None
for i, l in enumerate(lines):
    if l.strip().endswith(":") and not l[:1].isspace():
        cur = l.strip()[:-1]
    m = re.match(r"(\s+\.text\s+start:)0x([0-9A-Fa-f]+)( end:)0x([0-9A-Fa-f]+)(.*)$", l)
    if m and cur:
        text.append([int(m.group(2), 16), int(m.group(4), 16), cur, i])
text.sort()

g = load_syms(f"{ROOT}/config/SB4E01/symbols.txt")
ps = load_syms(f"{P}/config/RMGK01/symbols.txt")
pt = []
cur = None
for l in open(f"{P}/config/RMGK01/splits.txt"):
    if l.strip().endswith(":") and not l[:1].isspace():
        cur = l.strip()[:-1]
    m = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", l)
    if m and cur:
        pt.append((int(m.group(1), 16), int(m.group(2), 16), cur))
pt.sort()
pst = [x[0] for x in pt]


def punit(name):
    if name not in ps:
        return None
    a = ps[name][0]
    i = bisect.bisect_right(pst, a) - 1
    return pt[i][2] if i >= 0 and a < pt[i][1] else None


fns = sorted((a, s, n) for n, (a, s) in g.items())
fstarts = [f[0] for f in fns]

# Petari identity of each SMG2 unit = the Petari unit holding most of its named functions (else its own name).
ident = {}
for t in text:
    i0, i1 = bisect.bisect_left(fstarts, t[0]), bisect.bisect_left(fstarts, t[1])
    c = collections.Counter(punit(n) for _, _, n in fns[i0:i1])
    c.pop(None, None)
    ident[t[2]] = {t[2]} | ({c.most_common(1)[0][0]} if c else set())

changes = []
gained = collections.Counter()
for k in range(len(text) - 1):
    A, B = text[k], text[k + 1]
    if B[0] <= A[1]:
        continue
    gap = [f for f in fns[bisect.bisect_left(fstarts, A[1]):bisect.bisect_left(fstarts, B[0])]]
    if not gap:
        continue
    own = []
    for a, s, n in gap:
        pu = punit(n)
        own.append("A" if pu in ident[A[2]] and A[2].startswith(prefix) else
                   "B" if pu in ident[B[2]] and B[2].startswith(prefix) else
                   "?" if pu is None else "X")
    lastA = max((i for i, o in enumerate(own) if o == "A"), default=-1)
    # A may only extend across A/? entries
    if lastA >= 0 and all(o in "A?" for o in own[:lastA + 1]):
        newend = gap[lastA + 1][0] if lastA + 1 < len(gap) else B[0]
        gained[A[2]] += sum(s for (a, s, n), o in zip(gap[:lastA + 1], own) if o == "A")
        A[1] = newend
    else:
        lastA = -1
    rest = list(range(lastA + 1, len(gap)))
    firstB = min((i for i in rest if own[i] == "B"), default=None)
    if firstB is not None and all(own[i] in "B?" for i in range(firstB, len(gap))):
        B[0] = gap[firstB][0]
        gained[B[2]] += sum(gap[i][1] for i in range(firstB, len(gap)) if own[i] == "B")

for t in text:
    i = t[3]
    m = re.match(r"(\s+\.text\s+start:)0x([0-9A-Fa-f]+)( end:)0x([0-9A-Fa-f]+)(.*)$", lines[i])
    new = f"{m.group(1)}0x{t[0]:08X}{m.group(3)}0x{t[1]:08X}{m.group(5)}"
    if new != lines[i]:
        changes.append(t[2])
        lines[i] = new

print(f"{len(changes)} units widened, {sum(gained.values())} bytes of Petari-known functions absorbed")
for u, b in gained.most_common(15):
    print(f"  +{b:6d}  {u}")
if write:
    open(SPLITS, "w").write("\n".join(lines))
    print("wrote", SPLITS)
