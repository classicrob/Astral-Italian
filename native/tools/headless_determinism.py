#!/usr/bin/env python3
"""Compare headless repeats/Scribble runs, or traces from two different binaries.

Each scenario uses the health sweep's fixed script, virtual epoch and fresh user
directories. Every CSV cell is compared exactly, including float bit columns.
Wall-clock timing sidecars are deliberately separate from the game trace.

  python3 native/tools/headless_determinism.py --headless /path/petari_headless \
      --out /outside/checkout/determinism --scenarios 1 --jobs 3
  python3 native/tools/headless_determinism.py --headless /path/pattern \
      --headless-b /path/zero --out /outside/checkout/uninit --scenarios 1

With --headless-b, run each binary once with the same plain environment. Only
a scene_epoch offset that is constant across EVERY row may be ignored; it is
reported separately along with the original, unfiltered differences.

Omit --scenarios for all scenarios. Matching traces are removed by default;
differences are retained. Completed results resume only with identical settings
and binary/script hashes. --rerun repeats the selected scenarios.
"""
import argparse
from collections import Counter
import concurrent.futures
import csv
import hashlib
import itertools
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import time

import headless_sweep
import stage_sweep


VARIANTS = ('plain-a', 'plain-b', 'scribble')
CROSS_VARIANTS = ('binary-a', 'binary-b')


def digest(path):
    result = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            result.update(chunk)
    return result.hexdigest()


def compare_traces(left, right, ignore_constant_scene_epoch=False):
    """Find the first differing row/column and count every changed cell."""
    first = None
    columns = Counter()
    first_by_column = {}
    differing_rows = 0
    row_count = 0
    other_rows = 0
    epoch_offsets = set()
    missing = object()
    with left.open(newline='') as lf, right.open(newline='') as rf:
        lr, rr = csv.reader(lf), csv.reader(rf)
        lh, rh = next(lr, []), next(rr, [])
        if lh != rh:
            return {'identical': False, 'first': {'row': None, 'loop': None, 'column': '__columns__',
                    'left': lh, 'right': rh}, 'differing_rows': None, 'columns': {}}
        epoch_col = lh.index('scene_epoch') if 'scene_epoch' in lh else None
        epoch_complete = epoch_col is not None
        for index, (a, b) in enumerate(itertools.zip_longest(lr, rr, fillvalue=missing)):
            row_count += 1
            if epoch_col is not None:
                try:
                    epoch_offsets.add(int(b[epoch_col]) - int(a[epoch_col]))
                except (TypeError, ValueError, IndexError):
                    epoch_complete = False
            if a == b:
                continue
            differing_rows += 1
            if a is missing or b is missing:
                other_rows += 1
                diff = {'row': index, 'loop': index, 'column': '__rows__',
                        'left': 'EOF' if a is missing else a, 'right': 'EOF' if b is missing else b}
                first = first or diff
                columns['__rows__'] += 1
                first_by_column.setdefault('__rows__', diff)
                continue
            other_changed = False
            for col, (av, bv) in enumerate(itertools.zip_longest(a, b, fillvalue=None)):
                if av == bv:
                    continue
                name = lh[col] if col < len(lh) else f'__extra_column_{col}__'
                diff = {'row': index, 'loop': a[0] if a else index, 'column': name, 'left': av, 'right': bv}
                first = first or diff
                columns[name] += 1
                first_by_column.setdefault(name, diff)
                other_changed = other_changed or name != 'scene_epoch'
            other_rows += other_changed
    result = {'identical': first is None, 'rows': row_count, 'first': first,
              'differing_rows': differing_rows, 'columns': dict(columns), 'first_by_column': first_by_column}
    if ignore_constant_scene_epoch:
        constant = epoch_complete and len(epoch_offsets) == 1
        offset = next(iter(epoch_offsets)) if constant else None
        ignored = constant and offset != 0
        result.update(scene_epoch_constant=constant, scene_epoch_offset=offset,
                      scene_epoch_offset_ignored=ignored, raw_identical=result['identical'],
                      raw_first=first, raw_columns=dict(columns), raw_differing_rows=differing_rows)
        if ignored:
            filtered = {name: diff for name, diff in first_by_column.items() if name != 'scene_epoch'}
            result.update(first=min(filtered.values(), key=lambda d: (d['row'], lh.index(d['column']) if d['column'] in lh else len(lh))) if filtered else None,
                          first_by_column=filtered, columns={name: n for name, n in columns.items() if name != 'scene_epoch'},
                          differing_rows=other_rows, identical=not filtered)
    return result


def run_variant(args, case, stage, scenario, variant, script):
    run = case / variant
    run.mkdir(parents=True, exist_ok=True)
    user, log, trace = run / 'user', run / 'app.log', run / 'trace.csv'
    if user.exists():
        raise RuntimeError(f'user directory must be fresh: {user}')
    # The harness refuses to overwrite any trace artifact. A rerun regenerates
    # this variant's trace and sidecars; no earlier completion marker may survive.
    for artifact in run.glob('trace.csv*'):
        artifact.unlink()
    binary = args.headless_b if variant == 'binary-b' else args.headless
    cmd = [str(binary), '--disc', str(args.disc), '--user', str(user),
           '--stage', stage, '--scenario', str(scenario), '--keyboard-script', str(script),
           '--trace', str(trace), '--epoch-ticks', '0']
    if re.fullmatch(r'WorldMap\d+Galaxy', stage):
        cmd.append('--no-player')
    if args.actor_trace:
        cmd.append('--actor-trace')
    env = {'PATH': '/usr/bin:/bin', 'HOME': os.environ.get('HOME', '/'), 'SDL_AUDIODRIVER': 'dummy',
           'PETARI_FAST_BOOT': '1', 'TMPDIR': str(run), 'ASAN_OPTIONS': 'detect_leaks=0:abort_on_error=0',
           'UBSAN_OPTIONS': 'print_stacktrace=1'}
    if variant == 'scribble':
        env.update(MallocNanoZone='0', MallocScribble='1')
    started = time.monotonic()
    timed_out = False
    try:
        with log.open('wb') as stream:
            proc = subprocess.Popen(cmd, stdout=stream, stderr=subprocess.STDOUT, env=env, start_new_session=True)
            while True:
                try:
                    proc.wait(timeout=2)
                    break
                except subprocess.TimeoutExpired:
                    if time.monotonic() - started > args.timeout or log.stat().st_size > 400_000_000:
                        timed_out = True
                        os.killpg(proc.pid, signal.SIGKILL)
                        proc.wait()
                        break
    finally:
        if user.is_dir():
            shutil.rmtree(user)
    headless_sweep.trim_log(log)
    text = log.read_text(errors='replace')
    info = {'variant': variant, 'binary': str(binary), 'rc': proc.returncode, 'timed_out': timed_out,
            'elapsed': round(time.monotonic() - started, 2), 'trace': str(trace),
            'input_done': '[input-script] done' in text,
            'complete': trace.with_name(trace.name + '.complete').is_file()}
    if trace.is_file():
        with trace.open(newline='') as stream:
            reader = csv.reader(stream)
            header = next(reader, [])
            info.update(columns=len(header), rows=sum(1 for _ in reader), sha256=digest(trace))
    info['ok'] = (info['rc'] == 0 and not timed_out and info['input_done'] and info['complete']
                  and info.get('rows') == args.frames + 1)
    if not info['ok']:
        info['log_tail'] = text[-4000:]
    return info


def run_case(args, stage, scenario, script, config):
    case = args.out / 'runs' / f'{stage}-s{scenario}'
    result_path = case / 'result.json'
    if result_path.is_file() and not args.rerun:
        cached = json.loads(result_path.read_text())
        if cached.get('config') != config:
            raise RuntimeError(f'cached settings/binary changed; use a new --out or --rerun: {case}')
        if not args.keep_traces or all(Path(run['trace']).is_file() for run in cached['runs']):
            return cached
    cross = bool(getattr(args, 'headless_b', None))
    names = CROSS_VARIANTS if cross else VARIANTS
    variants = [run_variant(args, case, stage, scenario, variant, script) for variant in names]
    result = {'stage': stage, 'scenario': scenario, 'config': config, 'runs': variants, 'comparisons': {}}
    if all(Path(run['trace']).is_file() and run.get('columns') for run in variants):
        for name, other in ((('binary', 1),) if cross else (('repeat', 1), ('scribble', 2))):
            result['comparisons'][name] = compare_traces(Path(variants[0]['trace']), Path(variants[other]['trace']),
                                                       ignore_constant_scene_epoch=cross)
    if all(run['ok'] for run in variants):
        result['outcome'] = 'IDENTICAL' if all(c['identical'] for c in result['comparisons'].values()) else 'DIFFERENT'
    else:
        result['outcome'] = 'RUN_FAILED'
    result_path.write_text(json.dumps(result, indent=2) + '\n')
    if result['outcome'] == 'IDENTICAL' and not args.keep_traces:
        for variant in names:
            for trace in (case / variant).glob('trace.csv*'):
                trace.unlink()
    return result


def write_reports(out, results):
    results = sorted(results, key=lambda r: (r['stage'], r['scenario']))
    (out / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
    counts = Counter(r['outcome'] for r in results)
    lines = ['# Headless determinism sweep', '', f'{len(results)} scenarios: ' + ', '.join(f'{n} {k}' for k, n in sorted(counts.items())), '',
             '| stage | scenario | outcome | comparison | first loop | column | left | right | constant epoch offset |',
             '|---|---:|---|---|---:|---|---|---|---:|']
    with (out / 'results.csv').open('w', newline='') as stream:
        writer = csv.writer(stream)
        writer.writerow(['stage', 'scenario', 'outcome', 'comparison', 'first_loop', 'column', 'left', 'right', 'constant_epoch_offset'])
        for result in results:
            comparisons = result['comparisons'] or {'': {}}
            for name, comparison in comparisons.items():
                diff = comparison.get('first') or {}
                row = [result['stage'], result['scenario'], result['outcome'], name,
                       diff.get('loop', ''), diff.get('column', ''), diff.get('left', ''), diff.get('right', ''),
                       comparison.get('scene_epoch_offset', '')]
                writer.writerow(row)
                if result['outcome'] != 'IDENTICAL' or comparison.get('scene_epoch_offset_ignored'):
                    lines.append('| ' + ' | '.join(str(v).replace('|', '\\|').replace('\n', ' ')[:180] for v in row) + ' |')
    (out / 'table.md').write_text('\n'.join(lines) + '\n')
    return counts


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--headless', type=Path, required=True)
    parser.add_argument('--headless-b', type=Path, help='compare a second binary; ignore only a constant scene_epoch offset')
    parser.add_argument('--disc', type=Path, default=Path.home() / 'MacioGalaxy2-gamedata/SB4E01-extracted')
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--stages', help='comma-separated galaxy names')
    parser.add_argument('--scenarios', help='scenario range, for example 1 or 1-3; default all')
    parser.add_argument('--frames', type=int, default=3600)
    parser.add_argument('--jobs', type=int, default=3, choices=range(1, 4))
    parser.add_argument('--script', default='default', help='health sweep default or keyboard script file')
    parser.add_argument('--timeout', type=int, default=600)
    parser.add_argument('--keep-traces', action='store_true', help='also retain identical traces')
    parser.add_argument('--actor-trace', action='store_true')
    parser.add_argument('--rerun', action='store_true')
    args = parser.parse_args()
    if args.frames <= 0 or args.timeout <= 0:
        parser.error('--frames and --timeout must be positive')
    args.headless, args.disc, args.out = (p.resolve() for p in (args.headless, args.disc, args.out))
    if args.headless_b:
        args.headless_b = args.headless_b.resolve()
    source = headless_sweep.DEFAULT_SCRIPT if args.script == 'default' else Path(args.script).read_text()
    script_text = headless_sweep.script_text(args.frames, source)
    args.out.mkdir(parents=True, exist_ok=True)
    script = args.out / 'keys.txt'
    config = {'binary': str(args.headless), 'binary_sha256': digest(args.headless), 'disc': str(args.disc),
              'script_sha256': hashlib.sha256(script_text.encode()).hexdigest(), 'frames': args.frames,
              'epoch_ticks': 0, 'variants': list(VARIANTS), 'actor_trace': args.actor_trace}
    if args.headless_b:
        config.update(binary_b=str(args.headless_b), binary_b_sha256=digest(args.headless_b),
                      variants=list(CROSS_VARIANTS), ignore_constant_scene_epoch=True)
    manifest = args.out / 'manifest.json'
    if manifest.is_file() and json.loads(manifest.read_text()) != config:
        parser.error('output directory belongs to different settings/binary; use a new --out')
    manifest.write_text(json.dumps(config, indent=2) + '\n')
    script.write_text(script_text)
    stages = set(args.stages.split(',')) if args.stages else None
    scenarios = stage_sweep.parse_range(args.scenarios) if args.scenarios else None
    selected = [(s['stage'], sc['scenario']) for s in stage_sweep.enumerate_stages(args.disc / 'files')
                if stages is None or s['stage'] in stages for sc in s['scenarios']
                if scenarios is None or sc['scenario'] in scenarios]
    if not selected:
        parser.error('no matching stage/scenario pairs')
    results = []
    started = time.monotonic()
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run_case, args, stage, scenario, script, config) for stage, scenario in selected]
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            results.append(result)
            detail = []
            for name, comparison in result['comparisons'].items():
                if comparison.get('scene_epoch_offset_ignored'):
                    detail.append(f"{name}: constant scene_epoch offset {comparison['scene_epoch_offset']:+d} (ignored)")
                if comparison.get('first'):
                    d = comparison['first']
                    fields = ','.join(comparison['columns'])
                    detail.append(f"{name}: loop {d['loop']} {d['column']} {d['left']!r} -> {d['right']!r} (columns: {fields})")
            print(f"[{len(results)}/{len(selected)}] {result['stage']} s{result['scenario']}: {result['outcome']} {'; '.join(detail)}", flush=True)
            write_reports(args.out, results)
    counts = write_reports(args.out, results)
    print(f'done in {time.monotonic() - started:.0f}s: {dict(counts)}')
    return 0 if all(r['outcome'] == 'IDENTICAL' for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
