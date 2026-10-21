#!/usr/bin/env python3
"""Bounded unattended Yoshi retail replay using a pinned converted input tape."""
import argparse
import configparser
import csv
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import socket
import subprocess
import sys
import time
import tempfile

from kpad_records import RawSuffix, digest
from kpad_script import contract

TAPE_SHA = '8c49946eb8f48257c8dd7b0d6f45abfb4a7e56db23f639f44c9509b364ad91ae'
KPD_SHA = 'a381fffbb8cc0d16dd550f839d464ee803e963f34bc2d38347d90b49b9b588d8'
DOLPHIN_SHA = '888de10b7c6e8297aab2e4d51b050589b8dec9f13c889b7f094b3e92b9285e77'
ISO_SHA = '495188876a68fecfd8d2614e84435e5a1b76c50aaf35e7fc99204771be78dc4c'


def write_json(path, value):
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(value, indent=2) + '\n')
    temporary.replace(path)


def state(root, phase, output, pid=None):
    (root / 'STATE').write_text(f'HOST {socket.gethostname()} | SOURCE selector02185568+raw-adapter | BINARY {DOLPHIN_SHA} | PHASE {phase} | OUTPUT {output} | PID {pid or "none"} | RESUME fresh phase if interrupted\n')


def require_hash(path, expected):
    actual = digest(path)
    if actual != expected:
        raise RuntimeError(f'hash mismatch: {path}: {actual}')
    return actual


def prefix(source, target, frames):
    with source.open() as stream, target.open('x') as output:
        for line in stream:
            if json.loads(line)['frame'] >= frames:
                break
            output.write(line)


def bind_actual_suffix(capture, source, origin, frames, observed_connection):
    expected = iter(contract.rows_from_jsonl(source))
    actual = (row for row in contract.rows_from_jsonl(capture / 'inputs.jsonl') if row['frame'] > origin)
    target = capture / 'observed-inputs.jsonl'
    partial = target.with_suffix('.jsonl.partial')
    reads, differences, first = 0, 0, None
    with partial.open('x') as stream:
        for row in actual:
            normalized = dict(row, frame=row['frame'] - origin - 1)
            wanted = next(expected, None)
            if wanted and not observed_connection and normalized['connection'] != wanted['connection']:
                raise RuntimeError('synthetic/actual connection mismatch is an input-delivery boundary, not gameplay divergence')
            candidate = dict(normalized, connection=wanted['connection']) if wanted and observed_connection else normalized
            if wanted != candidate:
                raise RuntimeError(f'exported source sample/read/history changed at {normalized["frame"]}:{normalized["read_ordinal"]}')
            if normalized['connection'] != wanted['connection']:
                differences += 1
                if first is None:
                    first = dict(input_frame=normalized['frame'], first_affected_trace_frame=normalized['frame'] + 1,
                                 source_frame=row['frame'], read_ordinal=row['read_ordinal'], channel=row['channel'],
                                 synthetic=wanted['connection'], observed=normalized['connection'],
                                 classification='input-delivery boundary; not gameplay divergence')
            stream.write(json.dumps(normalized, separators=(',', ':')) + '\n')
            reads += 1
        if next(expected, None) is not None: raise RuntimeError('exported suffix is truncated')
    binary = capture / 'observed-inputs.kpd'
    binary_partial = binary.with_suffix('.kpd.partial')
    previous_tempdir = tempfile.tempdir
    try:
        tempfile.tempdir = str(capture)
        with binary_partial.open('xb') as stream:
            contract.encode_rows(contract.rows_from_jsonl(partial), frames, stream)
    finally:
        tempfile.tempdir = previous_tempdir
    binary_partial.rename(binary)
    partial.rename(target)
    return dict(schema_version=1, status='captured', reads=reads, frames=frames,
                path=str(target), sha256=digest(target), kpd_path=str(binary), kpd_sha256=digest(binary),
                samples_history='identical to pinned conversion; frame labels only rebased',
                connections='actual read-only WPADProbe' if observed_connection else 'source connection verified',
                connection_difference_reads=differences, first_connection_difference=first,
                preconverted_native_comparison_through_trace_frame=first['input_frame'] if first else frames,
                matched_native_replay_requires='this observed-inputs.jsonl, never the old synthetic-connection tape')


def verify_export(case, source, frames):
    manifest = json.loads((case / 'capture/manifest.json').read_text())
    if manifest['status'] != 'captured; repeat qualification required' or manifest['frames'] != frames + 1:
        raise RuntimeError(f'incomplete capture: {manifest.get("error", manifest["status"])}')
    if manifest['raw_input_suffix']['status'] != 'captured':
        raise RuntimeError('raw suffix did not finalize')
    if manifest['stage_entry']['status'] != 'requested' or not manifest['stage_entry']['destination_branch_skipped']:
        raise RuntimeError('selector runtime acceptance failed')
    origin = manifest['sync']['source_frame']
    if manifest['raw_input_suffix']['source_start'] != origin + 1:
        raise RuntimeError('suffix/trace anchor phase mismatch')
    binding = bind_actual_suffix(case / 'capture', source, origin, frames,
                                 manifest['raw_input_suffix'].get('connection_policy') == 'observed read-only probe')
    reads = binding['reads']
    if binding['connection_difference_reads'] != manifest['raw_input_suffix'].get('connection_difference_reads', 0):
        raise RuntimeError('actual export connection differences disagree with producer receipt')
    with (case / 'capture/oracle.csv').open() as stream:
        rows = csv.DictReader(stream)
        first = next(rows)
        if first['stage'] != 'YosshiHomeGalaxy' or first['scenario'] != '1' or first['init_state'] != '6':
            raise RuntimeError('wrong accepted ready target')
    return dict(status='verified diagnostic only', qualification='BLOCKED', measured_qualified_frames=None,
                delivered_input=binding, suffix_reads=reads, natural_ready_seed=first['rng_seed'], source_origin=origin,
                oracle_sha256=digest(case / 'capture/oracle.csv'), input_export_sha256=digest(case / 'capture/inputs.jsonl'),
                collection_elapsed_seconds=manifest['collection_elapsed_seconds'],
                suffix_elapsed_seconds=manifest['suffix_elapsed_seconds'], stage_clear=manifest.get('stage_clear'))


def run_case(root, tools, profile_source, source, frames, label, budget, receipt):
    case = root / label
    case.mkdir()
    profile = case / 'profile'
    shutil.copytree(profile_source, profile)
    endpoint = case / 'gdb.sock'
    ini = profile / 'Config/Dolphin.ini'
    before = configparser.ConfigParser(); before.read(ini)
    config = configparser.ConfigParser(); config.read(ini)
    config.set('General', 'GDBSocket', str(endpoint))
    with ini.open('w') as stream: config.write(stream)
    after = configparser.ConfigParser(); after.read(ini)
    before.set('General', 'GDBSocket', str(endpoint))
    if {section: dict(before[section]) for section in before} != {section: dict(after[section]) for section in after}:
        raise RuntimeError('profile has a change other than private socket path')
    command = [sys.executable, str(tools / 'dolphin_stage_entry.py'), '--entry-stage', 'YosshiHomeGalaxy',
               '--entry-scenario', '1', '--entry-dol', receipt['dol'],
               '--clear-witness', 'collect', '--slot-held', '--dolphin', receipt['dolphin'],
               '--disc', receipt['disc'], '--user', str(profile), '--socket', str(endpoint),
               '--input-script', str(root / 'boot-menu.json'), '--sync-stage', 'YosshiHomeGalaxy',
               '--sync-scenario', '1', '--sync-input-records', str(source),
               '--sync-input-records-sha256', digest(source), '--sync-observed-connection', '--frames', str(frames + 1),
               '--wall-timeout', str(budget), '--yoshi-replay-budget', '--output', str(case / 'capture')]
    state(root, f'{label}:pre-spawn', case)
    phase = dict(label=label, status='incomplete', frames=frames, argv=command,
                 started_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()))
    receipt['phases'].append(phase)
    write_json(root / 'receipt.json', receipt)
    with (case / 'driver.log').open('x') as log:
        child = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
        phase['collector_pid'] = child.pid
        state(root, f'{label}:running', case, child.pid)
        write_json(root / 'receipt.json', receipt)
        try:
            phase['exit_code'] = child.wait(timeout=budget + 120)
        finally:
            if child.poll() is None:
                child.terminate()
                try: child.wait(timeout=20)
                except subprocess.TimeoutExpired:
                    child.kill(); child.wait()
    if phase['exit_code'] != 0:
        raise RuntimeError(f'{label} failed with exit {phase["exit_code"]}; preserve {case}')
    result = verify_export(case, source, frames)
    if frames == 30000:
        require_hash(case / 'capture/validated-inputs.kpd', KPD_SHA)
    manifest = json.loads((case / 'capture/manifest.json').read_text())
    pid = manifest.get('dolphin_pid')
    if pid:
        try: os.kill(pid, 0)
        except ProcessLookupError: pass
        else: raise RuntimeError(f'owned Dolphin PID {pid} still present after cleanup; do not signal an unverified PID')
    phase.update(status='complete', result=result,
                 completed_utc=time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()))
    write_json(case / 'result.json', result)
    write_json(root / 'receipt.json', receipt)
    state(root, f'{label}:complete; owned Dolphin gone', case)
    return result


def accepted_prefix(path, tools, profile_hashes, boot, host, full_source=None):
    path=Path(path).expanduser().resolve()
    if (Path.home() / 'MacioGalaxy2-gamedata') not in path.parents:
        raise RuntimeError('accepted prefix must stay private')
    result=json.loads(path.read_text())
    manifest=json.loads((path.parent / 'capture/manifest.json').read_text())
    producer=json.loads((path.parent.parent / 'receipt.json').read_text())
    if (result.get('status')!='verified diagnostic only' or result.get('suffix_reads')!=4800 or
        manifest.get('status')!='captured; repeat qualification required' or manifest.get('frames')!=1201 or
        manifest.get('raw_input_suffix',{}).get('connection_policy')!='observed read-only probe'):
        raise RuntimeError('prefix is not a finalized observed-connection acceptance')
    if producer.get('status')!='complete diagnostic only' or producer.get('host')!=host:
        raise RuntimeError('accepted prefix must be complete on this host; otherwise repeat acceptance')
    if producer['source_profile_files']!=profile_hashes:
        raise RuntimeError('accepted prefix used a different source profile; repeat acceptance')
    helpers=('dolphin_collect.py','dolphin_stage_entry.py','kpad_script.py','kpad_records.py','kpad_observe.py','stage_clear_witness.py')
    for name in helpers:
        if producer['source_files'].get(name)!=digest(tools/name):
            raise RuntimeError('runtime helper differs from accepted prefix: '+name)
    if manifest['input_script_sha256']!=digest(boot) or manifest['input_contract_sha256']!=digest(tools.parents[2]/'native/tools/headless.py'):
        raise RuntimeError('boot script or strict input contract changed; repeat acceptance')
    capture=path.parent/'capture'
    require_hash(capture/'oracle.csv',result['oracle_sha256'])
    require_hash(capture/'inputs.jsonl',result['input_export_sha256'])
    require_hash(Path(result['delivered_input']['path']),result['delivered_input']['sha256'])
    require_hash(Path(result['delivered_input']['kpd_path']),result['delivered_input']['kpd_sha256'])
    source=Path(manifest['raw_input_suffix']['path']).resolve()
    if (Path.home()/'MacioGalaxy2-gamedata') not in source.parents:
        raise RuntimeError('accepted source prefix left private storage')
    require_hash(source,manifest['raw_input_suffix']['sha256'])
    origin=manifest['sync']['source_frame']
    if result['source_origin']!=origin or manifest['sync'].get('rng_forced') is not False:
        raise RuntimeError('accepted source origin/seed policy changed')
    actual=(row for row in contract.rows_from_jsonl(capture/'inputs.jsonl') if row['frame']>origin)
    delivered=iter(contract.rows_from_jsonl(Path(result['delivered_input']['path'])))
    full=iter(contract.rows_from_jsonl(full_source)) if full_source else None
    reads=0
    try:
        for wanted in contract.rows_from_jsonl(source):
            if full is not None and next(full,None)!=wanted:
                raise RuntimeError('full input prefix differs from accepted source')
            have=next(actual,None)
            normalized=dict(have,frame=have['frame']-origin-1) if have else None
            if normalized is None or dict(normalized,connection=wanted['connection'])!=wanted or next(delivered,None)!=normalized:
                raise RuntimeError('accepted prefix sample/read/delivery evidence disagrees')
            reads+=1
        if reads!=4800 or next(actual,None) is not None or next(delivered,None) is not None:
            raise RuntimeError('accepted prefix actual read accounting is incomplete')
    finally:
        if full is not None: full.close()
        actual.close(); delivered.close()
    with (capture/'oracle.csv').open() as stream:
        rows=csv.DictReader(stream); first=None; count=0
        for index,row in enumerate(rows):
            if row['frame']!=str(index): raise RuntimeError('accepted trace is not dense')
            if first is None: first=row
            count+=1
    if (count!=1201 or first['stage']!='YosshiHomeGalaxy' or first['scenario']!='1' or first['init_state']!='6' or
        first['rng_seed']!=result['natural_ready_seed'] or first['rng_seed']!=str(manifest['sync']['rng_seed'])):
        raise RuntimeError('accepted ready trace/seed/count changed')
    projected=producer.get('full_projection_seconds')
    if type(projected) not in (int,float) or not 0<projected<=10680:
        raise RuntimeError('accepted prefix does not fit full-run guard')
    return dict(path=str(path),sha256=digest(path),projection_seconds=projected,
                source_origin=result['source_origin'],natural_ready_seed=result['natural_ready_seed'],
                mode='acceptance reused only; full capture starts from fresh boot, no state/tape stitching')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', required=True, type=Path)
    parser.add_argument('--slot-held', action='store_true', required=True)
    parser.add_argument('--tape', type=Path)
    parser.add_argument('--profile-template', type=Path)
    parser.add_argument('--boot-menu', type=Path)
    parser.add_argument('--prefix-only', action='store_true')
    parser.add_argument('--full-only', action='store_true')
    parser.add_argument('--accepted-prefix', type=Path)
    parser.add_argument('--overall-budget', type=int)
    args = parser.parse_args()
    if args.full_only and (args.prefix_only or not args.accepted_prefix): parser.error('--full-only requires accepted prefix and excludes --prefix-only')
    if args.accepted_prefix and not args.full_only: parser.error('--accepted-prefix requires --full-only')
    if args.overall_budget is not None:
        if not 60 <= args.overall_budget <= 14400: parser.error('overall budget must be 60..14400')
        signal.setitimer(signal.ITIMER_REAL, args.overall_budget)
    deadline = time.monotonic() + args.overall_budget if args.overall_budget else None
    root = args.root.expanduser().resolve()
    private = Path.home() / 'MacioGalaxy2-gamedata'
    if private not in root.parents or root.exists():
        parser.error('use a fresh private output directory')
    root.mkdir(parents=True)
    os.chmod(root, 0o700)
    state(root, 'preflight hashing', root, os.getpid())
    receipt = dict(schema_version=1, status='incomplete', host=socket.gethostname(), ui_mode='visible',
                   nice=os.getpriority(os.PRIO_PROCESS, 0), qualification='BLOCKED', measured_qualified_frames=None,
                   approval='PORT thr_brg5rhzvas unattended full Yoshi replay on M1',
                   anchor=522, anchor_method='509+13 proxy intro-event alignment; exact GX-ready/raw KPAD UNKNOWN',
                   input_history='pinned eee95167 samples/history; actual read-only probe in export; no clamp or seed force', phases=[])
    try:
        tools = Path(__file__).resolve().parent
        receipt['source_files'] = {str(path.relative_to(tools)): digest(path) for path in sorted(tools.glob('*.py'))}
        receipt['dolphin'] = str(private / 'Dolphin-2609a.app/Contents/MacOS/Dolphin')
        receipt['disc'] = str(private / 'SB4E01.iso')
        require_hash(receipt['dolphin'], DOLPHIN_SHA)
        require_hash(receipt['disc'], ISO_SHA)
        receipt['disc_sha256'] = ISO_SHA
        receipt['dol'] = str(private / 'SB4E01-extracted/sys/main.dol')
        receipt['dol_sha256'] = digest(receipt['dol'])
        source = (args.tape or private / 'solp-replay-stage1/converted-anchor522-r1/inputs.jsonl').expanduser().resolve()
        if private not in source.parents: raise RuntimeError('input must stay private')
        require_hash(source, TAPE_SHA)
        require_hash(source.with_name('inputs.kpd'), KPD_SHA)
        validation = root / 'input-validation'
        validation.mkdir()
        validated = RawSuffix(source, TAPE_SHA, 30000, validation, contract)
        try:
            if validated.receipt['kpd_sha256'] != KPD_SHA:
                raise RuntimeError('strict contract re-encoding differs from pinned converter KPD')
            receipt['input_preflight'] = dict(status='validated source only; not delivery evidence',
                                              jsonl_sha256=TAPE_SHA, kpd_sha256=KPD_SHA)
        finally:
            validated.close(False)
        profile = (args.profile_template or private / 'sold/human01/candidate-r1-profile').expanduser().resolve()
        if private not in profile.parents: raise RuntimeError('profile must stay private')
        profile_hashes = {str(path.relative_to(profile)): digest(path) for path in sorted(profile.rglob('*')) if path.is_file()}
        receipt['source_profile_files'] = profile_hashes
        boot = (args.boot_menu or private / 'sold/human01/candidate-r1-dolphin/input-script.json').expanduser().resolve()
        if private not in boot.parents: raise RuntimeError('menu must stay private')
        shutil.copyfile(boot, root / 'boot-menu.json')
        if args.full_only:
            receipt['accepted_prefix']=accepted_prefix(args.accepted_prefix,tools,profile_hashes,boot,socket.gethostname(),source)
            phases=[('full-30000',30000,10800)]
        else:
            phases = [('selector-16', 16, 1500), ('prefix-1200', 1200, 1500)]
            if not args.prefix_only: phases.append(('full-30000', 30000, 10800))
        for label, count, budget in phases:
            if deadline:
                budget = min(budget, int(deadline - time.monotonic()) - 30)
                if budget < 1: raise RuntimeError('overall budget exhausted before next phase')
            tape = source
            if count != 30000:
                tape = root / f'{label}.jsonl'
                prefix(source, tape, count)
            result = run_case(root, tools, profile, tape, count, label, budget, receipt)
            if count == 1200:
                projected = (result['collection_elapsed_seconds'] - result['suffix_elapsed_seconds'] +
                             result['suffix_elapsed_seconds'] / count * 30000)
                receipt['full_projection_seconds'] = projected
                write_json(root / 'receipt.json', receipt)
                if not args.prefix_only and projected > 10680:
                    raise RuntimeError(f'prefix projects {projected:.1f}s full run beyond 10800s guard; full not started')
            require_hash(source, TAPE_SHA)
            if profile_hashes != {str(path.relative_to(profile)): digest(path) for path in sorted(profile.rglob('*')) if path.is_file()}:
                raise RuntimeError('source profile changed')
        receipt['status'] = 'complete diagnostic only'
    except BaseException as exc:
        receipt['error'] = f'{type(exc).__name__}: {exc}'
        state(root, 'failed; no partial output accepted', root)
        raise
    finally:
        write_json(root / 'receipt.json', receipt)
    signal.setitimer(signal.ITIMER_REAL, 0)
    state(root, 'requested captures complete; analysis pending', root)
    return 0


if __name__ == '__main__':
    def interrupted(number, frame):
        raise RuntimeError(f'replay interrupted by {signal.Signals(number).name}')
    signal.signal(signal.SIGTERM, interrupted)
    signal.signal(signal.SIGINT, interrupted)
    signal.signal(signal.SIGALRM, interrupted)
    sys.exit(main())
