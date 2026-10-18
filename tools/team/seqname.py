#!/usr/bin/env python3
"""Name unnamed target functions (fn_XXXXXXXX) by ORDER + similarity, for units whose source order follows
the target (Petari ports, unity TUs such as SMG2 Mario.o = all Petari Mario*.cpp Mario:: methods).

  seqname.py UNIT [--min 0.80] [--write]

Aligns the target's functions (address order) with ours (object order) by dynamic programming:
same-named pairs are anchors; a target fn_ may pair with one of our functions that has no same-named
target symbol, scored by opcode-sequence similarity (difflib ratio on masked words' primary/extended
opcodes, plus a size check). Pairs scoring >= --min (exact opcodes for <= 8 instructions) are renamed in symbols.txt with --write.
Unlike namefn.py this also names NEAR matches (layout/enum diffs still pending), so it only trusts pairs
that are both in sequence and similar; review the printed list before --write.
"""
import difflib, os, re, subprocess, sys

sys.path.insert(0, os.path.dirname(__file__))
import namefn  # noqa: E402

ROOT = os.getcwd()
SYMS = f"{ROOT}/config/SB4E01/symbols.txt"


def ops(seq):
    out = []
    for w in seq:
        p = w >> 26
        out.append((p << 10) | ((w >> 1) & 0x3FF) if p in (19, 31, 59, 63) else p)
    return out


def main():
    a = sys.argv[1:]
    write = "--write" in a
    mn = float(a[a.index("--min") + 1]) if "--min" in a else 0.80
    u = re.sub(r"\.(cpp|c)$", "", a[0].removeprefix("src/").removeprefix("main/"))
    t, o = f"{ROOT}/build/SB4E01/obj/{u}.o", f"{ROOT}/build/SB4E01/src/{u}.o"
    T = namefn.funcs(t); torder = dict(namefn.funcs.order)
    O = namefn.funcs(o); oorder = dict(namefn.funcs.order)
    tl = sorted(T, key=lambda n: torder[n])
    ol = [n for n in sorted(O, key=lambda n: oorder[n]) if not n.startswith(("@", "lbl_", "gap_"))]
    free = set(n for n in ol if n not in T)
    to = {n: ops(T[n]) for n in tl}
    oo = {n: ops(O[n]) for n in ol}

    def score(i, j):
        tn, on = tl[i], ol[j]
        if tn == on:
            return 5.0
        if not tn.startswith("fn_") or on not in free:
            return None
        lt, lo = len(to[tn]), len(oo[on])
        if min(lt, lo) < 0.6 * max(lt, lo):
            return None
        r = difflib.SequenceMatcher(None, to[tn], oo[on], autojunk=False).ratio()
        return r if r >= 0.5 else None

    if "--free" in a:
        # order-free: mutual best similarity, with a margin over the runner-up
        fns = [x for x in tl if x.startswith("fn_")]
        cand = [x for x in ol if x in free]
        sim = {}
        for tn in fns:
            for on in cand:
                lt, lo = len(to[tn]), len(oo[on])
                if min(lt, lo) < 0.8 * max(lt, lo):
                    continue
                sm = difflib.SequenceMatcher(None, to[tn], oo[on], autojunk=False)
                if sm.real_quick_ratio() < mn or sm.quick_ratio() < mn:
                    continue
                r = sm.ratio()
                if r >= 0.5:
                    sim[(tn, on)] = r
        bt, bo = {}, {}
        for (tn, on), r in sim.items():
            bt.setdefault(tn, []).append((r, on)); bo.setdefault(on, []).append((r, tn))
        pairs = []
        for tn, lst in bt.items():
            lst.sort(reverse=True)
            r, on = lst[0]
            ob = sorted(bo[on], reverse=True)
            margin = min(r - (lst[1][0] if len(lst) > 1 else 0), r - (ob[1][0] if len(ob) > 1 else 0))
            if ob[0][1] == tn and margin >= 0.05:
                pairs.append((tn, on, r))
        pairs.sort(key=lambda x: torder[x[0]])
        return finish(pairs, mn, write, {n: len(to[n]) for n in tl})
    n, m = len(tl), len(ol)
    # banded DP would be faster; units are a few hundred functions, fine.
    NEG = -1.0
    D = [[0.0] * (m + 1) for _ in range(n + 1)]
    B = [[0] * (m + 1) for _ in range(n + 1)]
    cache = {}
    for i in range(n - 1, -1, -1):
        for j in range(m - 1, -1, -1):
            best, bk = D[i + 1][j], 1
            if D[i][j + 1] > best:
                best, bk = D[i][j + 1], 2
            s = score(i, j)
            if s is not None and D[i + 1][j + 1] + s > best:
                best, bk = D[i + 1][j + 1] + s, 3
                cache[(i, j)] = s
            D[i][j], B[i][j] = best, bk
    i = j = 0
    pairs = []
    while i < n and j < m:
        if B[i][j] == 3:
            if tl[i] != ol[j]:
                pairs.append((tl[i], ol[j], cache[(i, j)]))
            i += 1; j += 1
        elif B[i][j] == 1:
            i += 1
        else:
            j += 1
    return finish(pairs, mn, write, {n: len(to[n]) for n in tl})


def finish(pairs, mn, write, small=None):
    ren = {}
    for tn, on, s in pairs:
        ok = s >= mn and (s == 1.0 or not small or small.get(tn, 99) > 8)
        print(f"{'OK ' if ok else '-- '} {s:.2f} {tn} -> {on}")
        if ok:
            ren[tn] = on
    print(f"{len(ren)} renames >= {mn} ({len(pairs)} aligned fn_ pairs)")
    if write and ren:
        lines = open(SYMS).read().split("\n")
        for k, l in enumerate(lines):
            mm = re.match(r"(fn_[0-9A-F]{8}) = ", l)
            if mm and mm.group(1) in ren:
                lines[k] = ren[mm.group(1)] + l[len(mm.group(1)):]
        open(SYMS, "w").write("\n".join(lines))
        print("written; run tools/team/nj.sh")


if __name__ == "__main__":
    main()
