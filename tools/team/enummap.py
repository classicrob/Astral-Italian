#!/usr/bin/env python3
"""Recover SMG2 values of Petari enum names (ATYPE_*, ACTMES_*, ...) by voting over matching call sites.

  enummap.py [--min N] [--prefix ATYPE_,ACTMES_]      run from a worktree root after `ninja`

For every Petari source function, list the calls whose arguments use an enum name (MR::addHitSensor*(.., ATYPE_X, ..),
MR::isSensorType(s, ATYPE_X), MR::sendArbitraryMsg(ACTMES_X, ..), x->isType(ATYPE_X), ...). For the same function in
the SMG2 target object, list the same calls with the immediate loaded into the matching argument register (li rN, imm
shortly before the bl). When a function has the same number of such calls on both sides, pair them in order and vote
NAME -> value. Prints a table (name, value, votes, conflicts) and an enum block you can paste into a header.
"""
import collections, glob, os, re, subprocess, sys

ROOT = os.getcwd()
PET = os.path.expanduser(os.environ.get("TRIAL_PETARI", "~/MacioGalaxy2-team/Petari"))
OD = f"{ROOT}/build/binutils/powerpc-eabi-objdump"
prefixes = (sys.argv[sys.argv.index("--prefix") + 1] if "--prefix" in sys.argv else "ATYPE_,ACTMES_").split(",")
minv = int(sys.argv[sys.argv.index("--min") + 1]) if "--min" in sys.argv else 1


def split_args(s):
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "(":
            depth += 1
        elif ch == ")":
            if depth == 0:
                break
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip()); cur = ""
        else:
            cur += ch
    out.append(cur.strip())
    return out


def src_calls(path):
    """{(Class, method): [(callee, argindex, ENUM)]} in source order. argindex counts `this` for member calls."""
    txt = open(path, encoding="utf-8", errors="ignore").read()
    res, cur = {}, None
    for line in txt.split("\n"):
        m = re.match(r"^[\w:<>*&\s]*?\b(\w+)::(~?\w+)\([^;]*$", line)
        if m and not line.startswith((" ", "\t")):
            cur = (m.group(1), m.group(2)); res.setdefault(cur, [])
        if not cur:
            continue
        for cm in re.finditer(r"(MR::\w+|->\s*(\w+)|\b(\w+))\(", line):
            callee = cm.group(2) or cm.group(1).replace("MR::", "")
            if callee in ("if", "while", "for", "switch", "return", "sizeof"):
                continue
            args = split_args(line[cm.end():])
            member = cm.group(2) is not None
            for i, a in enumerate(args):
                e = re.fullmatch(r"(?:\(\w+\))?\s*(" + "|".join(prefixes) + r")\w+", a)
                if e:
                    res[cur].append((callee, i + (1 if member else 0), a.split(")")[-1].strip()))
    return res


def tgt_calls(obj):
    out = subprocess.run([OD, "-dr", "--no-show-raw-insn", obj], capture_output=True, text=True).stdout
    res, fn, recent = {}, None, []
    for l in out.splitlines():
        m = re.match(r"^[0-9a-f]+ <(.+)>:", l)
        if m:
            fn = m.group(1); res[fn] = []; recent = []; continue
        m = re.match(r"^\s+[0-9a-f]+:\s+li\s+r(\d+),(-?\d+)$", l)
        if m:
            recent.append((int(m.group(1)), int(m.group(2)))); recent = recent[-12:]; continue
        m = re.search(r"R_PPC_REL24\s+(\S+)", l)
        if m and fn:
            callee = re.match(r"(\w+?)__", m.group(1))
            res[fn].append((callee.group(1) if callee else m.group(1), dict(recent)))
            recent = []
    return res


def demangle(n):
    m = re.match(r"^(~?\w+?)__(\d+)(\w+)", n)
    if not m:
        return None
    k = int(m.group(2))
    return (m.group(3)[:k], m.group(1).replace("__dt", "~").replace("__ct", m.group(3)[:k]))


votes = collections.defaultdict(collections.Counter)
for obj in glob.glob(f"{ROOT}/build/SB4E01/obj/Game/**/*.o", recursive=True):
    unit = os.path.relpath(obj, f"{ROOT}/build/SB4E01/obj")[:-2]
    src = f"{PET}/src/{unit}.cpp"
    if not os.path.exists(src):
        continue
    S = src_calls(src)
    if not any(S.values()):
        continue
    T = tgt_calls(obj)
    for tfn, calls in T.items():
        key = demangle(tfn)
        if not key or key not in S or not S[key]:
            continue
        for callee, argi, name in S[key]:
            pass
        # pair per callee in order
        bycallee = collections.defaultdict(list)
        for callee, argi, name in S[key]:
            bycallee[callee].append((argi, name))
        for callee, items in bycallee.items():
            tc = [regs for c, regs in calls if c == callee]
            if len(tc) != len(items):
                continue
            for (argi, name), regs in zip(items, tc):
                r = 3 + argi
                if r in regs:
                    votes[name][regs[r]] += 1

rows = []
for name, c in votes.items():
    (val, n), *rest = c.most_common()
    if n >= minv:
        rows.append((name, val, n, sum(x for _, x in rest)))
rows.sort()
for name, val, n, bad in rows:
    print(f"{name:45s} = 0x{val & 0xFFFFFFFF:X}  votes={n}{'  conflicts=' + str(bad) if bad else ''}")
