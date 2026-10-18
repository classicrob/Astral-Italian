#!/usr/bin/env python3
"""Team helper for porting Petari (SMG1) code into this SMG2 decomp.

  sg.py units [PREFIX] [--top N]   rank units by unmatched bytes that have a Petari counterpart
  sg.py unit UNIT                  build UNIT and list its functions: match %, size, Petari size/unit
  sg.py diff UNIT SYMBOL           build UNIT and print the mismatching instructions of SYMBOL
  sg.py petari SYMBOL              where SYMBOL lives in Petari (unit, source file, size)

UNIT may be "Game/AreaObj/AreaObj", "src/Game/AreaObj/AreaObj.cpp" or "main/Game/AreaObj/AreaObj".
SYMBOL is a mangled name or a unique substring of one (mangled or demangled).
Run from the repo root (or any worktree root). Petari reference: $PETARI (default ~/MacioGalaxy2-team/Petari).
"""
import bisect, json, os, re, subprocess, sys, tempfile, time
from collections import defaultdict

ROOT = os.getcwd()
VER = "SB4E01"
PETARI = os.environ.get("PETARI", os.path.expanduser("~/MacioGalaxy2-team/Petari"))
PVER = "RMGK01"


def load_syms(path):
    d = {}
    for l in open(path):
        m = re.match(r"(\S+) = \.text:0x([0-9A-Fa-f]+); // type:function size:0x([0-9A-Fa-f]+)", l)
        if m:
            d[m.group(1)] = (int(m.group(2), 16), int(m.group(3), 16))
    return d


def load_splits(path):
    out, cur = [], None
    for l in open(path):
        if l.strip().endswith(":") and not l[0].isspace():
            cur = l.strip()[:-1]
        m = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+) end:0x([0-9A-Fa-f]+)", l)
        if m and cur:
            out.append((int(m.group(1), 16), int(m.group(2), 16), cur))
    out.sort()
    return out


class Splits:
    def __init__(self, path):
        self.s = load_splits(path)
        self.starts = [x[0] for x in self.s]

    def unit(self, addr):
        i = bisect.bisect_right(self.starts, addr) - 1
        if i >= 0 and addr < self.s[i][1]:
            return self.s[i][2]
        return None


_cache = {}


def smg2():
    if "g" not in _cache:
        _cache["g"] = (load_syms(f"{ROOT}/config/{VER}/symbols.txt"), Splits(f"{ROOT}/config/{VER}/splits.txt"))
    return _cache["g"]


def petari():
    if "p" not in _cache:
        _cache["p"] = (load_syms(f"{PETARI}/config/{PVER}/symbols.txt"), Splits(f"{PETARI}/config/{PVER}/splits.txt"))
    return _cache["p"]


def norm_unit(u):
    u = u.removeprefix("main/").removeprefix("src/")
    u = re.sub(r"\.(cpp|c)$", "", u)
    return u


def objdiff_unit(u):
    cfg = json.load(open(f"{ROOT}/objdiff.json"))
    for x in cfg["units"]:
        if x["name"] == "main/" + u:
            return x
    sys.exit(f"unit main/{u} not in objdiff.json (run configure.py / ninja once?)")


def ensure_configured(u):
    """Re-run configure.py (under a lock) if UNIT has a source file objdiff.json doesn't know about yet."""
    x = objdiff_unit(u)
    if x.get("base_path") or not any(os.path.exists(f"{ROOT}/src/{u}{e}") for e in (".cpp", ".c")):
        return x
    lock = f"{ROOT}/build/.build.lock"  # shared with tools/team/nj.sh
    while True:
        try:
            os.mkdir(lock)
            open(f"{lock}/pid", "w").write(str(os.getpid()))
            break
        except FileExistsError:
            try:
                pid = int(open(f"{lock}/pid").read())
                os.kill(pid, 0)
            except (OSError, ValueError):
                subprocess.run(["rm", "-rf", lock])  # stale lock from a dead process
                continue
            time.sleep(0.5)
    try:
        subprocess.run(["python3", "configure.py"], cwd=ROOT, capture_output=True, check=True)
    finally:
        subprocess.run(["rm", "-rf", lock])
    return objdiff_unit(u)


def compile_unit(base):
    """Compile one object without invoking ninja's state (safe for parallel agents in one worktree)."""
    cmds = subprocess.run(["ninja", "-t", "commands", base], cwd=ROOT, capture_output=True, text=True).stdout.strip().splitlines()
    if not cmds:
        sys.exit(f"no build command for {base}")
    os.makedirs(os.path.dirname(f"{ROOT}/{base}"), exist_ok=True)
    r = subprocess.run(cmds[-1], shell=True, cwd=ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        sys.stdout.write((r.stdout + r.stderr)[-6000:])
        sys.exit("compile failed")


def build_and_diff(u):
    x = ensure_configured(u)
    base = x.get("base_path")
    if base:
        compile_unit(base)
    fd, out = tempfile.mkstemp(suffix=".json")
    os.close(fd)
    r = subprocess.run([f"{ROOT}/build/tools/objdiff-cli", "-L", "error", "diff", "-p", ROOT, "-u", "main/" + u, "-o", out],
                       cwd=ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(r.stderr)
    d = json.load(open(out))
    os.unlink(out)
    return d, base is not None


def petari_info(name):
    ps, psp = petari()
    if name not in ps:
        return None
    a, s = ps[name]
    pu = psp.unit(a)
    return s, pu


def cmd_units(argv):
    prefix = argv[0] if argv and not argv[0].startswith("--") else ""
    top = int(argv[argv.index("--top") + 1]) if "--top" in argv else 40
    g, gsp = smg2()
    ps, _ = petari()
    rep = {}
    rp = f"{ROOT}/build/{VER}/report.json"
    if os.path.exists(rp):
        for u in json.load(open(rp))["units"]:
            for f in u.get("functions", []):
                rep[f["name"]] = float(f.get("fuzzy_match_percent", 0))
    agg = defaultdict(lambda: [0, 0, 0, 0])  # bytes, unmatched bytes w/ petari name, unmatched same-size, unmatched
    for n, (a, s) in g.items():
        u = gsp.unit(a)
        if u is None or not u.startswith(prefix):
            continue
        u = norm_unit(u)
        rec = agg[u]
        rec[0] += s
        if rep.get(n, 0) >= 100:
            continue
        rec[3] += s
        if n in ps:
            rec[1] += s
            if ps[n][1] == s:
                rec[2] += s
    print(f"{'unit':60s} {'bytes':>7} {'unmatched':>9} {'petari':>7} {'samesz':>7} src")
    for u, (b, pb, sb, ub) in sorted(agg.items(), key=lambda kv: -kv[1][1])[:top]:
        src = "y" if os.path.exists(f"{ROOT}/src/{u}.cpp") or os.path.exists(f"{ROOT}/src/{u}.c") else "-"
        print(f"{u:60s} {b:7d} {ub:9d} {pb:7d} {sb:7d} {src}")


def cmd_unit(argv):
    u = norm_unit(argv[0])
    d, built = build_and_diff(u)
    left = d["left"]["symbols"]
    rows = []
    for s in left:
        if s.get("kind", "") == "SYMBOL_SECTION" or not s.get("instructions") and int(s.get("size", 0)) == 0:
            continue
        if "instructions" not in s and s.get("section") is None and int(s.get("size", 0)) == 0:
            continue
        name = s["name"]
        if name.startswith(("@", "lbl_", "...", ".", "gap_")):
            continue
        if not s.get("instructions"):
            continue
        mp = s.get("match_percent")
        pi = petari_info(name)
        rows.append((mp if mp is not None else -1, int(s["size"]), name, pi))
    tot = sum(r[1] for r in rows)
    done = sum(r[1] * max(r[0], 0) / 100 for r in rows)
    print(f"{u}: {len(rows)} functions, {tot} bytes, {100 * done / max(tot, 1):.1f}% fuzzy" + ("" if built else "  (no source yet)"))
    for mp, sz, name, pi in sorted(rows, key=lambda r: (r[0] >= 100, r[0])):
        ptxt = "not in Petari"
        if pi:
            ptxt = f"petari {pi[0]:#x}{' SAME' if pi[0] == sz else ''} {pi[1] or '?'}"
        m = "  ----" if mp < 0 else f"{mp:6.1f}"
        print(f"{m} {sz:#7x}  {name}  [{ptxt}]")


def fmt(i):
    ins = i.get("instruction")
    return ins.get("formatted", "") if ins else ""


def cmd_diff(argv):
    u = norm_unit(argv[0])
    want = argv[1]
    d, _ = build_and_diff(u)
    left, right = d["left"]["symbols"], d["right"]["symbols"]
    cands = [s for s in left if s.get("instructions") and (s["name"] == want or want in s["name"] or want in s.get("demangled_name", ""))]
    exact = [s for s in cands if s["name"] == want]
    cands = exact or cands
    if len(cands) != 1:
        sys.exit("ambiguous/none: " + ", ".join(s["name"] for s in cands[:20]))
    L = cands[0]
    # target_symbol is 1-based (objdiff keeps a null symbol at 0); prefer pairing by name
    R = next((r for r in right if r["name"] == L["name"]), None)
    if R is None and L.get("target_symbol"):
        R = right[L["target_symbol"] - 1]
    print(f"{L['name']}  match={L.get('match_percent')}  size={L['size']}  (left=TARGET  right=OURS)")
    if R is None:
        print("no counterpart compiled yet; target asm:")
        for i in L["instructions"]:
            print("   ", fmt(i))
        return
    li, ri = L["instructions"], R.get("instructions", [])
    n = max(len(li), len(ri))
    bad = [k for k in range(n) if (k < len(li) and "diff_kind" in li[k]) or (k < len(ri) and "diff_kind" in ri[k])]
    show = set()
    for k in bad:
        show.update(range(max(0, k - 2), min(n, k + 3)))
    last = -1
    for k in sorted(show):
        if k != last + 1:
            print("   ...")
        a = fmt(li[k]) if k < len(li) else ""
        b = fmt(ri[k]) if k < len(ri) else ""
        kind = (li[k].get("diff_kind", "") if k < len(li) else "") or (ri[k].get("diff_kind", "") if k < len(ri) else "")
        mark = {"DIFF_REPLACE": "R", "DIFF_DELETE": "-", "DIFF_INSERT": "+", "DIFF_ARG_MISMATCH": "~", "DIFF_OP_MISMATCH": "O"}.get(kind, " " if not kind else "?")
        print(f"{k * 4:5x} {mark} {a:44s} | {b}")
        last = k
    if not bad:
        print("   (no instruction diffs; check relocations/data)")


def cmd_petari(argv):
    want = argv[0]
    ps, psp = petari()
    hits = [n for n in ps if n == want] or [n for n in ps if want in n]
    for n in hits[:20]:
        a, s = ps[n]
        pu = psp.unit(a)
        src = f"{PETARI}/src/{pu}" if pu else "?"
        print(f"{n}  size={s:#x}  {pu}  {src if os.path.exists(src) else ''}")


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
        print(__doc__)
        return
    {"units": cmd_units, "unit": cmd_unit, "diff": cmd_diff, "petari": cmd_petari}[sys.argv[1]](sys.argv[2:])


if __name__ == "__main__":
    main()
