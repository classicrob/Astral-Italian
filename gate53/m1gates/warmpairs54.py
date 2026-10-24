#!/usr/bin/env python3
"""5.4 warm-cost check (head): alternating warm hybrid vs off pairs. Per case: focused gameplay frame-interval p50/p95/p99/max
and frames > 20 ms from frames.csv, plus whether the hybrid arm used any fallback ([gx uber frames] fallback_frames) and the
effective mode line. Verdict per stage: hybrid p99 "reliably worse" = worse in EVERY pair by more than 0.5 ms.
  warmpairs54.py <port53 dir> <label prefix> <stage>... -> JSON on stdout
"""
import csv, json, os, re, sys

def stats(d):
    iv = sorted(float(x['interval_ms']) for x in csv.DictReader(open(os.path.join(d, 'frames.csv')))
                if x['phase'] == 'gameplay' and x['unfocused'] == '0')
    q = lambda p: iv[min(len(iv) - 1, int(p * len(iv)))]
    log = open(os.path.join(d, 'app.log'), errors='replace').read()
    fb = re.findall(r'\[gx uber frames\].*?fallback_frames=(\d+)', log)
    mode = re.findall(r'\[gx uber\] mode=(\S+) source=(\S+)', log)
    return {'frames': len(iv), 'p50': q(0.50), 'p95': q(0.95), 'p99': q(0.99), 'max': iv[-1], 'over20': sum(v > 20 for v in iv),
            'fallback_frames': int(fb[-1]) if fb else None, 'mode': mode[0] if mode else None}

def main():
    root, prefix, stages = sys.argv[1], sys.argv[2], sys.argv[3:]
    out = {}
    for st in stages:
        pairs = []
        for r in (1, 2, 3):
            h = stats(os.path.join(root, f'{prefix}-{st}-r{r}-hybrid')); o = stats(os.path.join(root, f'{prefix}-{st}-r{r}-off'))
            pairs.append({'round': r, 'hybrid': h, 'off': o, 'p99_delta_ms': round(h['p99'] - o['p99'], 3)})
        worse = all(p['p99_delta_ms'] > 0.5 for p in pairs)
        out[st] = {'pairs': pairs, 'hybrid_p99_reliably_worse': worse,
                   'hybrid_fallback_frames': [p['hybrid']['fallback_frames'] for p in pairs]}
    print(json.dumps(out, indent=1))

main()
