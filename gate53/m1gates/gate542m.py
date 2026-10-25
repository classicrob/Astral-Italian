#!/usr/bin/env python3
"""5.4.2 M1 menu scorer; gate533 remains unchanged.

Every non-gameplay frame after the first explicitly presented frame: <=250ms.
Title arms additionally require real UI-global100 and >=30s post100 samples;
menu p99 <=33ms while showing. Post100: M1 <=33ms with zero compile work; M4 <=22ms.
Historical runs without explicit state events are split-unscorable, never guessed.
--window-seconds reports the same fixed interval for unsplit build comparison.
"""
import argparse
import csv
import hashlib
import json
import math
from pathlib import Path
import re

PREFIX = '[graphics preparation] '
KNOWN = {'initial', 'shown', 'hidden', 'actual-global-100', 'title-observation-progress',
         'title-observation-complete', 'title-observation-timeout'}


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def stats(rows):
    values = sorted(float(r['interval_ms']) for r in rows)
    return {'frames': len(values), 'p99_ms': values[max(0, int(.99 * len(values)) - 1)] if values else None,
            'max_ms': max(values) if values else None, 'over50': sum(v > 50 for v in values)}


def state_events(text):
    result, errors = [], []
    for number, line in enumerate(text.splitlines(), 1):
        if not line.startswith(PREFIX):
            continue
        fields = dict(re.findall(r'(\w+)=([^ ]+)', line[len(PREFIX):]))
        try:
            event = fields['event']
            if event not in KNOWN or fields['version'] != '1' or fields['clock'] != 'frame-csv-interval' or fields['source'] != 'ui-specialized-global':
                raise ValueError('unknown event/version/clock/provenance')
            for key in ('frame', 'polled', 'valid', 'visible', 'complete', 'done', 'total'):
                fields[key] = int(fields[key])
                if fields[key] < 0: raise ValueError('negative state')
            for key in ('polled', 'valid', 'visible', 'complete'):
                if fields[key] not in (0, 1): raise ValueError('invalid boolean')
            fields['elapsed_s'] = float(fields['elapsed_s'])
            if not math.isfinite(fields['elapsed_s']) or fields['elapsed_s'] < 0:
                raise ValueError('invalid elapsed clock')
            if result and (fields['frame'] < result[-1]['frame'] or fields['elapsed_s'] < result[-1]['elapsed_s']):
                raise ValueError('state clock moved backwards')
            if event == 'actual-global-100' and not (fields['polled'] and fields['valid'] and fields['complete'] and fields['total'] > 0 and fields['done'] >= fields['total']):
                raise ValueError('false global100 state')
            fields.update(log_line=number, raw=line)
            result.append(fields)
        except (KeyError, ValueError) as exc:
            errors.append({'line': number, 'error': str(exc), 'raw': line})
    return result, errors


def deadline_requirements(values):
    result = {}
    for value in values:
        match = re.fullmatch(r'([A-Za-z0-9_-]+):([1-9][0-9]*)', value)
        if not match or match[1] in result:
            raise ValueError('deadline requirement must be unique Stage:positive-count')
        result[match[1]] = int(match[2])
    return result


def deadline_admissions(text, required):
    result = {}
    for stage, count in required.items():
        lines = [line for line in text.splitlines() if line.startswith('[gx first-use deadlines] stage=' + stage + ' ')]
        expected = f'[gx first-use deadlines] stage={stage} policy=frame-then-cost reason=validated configs={count}'
        result[stage] = {'expected_configs': count, 'observed_lines': lines,
                         'passed': bool(lines) and all(line == expected for line in lines)}
    return result


def frontend_pre_present(text):
    lines = [line for line in text.splitlines() if line.startswith('[gx frontend pre-present] ')]
    evidence = {'lines': lines, 'passed': False}
    if len(lines) != 1: return evidence
    f = dict(re.findall(r'(\w+)=([^ ]+)', lines[0]))
    try:
        for key in ('version', 'keys', 'completed', 'pending', 'failed', 'manifest_errors', 'cap_ms', 'workers', 'pumps', 'main_thread', 'presented_frames'): f[key] = int(f[key])
        for key in ('elapsed_ms', 'pump_max_gap_ms'):
            f[key] = float(f[key])
            if not math.isfinite(f[key]) or f[key] < 0: raise ValueError('invalid timing')
        keys = f['key_list'].rstrip(',').split(',')
        evidence['passed'] = (f['version'] == 1 and f['result'] == 'ready' and 0 < f['keys'] <= 2048 and
            f['keys'] == f['completed'] == len(keys) == len(set(keys)) and all(re.fullmatch('[0-9a-f]{16}', k) for k in keys) and
            f['pending'] == f['failed'] == f['manifest_errors'] == f['presented_frames'] == 0 and
            f['cap_ms'] == 5000 and f['elapsed_ms'] <= f['cap_ms'] and f['workers'] > 0 and f['pumps'] > 0 and f['main_thread'] == 1)
        evidence['fields'] = f
    except (KeyError, ValueError) as exc: evidence['error'] = str(exc)
    return evidence


def score(folder, title=False, window_seconds=300.0, require_deadlines=None, *, tier, require_frontend=False):
    if tier not in ('m1', 'm4'): raise ValueError('explicit tier m1 or m4 required')
    post_limit = 33 if tier == 'm1' else 22
    folder = Path(folder)
    text = (folder / 'app.log').read_text(errors='replace')
    with (folder / 'frames.csv').open() as file:
        rows = list(csv.DictReader(file))
    events, errors = state_events(text)
    presented = [int(r['presented_frame']) for r in rows if r.get('presented_frame', '').strip()]
    first = min(presented) if presented else None
    elapsed, last_frame, timed = 0.0, -1, []
    for row in rows:
        frame, ms = int(row['frame']), float(row['interval_ms'])
        if frame <= last_frame or not math.isfinite(ms) or ms < 0:
            raise ValueError('CSV frame order/interval invalid')
        last_frame = frame
        if first is not None and frame >= first:
            elapsed += ms / 1000
            timed.append(dict(row, elapsed_s=elapsed))
    nonplay = [r for r in timed if r['phase'] != 'gameplay']
    menu = [r for r in nonplay if r['phase'] in ('startup', 'menu', 'loading', 'load', 'transition')]
    gameplay = [r for r in timed if r['phase'] == 'gameplay']
    over250 = [r for r in nonplay if float(r['interval_ms']) > 250]
    over100 = [r for r in gameplay if float(r['interval_ms']) > 100]
    initial = [e for e in events if e['event'] == 'initial']
    complete = [e for e in events if e['event'] == 'actual-global-100']
    finish = [e for e in events if e['event'] == 'title-observation-complete']
    showing, post = [], []
    visible = False
    cursor = 0
    completion_frame = complete[0]['frame'] if len(complete) == 1 else None
    for row in menu:
        frame = int(row['frame'])
        while cursor < len(events) and events[cursor]['frame'] <= frame:
            visible = bool(events[cursor]['visible']); cursor += 1
        if visible: showing.append(row)
        if completion_frame is not None and frame >= completion_frame: post.append(row)
    split_status = 'scorable'
    if not events: split_status = 'split unscorable (no signal)'
    elif errors or len(initial) != 1 or len(complete) > 1: split_status = 'split unscorable (invalid signal)'
    elif not complete: split_status = 'no completion observed'
    elif not post: split_status = 'no post-100 samples'
    compile_errors, compile_rows = [], []
    for row in post:
        try:
            wait, resolves = float(row['pipeline_wait_ms']), int(row['pipeline_resolves'])
            if not math.isfinite(wait) or wait < 0 or resolves < 0: raise ValueError('invalid compile counters')
            if wait or resolves: compile_rows.append(row)
        except (KeyError, ValueError) as exc: compile_errors.append({'frame': row['frame'], 'error': str(exc)})
    post_compile_free = bool(post) and not compile_errors and not compile_rows
    post_seconds = sum(float(r['interval_ms']) / 1000 for r in post)
    checks = {'first_presented_frame_observed': first is not None, 'timing_rows': bool(timed),
              'no_non_gameplay_over250': not over250, 'no_gameplay_over100': not over100, 'state_events_valid': not errors}
    if title:
        checks.update(no_gameplay_in_title=not gameplay, explicit_split_scorable=split_status == 'scorable',
                      real_global100=len(complete) == 1, self_ended_observation=len(finish) == 1,
                      no_state_errors=not errors, within15minutes=len(finish) == 1 and finish[0]['elapsed_s'] <= 900,
                      post100_30seconds=len(finish) == 1 and len(complete) == 1 and
                      finish[0]['elapsed_s'] - complete[0]['elapsed_s'] >= 30,
                      post100_samples=post_seconds >= 30,
                      showing_p99_le33=not showing or stats(showing)['p99_ms'] <= 33,
                      post100_p99_within_tier=bool(post) and stats(post)['p99_ms'] <= post_limit)
        if tier == 'm1': checks['post100_compile_free'] = post_compile_free
    frontend = frontend_pre_present(text)
    if require_frontend: checks['frontend_generic_pre_present'] = frontend['passed']
    admissions = deadline_admissions(text, require_deadlines or {})
    if admissions: checks['required_deadline_sidecars_admitted'] = all(r['passed'] for r in admissions.values())
    window = [r for r in menu if r['elapsed_s'] <= window_seconds]
    result = {'frontend_pre_present': frontend, 'tier': tier, 'post100_compile_evidence': {'proven_compile_free': post_compile_free, 'rows': len(post), 'rows_with_compile_work': compile_rows, 'errors': compile_errors, 'required_for_tier': tier == 'm1'}, 'passed': all(checks.values()), 'case': str(folder), 'arm': 'title' if title else 'direct',
              'checks': checks, 'thresholds_ms': {'non_gameplay_max': 250, 'showing_p99': 33, 'post100_p99': post_limit,
                                               'gameplay_max': 100}, 'split_status': split_status, 'deadline_admission': admissions,
              'first_presented_frame': first, 'first_presented_source': 'explicit frames.csv presented_frame IDs',
              'unsplit': stats(menu), 'all_non_gameplay': stats(nonplay),
              'legacy_focused_menu': stats([r for r in rows if r['phase'] in ('startup', 'menu') and r.get('unfocused') == '0']),
              'metric_scopes': {'unsplit': 'startup/menu/loading/load/transition after first presentation; no focus exclusions',
                                'legacy_focused_menu': 'gate533 startup/menu focused rows, including before first presentation',
                                'max250': 'every phase except gameplay after first presentation, including unfocused rows'},
              'showing': stats(showing), 'post100': stats(post),
              'post100_sample_seconds': post_seconds, 'non_gameplay_over250': over250, 'gameplay_over100': over100,
              'fixed_window': {'seconds': window_seconds, 'complete': elapsed >= window_seconds,
                               'origin': 'first explicitly presented frame; sum of recorded seam intervals',
                               'scope': 'non-gameplay intervals ending within 0..T; independent of run tail length',
                               'stats': stats(window)}, 'events': events, 'state_errors': errors,
              'artifacts': {n: {'path': str(folder / n), 'sha256': sha(folder / n)} for n in ('app.log', 'frames.csv')}}
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('cases', nargs='+', type=Path)
    parser.add_argument('--require-frontend-prepresent', action='store_true')
    parser.add_argument('--tier', choices=('m1', 'm4'), required=True)
    parser.add_argument('--title', action='store_true')
    parser.add_argument('--window-seconds', type=float, default=300)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--require-deadline-stage', action='append', default=[], metavar='Stage:count',
                        help='Require exact validated frame-then-cost runtime admission; never use for historical baseline')
    args = parser.parse_args()
    if not math.isfinite(args.window_seconds) or args.window_seconds <= 0:
        parser.error('window-seconds must be positive and finite')
    try: required = deadline_requirements(args.require_deadline_stage)
    except ValueError as exc: parser.error(str(exc))
    results = [score(p, args.title, args.window_seconds, required, tier=args.tier, require_frontend=args.require_frontend_prepresent) for p in args.cases]
    if args.out:
        args.out.write_text(json.dumps({'passed': all(r['passed'] for r in results), 'runs': results}, indent=2) + '\n')
    for result in results: print(json.dumps(result))
    return 0 if all(r['passed'] for r in results) else 1


if __name__ == '__main__': raise SystemExit(main())
