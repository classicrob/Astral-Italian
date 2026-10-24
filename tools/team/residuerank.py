#!/usr/bin/env python3
"""rank.py REPORT [--fns]: residue by unit and by fuzzy band."""
import json, sys, collections
r = json.load(open(sys.argv[1]))
m = r['measures']; print({k: m[k] for k in m if 'code' in k or 'func' in k})
rows = []; band = collections.Counter(); bandn = collections.Counter()
for u in r['units']:
    for f in u.get('functions') or []:
        z = f.get('fuzzy_match_percent') or 0
        if z >= 100: continue
        sz = int(f['size']); rows.append((u['name'][5:], f['name'], sz, z))
        b = 'none' if z == 0 else '<50' if z < 50 else '50-80' if z < 80 else '80-95' if z < 95 else '95-99' if z < 99 else '99+'
        band[b] += sz; bandn[b] += 1
print('total residue', sum(x[2] for x in rows), len(rows))
for b in ['none', '<50', '50-80', '80-95', '95-99', '99+']: print(f'{b:6} {band[b]:8} B {bandn[b]:5} fns')
area = collections.Counter()
for un, fn, sz, z in rows:
    p = un.split('/'); area['/'.join(p[:2]) if p[0] in ('Game',) else p[0] + '/' + (p[1] if len(p) > 1 else '')] += sz
print('--- areas'); [print(f'{v:8} {k}') for k, v in area.most_common(25)]
unit = collections.Counter(); un_n = collections.Counter()
for un, fn, sz, z in rows: unit[un] += sz; un_n[un] += 1
print('--- units'); [print(f'{v:8} {un_n[k]:4} {k}') for k, v in unit.most_common(40)]
if '--fns' in sys.argv:
    print('--- biggest fns'); [print(f'{sz:6} {z:6.2f} {un} {fn}') for un, fn, sz, z in sorted(rows, key=lambda x: -x[2])[:60]]
