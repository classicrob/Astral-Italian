#!/usr/bin/env python3
"""Transfer MacioGalaxy's native-port hooks onto the SMG2 sources.

MacioGalaxy (~/MacioGalaxy) is Petari forked at e5bc761c0 plus PETARI_NATIVE hooks. For every file
it changed under src/, include/ and libs/, this tool:

1. splits the change (base -> MacioGalaxy HEAD) into hunk groups, where a group is a run of diff
   opcodes whose preprocessor conditionals balance (an `#ifdef ... #else ... #endif` hook whose
   `#endif` lands in a later opcode stays in one group);
2. classifies each group by what it does to the Wii side, evaluated with unifdef
   (-UGALAXY_NATIVE -D__MWERKS__) and compared after stripping comments and blank lines:
   - guarded: the Wii text is unchanged (a real hook);
   - comment: only comments/whitespace changed (skipped);
   - unconditional: the Wii-side code changed too (MacioGalaxy bug fixes, clang-only syntax fixes such
     as `case N: {`): never applied, listed for manual porting;
   - feature: a guarded hook for a deferred MacioGalaxy feature (editor, test server, mods; see
     FEATURE_RE): skipped, listed for P3;
3. three-way merges each guarded group into the SMG2 file (`git merge-file`, base = e5bc761c0) and
   keeps the result only if it has no conflict and the SMG2 file's Wii side is unchanged:
   - clean: the SMG2 file still equals the Petari base there;
   - adapted: merged into an SMG2 file that differs from Petari;
   - manual: conflict (SMG2 changed the same lines) or the merge altered the Wii side.

PETARI_NATIVE is renamed to GALAXY_NATIVE (team-lead decision); the report records the mapping.
Added files: native infrastructure (Native*/BigEndian*) is copied; editor/test-server/mod files are
listed as deferred.

Output: --out DIR gets REPORT.md, report.json, manual/<path>.diff (unapplied groups with context),
and with --overlay a full copy of the tree (src, include, libs, native) with the hooks applied, for
compile audits. --apply writes the merged files into --tree in place (use --only to limit it to
directories you own).
"""
import argparse
import difflib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path

MACRO_FROM = "PETARI_NATIVE"
MACRO_TO = "GALAXY_NATIVE"
MACRO_RE = re.compile(r"\b%s\b" % MACRO_FROM)
PP_OPEN = re.compile(r"^\s*#\s*if")
PP_CLOSE = re.compile(r"^\s*#\s*endif")
# Deferred MacioGalaxy features (P3 mods, level editor, test server); their hooks are not transferred.
FEATURE_RE = re.compile(r"Editor|TestServer|ClassSweep|Odyssey|Cappy|GenericFigure|PhotoCamera|CutsceneSkip|"
                        r"NativeUnlockedSave|PointMove|GenericWorld|GenericShapes|PETARI_MODS|petari::mods|"
                        r"\bmods?::|ModSettings|OVERWRITE_EMIT|PETARI_OVERWRITE")
NATIVE_ADDED_RE = re.compile(r"Native|native|BigEndian")
FUNC_RE = re.compile(r"^(?!\s)(?!(?:if|for|while|switch|return|else|do|case|namespace|class|struct|enum|typedef|#)\b)"
                     r"[\w:<>,\*&~ ]*?\b([\w]+(?:::[~\w]+)*)\s*\([^;]*$")
SOURCE_EXTS = {".c", ".cpp", ".h", ".hpp", ".inc", ".cc"}


def git(macio, *args):
    return subprocess.run(["git", "-C", str(macio)] + list(args), capture_output=True, check=True).stdout


def decode(data):
    return data.decode("utf-8", errors="surrogateescape")


def encode(text):
    return text.encode("utf-8", errors="surrogateescape")


def lines_of(text):
    return text.splitlines(keepends=True)


def wii_side(text):
    """The text the Wii (MWCC) build sees: GALAXY_NATIVE undefined, __MWERKS__ defined."""
    r = subprocess.run(["unifdef", "-U" + MACRO_TO, "-D__MWERKS__"], input=encode(text), capture_output=True)
    if r.returncode not in (0, 1):
        return None
    return decode(r.stdout)


_COMMENT_RE = re.compile(r"//[^\n]*|/\*.*?\*/", re.S)


def normalize(text):
    if text is None:
        return None
    text = _COMMENT_RE.sub(" ", text)
    return [re.sub(r"\s+", " ", l).strip() for l in text.splitlines() if l.strip()]


# Wii-neutral rewrites: MacioGalaxy often replaces Wii code with a macro or typedef whose Wii-side
# definition expands back to the original (JSU_BE(f32) -> f32, JUT_RESTIMG_OFFSET(x) -> x,
# JASAllocationSize -> u32), adds `this->`/`template`/`typename` that MWCC does not need, or widens
# u32 to uintptr_t (identical on the Wii). canon() compares Wii sides modulo those rewrites.
_TOKEN_RE = re.compile(r"[A-Za-z_]\w*|\d[\w.]*|::|->|\S")
_NEUTRAL_INCLUDE_RE = re.compile(r"^\s*#\s*include\s*<(stdint\.h|cstdint|stddef\.h|cstddef|cstring|string\.h)>")
_TYPE_CLASSES = {"U32": [["u32"], ["unsigned", "long"], ["uintptr_t"], ["size_t"], ["uint32_t"], ["std", "::", "size_t"],
                         ["std", "::", "uintptr_t"], ["unsigned", "long", "int"]],
                 "S32": [["s32"], ["long"], ["signed", "long"], ["intptr_t"], ["int32_t"], ["std", "::", "intptr_t"]]}
ALIASES = {}  # name -> (params or None, body tokens), learned from MacioGalaxy's Wii-side definitions
_DEF_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)(\(([^)]*)\))?\s*(.*?)\s*$")
_TYPEDEF_RE = re.compile(r"^\s*typedef\s+([^;{}()]+?)\s+([A-Za-z_]\w*)\s*;\s*$")
_USING_RE = re.compile(r"^\s*using\s+([A-Za-z_]\w*)\s*=\s*([^;{}]+);\s*$")


def tokens(text):
    text = _COMMENT_RE.sub(" ", text)
    return _TOKEN_RE.findall(text)


def learn_aliases(new_wii, old_wii):
    """Record definitions that exist on the Wii side of new_wii but not of old_wii."""
    old = set(l.strip() for l in old_wii.splitlines()) if old_wii else set()
    for line in new_wii.splitlines():
        if line.strip() in old:
            continue
        m = _DEF_RE.match(line)
        if m and not line.rstrip().endswith("\\"):
            params = [x.strip() for x in m.group(3).split(",")] if m.group(2) else None
            ALIASES[m.group(1)] = (params, tokens(m.group(4)))
            continue
        m = _TYPEDEF_RE.match(line)
        if m:
            ALIASES[m.group(2)] = (None, tokens(m.group(1)))
            continue
        m = _USING_RE.match(line)
        if m:
            ALIASES[m.group(1)] = (None, tokens(m.group(2)))


def _is_alias_def(line):
    for rx, grp in ((_DEF_RE, 1), (_TYPEDEF_RE, 2), (_USING_RE, 1)):
        m = rx.match(line)
        if m and m.group(grp) in ALIASES:
            return True
    return False


def _expand(toks, depth=0):
    out, i = [], 0
    while i < len(toks):
        t = toks[i]
        alias = ALIASES.get(t) if depth < 4 else None
        if alias is None:
            out.append(t)
            i += 1
            continue
        params, body = alias
        if params is None:
            out += _expand(body, depth + 1)
            i += 1
            continue
        if i + 1 >= len(toks) or toks[i + 1] != "(":
            out.append(t)
            i += 1
            continue
        args, cur, level, j = [], [], 0, i + 1
        while j < len(toks):
            tk = toks[j]
            if tk == "(":
                level += 1
                if level == 1:
                    j += 1
                    continue
            elif tk == ")":
                level -= 1
                if level == 0:
                    break
            if tk == "," and level == 1:
                args.append(cur)
                cur = []
            else:
                cur.append(tk)
            j += 1
        args.append(cur)
        sub = []
        for bt in body:
            if bt in params and params.index(bt) < len(args):
                sub += args[params.index(bt)]
            else:
                sub.append(bt)
        out += _expand(sub, depth + 1)
        i = j + 1
    return out


def canon(text):
    """Token stream of the Wii side, modulo the neutral rewrites above."""
    wii = wii_side(text)
    if wii is None:
        return None
    lines = [l for l in wii.splitlines() if not _NEUTRAL_INCLUDE_RE.match(l) and not _is_alias_def(l)]
    toks = _expand(tokens("\n".join(lines)))
    out, i = [], 0
    while i < len(toks):
        t = toks[i]
        if t == "this" and i + 1 < len(toks) and toks[i + 1] == "->":
            i += 2
            continue
        if t in ("template", "typename") and out and out[-1] in ("->", ".", "::", "<", ","):
            i += 1
            continue
        if t == "typename":
            i += 1
            continue
        matched = False
        for cls, forms in _TYPE_CLASSES.items():
            for form in sorted(forms, key=len, reverse=True):
                if toks[i:i + len(form)] == form:
                    out.append(cls)
                    i += len(form)
                    matched = True
                    break
            if matched:
                break
        if not matched:
            out.append(t)
            i += 1
    return out


def merge3(ours, base, theirs):
    with tempfile.TemporaryDirectory() as d:
        paths = []
        for name, text in (("ours", ours), ("base", base), ("theirs", theirs)):
            p = os.path.join(d, name)
            with open(p, "wb") as f:
                f.write(encode(text))
            paths.append(p)
        r = subprocess.run(["git", "merge-file", "-p", "-L", "smg2", "-L", "petari", "-L", "macio"] + paths,
                           capture_output=True)
        if r.returncode < 0 or r.returncode > 127:
            return None, -1
        return decode(r.stdout), r.returncode


def hunk_groups(base_lines, theirs_lines):
    ops = [op for op in difflib.SequenceMatcher(None, base_lines, theirs_lines, autojunk=False).get_opcodes()
           if op[0] != "equal"]
    groups, cur, depth = [], [], 0
    for op in ops:
        tag, i1, i2, j1, j2 = op
        cur.append(op)
        for l in theirs_lines[j1:j2]:
            if PP_OPEN.match(l):
                depth += 1
            elif PP_CLOSE.match(l):
                depth -= 1
        for l in base_lines[i1:i2]:
            if PP_OPEN.match(l):
                depth -= 1
            elif PP_CLOSE.match(l):
                depth += 1
        if depth == 0:
            groups.append(cur)
            cur = []
    if cur:
        groups.append(cur)
    return groups


def apply_group(base_lines, theirs_lines, group):
    out, pos = [], 0
    for tag, i1, i2, j1, j2 in group:
        out += base_lines[pos:i1]
        out += theirs_lines[j1:j2]
        pos = i2
    out += base_lines[pos:]
    return "".join(out)


def group_text(base_lines, theirs_lines, group):
    out = []
    for tag, i1, i2, j1, j2 in group:
        out += ["-" + l for l in base_lines[i1:i2]] + ["+" + l for l in theirs_lines[j1:j2]]
    return "".join(out)


def enclosing_function(base_lines, line):
    for k in range(min(line, len(base_lines) - 1), -1, -1):
        m = FUNC_RE.match(base_lines[k])
        if m:
            return m.group(1)
    return ""


def find_ours(tree, rel, index):
    p = tree / rel
    if p.exists():
        return rel
    top = rel.split("/", 1)[0]
    cands = [c for c in index.get(Path(rel).name, []) if c.startswith(top + "/")]
    if len(cands) == 1:
        return cands[0]
    return None


def build_index(tree):
    index = defaultdict(list)
    for top in ("src", "include", "libs"):
        for p in (tree / top).rglob("*"):
            if p.is_file() and p.suffix in SOURCE_EXTS | {".S", ".s"}:
                index[p.name].append(str(p.relative_to(tree)))
    return index


def process_file(rel, base, theirs, tree, index):
    rec = {"path": rel, "groups": []}
    ours_rel = find_ours(tree, rel, index)
    rec["smg2_path"] = ours_rel
    base_lines, theirs_lines = lines_of(base), lines_of(theirs)
    groups = hunk_groups(base_lines, theirs_lines)
    if ours_rel is None:
        rec["status"] = "missing"
        for g in groups:
            entry = {"line": g[0][1] + 1, "kind": "missing", "function": enclosing_function(base_lines, g[0][1]),
                     "text": group_text(base_lines, theirs_lines, g)}
            entry["wii"] = wii_class(base, apply_group(base_lines, theirs_lines, g), entry["text"])
            entry["_group"] = g
            rec["groups"].append(entry)
        return rec, None
    ours = decode((tree / ours_rel).read_bytes())
    rec["identical_to_petari"] = ours == base
    acc = ours
    for g in groups:
        theirs_g = apply_group(base_lines, theirs_lines, g)
        entry = {"line": g[0][1] + 1, "function": enclosing_function(base_lines, g[0][1]),
                 "text": group_text(base_lines, theirs_lines, g)}
        entry["wii"] = cls = wii_class(base, theirs_g, entry["text"])
        if cls in ("comment", "unconditional", "feature"):
            entry["kind"] = cls
        else:
            merged, conflicts = merge3(acc, base, theirs_g)
            if merged is None or conflicts != 0:
                entry["kind"] = "manual"
                entry["reason"] = "conflict" if conflicts > 0 else "merge error"
            elif not wii_equal(cls, merged, acc):
                entry["kind"] = "manual"
                entry["reason"] = "merge changed the Wii side"
            else:
                entry["kind"] = "clean" if ours == base else "adapted"
                acc = merged
        rec["groups"].append(entry)
    finish(rec, acc != ours)
    return rec, (acc if acc != ours else None)


def finish(rec, changed):
    kinds = Counter(e["kind"] for e in rec["groups"])
    rec["kinds"] = dict(kinds)
    if rec.get("smg2_path") is None:
        rec["status"] = "missing" if not kinds["relocated"] else "relocated"
    elif not changed:
        rec["status"] = "unchanged"
    elif kinds["manual"]:
        rec["status"] = "partial"
    else:
        rec["status"] = "applied"


def wii_class(base, theirs_g, text):
    """comment / guarded / neutral / unconditional / feature for one hunk group applied to base."""
    if normalize(theirs_g) == normalize(base):
        return "comment"
    if FEATURE_RE.search(text):
        return "feature"
    if normalize(wii_side(theirs_g)) == normalize(wii_side(base)):
        return "guarded"
    c = canon(theirs_g)
    if c is not None and c == canon(base):
        return "neutral"
    return "unconditional"


def wii_equal(cls, a, b):
    if cls == "guarded":
        return normalize(wii_side(a)) == normalize(wii_side(b))
    ca = canon(a)
    return ca is not None and ca == canon(b)


def relocate(rec, base, theirs, files_state, line_index):
    """Patch-style transfer of a missing file's groups into the SMG2 file that holds the same code
    (SMG2 merged several Petari TUs, e.g. Player's unity TUs). Context must match uniquely."""
    base_lines, theirs_lines = lines_of(base), lines_of(theirs)
    for entry in rec["groups"]:
        g = entry.pop("_group", None)
        if g is None or entry["wii"] not in ("guarded", "neutral"):
            continue
        i1, i2 = g[0][1], g[-1][2]
        for ctx in (3, 2):
            pre, post = base_lines[max(0, i1 - ctx):i1], base_lines[i2:i2 + ctx]
            pattern = pre + base_lines[i1:i2] + post
            key = [k for k in (norm_line(l) for l in pattern) if k]
            if len(key) < 3:
                continue
            rare = min(key, key=lambda k: len(line_index.get(k, ())))
            hits = []
            for path in sorted(line_index.get(rare, ())):
                text = files_state[path]
                hits += [(path, m) for m in find_block(lines_of(text), key)]
            if len(hits) != 1:
                entry["reason"] = f"context matches {len(hits)} places"
                continue
            path, (s0, s1) = hits[0]
            cur = lines_of(files_state[path])
            body, pos = [], i1
            for tag, a1, a2, b1, b2 in g:
                body += base_lines[pos:a1] + theirs_lines[b1:b2]
                pos = a2
            body += base_lines[pos:i2]
            repl = "".join(pre + body + post)
            new = "".join(cur[:s0]) + repl + "".join(cur[s1:])
            if not wii_equal(entry["wii"], new, files_state[path]):
                entry["reason"] = "relocation changed the Wii side"
                break
            files_state[path] = new
            entry["kind"] = "relocated"
            entry["target"] = path
            entry.pop("reason", None)
            break
    finish(rec, False)


def norm_line(l):
    return re.sub(r"\s+", " ", _COMMENT_RE.sub(" ", l)).strip()


def find_block(lines, key):
    """(start, end) line spans in lines whose non-blank normalized lines equal key."""
    norm = [(i, norm_line(l)) for i, l in enumerate(lines)]
    norm = [(i, k) for i, k in norm if k]
    out = []
    for n in range(len(norm) - len(key) + 1):
        if norm[n][1] == key[0] and all(norm[n + m][1] == key[m] for m in range(len(key))):
            out.append((norm[n][0], norm[n + len(key) - 1][0] + 1))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--macio", type=Path, default=Path.home() / "MacioGalaxy")
    ap.add_argument("--base", default="e5bc761c0")
    ap.add_argument("--head", default="HEAD")
    ap.add_argument("--tree", type=Path, default=Path("."), help="SMG2 tree to read (and with --apply, write)")
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--overlay", action="store_true", help="write a full hooked copy of the tree to OUT/overlay")
    ap.add_argument("--apply", action="store_true", help="write merged files into --tree")
    ap.add_argument("--only", action="append", default=[], help="path prefix filter (repeatable)")
    args = ap.parse_args()
    tree = args.tree.resolve()
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)

    status = decode(git(args.macio, "diff", "--name-status", "--no-renames", args.base, args.head,
                        "--", "src", "include", "libs"))
    index = build_index(tree)
    files, merged_files, added, todo = [], {}, [], []
    for line in status.splitlines():
        st, rel = line.split("\t", 1)
        if args.only and not any(rel.startswith(o) for o in args.only):
            continue
        if st == "A":
            kind = "deferred-feature" if FEATURE_RE.search(rel) else (
                "copy" if NATIVE_ADDED_RE.search(rel) else "deferred-review")
            exists = (tree / rel).exists()
            added.append({"path": rel, "kind": "exists" if exists else kind})
            text = MACRO_RE.sub(MACRO_TO, decode(git(args.macio, "show", f"{args.head}:{rel}")))
            if Path(rel).suffix in SOURCE_EXTS and kind != "deferred-feature":
                learn_aliases(wii_side(text) or "", None)
            if kind == "copy" and not exists:
                merged_files[rel] = text
            continue
        if st != "M" or Path(rel).suffix not in SOURCE_EXTS:
            files.append({"path": rel, "status": "skipped-" + st})
            continue
        base = decode(git(args.macio, "show", f"{args.base}:{rel}"))
        theirs = MACRO_RE.sub(MACRO_TO, decode(git(args.macio, "show", f"{args.head}:{rel}")))
        learn_aliases(wii_side(theirs) or "", wii_side(base))
        todo.append((rel, base, theirs))
    # Aliases whose Wii-side body is the name itself (e.g. `#define override`) are not rewrites.
    for name in [n for n, (params, body) in ALIASES.items() if body == [n]]:
        del ALIASES[name]
    print(f"learned {len(ALIASES)} Wii-side aliases", file=sys.stderr)

    missing = []
    for rel, base, theirs in todo:
        rec, merged = process_file(rel, base, theirs, tree, index)
        files.append(rec)
        if merged is not None:
            merged_files[rec["smg2_path"]] = merged
        if rec["status"] == "missing":
            missing.append((rec, base, theirs))
        print(f"{rec['status']:9} {rel}", file=sys.stderr)

    # Relocate the groups of Petari files that SMG2 merged into other TUs.
    files_state, line_index = {}, defaultdict(set)
    for top in ("src", "include", "libs"):
        for p in (tree / top).rglob("*"):
            if p.is_file() and p.suffix in SOURCE_EXTS:
                rel = str(p.relative_to(tree))
                files_state[rel] = merged_files.get(rel) or decode(p.read_bytes())
                for l in files_state[rel].splitlines():
                    k = norm_line(l)
                    if k:
                        line_index[k].add(rel)
    before = dict(files_state)
    for rec, base, theirs in missing:
        relocate(rec, base, theirs, files_state, line_index)
    for rel, text in files_state.items():
        if text != before[rel]:
            merged_files[rel] = text
    for rec in files:
        for g in rec.get("groups", []):
            g.pop("_group", None)

    if args.overlay:
        ov = out / "overlay"
        if ov.exists():
            shutil.rmtree(ov)
        for top in ("src", "include", "libs", "native"):
            if (tree / top).exists():
                shutil.copytree(tree / top, ov / top, symlinks=True)
        for rel, text in merged_files.items():
            (ov / rel).parent.mkdir(parents=True, exist_ok=True)
            (ov / rel).write_bytes(encode(text))
    if args.apply:
        for rel, text in merged_files.items():
            (tree / rel).parent.mkdir(parents=True, exist_ok=True)
            (tree / rel).write_bytes(encode(text))

    manual_dir = out / "manual"
    if manual_dir.exists():
        shutil.rmtree(manual_dir)
    for rec in files:
        todo = [g for g in rec.get("groups", []) if g["kind"] in ("manual", "unconditional", "missing")]
        if not todo:
            continue
        p = manual_dir / (rec["path"] + ".diff")
        p.parent.mkdir(parents=True, exist_ok=True)
        with open(p, "w", encoding="utf-8", errors="surrogateescape") as f:
            for g in todo:
                f.write(f"@@ {g['kind']}{' (' + g['reason'] + ')' if 'reason' in g else ''} "
                        f"petari line {g['line']} in {g['function'] or '?'}\n{g['text']}\n")

    write_report(out, files, added, args)
    return 0


def top_dir(path):
    parts = path.split("/")
    if parts[0] == "src" and len(parts) > 2:
        return "/".join(parts[:3])
    if parts[0] == "include" and len(parts) > 2:
        return "/".join(parts[:3])
    if parts[0] == "libs" and len(parts) > 3:
        return "/".join(parts[:4]) if parts[2] == "include" else "/".join(parts[:2])
    return "/".join(parts[:2])


def write_report(out, files, added, args):
    kinds_total = Counter()
    status_total = Counter()
    by_dir = defaultdict(Counter)
    for rec in files:
        status_total[rec["status"]] += 1
        for g in rec.get("groups", []):
            kinds_total[g["kind"]] += 1
            if g["kind"] in ("clean", "adapted", "relocated"):
                kinds_total["applied-" + g.get("wii", "?")] += 1
            by_dir[top_dir(rec["path"])][g["kind"]] += 1
    report = {"macro": {MACRO_FROM: MACRO_TO}, "macio_base": args.base, "macio_head": args.head,
              "file_status": status_total, "group_kinds": kinds_total, "added": added, "files": files}
    (out / "report.json").write_text(json.dumps(report, indent=1, default=dict) + "\n")
    cols = ["clean", "adapted", "relocated", "manual", "unconditional", "feature", "missing", "comment"]
    md = ["# Hook transfer report", "",
          f"MacioGalaxy {args.base}..{args.head} onto {args.tree}. Macro mapping: {MACRO_FROM} -> {MACRO_TO}.", "",
          "Hook groups: " + ", ".join(f"{k} {kinds_total[k]}" for k in cols + ["applied-guarded", "applied-neutral"]), "",
          "Files: " + ", ".join(f"{k} {v}" for k, v in sorted(status_total.items())), "",
          "- clean/adapted: applied (Wii side verified unchanged with unifdef -UGALAXY_NATIVE -D__MWERKS__).",
          "  `applied-guarded` hooks change nothing on the Wii side; `applied-neutral` ones rewrite Wii code into"
          " a form that expands to the same tokens (JSU_BE(T) -> T, `this->`, u32 -> uintptr_t, ...); the DOL"
          " hash check at landing is the final proof.",
          "- relocated: hook from a Petari file that SMG2 merged into another TU, applied by unique context match.",
          "- manual: guarded hook that conflicts with SMG2 changes; port by hand (manual/<path>.diff).",
          "- unconditional: MacioGalaxy changed Wii-visible code (bug fix or clang syntax fix); not applied;"
          " port natively behind GALAXY_NATIVE where clang needs it.",
          "- feature: hook for a deferred MacioGalaxy feature (editor, test server, mods); P3.",
          "- missing: no SMG2 file at that path or with that unique basename.", "",
          "| dir | " + " | ".join(cols) + " |", "|---|" + "---:|" * len(cols)]
    for d in sorted(by_dir):
        md.append(f"| {d} | " + " | ".join(str(by_dir[d][k]) for k in cols) + " |")
    md += ["", "## Added files", "", "| file | action |", "|---|---|"]
    md += [f"| {a['path']} | {a['kind']} |" for a in added]
    md += ["", "## Files with manual work (manual + unconditional + missing groups)", ""]
    for rec in sorted(files, key=lambda r: r["path"]):
        k = rec.get("kinds", {})
        n = k.get("manual", 0) + k.get("unconditional", 0)
        if rec["status"] == "missing":
            md.append(f"- {rec['path']}: missing in SMG2 ({len(rec['groups'])} groups)")
        elif n:
            md.append(f"- {rec['path']}: manual {k.get('manual', 0)}, unconditional {k.get('unconditional', 0)}")
    (out / "REPORT.md").write_text("\n".join(md) + "\n")
    print(f"groups: {dict(kinds_total)}; files: {dict(status_total)}; report: {out / 'REPORT.md'}")


if __name__ == "__main__":
    raise SystemExit(main())
