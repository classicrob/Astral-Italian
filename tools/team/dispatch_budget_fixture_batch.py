"""One source-reviewed disposable diagnostic batch, never a full-proof runner.

Inert without explicit plan pin. SOURCE ONLY until an exact new allocation.
Deadlines are enforced by fixture timers and per-case checks; filesystem/syscall
latency is not an externally guaranteed hard wall-clock bound.
"""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import sys
import time
import traceback
import unittest
from unittest.mock import patch

import dispatch_budget as budget
import test_dispatch_budget as tests

NOW = time.monotonic  # Keep real elapsed time even inside no-process model mocks.
MIB = 1024 * 1024
CASES = [
    'OwnershipSourceModels', 'LeaderReapSourceModels', 'WorkerOwnershipSourceModels',
    'LiveControls.test_growth_crossing_stops_real_python_child',
    'LiveControls.test_synthetic_floor_crossing_stops_real_python_child',
    'RealLeaderWorkerControls.test_actual_term_ignored_leader',
    'RealLeaderWorkerControls.test_actual_leader_exit_surviving_worker',
    'LiveControls.test_python_work_crossing',
    'LiveControls.test_packaging_continues_original_baseline',
]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plan', type=Path, required=True)
    parser.add_argument('--execute-plan-sha256', required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    plan_bytes = args.plan.read_bytes()
    if hashlib.sha256(plan_bytes).hexdigest() != args.execute_plan_sha256:
        raise RuntimeError('plan pin mismatch')
    plan = json.loads(plan_bytes)
    if plan['cases'] != CASES or plan['total_seconds'] != 90 or plan['output_bytes'] != 64*MIB:
        raise RuntimeError('allocation scope/caps mismatch')
    for name, pin in plan['source_files'].items():
        if Path(name).name != name:
            raise RuntimeError('unconfined source name')
        if hashlib.sha256(Path(__file__).with_name(name).read_bytes()).hexdigest() != pin:
            raise RuntimeError('source pin mismatch: '+name)
    if (str(Path(sys.executable).resolve()) != plan['python']['executable'] or
            hashlib.sha256(Path(sys.executable).read_bytes()).hexdigest() != plan['python']['sha256'] or
            sys.version != plan['python']['version']):
        raise RuntimeError('Python interpreter/context drift')
    base = Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-budget-source-controls'
    if args.output.parent.resolve() != base.resolve() or args.output.exists():
        raise RuntimeError('fresh owned output required')
    if os.getpriority(os.PRIO_PROCESS, 0) < 19:
        raise RuntimeError('nice19 required')
    if budget.shutil.disk_usage(base).free < 50*1024**3:
        raise RuntimeError('50GiB floor before tiny diagnostic')
    lock = (Path.home()/'MacioGalaxy2-team/slots/native.lock').open('a')
    fcntl.lockf(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    start = NOW()
    args.output.mkdir(mode=0o700)
    fixtures = args.output/'fixtures'
    fixtures.mkdir(mode=0o700)
    (args.output/'invocation.json').write_text(json.dumps({
        'argv': sys.argv, 'pid': os.getpid(), 'plan_sha256': args.execute_plan_sha256,
        'source_files': plan['source_files'], 'start_monotonic': start,
        'caps': {'total_seconds':90, 'output_bytes':64*MIB},
        'semantics':'sampled checks and bounded fixtures; no syscall-latency guarantee',
    }, indent=2)+'\n')
    os.environ['SOLP_BUDGET_FIXTURE_ROOT'] = str(fixtures)
    os.umask(0o077)
    stream = (args.output/'stdout.log').open('x')
    result = None
    execution_error = None
    original_remaining = budget.LiveBudgetJob.remaining

    def remaining(job):
        own = original_remaining(job)
        left = 86 - (NOW()-start)  # Reserve existing four-second cleanup grace.
        if left <= 0:
            raise budget.DiskBudgetExceeded('diagnostic batch work deadline')
        return min(own, left)

    class CappedResult(unittest.TextTestResult):
        def startTest(self, test):
            if NOW()-start >= 75 or budget.allocated_bytes(args.output) >= 64*MIB:
                # Reserve up to ten seconds for an unsignalled fixture's expiry.
                raise RuntimeError('batch admission time/output cap')
            super().startTest(test)

    try:
        suite = unittest.TestSuite(unittest.defaultTestLoader.loadTestsFromName(name, tests) for name in CASES)
        with patch.object(budget.LiveBudgetJob, 'remaining', remaining), patch.object(sys, 'stderr', stream), patch.object(sys, 'stdout', stream):
            result = unittest.TextTestRunner(stream=stream, resultclass=CappedResult, failfast=True, verbosity=2).run(suite)
    except BaseException as error:
        execution_error = {'error':repr(error), 'traceback':traceback.format_exc()}
        stream.write(execution_error['traceback'])
    finally:
        try:
            stream.flush()
            # No new signals/probes on failure. Let any unconfirmed tiny fixtures
            # reach their independent ten-second self-expiry; retain failure status.
            if result is None or not result.wasSuccessful():
                until = min(start+89, NOW()+10.1)
                while NOW() < until:
                    time.sleep(min(0.05, max(0.0, until-NOW())))
            elapsed = NOW()-start
            allocated = budget.allocated_bytes(args.output)
            record = {'status':'PASS' if execution_error is None and result is not None and result.wasSuccessful() and elapsed <= 90 and allocated <= 64*MIB else 'FAIL_CLOSED',
                      'tests_run':result.testsRun if result is not None else None,
                      'seconds':elapsed, 'allocated_bytes':allocated, 'execution_error':execution_error,
                      'limit':'No production/full-proof qualification; no absence inferred from natural expiry.',
                      'actual_child_exit_status':'See journaled wait-reap per fixture; missing status stays UNKNOWN.',
                      'retention':'All fixture roots, identity, lineage, per-op and cleanup records retained.'}
            (args.output/'completion.json').write_text(json.dumps(record, indent=2)+'\n')
        finally:
            stream.close()
            fcntl.lockf(lock, fcntl.LOCK_UN)
            lock.close()
    return 0 if record['status']=='PASS' else 1


if __name__ == '__main__':
    raise SystemExit(main())
