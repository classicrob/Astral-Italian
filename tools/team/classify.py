#!/usr/bin/env python3
"""Classify every unmatched function's diff: RELOC (only relocation-target identity differs, text same),
REG (only register operands differ, plus RELOC), STRUCT (anything else). Run from a built worktree root.
usage: classify.py REPORT.json OUTDIR [unit-prefix]   (writes OUTDIR/../classes.tsv: unit fn class size ndiff match%)"""
import json, sys, os, subprocess, collections, concurrent.futures as cf
rep, outd = sys.argv[1], sys.argv[2]
pref = sys.argv[3] if len(sys.argv) > 3 else ""
r = json.load(open(rep))
units = {}
for u in r['units']:
    if not u['name'].startswith('main/' + pref): continue
    bad = [f for f in (u.get('functions') or []) if (f.get('fuzzy_match_percent') or 0) < 100]
    if bad: units[u['name']] = bad
def run(un):
    out = os.path.join(outd, un.replace('/', '__') + '.json')
    p = subprocess.run(['build/tools/objdiff-cli', '-L', 'error', 'diff', '-p', '.', '-u', un, '-o', out], capture_output=True, text=True)
    return un, out if p.returncode == 0 else None
def regs(ins):
    return [p['arg'].get('opaque') for p in ins['parts'] if 'arg' in p]
def classify(L, R):
    li, ri = L['instructions'], R.get('instructions', [])
    if len(li) != len(ri): return 'STRUCT', 0
    worst = 'MATCH'; n = 0
    for a, b in zip(li, ri):
        k = a.get('diff_kind') or b.get('diff_kind')
        if not k: continue
        n += 1
        if k != 'DIFF_ARG_MISMATCH': return 'STRUCT', n
        A, B = a['instruction'], b['instruction']
        pa = [p for p in A['parts']]; pb = [p for p in B['parts']]
        if len(pa) != len(pb): return 'STRUCT', n
        kind = 'RELOC'
        for x, y in zip(pa, pb):
            if x == y: continue
            xa, ya = x.get('arg', {}), y.get('arg', {})
            if xa.get('reloc') and ya.get('reloc'): continue
            if 'opaque' in xa and 'opaque' in ya and xa['opaque'][:1] in 'rf' and ya['opaque'][:1] in 'rf' and xa['opaque'][1:].isdigit():
                kind = 'REG'; continue
            # numeric immediate differing but same text with reloc present -> reloc identity of pooled data
            if A.get('relocation') and B.get('relocation') and x == y: continue
            if A.get('relocation') and B.get('relocation'):
                kind = 'RELOCOFF' if kind == 'RELOC' else kind; continue
            return 'STRUCT', n
        if A['formatted'] == B['formatted'] and kind == 'RELOC': kind = 'RELOC'
        order = ['MATCH', 'RELOC', 'RELOCOFF', 'REG']
        if order.index(kind) > order.index(worst): worst = kind
    return worst, n
res = []
with cf.ThreadPoolExecutor(6) as ex:
    for un, out in ex.map(run, units):
        if not out: res.append((un, '*', 'NODIFF', 0, 0)); continue
        d = json.load(open(out)); os.unlink(out)
        if 'right' not in d:
            for L in d['left']['symbols']:
                if L.get('instructions') and (L.get('match_percent') or 0) < 100: res.append((un, L['name'], 'NOSRC', int(L['size']), 0))
            continue
        right = d['right']['symbols']
        byname = {s['name']: s for s in right}
        for L in d['left']['symbols']:
            if not L.get('instructions') or (L.get('match_percent') or 0) >= 100: continue
            R = byname.get(L['name'])
            if R is None and L.get('target_symbol'): R = right[L['target_symbol'] - 1]
            if R is None: res.append((un, L['name'], 'MISSING', int(L['size']), 0)); continue
            c, n = classify(L, R)
            res.append((un, L['name'], c, int(L['size']), n, round(L.get('match_percent') or 0, 2)))
with open(os.path.join(outd, '..', 'classes.tsv'), 'w') as f:
    for x in res: f.write('\t'.join(map(str, x)) + '\n')
tot = collections.Counter(); cnt = collections.Counter()
for x in res: tot[x[2]] += x[3]; cnt[x[2]] += 1
for k, v in tot.most_common(): print(k, v, cnt[k])
