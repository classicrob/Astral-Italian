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
    actual = (row for row in contract.rows_from_jsonl(case / 'capture/inputs.jsonl') if row['frame'] > origin)
    expected = iter(contract.rows_from_jsonl(source))
    reads = 0
    for row in actual:
        normalized = dict(row, frame=row['frame'] - origin - 1)
        if next(expected, None) != normalized:
            raise RuntimeError(f'exported suffix differs from pinned input at {normalized["frame"]}:{normalized["read_ordinal"]}')
        reads += 1
    if next(expected, None) is not None:
        raise RuntimeError('exported suffix is truncated')
    with (case / 'capture/oracle.csv').open() as stream:
        rows = csv.DictReader(stream)
        first = next(rows)
        if first['stage'] != 'YosshiHomeGalaxy' or first['scenario'] != '1' or first['init_state'] != '6':
            raise RuntimeError('wrong accepted ready target')
    return dict(status='verified diagnostic only', qualification='BLOCKED', measured_qualified_frames=None,
                suffix_reads=reads, natural_ready_seed=first['rng_seed'], source_origin=origin,
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
               '--sync-input-records-sha256', digest(source), '--frames', str(frames + 1),
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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', required=True, type=Path)
    parser.add_argument('--slot-held', action='store_true', required=True)
    args = parser.parse_args()
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
                   input_history='pinned eee95167 conversion; no clamp, rebase of history or seed force', phases=[])
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
        source = private / 'solp-replay-stage1/converted-anchor522-r1/inputs.jsonl'
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
        profile = private / 'sold/human01/candidate-r1-profile'
        profile_hashes = {str(path.relative_to(profile)): digest(path) for path in sorted(profile.rglob('*')) if path.is_file()}
        receipt['source_profile_files'] = profile_hashes
        shutil.copyfile(private / 'sold/human01/candidate-r1-dolphin/input-script.json', root / 'boot-menu.json')
        for label, count, budget in [('selector-16', 16, 1500), ('prefix-1200', 1200, 1500), ('full-30000', 30000, 10800)]:
            tape = source
            if count != 30000:
                tape = root / f'{label}.jsonl'
                prefix(source, tape, count)
            run_case(root, tools, profile, tape, count, label, budget, receipt)
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
    state(root, 'all captures complete; analysis pending', root)
    return 0


if __name__ == '__main__':
    def interrupted(number, frame):
        raise RuntimeError(f'replay interrupted by {signal.Signals(number).name}')
    signal.signal(signal.SIGTERM, interrupted)
    signal.signal(signal.SIGINT, interrupted)
    sys.exit(main())
