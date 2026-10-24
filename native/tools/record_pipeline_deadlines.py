#!/usr/bin/env python3
"""Warm, exact renderer-frame observations. Raw provenance stays OUTSIDE the seed pack.

plan --disc DISC [--batch 1/4]
run --app PERF.app --disc DISC --out OWNED_PORT53_M1G_DIR --heavy-lock LOCK
    [--native-lock LOCK] [--stage NAME ... | --batch 1/4] [--prime-count 1]
    [--cold-confirm-stage NAME] [--cold-confirm-app SAME_BUILD_DIFFERENT_BUNDLE_ID.app]

Every launch takes heavy then native. Cold confirmation uses a separate app/cache
namespace so the two warm recordings remain warm. The operator clears only that
owned cold app's Metal cache before invoking it; no user cache is ever removed here.
"""
import argparse
import contextlib
import csv
import fcntl
import hashlib
import json
import os
import plistlib
from pathlib import Path
import re
import signal
import sqlite3
import subprocess
import sys
import time

TOOLS = Path(__file__).resolve().parent
SCRIPT = TOOLS / 'perf_sweep_move.script'
VERSION = 2
QUIT_PID = r'''
ObjC.import('AppKit');
function run(argv) {
  var pid=Number(argv[0]), expectedPath=argv[1], expectedId=argv[2];
  if (!Number.isInteger(pid) || pid<=1 || !/^dev\.petari\.Petari\.perfrec[0-9]*$/.test(expectedId)) throw new Error('Invalid recording owner');
  var app=$.NSRunningApplication.runningApplicationWithProcessIdentifier(pid);
  if (!app || app.isTerminated || ObjC.unwrap(app.bundleIdentifier)!==expectedId || ObjC.unwrap(app.bundleURL.path)!==expectedPath)
    throw new Error('Refusing PID outside the exact recording perf app');
  if (!app.terminate) throw new Error('Orderly quit rejected');
  return JSON.stringify({pid:pid,app:expectedPath,bundle_id:expectedId,orderly_quit_requested:true});
}
'''


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def seed_identity(app, stage):
    directory = app / 'Contents/Resources/pipeline-seeds'
    stage_seed = directory / (stage + '.db')
    identity_seed = stage_seed if stage_seed.is_file() else directory / '__global__.db'
    with sqlite3.connect(identity_seed.as_uri() + '?mode=ro', uri=True) as db:
        renderer = db.execute('SELECT build_key FROM petari_cache_build').fetchone()[0]
    return {'renderer_key': renderer, 'stage_seed_sha256': sha(stage_seed) if stage_seed.is_file() else None,
            'stage_seed_missing': not stage_seed.is_file(), 'identity_seed': identity_seed.name}


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')


def observations(path, stage):
    rows = [json.loads(line) for line in Path(path).read_text().splitlines()]
    if not rows or rows[0] != {'event': 'header', 'version': VERSION, 'gx_config_version': 13,
                               'clock': 'active-frame-packet-id', 'source': 'find_pipeline-before-memo'}:
        raise ValueError('invalid first-use header')
    if rows[-1].get('event') != 'end' or rows[-1].get('complete') is not True:
        raise ValueError('capture lacks normal-exit completion footer')
    data = rows[1:-1]
    if rows[-1].get('records') != len(data):
        raise ValueError('footer record count mismatch')
    footer = rows[-1]
    for field in ('clock_first_frame', 'clock_last_frame', 'clock_transitions', 'lookup_calls'):
        if type(footer.get(field)) is not int or not 0 <= footer[field] < (1 << 64):
            raise ValueError('missing/invalid clock progression evidence')
    if not (footer['clock_last_frame'] > footer['clock_first_frame'] and
            0 < footer['clock_transitions'] < footer['lookup_calls'] and footer['lookup_calls'] >= len(data)):
        raise ValueError('capture clock did not progress')
    visits = {}
    for row in data:
        if row.get('event') != 'first_use' or not re.fullmatch(r'[A-Za-z0-9_-]{1,96}', row.get('stage', '')):
            raise ValueError('invalid stage/event')
        for field in ('visit', 'origin_frame', 'frame', 'relative_frame'):
            if type(row.get(field)) is not int or not 0 <= row[field] < (1 << 64):
                raise ValueError('invalid frame/visit integer')
        if row['frame'] - row['origin_frame'] != row['relative_frame'] or row['relative_frame'] >= 0xFFFFFFFF:
            raise ValueError('incoherent/unrepresentable frame origin')
        if not footer['clock_first_frame'] <= row['frame'] <= footer['clock_last_frame']:
            raise ValueError('recorded frame outside clock progression')
        if not re.fullmatch(r'[0-9a-f]{16}', row.get('config', '')):
            raise ValueError('invalid config hash')
        visit = visits.setdefault((row['stage'], row['visit']), {'origin': row['origin_frame'], 'configs': {}})
        if visit['origin'] != row['origin_frame'] or row['config'] in visit['configs']:
            raise ValueError('inconsistent origin/duplicate config in visit')
        visit['configs'][row['config']] = row['relative_frame']
    target = [v for (s, _), v in visits.items() if s == stage]
    if not any(row['frame'] > 0 for row in data):
        raise ValueError('all recorded frame IDs are zero')
    if len(target) != 1 or not target[0]['configs']:
        raise ValueError('expected one nonempty target-stage visit')
    return target[0]['configs']


def disagreements(a, b):
    return {key: [a.get(key), b.get(key)] for key in sorted(a.keys() | b.keys()) if a.get(key) != b.get(key)}


def select(disc, explicit=(), batch=None):
    if explicit:
        stages = list(explicit)
    else:
        import stage_sweep
        stages = [r['stage'] for r in stage_sweep.enumerate_stages(Path(disc) / 'files')
                  if any(s['scenario'] == 1 for s in r['scenarios'])]
    if any(not re.fullmatch(r'[A-Za-z0-9_-]{1,80}', s) for s in stages) or len(stages) != len(set(stages)):
        raise ValueError('invalid/duplicate stages')
    if batch:
        index, count = map(int, batch.split('/'))
        if not 1 <= index <= count <= len(stages):
            raise ValueError('invalid batch index/count')
        width = (len(stages) + count - 1) // count
        stages = stages[(index - 1) * width:index * width]
    return stages


@contextlib.contextmanager
def slots(heavy, native):
    with Path(heavy).open('a') as h:
        fcntl.lockf(h, fcntl.LOCK_EX)
        if native:
            with Path(native).open('a') as n:
                fcntl.lockf(n, fcntl.LOCK_EX)
                yield
        else:
            yield


def launch(args, stage, name, app=None, cold=False):
    app = app or args.app
    folder = args.out / stage / name
    folder.mkdir(parents=True, exist_ok=False)
    cache = args.out / stage / ('cold-cache' if cold else 'warm-cache')
    cache.mkdir(exist_ok=True)
    capture = folder / 'first-use.jsonl'
    env = dict(os.environ)
    for key in list(env):
        if key.startswith('PETARI_'):
            del env[key]
    env.update(SDL_AUDIO_DRIVER='dummy', PETARI_MODS='none', PETARI_CACHE_DIR=str(cache),
               PETARI_FRAME_CSV=str(folder / 'frames.csv'), PETARI_FRAME_STATS='1', PETARI_PIPELINE_DIAG='1',
               PETARI_ROUTE_RECORD=str(folder / 'route.csv'))
    if not args.title:
        env.update(PETARI_INPUT_SCRIPT=str(SCRIPT), PETARI_INPUT_SCRIPT_STEADY='60', PETARI_INPUT_SCRIPT_QUIT='1')
    recording = not name.startswith('prime')
    if recording:
        env['PETARI_PIPELINE_RECORD_FIRST_USE'] = str(capture)
    executable = app / 'Contents/MacOS/Petari'
    before = sha(executable)
    identity = seed_identity(app, stage)
    renderer = identity['renderer_key']
    cmd = [str(executable), '--disc', str(args.disc), '--user', str(folder / 'user')]
    if not args.title:
        cmd += ['--stage', stage, '--scenario', '1']
    write(folder / 'launch.json', {'command': cmd, 'environment': {k: v for k, v in env.items()
          if k.startswith(('PETARI_', 'SDL_'))}, 'exe_sha256': before, 'script_sha256': None if args.title else sha(SCRIPT),
          'nice': os.getpriority(os.PRIO_PROCESS, 0), 'cold_namespace': cold, 'renderer_key': renderer,
          **identity,
          'purpose': 'recording only; not performance acceptance', 'mode': 'no-input-title' if args.title else 'direct-stage',
          'load': os.getloadavg()})
    started = time.monotonic()
    with (folder / 'app.log').open('wb') as log:
        process = subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT, env=env)
        timed_out, quit_error = False, None
        try:
            if args.title:
                try:
                    process.wait(timeout=args.title_seconds)
                except subprocess.TimeoutExpired:
                    with (app / 'Contents/Info.plist').open('rb') as info:
                        bundle = plistlib.load(info)['CFBundleIdentifier']
                    sent = subprocess.run(['/usr/bin/osascript', '-l', 'JavaScript', '-e', QUIT_PID,
                                           str(process.pid), str(app), bundle], capture_output=True, text=True, timeout=30)
                    write(folder / 'orderly-quit.json', {'exit': sent.returncode, 'stdout': sent.stdout,
                                                       'stderr': sent.stderr, 'pid': process.pid, 'bundle_id': bundle})
                    if sent.returncode != 0: raise RuntimeError('Orderly recording-title quit rejected')
                    process.wait(timeout=30)
            else:
                process.wait(timeout=args.seconds)
        except subprocess.TimeoutExpired:
            timed_out = True
        except Exception as exc:
            quit_error = str(exc)
        finally:
            if process.poll() is None:
                process.send_signal(signal.SIGTERM)
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill(); process.wait()
    text = (folder / 'app.log').read_text(errors='replace')
    frames = list(csv.DictReader((folder / 'frames.csv').open())) if (folder / 'frames.csv').exists() else []
    fault = bool(re.search(r'EXC_BAD_ACCESS|\[hang\]|AddressSanitizer|UndefinedBehaviorSanitizer|Assertion failed', text))
    no_wait = not re.search(r'^\[gx uber wait\]', text, re.M) and all(float(r['pipeline_wait_ms']) == 0 for r in frames)
    no_pop = not re.search(r'^\[pop-in\] frame \d+ skipped_draws [1-9]', text, re.M)
    checks = {'exit_zero': process.returncode == 0, 'not_timed_out': not timed_out, 'no_faults': not fault,
              'script_done': args.title or '[input-script] done after 970 frames' in text, 'exe_unchanged': sha(executable) == before,
              'frame_rows': bool(frames)}
    if args.title:
        route = list(csv.DictReader((folder / 'route.csv').open())) if (folder / 'route.csv').exists() else []
        checks.update(no_quit_error=quit_error is None, title_300seconds=time.monotonic()-started >= 300,
                      no_stage_entry=bool(route) and all(r['stage'] in ('', 'FileSelect') for r in route),
                      no_input='PETARI_INPUT_SCRIPT' not in env and all(not any(float(r.get(k, 0)) for k in
                          ('move_x', 'move_y', 'jump', 'spin', 'crouch', 'shoot', 'pointer_valid')) for r in route))
    groups, error = {}, None
    if recording:
        try:
            groups = observations(capture, stage)
        except (OSError, ValueError, KeyError) as exc:
            error = str(exc)
        checks['valid_capture'] = bool(groups) and error is None
        if not cold:
            checks.update(no_pipeline_waits=no_wait, no_pop_in=no_pop)
    result = {'passed': all(checks.values()), 'checks': checks, 'stage': stage, 'scenario': 1,
              'name': name, 'folder': str(folder), 'process_exit': process.returncode, 'seconds': time.monotonic() - started,
              'capture_error': error, 'quit_error': quit_error, 'groups': groups, 'cold_confirmation': cold, 'no_pipeline_waits': no_wait,
              'no_pop_in': no_pop, 'recording_exe_sha256': before, 'launch_sha256': sha(folder / 'launch.json'),
              **identity,
              'app_log_sha256': sha(folder / 'app.log'), 'capture_sha256': sha(capture) if capture.exists() else None}
    write(folder / 'RESULT.json', result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['plan', 'run'])
    parser.add_argument('--disc', type=Path, required=True)
    parser.add_argument('--stage', action='append', default=[])
    parser.add_argument('--title', action='store_true', help='No-input FileSelect title, prime + two >=300s warm recordings')
    parser.add_argument('--title-seconds', type=float, default=315)
    parser.add_argument('--batch')
    parser.add_argument('--app', type=Path)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--heavy-lock', type=Path)
    parser.add_argument('--native-lock', type=Path)
    parser.add_argument('--build-metadata', type=Path)
    parser.add_argument('--prime-count', type=int, default=1)
    parser.add_argument('--seconds', type=int, default=240)
    parser.add_argument('--cold-confirm-stage')
    parser.add_argument('--cold-confirm-app', type=Path)
    args = parser.parse_args()
    if args.title and (args.stage or args.batch or args.cold_confirm_stage or args.title_seconds < 300):
        parser.error('--title needs its own batch/output, no --stage/--batch/--cold-confirm-stage; title-seconds>=300')
    stages = ['FileSelect'] if args.title else select(args.disc, args.stage, args.batch)
    if args.command == 'plan':
        print(json.dumps({'count': len(stages), 'stages': stages, 'scenario': 1,
                          'launches': len(stages) * (args.prime_count + 2),
                          'estimated_hours': len(stages) * (args.prime_count + 2) * (args.title_seconds if args.title else 50) / 3600}, indent=2))
        return 0
    if not args.app or not args.out or not args.heavy_lock or not args.build_metadata or args.prime_count < 1:
        parser.error('run requires --app --out --heavy-lock --build-metadata and prime-count>=1')
    args.app, args.disc, args.out = args.app.resolve(), args.disc.resolve(), args.out.resolve()
    if '/port53/m1g/' not in str(args.out) or args.out.exists():
        parser.error('out must be a NEW owned directory under port53/m1g')
    if args.cold_confirm_stage and (args.cold_confirm_stage not in stages or not args.cold_confirm_app):
        parser.error('cold confirmation requires selected stage and a separate owned app/cache identity')
    if args.cold_confirm_app and args.cold_confirm_app.resolve() == args.app:
        parser.error('cold confirmation app must have a separate owned Metal namespace')
    args.out.mkdir(parents=True)
    results = []
    for stage in stages:
        arms = []
        with slots(args.heavy_lock, args.native_lock):
            for i in range(args.prime_count):
                arms.append(launch(args, stage, f'prime-{i + 1}'))
            for i in (1, 2):
                arms.append(launch(args, stage, f'warm-{i}'))
            warm = arms[-2:]
            differences = disagreements(warm[0]['groups'], warm[1]['groups'])
            if args.cold_confirm_stage == stage:
                arms.append(launch(args, stage, 'cold-confirm', args.cold_confirm_app.resolve(), True))
                cold_diff = disagreements(warm[0]['groups'], arms[-1]['groups'])
            else:
                cold_diff = None
        result = {'stage': stage, 'arms': arms, 'warm_disagreements': differences,
                  'cold_disagreements': cold_diff, 'passed': all(x['passed'] for x in arms) and not differences
                  and (cold_diff is None or not cold_diff)}
        results.append(result)
        write(args.out / 'PROVENANCE.json', {'version': 1, 'recording_only': True, 'stages_planned': stages,
              'stage_results': results, 'script': None if args.title else str(SCRIPT), 'script_sha256': None if args.title else sha(SCRIPT),
              'mode': 'no-input-title' if args.title else 'direct-stage',
              'recording_build_metadata': json.loads(args.build_metadata.read_text()),
              'recording_build_metadata_sha256': sha(args.build_metadata),
              'all_completed': len(results) == len(stages), 'passed': len(results) == len(stages)
              and all(x['passed'] for x in results), 'failed_stages': [x['stage'] for x in results if not x['passed']]})
        print(json.dumps({'stage': stage, 'passed': result['passed'], 'warm_disagreements': len(differences),
                          'cold_disagreements': None if cold_diff is None else len(cold_diff)}), flush=True)
    return 0 if all(x['passed'] for x in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
