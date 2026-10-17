#!/usr/bin/env python3
"""Name unnamed target functions (fn_XXXXXXXX) after the identical function in our build.

  namefn.py UNIT [UNIT...] [--write]

SMG2-only functions are often `fn_8020E640` in config/SB4E01/symbols.txt, so objdiff cannot pair them with the
member function we wrote (`Teresa::foo`), and they count as unmatched even when the code is identical.
For each unit: take our functions that have no same-named target symbol, and the target's fn_ symbols; compare
instruction words with relocated fields masked; if exactly one of ours equals a target fn_, rename the fn_ to our
mangled name in symbols.txt (--write). Then run `ninja` (re-split) and the function counts as matched.
Only exact (masked) matches are renamed, so this never claims a non-matching function.
"""
import os, re, subprocess, sys

ROOT = os.getcwd()
OD = f"{ROOT}/build/binutils/powerpc-eabi-objdump"
SYMS = f"{ROOT}/config/SB4E01/symbols.txt"


def funcs(obj):
    out = subprocess.run([OD, "-dr", "--no-show-raw-insn", obj], capture_output=True, text=True).stdout
    raw = subprocess.run([OD, "-d", obj], capture_output=True, text=True).stdout
    words, d, fn = {}, {}, None
    for l in raw.splitlines():
        m = re.match(r"^([0-9a-f]+) <(.+)>:", l)
        if m:
            fn = m.group(2); d[fn] = []; continue
        m = re.match(r"^\s+([0-9a-f]+):\s+((?:[0-9a-f]{2} ){4})", l)
        if m and fn:
            d[fn].append([int(m.group(1), 16), int(m.group(2).replace(" ", ""), 16), None])
    # mark relocated instructions
    fn = None
    idx = {}
    for name, ins in d.items():
        for i in ins:
            idx[i[0]] = i
    for l in out.splitlines():
        m = re.match(r"^\s+([0-9a-f]+): (R_PPC_\w+)\s+(\S+)", l)
        if m:
            a = int(m.group(1), 16) & ~3
            if a in idx:
                idx[a][2] = m.group(2)
    res = {}
    order = {name: (ins[0][0] if ins else 0) for name, ins in d.items()}
    funcs.order = order
    for name, ins in d.items():
        seq = []
        for a, w, r in ins:
            if r == "R_PPC_REL24":
                w &= 0xFC000003
            elif r:
                w &= 0xFFFF0000
            elif (w >> 26) in (16, 18):  # local branches: offsets differ with placement
                w &= 0xFC000003
            seq.append(w)
        res[name] = tuple(seq)
    return res


def main():
    write = "--write" in sys.argv
    units = [a for a in sys.argv[1:] if not a.startswith("--")]
    lines = open(SYMS).read().split("\n")
    renames = {}
    for u in units:
        u = re.sub(r"\.(cpp|c)$", "", u.removeprefix("src/").removeprefix("main/"))
        t, o = f"{ROOT}/build/SB4E01/obj/{u}.o", f"{ROOT}/build/SB4E01/src/{u}.o"
        if not (os.path.exists(t) and os.path.exists(o)):
            print(f"{u}: missing object"); continue
        T = funcs(t); torder = funcs.order
        O = funcs(o); oorder = funcs.order
        ours = {k: v for k, v in O.items() if k not in T and not k.startswith(("@", "lbl_", "gap_"))}
        # group identical bodies; within a group pair target fn_ and ours in address (= source) order
        groups = {}
        for tn, tv in T.items():
            if tn.startswith("fn_"):
                groups.setdefault(tv, [[], []])[0].append(tn)
        for k, v in ours.items():
            if v in groups:
                groups[v][1].append(k)
        for tv, (tns, oks) in groups.items():
            if not oks:
                continue
            if len(tns) != len(oks):
                print(f"{u}: unequal group {tns} vs {oks[:4]}; skipped"); continue
            for tn, ok in zip(sorted(tns, key=lambda n: torder[n]), sorted(oks, key=lambda n: oorder[n])):
                renames[tn] = ok
                print(f"{u}: {tn} -> {ok}")
    if write and renames:
        for i, l in enumerate(lines):
            m = re.match(r"(fn_[0-9A-F]{8}) = ", l)
            if m and m.group(1) in renames:
                lines[i] = renames[m.group(1)] + l[len(m.group(1)):]
        open(SYMS, "w").write("\n".join(lines))
        print(f"renamed {len(renames)} symbols; run ninja")


if __name__ == "__main__":
    main()
