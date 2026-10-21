#!/usr/bin/env python3
"""Private-data acceptance: run a strict tape twice and compare every trace byte.

Invoke under native.lock/nice; this is deliberately not a retail-data CTest.
Both runs retain every reload/loading row. Inputs and natural seeds are unchanged.
"""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import signal
import socket
import subprocess
import time


def digest(path):
    result = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            result.update(chunk)
    return result.hexdigest()


def first_difference(left, right):
    with left.open(newline='') as a, right.open(newline='') as b:
        ar, br = csv.DictReader(a), csv.DictReader(b)
        if ar.fieldnames != br.fieldnames:
            return {'error': 'different headers'}
        for index, (x, y) in enumerate(zip(ar, br)):
            fields = {key: {'run1': x[key], 'run2': y[key]} for key in x if x[key] != y[key]}
            if fields:
                return {'row': index, 'frame': x['frame'], 'fields': fields}
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('executable', 'disc', 'inputs', 'out'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--stage', default='YosshiHomeGalaxy')
    parser.add_argument('--frames', type=int, default=30000)
    parser.add_argument('--timeout', type=int, default=540)
    parser.add_argument('--asan-options')
    parser.add_argument('--state-file', type=Path)
    args = parser.parse_args()
    if not 1 <= args.timeout <= 540 or args.frames < 1:
        parser.error('timeout must be 1..540 and frames positive')
    root = Path(__file__).resolve().parents[2]
    for path in (args.disc, args.inputs, args.out):
        if path.resolve() == root or root in path.resolve().parents:
            parser.error('all game data and outputs must remain outside the checkout')
    args.out.mkdir(mode=0o700, parents=True, exist_ok=False)
    executable = args.executable.resolve()
    binary_sha = digest(executable)
    receipt = {'schema_version': 1, 'host': socket.gethostname(), 'executable': str(executable),
               'executable_sha256': binary_sha, 'input_sha256': digest(args.inputs),
               'frames': args.frames, 'stage': args.stage, 'nice': os.getpriority(os.PRIO_PROCESS, 0),
               'status': 'prepared', 'qualification': 'DIAGNOSTIC_ONLY', 'runs': []}
    child = None

    def persist():
        (args.out / 'repeat.json').write_text(json.dumps(receipt, indent=2) + '\n')
        if args.state_file:
            args.state_file.write_text(f"HOST {receipt['host']} | SOURCE virtualasync candidate | BINARY {binary_sha} | "
                                      f"PHASE Yoshi reloadrepeat {receipt['status']} nice19 native.lock | OUTPUT {args.out} | "
                                      f"OWNED PID runner{os.getpid()} child{child.pid if child else None} | "
                                      "RESUME preserve failures; monitor USERINCOMING; no serverwindows\n")

    def stop_child():
        if child is None or child.poll() is not None:
            return
        os.killpg(child.pid, signal.SIGTERM)
        try:
            child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            os.killpg(child.pid, signal.SIGKILL)
            child.wait()

    def interrupted(signum, _frame):
        stop_child()
        receipt['status'] = 'interrupted'
        persist()
        raise SystemExit(128 + signum)

    for sig in (signal.SIGINT, signal.SIGTERM):
        signal.signal(sig, interrupted)
    persist()
    try:
        for number in (1, 2):
            if digest(executable) != binary_sha:
                raise ValueError('binary changed between runs')
            case = args.out / f'run{number}'
            case.mkdir(mode=0o700)
            trace = case / 'native.csv'
            command = [str(executable), '--disc', str(args.disc.resolve()), '--user', str(case / 'user'),
                       '--inputs', str(args.inputs.resolve()), '--trace', str(trace), '--epoch-ticks', '0',
                       '--stage', args.stage, '--scenario', '1', '--sync-stage', args.stage,
                       '--sync-scenario', '1', '--inputs-from-sync']
            env = {key: value for key, value in os.environ.items()
                   if not key.startswith(('PETARI_', 'ASTRAL_'))}
            env.update(PETARI_MODS='none', PETARI_TRACE_BOOT='1', PETARI_CACHE_DIR=str(case / 'cache'), LC_ALL='C', LANG='C')
            if args.asan_options is not None:
                env['ASAN_OPTIONS'] = args.asan_options
            run = {'number': number, 'command': command, 'start': time.time(),
                   'ASAN_OPTIONS': env.get('ASAN_OPTIONS'), 'status': 'running'}
            receipt['runs'].append(run)
            with (case / 'native.log').open('x') as log:
                child = subprocess.Popen(command, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                run['pid'] = child.pid
                receipt['status'] = f'run{number}-running'
                persist()
                try:
                    code = child.wait(timeout=args.timeout)
                except subprocess.TimeoutExpired:
                    stop_child()
                    raise ValueError(f'run{number} exceeded {args.timeout}s')
            run.update(exit_code=code, elapsed=time.time() - run['start'])
            if code != 0:
                raise ValueError(f'run{number} exited {code}')
            if Path(str(trace) + '.complete').read_text() != str(args.frames + 1) + '\n':
                raise ValueError('missing/invalid completion count')
            with trace.open(newline='') as stream:
                reader = csv.DictReader(stream)
                count = 0
                for count, row in enumerate(reader, 1):
                    if row['frame'] != str(count - 1):
                        raise ValueError('noncontiguous trace')
                if count != args.frames + 1:
                    raise ValueError('incorrect trace length')
            origin = json.loads(Path(str(trace) + '.sync.json').read_text())
            run.update(status='complete', rows=count, trace_sha256=digest(trace), sync_origin=origin,
                       timing_sha256=digest(Path(str(trace) + '.timing.csv')))
            child = None
            persist()
        receipt['byte_identical'] = receipt['runs'][0]['trace_sha256'] == receipt['runs'][1]['trace_sha256']
        receipt['timing_byte_identical'] = receipt['runs'][0]['timing_sha256'] == receipt['runs'][1]['timing_sha256']
        receipt['first_difference'] = first_difference(args.out / 'run1/native.csv', args.out / 'run2/native.csv')
        receipt['status'] = 'repeatable' if receipt['byte_identical'] else 'diverged'
        persist()
        return 0 if receipt['byte_identical'] else 1
    except (OSError, ValueError) as exc:
        stop_child()
        receipt.update(status='failed', error=str(exc))
        persist()
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
