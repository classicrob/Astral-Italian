#!/usr/bin/env python3
"""Summarize a thrprobe TSV: per thread, name, base/cur priority range, rt flag, CPU ms used between two sample times.
  thrsum.py <tsv> <t0> <t1>"""
import sys, collections
rows = collections.defaultdict(dict)
for line in open(sys.argv[1]):
    f = line.rstrip('\n').split('\t')
    kv = dict(x.split('=', 1) for x in f[3:])
    rows[f[1]][float(f[0])] = (f[2], int(kv['base']), int(kv['cur']), int(kv['rt']), int(kv['cpu_ms']), int(kv['policy']))
t0, t1 = float(sys.argv[2]), float(sys.argv[3])
out = []
for tid, s in rows.items():
    ts = sorted(t for t in s if t0 <= t <= t1)
    if not ts: continue
    name = s[ts[-1]][0]; bases = {s[t][1] for t in ts}; curs = [s[t][2] for t in ts]; rt = max(s[t][3] for t in ts)
    cpu = s[ts[-1]][4] - s[ts[0]][4]
    out.append((cpu, tid, name, min(bases), max(bases), min(curs), max(curs), rt))
print(f"{'cpu_ms':>7} {'base':>7} {'cur':>7} rt  name   (window {t0}-{t1} s)")
for cpu, tid, name, b0, b1, c0, c1, rt in sorted(out, key=lambda x: (-x[3], -x[0])):
    print(f"{cpu:7d} {b0:3d}-{b1:<3d} {c0:3d}-{c1:<3d} {rt}   {name} [{tid}]")
