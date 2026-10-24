#!/usr/bin/env python3
"""cmprep.py BASE.json NEW.json : overall matched code delta + per-unit matched-code changes (drops flagged)."""
import json, sys
def load(p):
    r = json.load(open(p)); u = {}
    for x in r['units']:
        m = x.get('measures', {})
        u[x['name']] = (int(m.get('matched_code', 0) or 0), int(m.get('matched_data', 0) or 0), int(m.get('matched_functions', 0) or 0))
    return r['measures'], u
bm, bu = load(sys.argv[1]); nm, nu = load(sys.argv[2])
print(f"code {bm['matched_code']} -> {nm['matched_code']} ({int(nm['matched_code'])-int(bm['matched_code']):+d}) = {nm['matched_code_percent']:.4f}%  fns {bm['matched_functions']} -> {nm['matched_functions']}  data {bm['matched_data']} -> {nm['matched_data']}")
drops = 0
for k in sorted(set(bu) | set(nu)):
    b = bu.get(k, (0, 0, 0)); n = nu.get(k, (0, 0, 0))
    if b != n:
        tag = 'DROP' if n[0] < b[0] or n[1] < b[1] else '    '
        drops += tag == 'DROP'
        print(f"{tag} {k[5:]}: code {b[0]}->{n[0]} data {b[1]}->{n[1]} fns {b[2]}->{n[2]}")
print('DROPS', drops)
