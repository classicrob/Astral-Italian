#!/usr/bin/env python3
"""B-owned candidate cohort: alternating Yoshi/Fluffy/Starship x3, then two evidence spot checks.

Uses an already prepared, hash-bound server candidate perfm1b app. Each arm owns
heavy->native independently. gate533 is retained verbatim alongside gate542m;
title and server no-regression proof remain separate required dependencies.
"""
import argparse
import contextlib
import fcntl
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys

import common
import gate542m
import tier_cold54m

OWN_ID = 'dev.petari.Petari.perfm1b'
STAGES = ('YosshiHomeGalaxy', 'MokumokuValleyGalaxy', 'MarioFaceShipGalaxy')
SPOTS = ('IslandFleetGalaxy', 'VsKoopaLv1Galaxy')


@contextlib.contextmanager
def native_only():
    with common.LOCK.open('a') as native:
        fcntl.lockf(native, fcntl.LOCK_EX)
        yield


def clear_owned_metal():
    base = Path(subprocess.check_output(['/usr/bin/getconf', 'DARWIN_USER_CACHE_DIR'], text=True).strip())
    target = base / OWN_ID
    assert base.is_absolute() and base != Path('/') and target.name == 'dev.petari.Petari.perfm1b'
    if target.exists(): shutil.rmtree(target)


def arm(args, stage, repeat, spot):
    common.ID, common.native_lock, common.clear_metal = OWN_ID, native_only, clear_owned_metal
    app, prep_path = args.app.resolve(), args.prep.resolve()
    prep = json.loads(prep_path.read_text())
    build = prep.get('build', prep.get('build_metadata'))
    assert prep['build_kind'] == 'server-candidate' and prep['bundle_id'] == OWN_ID
    assert Path(prep['app']).resolve() == app and build['source'] == prep['source_commit']
    assert build['binary_sha256'] == prep['original_exe_sha256']
    assert common.sha(app / 'Contents/MacOS/Petari') == prep['perf_sha256']
    before_resources = common.tree(app / 'Contents/Resources')
    if 'resource_tree' in prep: assert before_resources == prep['resource_tree']
    original = Path(prep['original_app'])
    assert original.resolve().is_relative_to((common.ROOT / 'candidates').resolve())
    assert common.sha(original / 'Contents/MacOS/Petari') == prep['original_exe_sha256']
    assert common.tree(original / 'Contents/Resources') == before_resources
    out = common.ROOT / 'runs' / f'{args.label}-{stage}-r{repeat}'
    process = common.run(app, out, stage=stage, script=common.REPO / 'native/tools/perf_sweep_move.script',
                         extra={'PETARI_PIPELINE_DIAG': '0', 'PETARI_TRACE_BOOT': '0'}, cold=True, seconds=240)
    measured = tier_cold54m.analyze(out)
    deadline_counts = {'YosshiHomeGalaxy':183, 'MokumokuValleyGalaxy':168, 'MarioFaceShipGalaxy':74, 'VsKoopaLv1Galaxy':207}
    # Island is an explicitly unqualified recording fallback; preserve its spot evidence.
    scored = gate542m.score(out, require_deadlines={stage:deadline_counts[stage]} if stage in deadline_counts else None)
    old = subprocess.run([sys.executable, '-B', str(common.HERE / 'gate533.py'), str(out)], capture_output=True, text=True)
    legacy = json.loads(old.stdout)
    text = (out / 'app.log').read_text(errors='replace')
    valid = sum(r['stage'] == stage and r['valid'] == '1' for r in common.rows(out / 'route.csv'))
    checks = {'normal_exit': process['completed'] and not process['timed_out'], 'no_faults': process['no_faults'],
              'binary_unchanged': process['binary_unchanged'], 'seed_unchanged': process['seed_unchanged'],
              'resources_unchanged': common.tree(app / 'Contents/Resources') == before_resources,
              'valid_stage_rows': valid >= 300, 'script_completed': '[input-script] done after 970 frames' in text,
              'fifo_present': 'present mode Fifo' in text, 'gameplay_and_menu_observed':
              legacy['gameplay_frames'] > 0 and legacy['menu_frames'] > 0,
              'gate533_unchanged': legacy['result'] == 'PASS', 'gate542m': scored['passed'],
              'raw_gameplay_over100': measured['gameplay_over100'] == 0}
    result = {'passed': all(checks.values()), 'checks': checks, 'stage': stage, 'repeat': repeat, 'case': str(out),
              'evidence_spot_check': spot, 'spot_check_source': 'two non-gate stages from the frozen perf table worst three',
              'build_kind': 'server-candidate', 'source_commit': prep['source_commit'],
              'original_exe_sha256': prep['original_exe_sha256'], 'perf_exe_sha256': prep['perf_sha256'],
              'renderer_key': build['renderer_key'], 'prep': str(prep_path), 'prep_sha256': common.sha(prep_path),
              'process': process, 'valid_stage_rows': valid, 'measurements': measured, 'gate542m': scored,
              'legacy_gate_observation': legacy, 'legacy_gate_exit': old.returncode,
              'pipeline_diag': 0, 'trace_boot': 0, 'nice': 0, 'bundle_id': OWN_ID,
              'gate_sources': {n: common.sha(common.HERE / n) for n in
                               ('cold542m.py', 'gate542m.py', 'gate533.py', 'tier_cold54m.py', 'common.py')},
              'raw_artifacts': {n: common.sha(out / n) for n in ('frames.csv', 'app.log', 'launch.json', 'quiet.json', 'route.csv')},
              'acceptance_scope': 'M1 cold cohort only; title, baseline comparison and M4 no-regression required separately'}
    common.write(out / 'RESULT.json', result)
    common.trim(out)
    print(json.dumps({'case': str(out), 'passed': result['passed'], 'gameplay_over100': measured['gameplay_over100'],
                      'menu': scored['unsplit'], 'non_gameplay_max': scored['all_non_gameplay']['max_ms']}), flush=True)
    return 0 if result['passed'] else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--app', type=Path, required=True)
    parser.add_argument('--prep', type=Path, required=True)
    parser.add_argument('--label', required=True)
    parser.add_argument('--arm', type=int, help=argparse.SUPPRESS)
    args = parser.parse_args()
    assert re.fullmatch(r'[A-Za-z0-9_-]+', args.label) and os.getpriority(os.PRIO_PROCESS, 0) == 0
    plan = [(stage, repeat, False) for repeat in (1, 2, 3) for stage in STAGES] + [(stage, 1, True) for stage in SPOTS]
    if args.arm is not None:
        assert 0 <= args.arm < len(plan)
        return arm(args, *plan[args.arm])
    base = common.ROOT / 'campaigns' / args.label
    base.mkdir(parents=True, exist_ok=False)
    results = []
    for index, (stage, repeat, spot) in enumerate(plan):
        command = ['/usr/bin/lockf', '-k', str(common.HEAVY), sys.executable, '-B', __file__,
                   '--app', str(args.app), '--prep', str(args.prep), '--label', args.label, '--arm', str(index)]
        with (base / f'arm-{index}.log').open('wb') as log:
            status = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT)
        path = common.ROOT / 'runs' / f'{args.label}-{stage}-r{repeat}' / 'RESULT.json'
        result = json.loads(path.read_text()) if path.exists() else {'passed': False, 'error': 'missing arm receipt',
                                                                    'case': str(path.parent), 'driver_exit': status.returncode}
        results.append(result)
        target = results[:9]
        common.write(base / 'COHORT.json', {'plan': plan, 'runs': results, 'completed_all_runs': len(results) == len(plan),
                    'cold_gate_9_passed': len(target) == 9 and all(r['passed'] for r in target),
                    'spot_checks_are_additional_evidence': True, 'release_acceptance_claimed': False,
                    'pending_dependencies': ['title3', 'same-window541comparison', 'server-no-regression']})
        print('END', index, stage, repeat, 'passed', result['passed'], flush=True)
        if 'error' in result: return 1
    return 0 if all(r['passed'] for r in results[:9]) else 1


if __name__ == '__main__': raise SystemExit(main())
