#!/usr/bin/env python3
"""Owned B title arm: true-cold, no input, per-arm heavy->native, exact-app/PREP receipts.

--baseline uses orderly PID-specific AppleEvent quit after315s, and reports only
historical unsplit evidence. Final builds self-end with PETARI_TITLE_OBSERVE=1.
Neither path sends a key, pointer event or Accessibility request.
"""
import argparse
import fcntl
import json
import os
from pathlib import Path
import plistlib
import shutil
import subprocess
import sys
import time

import common
import gate542m

HERE = Path(__file__).resolve().parent
OWN_ID = 'dev.petari.Petari.perfm1b'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--app', type=Path, required=True)
    p.add_argument('--prep', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--baseline', action='store_true')
    p.add_argument('--label', required=True)
    p.add_argument('--heavy-held', action='store_true', help=argparse.SUPPRESS)
    args = p.parse_args()
    if not args.heavy_held:
        return subprocess.run(['/usr/bin/lockf', '-k', str(common.HEAVY), sys.executable, '-B', __file__,
                               *sys.argv[1:], '--heavy-held']).returncode
    with common.LOCK.open('a') as native:
        fcntl.lockf(native, fcntl.LOCK_EX)
        return arm(args)


def arm(args):
    app, out = args.app.resolve(), args.out.resolve()
    assert out.is_relative_to(common.ROOT.resolve()) and not out.exists() and out != common.ROOT.resolve()
    prep = json.loads(args.prep.read_text())
    assert prep['bundle_id'] == OWN_ID and Path(prep['app']).resolve() == app
    assert prep['build_kind'] == 'server-candidate'
    assert prep['build']['source'] == prep['source_commit']
    assert prep['build']['binary_sha256'] == prep['original_exe_sha256']
    if args.baseline:
        assert prep['source_commit'] == '07223918c4f3cf2a648c77b70d66357f771cf7a1'
        assert prep['original_exe_sha256'] == '2d17baf22e9ac52353989f9b078187047106f7dc2fab94a921c4aeb46a3fe3f6'
    executable = app / 'Contents/MacOS/Petari'
    before = common.sha(executable)
    assert before == prep['perf_sha256']
    with (app / 'Contents/Info.plist').open('rb') as file:
        assert plistlib.load(file)['CFBundleIdentifier'] == OWN_ID
    resources = common.tree(app / 'Contents/Resources')
    common.disk(); out.mkdir(parents=True)
    common.wait_quiet(out / 'quiet.json')
    base = Path(subprocess.check_output(['/usr/bin/getconf', 'DARWIN_USER_CACHE_DIR'], text=True).strip())
    target = base / OWN_ID
    assert base.is_absolute() and target.name == 'dev.petari.Petari.perfm1b' and base != Path('/')
    if target.exists(): shutil.rmtree(target)
    cache, user = out / 'cache', out / 'user'
    cache.mkdir(); user.mkdir()
    env = {k: v for k, v in os.environ.items() if not k.startswith(('PETARI_', 'ASTRAL_', 'DYLD_', 'THRPROBE_'))}
    env.update(SDL_AUDIO_DRIVER='dummy', PETARI_MODS='none', PETARI_CACHE_DIR=str(cache),
               PETARI_FRAME_CSV=str(out / 'frames.csv'), PETARI_FRAME_STATS='1', PETARI_PIPELINE_DIAG='0',
               PETARI_ROUTE_RECORD=str(out / 'route.csv'), PETARI_TRACE_BOOT='1',
               PYTHONDONTWRITEBYTECODE='1', LANG='C', LC_ALL='C')
    if not args.baseline:
        env.update(PETARI_TITLE_OBSERVE='1', PETARI_TITLE_OBSERVE_MIN_SECONDS='315')
    command = [str(executable), '--disc', str(common.DISC), '--user', str(user)]
    started = time.monotonic(); timeout = 315 if args.baseline else 900
    launch = {'command': command, 'environment': {k: v for k, v in env.items() if k.startswith(('PETARI_', 'SDL_'))},
              'label': args.label, 'baseline': args.baseline, 'build_kind': prep['build_kind'],
              'source_commit': prep['source_commit'], 'original_exe_sha256': prep['original_exe_sha256'],
              'perf_exe_sha256': before, 'prep': str(args.prep), 'prep_sha256': common.sha(args.prep),
              'bundle_id': OWN_ID, 'cold_metal_cleared': True, 'fresh_private_cache': str(cache),
              'no_input': True, 'nice': os.getpriority(os.PRIO_PROCESS, 0), 'load': os.getloadavg(),
              'comparison_window_seconds': 300, 'native_observation_minimum_seconds': None if args.baseline else 315,
              'hard_cap_seconds': 900, 'start_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())}
    assert launch['nice'] == 0
    with (out / 'app.log').open('wb') as log:
        proc = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, env=env)
        launch['pid'] = proc.pid; common.write(out / 'launch.json', launch)
        timed_out, orderly, error = False, None, None
        try:
            if args.baseline:
                while proc.poll() is None and time.monotonic() - started < timeout:
                    time.sleep(min(1, max(.01, timeout - (time.monotonic() - started))))
                if proc.poll() is None:
                    quit_command = ['/usr/bin/osascript', '-l', 'JavaScript', str(HERE / 'quit_owned_app.js'), str(proc.pid), str(app)]
                    sent = subprocess.run(quit_command, capture_output=True, text=True, timeout=30)
                    orderly = {'command': quit_command, 'exit': sent.returncode, 'stdout': sent.stdout, 'stderr': sent.stderr}
                    common.write(out / 'orderly-quit.json', orderly)
                    if sent.returncode != 0: raise RuntimeError('Owned orderly quit rejected: see orderly-quit.json')
                    proc.wait(timeout=30)
            else:
                proc.wait(timeout=900)
        except subprocess.TimeoutExpired:
            timed_out = True
        except Exception as exc:
            error = str(exc)
        finally:
            if proc.poll() is None:
                proc.terminate()
                try: proc.wait(timeout=10)
                except subprocess.TimeoutExpired: proc.kill(); proc.wait()
    text = (out / 'app.log').read_text(errors='replace')
    process = {'exit': proc.returncode, 'timed_out': timed_out, 'seconds': time.monotonic() - started,
               'no_faults': not common.FAULT.search(text), 'binary_unchanged': common.sha(executable) == before,
               'resources_unchanged': common.tree(app / 'Contents/Resources') == resources,
               'frames_csv_flushed': (out / 'frames.csv').exists(), 'orderly_quit': orderly,
               'normal_exit': proc.returncode == 0}
    process['error'] = error
    common.write(out / 'process.json', process)
    scored = gate542m.score(out, title=not args.baseline, window_seconds=300) if process['frames_csv_flushed'] else None
    route = common.rows(out / 'route.csv')
    neutral = all(not any(float(r.get(k, 0)) for k in ('move_x', 'move_y', 'jump', 'spin', 'crouch', 'shoot', 'pointer_valid')) for r in route)
    checks = {'normal_exit': process['normal_exit'], 'not_timed_out': not timed_out, 'no_runner_error': error is None,
              'no_faults': process['no_faults'],
              'binary_unchanged': process['binary_unchanged'], 'resources_unchanged': process['resources_unchanged'],
              'frames_flushed': process['frames_csv_flushed'], 'no_input_in_route': neutral and bool(route),
              'no_stage_entry': all(r['stage'] in ('', 'FileSelect') for r in route),
              'fixed_window_complete': scored is not None and scored['fixed_window']['complete']}
    if not args.baseline: checks['gate542m'] = scored is not None and scored['passed']
    result = {'passed': all(checks.values()), 'checks': checks, 'label': args.label, 'baseline': args.baseline,
              'source_commit': prep['source_commit'], 'perf_exe_sha256': before, 'original_exe_sha256': prep['original_exe_sha256'],
              'process': process, 'gate542m': scored, 'comparison_window_seconds': 300,
              'baseline_scope': 'retained unsplit evidence, no missing-signal acceptance claim' if args.baseline else None,
              'prep': str(args.prep), 'prep_sha256': common.sha(args.prep), 'launch_sha256': common.sha(out / 'launch.json')}
    common.write(out / 'RESULT.json', result)
    common.trim(out)
    print(json.dumps(result), flush=True)
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
