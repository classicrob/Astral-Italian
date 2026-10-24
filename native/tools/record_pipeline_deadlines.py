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
import io
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
STATIONARY_SCRIPT = TOOLS / 'perf_sweep_stationary.script'
VERSION = 3
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
    with contextlib.closing(sqlite3.connect(identity_seed.as_uri() + '?mode=ro', uri=True)) as db:
        renderer = db.execute('SELECT build_key FROM petari_cache_build').fetchone()[0]
    return {'renderer_key': renderer, 'stage_seed_sha256': sha(stage_seed) if stage_seed.is_file() else None,
            'stage_seed_missing': not stage_seed.is_file(), 'identity_seed': identity_seed.name}


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')


def tree(path):
    return {str(p.relative_to(path)): sha(p) for p in sorted(path.rglob('*')) if p.is_file()}


def map_observation(rows, stage, frames):
    if not re.fullmatch(r'WorldMap0[1-8]Galaxy', stage) or type(frames) is not int or frames < 970:
        raise ValueError('map observation needs a real world map and >=970-frame fixed window')
    target = [r for r in rows if r['stage'] == stage and r['scenario'] == '1']
    first = int(target[0]['frame']) if target else None
    last = int(target[-1]['frame']) if target else None
    neutral_fields = ('move_x','move_y','jump','spin','crouch','shoot','pointer_valid',
                      'camera_left','camera_right','camera_center','camera_up','camera_down')
    return {'stage_identity_route_row': first, 'requested_frames': frames,
            'requested_end_route_row': first + frames if first is not None else None,
            'last_target_route_row': last,
            'observed_route_intervals': last - first if first is not None else 0,
            'checks': {'target_map_observed': bool(target),
                       'target_player_valid': any(r['valid'] == '1' for r in target),
                       'fixed_window_complete': first is not None and last - first >= frames,
                       'no_scene_departure': first is not None and all(r['stage'] == stage and r['scenario'] == '1' for r in rows if int(r['frame']) >= first),
                       'no_input': bool(rows) and all(not any(float(r.get(k,0)) for k in neutral_fields) for r in rows)}}


def wait_map_window(process, path, stage, frames, timeout):
    deadline = time.monotonic() + timeout
    while process.poll() is None:
        if time.monotonic() >= deadline: raise subprocess.TimeoutExpired('map observation', timeout)
        if path.exists():
            # The route writer flushes every60 rows. Ignore an incomplete final
            # CSV line while polling; preserve and validate ALL rows after quit.
            text = path.read_text()
            if '\n' in text:
                rows = list(csv.DictReader(io.StringIO(text.rsplit('\n',1)[0] + '\n')))
                status = map_observation(rows, stage, frames)
                if all(status['checks'].values()): return status
        time.sleep(.1)
    raise RuntimeError('map process exited before its fixed observation window')


def replay_checks(report):
    """Whole-session replay must retain all v1 state, pointer and checkpoint checks."""
    segments = report.get('segments', [])
    return {
        'whole_session': report.get('mode') == 'whole-session',
        'finished': report.get('finished') is True and not report.get('stopped') and bool(segments),
        'v1_policy': report.get('anchor_policy') == 'interactive-and-play-ready-v1',
        'bit_exact': report.get('interactive_position_bit_exact') is True,
        'zero_position_error': bool(segments) and all(s.get('max_position_error') == 0 for s in segments),
        'segments_complete': bool(segments) and all(s.get('started') is True and s.get('completed') is True and not s.get('diverged') for s in segments),
        'all_state_and_pointer_rows': bool(segments) and all(
            type(s.get('identity_rows_skipped')) is int and 0 <= s['identity_rows_skipped'] <= 30 and
            s.get('rows_compared', -1) + s['identity_rows_skipped'] == s.get('tape_frames', -2) + 1 and
            s.get('pointer_rows_compared') == s.get('tape_frames', -1) for s in segments),
        'bounded_identity': bool(segments) and all(s.get('identity_offset_checked') is True and s.get('identity_offset_passed') is True and
            s.get('identity_offset_limit') == 30 and type(s.get('identity_frame_offset')) is int and abs(s['identity_frame_offset']) <= 30 for s in segments),
        'exact_checkpoints': bool(segments) and all(s.get('play_ready_required') is False or
            (s.get('play_ready_required') is True and all((s.get('play_ready_state_check') or {}).get(k) is True for k in ('checked','passed','position_bit_exact')) and
             ((s.get('play_ready_state_check') or {}).get('rng') or {}).get('verified') is True) for s in segments),
    }


def observations(path, stage):
    rows = [json.loads(line) for line in Path(path).read_text().splitlines()]
    if not rows or rows[0] != {'event': 'header', 'version': VERSION, 'gx_config_version': 13,
                               'clock': 'active-frame-packet-id', 'source': 'find_pipeline-before-memo',
                               'origin_policy': 'next-whole-packet'}:
        raise ValueError('invalid first-use header')
    if rows[-1].get('event') != 'end' or rows[-1].get('complete') is not True:
        raise ValueError('capture lacks normal-exit completion footer')
    body = rows[1:-1]
    data = [r for r in body if r.get('event') == 'first_use']
    starts = [r for r in body if r.get('event') == 'stage_begin']
    if any(r.get('event') not in ('first_use', 'stage_begin') for r in body):
        raise ValueError('unknown capture event')
    if rows[-1].get('records') != len(data):
        raise ValueError('footer record count mismatch')
    footer = rows[-1]
    if not starts or footer.get('stage_requests') != len(starts) or footer.get('stage_begins') != len(starts) or footer.get('pending_stage_requests') != 0:
        raise ValueError('stage boundary accounting mismatch')
    for field in ('clock_first_frame', 'clock_last_frame', 'clock_transitions', 'lookup_calls',
                  'stage_requests', 'stage_begins', 'pending_stage_requests'):
        if type(footer.get(field)) is not int or not 0 <= footer[field] < (1 << 64):
            raise ValueError('missing/invalid clock progression evidence')
    if not (footer['clock_last_frame'] > footer['clock_first_frame'] and
            0 < footer['clock_transitions'] < footer['lookup_calls'] and footer['lookup_calls'] >= len(data)):
        raise ValueError('capture clock did not progress')
    visits, current = {}, None
    for row in body:
        if not re.fullmatch(r'[A-Za-z0-9_-]{1,96}', row.get('stage', '')):
            raise ValueError('invalid stage/event')
        if row.get('event') == 'stage_begin':
            for field in ('visit', 'origin_frame'):
                if type(row.get(field)) is not int or not 0 <= row[field] < (1 << 64):
                    raise ValueError('invalid stage boundary')
            if 'request_packet' not in row:
                raise ValueError('missing explicit stage request packet')
            request = row['request_packet']
            if request is not None and (type(request) is not int or request < 0 or row['origin_frame'] != request + 1):
                raise ValueError('stage did not begin on the next whole packet')
            key = (row['stage'], row['visit'])
            if key in visits or current is not None and row['visit'] <= current[1]:
                raise ValueError('duplicate/backward stage boundary')
            visits[key] = {'origin': row['origin_frame'], 'configs': {}}
            current = key
            continue
        for field in ('visit', 'origin_frame', 'frame', 'relative_frame'):
            if type(row.get(field)) is not int or not 0 <= row[field] < (1 << 64):
                raise ValueError('invalid frame/visit integer')
        if row['frame'] - row['origin_frame'] != row['relative_frame'] or row['relative_frame'] >= 0xFFFFFFFF:
            raise ValueError('incoherent/unrepresentable frame origin')
        if not footer['clock_first_frame'] <= row['frame'] <= footer['clock_last_frame']:
            raise ValueError('recorded frame outside clock progression')
        if not re.fullmatch(r'[0-9a-f]{16}', row.get('config', '')):
            raise ValueError('invalid config hash')
        key = (row['stage'], row['visit'])
        if key != current:
            raise ValueError('lookup lacks its explicit packet stage boundary')
        visit = visits[key]
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
    script = STATIONARY_SCRIPT if stage in args.stationary_stage else SCRIPT
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
    tape = getattr(args, 'tape', None)
    fixture_before = None
    if tape:
        fixture_before = tree(args.tape_user_fixture)
        certified = json.loads(args.tape_user_receipt.read_text())
        if fixture_before != certified['files']: raise ValueError('tape fixture differs from receipt')
        subprocess.run(['/usr/bin/ditto', '--rsrc', '--extattr', str(args.tape_user_fixture), str(folder / 'user')], check=True)
        if tree(folder / 'user') != fixture_before: raise ValueError('tape user copy differs from fixture')
        meta = json.loads((tape / 'route.tape.json').read_text())
        if meta.get('lossy_samples') or meta.get('unencodable_samples'): raise ValueError('tape is not lossless')
        tape_before = {n: sha(tape / n) for n in ('route.kpd','route.csv','route.actions','route.tape.json')}
        mods = meta.get('mods', [])
        env.update(PETARI_MODS=','.join(mods + (['-OdysseyJumpDive'] if 'OdysseyJumpDive' not in mods else [])),
                   PETARI_TAPE_PLAY_SESSION='1', PETARI_TAPE_PLAY_QUIT='1', PETARI_TAPE_PLAY_OUT=str(folder / 'replay'))
        for key, value in (meta.get('replay_env') or {}).items():
            if key != 'PETARI_TEST_RETURN_TO_SHIP': raise ValueError('unsupported recorded scene hook')
            env[key] = str(value)
    elif not args.title and not args.map:
        env.update(PETARI_INPUT_SCRIPT=str(script), PETARI_INPUT_SCRIPT_STEADY='60', PETARI_INPUT_SCRIPT_QUIT='1')
    recording = not name.startswith('prime')
    if recording:
        env['PETARI_PIPELINE_RECORD_FIRST_USE'] = str(capture)
    executable = app / 'Contents/MacOS/Petari'
    before = sha(executable)
    identity = seed_identity(app, stage)
    renderer = identity['renderer_key']
    cmd = [str(executable), '--disc', str(args.disc), '--user', str(folder / 'user')]
    if tape:
        cmd += ['--replay-tape', str(tape)]
    elif not args.title:
        cmd += ['--stage', stage, '--scenario', '1']
    write(folder / 'launch.json', {'command': cmd, 'environment': {k: v for k, v in env.items()
          if k.startswith(('PETARI_', 'SDL_'))}, 'exe_sha256': before, 'script_sha256': None if args.title or args.map or tape else sha(script),
          'stationary_stage_override': stage in args.stationary_stage,
          'nice': os.getpriority(os.PRIO_PROCESS, 0), 'cold_namespace': cold, 'renderer_key': renderer,
          **identity,
          'purpose': 'recording only; not performance acceptance', 'mode': 'tape-driven' if tape else 'no-input-title' if args.title else 'no-input-map' if args.map else 'direct-stage',
          'map_frames': args.map_frames if args.map else None,
          'tape_files_sha256': tape_before if tape else None, 'save_fixture': str(args.tape_user_fixture) if tape else None,
          'fixture_files': fixture_before, 'tape_executable_hash_gated': False,
          'load': os.getloadavg()})
    started = time.monotonic()
    with (folder / 'app.log').open('wb') as log:
        process = subprocess.Popen(cmd, stdout=log, stderr=subprocess.STDOUT, env=env)
        timed_out, quit_error, map_quit_status = False, None, None
        try:
            if args.title or args.map:
                request_quit = False
                if args.map:
                    map_quit_status = wait_map_window(process, folder / 'route.csv', stage, args.map_frames, args.seconds)
                    request_quit = True
                else:
                    try:
                        process.wait(timeout=args.title_seconds)
                    except subprocess.TimeoutExpired:
                        request_quit = True
                if request_quit:
                    with (app / 'Contents/Info.plist').open('rb') as info:
                        bundle = plistlib.load(info)['CFBundleIdentifier']
                    sent = subprocess.run(['/usr/bin/osascript', '-l', 'JavaScript', '-e', QUIT_PID,
                                           str(process.pid), str(app), bundle], capture_output=True, text=True, timeout=30)
                    write(folder / 'orderly-quit.json', {'exit': sent.returncode, 'stdout': sent.stdout,
                                                       'stderr': sent.stderr, 'pid': process.pid, 'bundle_id': bundle})
                    if sent.returncode != 0: raise RuntimeError('Orderly owned recording quit rejected')
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
              'exe_unchanged': sha(executable) == before,
              'frame_rows': bool(frames)}
    native_replay, replay_sha = None, None
    if tape:
        report_path = folder / 'replay/divergence.json'
        native_replay = replay_checks(json.loads(report_path.read_text())) if report_path.exists() else {'report_present': False}
        replay_sha = sha(report_path) if report_path.exists() else None
        checks.update(strict_tape_replay=all(native_replay.values()),
                      tape_unchanged=tape_before == {n: sha(tape / n) for n in tape_before},
                      fixture_unchanged=tree(args.tape_user_fixture) == fixture_before,
                      no_jump_cut='jump cut' not in text)
    elif not args.map:
        checks['script_done'] = args.title or '[input-script] done after 970 frames' in text
    map_status = None
    if args.map:
        route = list(csv.DictReader((folder / 'route.csv').open())) if (folder / 'route.csv').exists() else []
        map_status = map_observation(route, stage, args.map_frames)
        sidecar = json.loads((folder / 'route.tape.json').read_text()) if (folder / 'route.tape.json').exists() else {}
        checks.update(map_status['checks'], orderly_map_quit=quit_error is None and map_quit_status is not None,
                      map_scene_ready=sidecar.get('complete') is True and any(r['stage'] == stage and r['scenario'] == 1 for r in sidecar.get('ready_seams',[])))
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
    if tape: result.update(replay_checks=native_replay, replay_report_sha256=replay_sha)
    if args.map:
        result.update(map_observation=map_status, map_quit_request=map_quit_status,
                      map_clock='route-csv-seam-frame',
                      map_quit_policy='970-or-configured seam intervals from map identity; CSV flush every60 rows and orderly-quit latency are retained, never trimmed',
                      orderly_quit_sha256=sha(folder / 'orderly-quit.json') if (folder / 'orderly-quit.json').exists() else None,
                      route_csv_sha256=sha(folder / 'route.csv') if (folder / 'route.csv').exists() else None,
                      route_sidecar_sha256=sha(folder / 'route.tape.json') if (folder / 'route.tape.json').exists() else None)
    write(folder / 'RESULT.json', result)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['plan', 'run'])
    parser.add_argument('--disc', type=Path, required=True)
    parser.add_argument('--stage', action='append', default=[])
    parser.add_argument('--stationary-stage', action='append', default=[], help='Explicit stage-only camera/jump/spin probe; preserves 970-frame duration')
    parser.add_argument('--title', action='store_true', help='No-input FileSelect title, prime + two >=300s warm recordings')
    parser.add_argument('--title-seconds', type=float, default=315)
    parser.add_argument('--map', action='store_true', help='Finite no-input map scene observation, default all8 world maps')
    parser.add_argument('--map-frames', type=int, default=970)
    parser.add_argument('--tape', type=Path, help='Whole-session native KPAD replay instead of the direct-stage script')
    parser.add_argument('--tape-user-fixture', type=Path)
    parser.add_argument('--tape-user-receipt', type=Path)
    parser.add_argument('--batch')
    parser.add_argument('--app', type=Path)
    parser.add_argument('--out', type=Path)
    parser.add_argument('--heavy-lock', type=Path)
    parser.add_argument('--native-lock', type=Path)
    parser.add_argument('--build-metadata', type=Path)
    parser.add_argument('--build-provenance', type=Path, help='Reuse the exact recording BUILD metadata bound by an earlier batch')
    parser.add_argument('--prime-count', type=int, default=1)
    parser.add_argument('--seconds', type=int, default=240)
    parser.add_argument('--cold-confirm-stage')
    parser.add_argument('--cold-confirm-app', type=Path)
    args = parser.parse_args()
    if args.tape and (args.title or args.map or args.batch or not args.stage or args.stationary_stage or args.cold_confirm_stage or
                      not args.tape_user_fixture or not args.tape_user_receipt):
        parser.error('--tape needs explicit --stage target(s), fixture and receipt; no title/batch/stationary/cold mode')
    if args.tape:
        args.tape = args.tape.resolve(); args.tape_user_fixture = args.tape_user_fixture.resolve()
        args.tape_user_receipt = args.tape_user_receipt.resolve()
    if args.title and (args.stage or args.batch or args.stationary_stage or args.cold_confirm_stage or args.title_seconds < 300):
        parser.error('--title needs its own batch/output, no --stage/--batch/--cold-confirm-stage; title-seconds>=300')
    if args.map and (args.title or args.batch or args.stationary_stage or args.cold_confirm_stage or args.map_frames < 970 or
                     any(not re.fullmatch(r'WorldMap0[1-8]Galaxy', s) for s in args.stage)):
        parser.error('--map needs >=970frames and only explicit WorldMap01-08 stages; no title/batch/stationary/cold mode')
    stages = ['FileSelect'] if args.title else [f'WorldMap0{i}Galaxy' for i in range(1,9)] if args.map and not args.stage else select(args.disc, args.stage, args.batch)
    if any(stage not in stages for stage in args.stationary_stage):
        parser.error('every --stationary-stage must be selected in this run')
    if args.command == 'plan':
        print(json.dumps({'count': len(stages), 'stages': stages, 'scenario': 1,
                          'launches': len(stages) * (args.prime_count + 2),
                          'estimated_hours': len(stages) * (args.prime_count + 2) * (args.title_seconds if args.title else 50) / 3600}, indent=2))
        return 0
    if not args.app or not args.out or not args.heavy_lock or bool(args.build_metadata) == bool(args.build_provenance) or args.prime_count < 1:
        parser.error('run requires --app --out --heavy-lock, exactly one of --build-metadata/--build-provenance, and prime-count>=1')
    if args.build_provenance:
        earlier = json.loads(args.build_provenance.read_text())
        if earlier.get('version') != 1 or earlier.get('recording_only') is not True:
            parser.error('unsupported source recording provenance')
        build_metadata = earlier['recording_build_metadata']
        build_metadata_sha = earlier['recording_build_metadata_sha256']
    else:
        build_metadata = json.loads(args.build_metadata.read_text())
        build_metadata_sha = sha(args.build_metadata)
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
              'stage_results': results, 'script': None if args.title or args.map or args.tape else str(SCRIPT), 'script_sha256': None if args.title or args.map or args.tape else sha(SCRIPT),
              'mode': 'tape-driven' if args.tape else 'no-input-title' if args.title else 'no-input-map' if args.map else 'direct-stage',
              'map_frames': args.map_frames if args.map else None,
              'tape': str(args.tape) if args.tape else None,
              'tape_user_fixture': str(args.tape_user_fixture) if args.tape else None,
              'tape_user_receipt_sha256': sha(args.tape_user_receipt) if args.tape else None,
              'stationary_stage_overrides': {stage: {'script': str(STATIONARY_SCRIPT), 'sha256': sha(STATIONARY_SCRIPT)}
                                            for stage in args.stationary_stage},
              'recording_build_metadata': build_metadata,
              'recording_build_metadata_sha256': build_metadata_sha,
              'build_metadata_source_provenance': {'path': str(args.build_provenance.resolve()), 'sha256': sha(args.build_provenance)} if args.build_provenance else None,
              'all_completed': len(results) == len(stages), 'passed': len(results) == len(stages)
              and all(x['passed'] for x in results), 'failed_stages': [x['stage'] for x in results if not x['passed']]})
        print(json.dumps({'stage': stage, 'passed': result['passed'], 'warm_disagreements': len(differences),
                          'cold_disagreements': None if cold_diff is None else len(cold_diff)}), flush=True)
    return 0 if all(x['passed'] for x in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
