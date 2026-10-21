#!/usr/bin/env python3
"""Per-galaxy table for native/tools/perf_sweep.sh output (advisor r30): warm-pass gameplay frame times (game-thread
p50/p99/max), late on-screen presents (intervals longer than the 60 fps cadence: more than refresh_hz/60 refreshes),
the worst input-to-present latency (frames after the game's read, and ms from the press), and whether the machine
was quiet. Pass line: 0 late presents warm.

  perf_table.py <out_dir> [--md]
"""
import re, sys, os, glob

def parse(stage_dir):
    log = os.path.join(stage_dir, 'warm.log')
    info = open(os.path.join(stage_dir, 'info.txt')).read() if os.path.exists(os.path.join(stage_dir, 'info.txt')) else ''
    row = {'stage': os.path.basename(stage_dir), 'quiet': 'NO' not in info and 'quiet=yes' in info,
           'script': 'warm end' in info and info.rstrip().split('script_done=')[-1].startswith('1')}
    if not os.path.exists(log):
        return row
    text = open(log, errors='replace').read()
    m = re.search(r'^  gameplay\s+(\d+)\s+[\d.]+\s+([\d.]+)\s+[\d.]+\s+([\d.]+)\s+([\d.]+)\s+\d+\s+\d+\s+(\d+)', text, re.M)
    if m:
        row.update(frames=int(m.group(1)), p50=float(m.group(2)), p99=float(m.group(3)), max=float(m.group(4)), late_game=int(m.group(5)))
    m = re.search(r'Petari present intervals \(on-screen, (\d+)\): p50 ([\d.]+) p95 [\d.]+ p99 ([\d.]+) max ([\d.]+).*?'
                  r'intervals of 1/2/3/4\+ refreshes (\d+)/(\d+)/(\d+)/(\d+); display ([\d.]+) Hz', text)
    if m:
        buckets = [int(m.group(i)) for i in range(5, 9)]
        hz = float(m.group(9)); per_frame = max(1, round(hz / 60.0))
        row.update(present_p99=float(m.group(3)), present_max=float(m.group(4)), hz=hz,
                   late_present=sum(buckets[per_frame:]), early_present=sum(buckets[:per_frame - 1]))
    lat = [(int(a), float(b)) for a, b in re.findall(r'\[latency\] press:.*\((?:frame \d+), (\d+) frames after the read\).*on screen ([\d.]+) ms', text)]
    if lat:
        row.update(lat_frames=max(a for a, _ in lat), lat_ms=max(b for _, b in lat), presses=len(lat))
    return row

def main():
    out = sys.argv[1]
    rows = [parse(d) for d in sorted(glob.glob(os.path.join(out, '*'))) if os.path.isdir(d)]
    head = '| stage | quiet | script | late presents | early presents | present p99/max ms | game p50/p99/max ms | late game frames | worst latency |'
    print(head); print('|' + '---|' * head.count('|')[:-1] if False else '|---|---|---|---|---|---|---|---|---|')
    for r in rows:
        f = lambda k, fmt='%s': (fmt % r[k]) if k in r else '-'
        print('| %s | %s | %s | %s | %s | %s/%s | %s/%s/%s | %s | %s |' % (
            r['stage'], 'yes' if r['quiet'] else 'NO', 'yes' if r['script'] else 'no', f('late_present'), f('early_present'),
            f('present_p99', '%.1f'), f('present_max', '%.1f'), f('p50', '%.1f'), f('p99', '%.1f'), f('max', '%.1f'), f('late_game'),
            ('%d frames after read, %.0f ms from press (%d presses)' % (r['lat_frames'], r['lat_ms'], r['presses'])) if 'lat_ms' in r else '-'))
    measured = [r for r in rows if 'late_present' in r]
    print('\nstages measured %d; with 0 late presents %d; quiet %d' % (len(measured), sum(r['late_present'] == 0 for r in measured),
                                                                       sum(r['quiet'] for r in rows)))

if __name__ == '__main__':
    main()
