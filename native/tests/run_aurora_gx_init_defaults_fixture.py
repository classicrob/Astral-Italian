#!/usr/bin/env python3
"""Run only the four pinned GXInit fixture steps after external allocation.

All deadlines/termination belong to the existing production bounded_run.py.
No app, build system, signal handler, retry or mutable command discovery.
"""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2) + '\n')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--plan', type=Path, required=True)
    p.add_argument('--plan-sha256', required=True)
    a = p.parse_args()
    assert sha(a.plan) == a.plan_sha256
    plan = json.loads(a.plan.read_text())
    assert plan['kind'] == 'EXACT_GXINIT_DEFAULTS_FOUR_STEP_FIXTURE'
    assert [s['id'] for s in plan['steps']] == ['compile-old', 'compile-candidate', 'old-negative', 'candidate']
    assert [s['expected_exit'] for s in plan['steps']] == [0, 0, 17, 0]
    assert [s['seconds'] for s in plan['steps']] == [90, 90, 5, 5]
    assert plan['outer_seconds'] == 235 and plan['allocation_ceiling_seconds'] == 240
    output = Path(plan['execution_output'])
    assert not output.exists(), 'one fresh attempt only'
    output.mkdir(mode=0o700)
    state_path = Path(plan['state'])
    def state(phase, child='none'):
        state_path.write_text('UTC=' + datetime.datetime.now(datetime.timezone.utc).isoformat() +
            ' phase=' + phase + ' inputs=' + a.plan_sha256 + ' output=' + str(output) +
            ' controllerPID=' + str(os.getpid()) + ' childPID=' + str(child) +
            ' resume=closed-receipt-only;no-retry\n')
    state('PRE-LOCK')
    rows = []
    with Path(plan['native_lock']).open('a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        os.setpriority(os.PRIO_PROCESS, 0, 19)
        try:
            for pin in plan['input_pins']:
                assert sha(pin['path']) == pin['sha256'], pin['path']
            for step in plan['steps']:
                command = [sys.executable, plan['bounded_runner'], str(step['seconds']), *step['argv']]
                state('PRE-SPAWN-' + step['id'])
                with (output / (step['id'] + '.log')).open('xb') as log:
                    child = subprocess.Popen(command, cwd=plan['cwd'], stdout=log, stderr=subprocess.STDOUT)
                    state('RUNNING-' + step['id'], child.pid)
                    rc = child.wait()
                state('COMPLETED-' + step['id'])
                rows.append({'step': step['id'], 'exit_code': rc, 'expected_exit': step['expected_exit']})
                write(output / 'steps.json', rows)
                if rc != step['expected_exit']:
                    raise SystemExit('fixture step failed; preserve output; no retry')
            dependencies = {}
            for arm in ('old', 'candidate'):
                depfile = output / (arm + '.d')
                text = depfile.read_text().replace('\\\n', ' ')
                target, rest = text.split(':', 1)
                assert shlex.split(target) == [str(output / arm)]
                deps = shlex.split(rest)
                dependencies[arm] = [{'path': str(Path(d).resolve()), 'sha256': sha(d)} for d in deps]
            for pin in plan['input_pins']:
                assert sha(pin['path']) == pin['sha256'], 'input changed: ' + pin['path']
            write(output / 'dependencies.json', dependencies)
            write(output / 'receipt.json', {'status': 'FOCUSED_CONTRACT_PASS_OLD_NEGATIVE_REJECTED',
                'plan_sha256': a.plan_sha256, 'steps': rows,
                'binaries': {arm: sha(output / arm) for arm in ('old', 'candidate')},
                'dependencies_sha256': sha(output / 'dependencies.json'),
                'limits': 'Extracted actual VAT contract fragments/setters/flush/inline writer; not full GXInit, parser or app acceptance; no generic runner qualification'})
        finally:
            fcntl.flock(lock, fcntl.LOCK_UN)
            state('RELEASED-LOCK')


if __name__ == '__main__':
    main()
