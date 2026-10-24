#!/usr/bin/env python3
"""5.3.3 true-cold gate (head 2026-10-23) over cold533.sh cases (normal route or direct stage):
  FAIL if any gameplay-phase frame is over 100 ms, or any menu / loading / startup hold is over 3,000 ms
  (per-frame [stall] lines and the loading summary lines), or if focused menu/startup frames have p99 over 25 ms
  (head: true-cold title-route menus must stay ~20 ms, the 26feb80a goal), and, for warm cases, if late on-screen presents
  (present_ms > 20 at 120 Hz, from the frame CSV) are 1 % or more of the gameplay frames.
  gate533.py <case dir> [<case dir> ...] [--warm]  -> one JSON line per case (and exit 1 if any case fails)
"""
import csv, json, os, re, sys

STALL = re.compile(r'^\[stall\] frame (\d+) ([\d.]+) ms phase=(\w+)(.*)$')
LOADSUM = re.compile(r'^\[stall\] (\d+) frames over the threshold during (\w+), worst ([\d.]+) ms')

def case(d, warm):
    log = open(os.path.join(d, 'app.log'), errors='replace').read().splitlines()
    big_play, holds, causes = [], [], {}
    for line in log:
        m = STALL.match(line)
        if m:
            frame, ms, phase, rest = int(m.group(1)), float(m.group(2)), m.group(3), m.group(4)
            cause = re.search(r'cause=(\S+)', rest)
            if phase == 'gameplay' and ms > 100:
                big_play.append((frame, ms, cause.group(1) if cause else '?', re.search(r'stage (\S+)', rest).group(1) if 'stage ' in rest else '?'))
            elif phase != 'gameplay' and ms > 3000:
                holds.append((frame, ms, phase))
            continue
        m = LOADSUM.match(line)
        if m and float(m.group(3)) > 3000:
            holds.append(('summary', float(m.group(3)), m.group(2)))
    bound = sum('complete-scene wait hit its' in l for l in log)
    play = over20 = late = 0
    menu = []
    path = os.path.join(d, 'frames.csv')
    if os.path.exists(path):
        for x in csv.DictReader(open(path)):
            if x['phase'] in ('menu', 'startup') and x['unfocused'] == '0':
                menu.append(float(x['interval_ms']))
            if x['phase'] != 'gameplay' or x['unfocused'] != '0':
                continue
            play += 1
            over20 += float(x['interval_ms']) > 20
            late += bool(x['present_ms']) and float(x['present_ms']) > 20
    late_pct = 100.0 * late / play if play else None
    menu_sorted = sorted(menu)
    menu_p99 = round(menu_sorted[max(0, int(0.99 * len(menu_sorted)) - 1)], 2) if menu_sorted else None
    fails = []
    if menu_p99 is not None and menu_p99 > 25: fails.append('menu p99 %.1f ms' % menu_p99)
    if big_play: fails.append('%d gameplay frames over 100 ms (worst %.0f ms)' % (len(big_play), max(b[1] for b in big_play)))
    if holds: fails.append('%d holds over 3 s (worst %.0f ms)' % (len(holds), max(h[1] for h in holds)))
    if warm and (late_pct is None or late_pct >= 1.0): fails.append('late presents %s %%' % (None if late_pct is None else round(late_pct, 2)))
    return {'case': os.path.basename(d), 'warm': warm, 'result': 'FAIL' if fails else 'PASS', 'fails': fails,
            'gameplay_over100': big_play[:12], 'holds_over3s': holds[:12], 'complete_scene_bound_hits': bound,
            'menu_frames': len(menu), 'menu_p99': menu_p99, 'menu_over50': sum(v > 50 for v in menu),
            'menu_max': round(max(menu), 1) if menu else None,
            'gameplay_frames': play, 'gameplay_over20': over20, 'late_presents': late,
            'late_present_pct': None if late_pct is None else round(late_pct, 2)}

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    warm = '--warm' in sys.argv
    bad = False
    for d in args:
        r = case(d, warm)
        bad |= r['result'] == 'FAIL'
        print(json.dumps(r))
    sys.exit(1 if bad else 0)

main()
