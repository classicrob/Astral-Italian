#!/usr/bin/env python3
"""Finite no-host-input recording/replay harness; no retries or receipt repairs.

Convert an old tape by strictly replaying it with actual-read capture. The same
command replays schema2 tapes on an instrumented reference and independently
checks all captured reads. Take heavy->native locks OUTSIDE this command. Use a
prepared owned perf app, exact copied fixture, and a fresh --out under m1g.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
from tape_phase_audit import POLICY, POWER_PROTOCOL, need, qualify, load_meta, power_events
from replay_tape import mods_env


def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()


def tree(p):
    return {str(f.relative_to(p)): sha(f) for f in sorted(p.rglob('*')) if f.is_file()}


def inventory(p):
    """All fixture bytes and xattrs; no attribute is removed or rewritten."""
    attrs = {}
    for f in [p, *sorted(p.rglob('*'))]:
        need(not f.is_symlink(), 'fixture symlink is unsupported')
        names = subprocess.check_output(['/usr/bin/xattr', str(f)], text=True).splitlines()
        values = {name: ''.join(subprocess.check_output(['/usr/bin/xattr', '-p', '-x', name, str(f)], text=True).split()).lower()
                  for name in sorted(names)}
        if values: attrs['.' if f == p else f.relative_to(p).as_posix()] = values
    return dict(files=tree(p), xattrs=attrs)


PRODUCTION_PIPELINE_ENV = dict(PETARI_PIPELINE_DIAG='0', PETARI_TRACE_BOOT='0')


def warm_source(source, exe, perf_sha):
    """Only a full successful production-setting fresh arm may prime warm proof."""
    prior = json.loads((source/'launch.json').read_text())
    result = json.loads((source/'RESULT.json').read_text())
    process = json.loads((source/'process.json').read_text())
    native = json.loads((source/'replay/divergence.json').read_text())
    need(prior['exe_sha256'] == perf_sha and prior['command'][0] == str(exe),
         'warm source must use the same exe and bundle')
    need(prior.get('warm_from') is None and result.get('passed') is True,
         'warm source must be a successful fresh arm')
    need(process.get('exit') == 0 and process.get('timed_out') is False and
         native.get('finished') is True and not native.get('stopped'),
         'warm source must finish strict replay normally')
    need(all(prior['environment'].get(k) == v for k,v in PRODUCTION_PIPELINE_ENV.items()),
         'warm source production diagnostics settings differ')
    need((source/'cache').is_dir(), 'warm source private cache missing')
    return dict(arm=str(source), launch_sha256=sha(source/'launch.json'),
                result_sha256=sha(source/'RESULT.json'), process_sha256=sha(source/'process.json'),
                native_report_sha256=sha(source/'replay/divergence.json'),
                production_pipeline_env=PRODUCTION_PIPELINE_ENV, cache_hashes=tree(source/'cache'))


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--app', type=Path, required=True)
    ap.add_argument('--disc', type=Path, required=True)
    ap.add_argument('--prep', type=Path, required=True)
    ap.add_argument('--tape', type=Path, required=True)
    ap.add_argument('--out', type=Path, required=True)
    ap.add_argument('--user-seed', type=Path)
    ap.add_argument('--fixture-receipt', type=Path, help='require every source/copied fixture byte and xattr to match this receipt')
    ap.add_argument('--stage', default='')
    ap.add_argument('--scenario', type=int, default=1)
    ap.add_argument('--bound', type=float, default=600)
    ap.add_argument('--observe-seconds', type=float, default=90)
    ap.add_argument('--real-audio', action='store_true')
    ap.add_argument('--post-roll-seconds',type=float,default=0,help='minimum neutral tail when converting legacy input (new canonical: >=150s)')
    ap.add_argument('--require-power-event',action='store_true')
    ap.add_argument('--minimum-recorded-neutral-seconds',type=float,default=150)
    ap.add_argument('--require-pointer-evidence', action='store_true', help='world-map case: >=30 actual valid pointer rows at >=3 positions')
    ap.add_argument('--allow-incomplete', action='store_true', help='explicitly use only the legacy tape declared checkpoint prefix')
    ap.add_argument('--warm-from', type=Path, help='copy a successful same-exe arm private cache; retain its Metal bundle namespace')
    ap.add_argument('--allowed-root', type=Path, default=Path.home()/'MacioGalaxy2-gamedata/port53/m1g')
    a = ap.parse_args(argv)
    a.app, a.tape, a.disc = a.app.resolve(), a.tape.resolve(), a.disc.resolve()
    a.out = a.out.resolve(); allowed = a.allowed_root.resolve()
    need(allowed in a.out.parents and a.out != allowed, 'output must be inside owned port53/m1g root')
    need(not a.out.exists(), 'fresh output required; no overwrite/resume')
    need(os.getpriority(os.PRIO_PROCESS, 0) == 0, 'nice0 required (use bash, not zsh BG_NICE)')
    need(shutil.disk_usage(allowed).free >= 50*1024**3, 'at least50GiB free required')
    need(0 < a.observe_seconds < a.bound <= 1800, 'invalid finite observation/bound')
    need(0<=a.post_roll_seconds<a.bound and a.minimum_recorded_neutral_seconds>=150,'invalid neutral-tail bound')
    meta = load_meta(a.tape/'route.tape.json')
    need(meta.get('schema', 1) in (1, 2), 'unknown tape schema')
    need(meta.get('complete') is True or a.allow_incomplete, 'incomplete source needs explicit --allow-incomplete')
    if meta.get('schema') == 2:
        launch = meta['launch']
        need(not a.stage or (a.stage == launch['stage'] and a.scenario == launch['scenario']), 'explicit launch disagrees with tape')
        a.stage, a.scenario = launch['stage'], launch['scenario']
    exe = a.app/'Contents/MacOS/Petari'
    prep = json.loads(a.prep.read_text())
    perf_sha = prep.get('perf_exe_sha256', prep.get('perf_sha256'))
    need(sha(exe) == perf_sha, 'PREP does not bind app exe')
    before_app = tree(a.app); before_tape = tree(a.tape)
    before_seed = tree(a.user_seed) if a.user_seed else {}
    fixture_binding = None
    if a.fixture_receipt:
        need(a.user_seed is not None, 'fixture receipt requires a user seed')
        receipt = json.loads(a.fixture_receipt.read_text())
        fixture_binding = {k: receipt[k] for k in ('files', 'xattrs')}
        need(inventory(a.user_seed) == fixture_binding, 'fixture differs from certified bytes/xattrs')
    warm_binding = None
    if a.warm_from:
        a.warm_from = a.warm_from.resolve()
        warm_binding = warm_source(a.warm_from, exe, perf_sha)
    a.out.mkdir(parents=True, mode=0o700)
    user = a.out/'user'
    if a.user_seed:
        subprocess.run(['/usr/bin/ditto', '--rsrc', '--extattr', str(a.user_seed), str(user)], check=True)
    else: user.mkdir()
    need(tree(user) == before_seed, 'private user copy differs')
    if fixture_binding: need(inventory(user) == fixture_binding, 'private fixture copy xattrs differ')
    if a.warm_from:
        shutil.copytree(a.warm_from/'cache', a.out/'cache', copy_function=shutil.copy2)
        need(tree(a.out/'cache') == warm_binding['cache_hashes'], 'warm cache copy differs')
    env = {k: v for k, v in os.environ.items() if not k.startswith(('PETARI_', 'ASTRAL_', 'DYLD_', 'THRPROBE_'))}
    env.update(ASTRAL_TAPE='off', PETARI_MODS=mods_env(meta['mods']),
               SDL_AUDIO_DRIVER='coreaudio' if a.real_audio else 'dummy',
               PETARI_CACHE_DIR=str(a.out/'cache'), PETARI_FRAME_CSV=str(a.out/'frames.csv'), PETARI_FRAME_STATS='1',
               PETARI_TAPE_RECORD=str(a.out), PETARI_TAPE_CAPTURE_REPLAY='1',
               PETARI_TAPE_PLAY=str(a.tape), PETARI_TAPE_PLAY_SESSION='1',
               PETARI_TAPE_PLAY_READ_MAP=str(a.out/'read-map.jsonl'), PETARI_TAPE_PLAY_OUT=str(a.out/'replay'),
               PETARI_TAPE_PLAY_QUIT='1', PETARI_TAPE_PLAY_OBSERVE_SECONDS=str(a.observe_seconds),
               PETARI_TAPE_PLAY_POST_ROLL_SECONDS=str(a.post_roll_seconds), **PRODUCTION_PIPELINE_ENV)
    for k, v in (meta.get('replay_env') or {}).items():
        need(k == 'PETARI_TEST_RETURN_TO_SHIP', 'unsupported recorded scene hook')
        env[k] = str(v)
    if a.real_audio: env.update(PETARI_SMOKE_REAL_AUDIO='1', PETARI_AUDIO_DIAG='1')
    command = [str(exe), '--disc', str(a.disc), '--user', str(user)]
    if a.stage: command += ['--stage', a.stage, '--scenario', str(a.scenario)]
    launch = dict(command=command, environment={k: v for k, v in env.items() if k.startswith(('PETARI_', 'ASTRAL_', 'SDL_AUDIO'))},
                  prep=str(a.prep.resolve()), prep_sha256=sha(a.prep), prep_contents=prep, exe_sha256=perf_sha,
                  native_schema_source=meta.get('schema', 1), kind='conversion' if meta.get('schema', 1) == 1 else 'schema2-reference-replay',
                  nice=0, bound_seconds=a.bound, host_input_used=False, locks='caller holds heavy then native',
                  allow_incomplete=a.allow_incomplete, warm_from=warm_binding,
                  require_power_event=a.require_power_event, post_roll_seconds=a.post_roll_seconds,
                  minimum_recorded_neutral_seconds=a.minimum_recorded_neutral_seconds,
                  cache_policy='private copied cache, same Metal bundle namespace' if a.warm_from else 'fresh private cache; caller certifies cold Metal namespace',
                  fixture_receipt=str(a.fixture_receipt.resolve()) if a.fixture_receipt else None,
                  fixture_receipt_sha256=sha(a.fixture_receipt) if a.fixture_receipt else None,
                  fixture_all_bytes_and_xattrs_verified=fixture_binding is not None,
                  source_sha256={p.name: sha(p) for p in (Path(__file__), Path(__file__).with_name('tape_phase_audit.py'))})
    process = dict(exit=None, timed_out=False)
    start = time.monotonic()
    with (a.out/'app.log').open('wb') as log:
        child = subprocess.Popen(command, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
        launch.update(pid=child.pid, child_nice_at_spawn=os.getpriority(os.PRIO_PROCESS, child.pid), start_monotonic=start)
        (a.out/'launch.json').write_text(json.dumps(launch, indent=2)+'\n')
        if launch['child_nice_at_spawn'] != 0:
            child.terminate()
            try: child.wait(timeout=15)
            except subprocess.TimeoutExpired: child.kill(); child.wait()
            raise ValueError('app did not inherit nice0; owned child terminated')
        try: process['exit'] = child.wait(timeout=a.bound)
        except subprocess.TimeoutExpired:
            process['timed_out'] = True; child.terminate()
            try: process['exit'] = child.wait(timeout=15)
            except subprocess.TimeoutExpired: child.kill(); process['exit'] = child.wait()
    process['seconds'] = time.monotonic()-start
    (a.out/'process.json').write_text(json.dumps(process, indent=2)+'\n')
    result = dict(passed=False, process=process, checks={}, error=None)
    try:
        checks = result['checks']
        checks.update(process_normal=process['exit'] == 0 and not process['timed_out'],
                      app_unchanged=before_app == tree(a.app), tape_unchanged=before_tape == tree(a.tape),
                      fixture_unchanged=not a.user_seed or before_seed == tree(a.user_seed),
                      fixture_xattrs_unchanged=not fixture_binding or inventory(a.user_seed) == fixture_binding,
                      warm_source_unchanged=not a.warm_from or warm_binding['cache_hashes'] == tree(a.warm_from/'cache'))
        need(all(checks.values()), 'process or immutable binding failed')
        proof = qualify(a.tape, a.out, a.out/'replay/divergence.json', a.out/'read-map.jsonl')
        result['input_proof'] = proof; checks['actual_input_bytes_and_phase_format'] = proof['passed']
        native = json.loads((a.out/'replay/divergence.json').read_text())
        observation = native.get('observation', {})
        checks['observation_completed'] = (observation.get('completed') is True and
            observation.get('elapsed_seconds', 0) >= a.observe_seconds and process['seconds'] >= a.observe_seconds)
        roll = observation.get('post_roll', {})
        checks['neutral_post_roll'] = (roll.get('started') is True and roll.get('loops', 0) > 0 and
            roll.get('neutral_read_calls', 0) > 0 and roll.get('input') == 'neutral')
        checks['lossless_recording'] = json.loads((a.out/'route.tape.json').read_text())['complete'] is True
        if a.require_power_event:
            recorded=load_meta(a.out/'route.tape.json');controls=power_events(recorded)
            checks['versioned_real_power_event']=(recorded.get('session_control_protocol')==POWER_PROTOCOL and bool(controls) and
                controls[-1]['kind']=='power-request' and controls[-1]['callback_observed'] is True)
            checks['recorded_neutral_tail_at_least_150s']=(len(controls)==2 and controls[0]['kind']=='neutral-tail-begin' and
                controls[-1]['neutral_elapsed_seconds']>=a.minimum_recorded_neutral_seconds)
            if meta.get('schema')==2:
                checks['source_has_versioned_power_event']=meta.get('session_control_protocol')==POWER_PROTOCOL

        captured_launch = json.loads((a.out/'route.tape.json').read_text()).get('launch')
        checks['actual_launch_metadata'] = captured_launch == dict(stage=a.stage, scenario=a.scenario)
        if meta.get('schema') == 2:
            phases = native.get('input_phases', [])
            checks['schema2_marker_checks'] = native['anchor_policy'] == POLICY and len(phases) == len(meta['input_phases']) and all(
                p.get('marker_checked') is True and p.get('marker_passed') is True and p.get('rng_verified') is True and
                (not p['exit_required'] or p['exit_checked'] is True and p['exit_passed'] is True) and
                p['checkpoints_required'] == p['checkpoints_checked'] for p in phases)
        map_segments = [p for p in native['segments'] if p['stage'].startswith('WorldMap')]
        if a.require_pointer_evidence:
            need(map_segments, 'required pointer case has no map segment')
            checks['pointer_coverage'] = sum(p['pointer_valid_rows_compared'] for p in map_segments) >= 30 and max(
                p['pointer_distinct_positions_compared'] for p in map_segments) >= 3
        if a.real_audio:
            import re
            text = (a.out/'app.log').read_text(errors='replace')
            counts = [int(v) for v in re.findall(r'underrun (\d+) frames', text)]
            result['audio_reports'] = len(counts); result['audio_underrun_frames'] = sum(counts)
            checks['real_audio_reports'] = len(counts) >= 50 and sum(counts) == 0
        result['passed'] = all(checks.values())
    except (OSError, ValueError, KeyError, TypeError, IndexError, AssertionError) as e:
        result['error'] = str(e)
    result['scope'] = 'Actual read capture + strict native replay; no retry, no acceptance substitution'
    (a.out/'RESULT.json').write_text(json.dumps(result, indent=2)+'\n')
    print(a.out/'RESULT.json', 'PASS' if result['passed'] else 'FAIL')
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
