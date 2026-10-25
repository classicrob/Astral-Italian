#!/usr/bin/env python3
"""SOL A 5.4.3 pacing diagnostic: frozen 5.4.2 title protocol and scorer.

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
import re
import shutil
import subprocess
import sys
import time

reference_parser = argparse.ArgumentParser(add_help=False)
reference_parser.add_argument('--reference-harness', type=Path, required=True)
reference_args, _ = reference_parser.parse_known_args()
REFERENCE = reference_args.reference_harness.resolve()
sys.path.insert(0, str(REFERENCE))
import common
import gate542m

HERE = REFERENCE
OWN_ID = 'dev.petari.Petari.perfm1a'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--reference-harness', type=Path, required=True, help='Pinned copy of the accepted title harness and scorer')
    p.add_argument('--app', type=Path, required=True)
    p.add_argument('--prep', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    p.add_argument('--require-frontend-prepresent', action='store_true')
    p.add_argument('--tier', choices=('m1', 'm4'), required=True)
    p.add_argument('--baseline', action='store_true')
    p.add_argument('--require-boot-responsiveness', action='store_true')
    p.add_argument('--keep-cache', action='store_true', help='Preserve this arm private cache for an explicit warm second launch')
    p.add_argument('--warm-from', type=Path, help='Reuse this completed matching prime arm private cache and existing Metal namespace; never clear')
    p.add_argument('--label', required=True)
    p.add_argument('--bundle-id', default=OWN_ID, help='Dedicated perf bundle ID; must match the prepared app')
    p.add_argument('--heavy-lock', type=Path, default=common.HEAVY)
    p.add_argument('--native-lock', type=Path, default=common.LOCK)
    p.add_argument('--disc', type=Path, default=common.DISC)
    p.add_argument('--require-deadline-stage', action='append', default=[], metavar='Stage:count')
    p.add_argument('--heavy-held', action='store_true', help=argparse.SUPPRESS)
    p.add_argument('--diagnostic-env', default='{}', help='Approved env-only pacing toggle, no source or predicate change')
    args = p.parse_args()
    args.diagnostic_env = json.loads(args.diagnostic_env)
    assert args.diagnostic_env in ({}, {'PETARI_UBERSHADER':'0'})
    if not re.fullmatch(r'dev\.petari\.Petari\.perf[A-Za-z0-9_-]+', args.bundle_id): p.error('dedicated perf bundle ID required')
    if args.baseline and args.require_deadline_stage: p.error('historical baseline has no deadline metadata')
    try: args.required_deadlines = gate542m.deadline_requirements(args.require_deadline_stage or ([] if args.baseline else ['FileSelect:69']))
    except ValueError as exc: p.error(str(exc))
    common.HEAVY, common.LOCK, common.DISC = args.heavy_lock.expanduser(), args.native_lock.expanduser(), args.disc.expanduser()
    if not args.heavy_held:
        return subprocess.run(['/usr/bin/lockf', '-k', str(common.HEAVY), sys.executable, '-B', __file__,
                               *sys.argv[1:], '--heavy-held']).returncode
    with common.LOCK.open('a') as native:
        fcntl.lockf(native, fcntl.LOCK_EX)
        return arm(args)


def arm(args):
    app, out = args.app.resolve(), args.out.resolve()
    own_id = args.bundle_id
    assert out.is_relative_to(common.ROOT.resolve()) and not out.exists() and out != common.ROOT.resolve()
    prep = json.loads(args.prep.read_text())
    assert prep['bundle_id'] == own_id and Path(prep['app']).resolve() == app
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
        assert plistlib.load(file)['CFBundleIdentifier'] == own_id
    resources = common.tree(app / 'Contents/Resources')
    assert resources == prep['resource_tree']
    original = Path(prep['original_app'])
    assert common.sha(original / 'Contents/MacOS/Petari') == prep['original_exe_sha256']
    assert common.tree(original / 'Contents/Resources') == resources
    gate_sources = {name: common.sha(HERE / name) for name in ('title542m.py','gate542m.py','quit_owned_app.js','common.py','startup542.py','boot_response542.py','boot_frames542.py','screenshot_boot542.py')}
    gate_sources['A_diagnostic_runner'] = common.sha(Path(__file__))
    common.disk(); out.mkdir(parents=True)
    common.wait_quiet(out / 'quiet.json')
    base = Path(subprocess.check_output(['/usr/bin/getconf', 'DARWIN_USER_CACHE_DIR'], text=True).strip())
    target = base / own_id
    assert base.is_absolute() and target.name == own_id and base != Path('/')
    warm_from = args.warm_from.resolve() if args.warm_from else None
    if warm_from:
        assert warm_from.is_relative_to(common.ROOT.resolve())
        prime = json.loads((warm_from / 'launch.json').read_text())
        completed = json.loads((warm_from / 'process.json').read_text())
        assert prime['perf_exe_sha256'] == before and prime['bundle_id'] == own_id
        assert prime.get('keep_cache') is True and completed['normal_exit']
        cache = Path(prime['environment']['PETARI_CACHE_DIR']).resolve()
        assert cache == warm_from / 'cache' and cache.is_dir() and target.is_dir()
    else:
        if target.exists(): shutil.rmtree(target)
        cache = out / 'cache'
        cache.mkdir()
    user = out / 'user'
    user.mkdir()
    cache_before = common.tree(cache)
    metal_before = common.tree(target) if target.exists() else {}
    env = {k: v for k, v in os.environ.items() if not k.startswith(('PETARI_', 'ASTRAL_', 'DYLD_', 'THRPROBE_'))}
    env.update(SDL_AUDIO_DRIVER='dummy', PETARI_MODS='none', PETARI_CACHE_DIR=str(cache),
               PETARI_FRAME_CSV=str(out / 'frames.csv'), PETARI_FRAME_STATS='1', PETARI_PIPELINE_DIAG='0',
               PETARI_ROUTE_RECORD=str(out / 'route.csv'), PETARI_TRACE_BOOT='0',
               PYTHONDONTWRITEBYTECODE='1', LANG='C', LC_ALL='C')
    if args.require_boot_responsiveness:env['PETARI_BOOT_SPLASH_CAPTURE']=str(out/'boot-splash.png')
    if not args.baseline:
        env.update(PETARI_TITLE_OBSERVE='1', PETARI_TITLE_OBSERVE_MIN_SECONDS='315')
    env.update(args.diagnostic_env)
    command = [str(executable), '--disc', str(common.DISC), '--user', str(user)]
    started = time.monotonic(); timeout = 315 if args.baseline else 900
    launch = {'command': command, 'environment': {k: v for k, v in env.items() if k.startswith(('PETARI_', 'SDL_'))},
              'gate_sources_at_launch': gate_sources, 'label': args.label, 'baseline': args.baseline, 'build_kind': prep['build_kind'],
              'source_commit': prep['source_commit'], 'original_exe_sha256': prep['original_exe_sha256'],
              'perf_exe_sha256': before, 'prep': str(args.prep), 'prep_sha256': common.sha(args.prep),
              'bundle_id': own_id, 'cold_metal_cleared': not bool(warm_from), 'fresh_private_cache': None if warm_from else str(cache),
              'warm_from': str(warm_from) if warm_from else None, 'keep_cache': args.keep_cache,
              'private_cache_before': cache_before, 'metal_namespace': str(target), 'metal_before': metal_before,
              'no_input': True, 'nice': os.getpriority(os.PRIO_PROCESS, 0), 'load': os.getloadavg(),
              'comparison_window_seconds': 300, 'native_observation_minimum_seconds': None if args.baseline else 315,
              'hard_cap_seconds': 900, 'start_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())}
    assert launch['nice'] == 0
    with (out / 'app.log').open('wb') as log:
        from startup542 import Observer
        observer = Observer(out)
        observer.popen_before()
        proc = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, env=env)
        observer.spawned(proc.pid)
        boot_probe = None
        if args.require_boot_responsiveness:
            from boot_response542 import Probe
            boot_probe = Probe(out, proc.pid, app, m1_screenshot=args.tier=='m1')
        launch['popen_clocks'] = {k:observer.result[k] for k in ('popen_call_before','popen_return_after')}
        launch['pid'] = proc.pid; common.write(out / 'launch.json', launch)
        timed_out, orderly, error = False, None, None
        try:
            if args.baseline:
                while proc.poll() is None and time.monotonic() - started < timeout:
                    time.sleep(min(1, max(.01, timeout - (time.monotonic() - started))))
                if proc.poll() is None:
                    quit_command = ['/usr/bin/osascript', '-l', 'JavaScript', str(HERE / 'quit_owned_app.js'), str(proc.pid), str(app), own_id]
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
    startup = observer.finish()
    boot_response = boot_probe.finish() if boot_probe else None
    text = (out / 'app.log').read_text(errors='replace')
    process = {'exit': proc.returncode, 'timed_out': timed_out, 'seconds': time.monotonic() - started,
               'no_faults': not common.FAULT.search(text), 'binary_unchanged': common.sha(executable) == before,
               'resources_unchanged': common.tree(app / 'Contents/Resources') == resources,
               'frames_csv_flushed': (out / 'frames.csv').exists(), 'orderly_quit': orderly,
               'normal_exit': proc.returncode == 0}
    process['error'] = error
    common.write(out / 'process.json', process)
    scored = gate542m.score(out, title=not args.baseline, window_seconds=300, require_deadlines=args.required_deadlines, tier=args.tier, require_frontend=args.require_frontend_prepresent) if process['frames_csv_flushed'] else None
    route = common.rows(out / 'route.csv')
    neutral = all(not any(float(r.get(k, 0)) for k in ('move_x', 'move_y', 'jump', 'spin', 'crouch', 'shoot', 'pointer_valid')) for r in route)
    checks = {'startup_observer_complete': startup.get('observer_joined') and not startup.get('observer_error') and 'title_first_draw' in startup,
              'normal_exit': process['normal_exit'], 'not_timed_out': not timed_out, 'no_runner_error': error is None,
              'no_faults': process['no_faults'],
              'binary_unchanged': process['binary_unchanged'], 'resources_unchanged': process['resources_unchanged'],
              'frames_flushed': process['frames_csv_flushed'], 'no_input_in_route': neutral and bool(route),
              'no_stage_entry': all(r['stage'] in ('', 'FileSelect') for r in route),
              'fixed_window_complete': scored is not None and scored['fixed_window']['complete']}
    if args.require_boot_responsiveness: checks['boot_responsive_visible'] = boot_response is not None and boot_response['passed']
    if not args.baseline:
        checks['native_title_draw_bound_verified'] = startup.get('native_title_draw_inside_external_bounds') is True
        checks['gate542m'] = scored is not None and scored['passed']
    result = {'measurement_kind':'543 pacing diagnostic; no acceptance claim', 'passed': all(checks.values()), 'checks': checks, 'label': args.label, 'baseline': args.baseline,
              'boot_response': boot_response, 'source_commit': prep['source_commit'], 'perf_exe_sha256': before, 'original_exe_sha256': prep['original_exe_sha256'],
              'process': process, 'gate542m': scored, 'comparison_window_seconds': 300,
              'baseline_scope': 'retained unsplit evidence, no missing-signal acceptance claim' if args.baseline else None,
              'prep': str(args.prep), 'prep_sha256': common.sha(args.prep), 'launch_sha256': common.sha(out / 'launch.json'),
              'gate_sources': gate_sources}
    common.write(out / 'RESULT.json', result)
    common.write(out / 'cache-provenance.json', {'private_path': str(cache), 'private_before': cache_before,
                 'private_after': common.tree(cache), 'metal_path': str(target), 'metal_before': metal_before,
                 'metal_after': common.tree(target) if target.exists() else {}, 'warm_from': str(warm_from) if warm_from else None,
                 'cleared': not bool(warm_from), 'keep_cache': args.keep_cache})
    common.trim(out, cache=not args.keep_cache)
    print(json.dumps(result), flush=True)
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
