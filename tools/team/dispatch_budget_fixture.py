"""Exact disposable leader/worker fixture ownership; SOURCE ONLY pending review.

Not a generic compiler/descendant ownership API. No signal mechanism is added.
"""
import json
import os

import dispatch_budget as budget
from dispatch_budget_fixture_child import publish


class WorkerFixtureJob(budget.LiveBudgetJob):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.worker_state = 'MISSING'
        self.worker_identity = None

    def observe_fixture(self):
        super().observe_fixture()
        if not self.groups:
            return
        if self.worker_state in ('CONTRADICTED', 'OBSERVATION_FAILED'):
            raise RuntimeError('worker observation failure is sticky')
        root = self.fixture_identity_path.parent
        identity_path = root / 'worker-identity.json'
        lineage_path = root / 'lineage.json'
        if not identity_path.exists() or not lineage_path.exists():
            if self.worker_state == 'VERIFIED':
                self.worker_state = 'OBSERVATION_FAILED'
                raise RuntimeError('verified worker evidence disappeared')
            return
        try:
            if len(self.groups) != 1:
                raise RuntimeError('fixture requires exactly one registered leader')
            leader = next(iter(self.groups))
            if self.ownership.get(leader, {}).get('status') != 'VERIFIED':
                raise RuntimeError('worker cannot verify before leader')
            worker = json.loads(identity_path.read_text())
            lineage = json.loads(lineage_path.read_text())
            if set(worker) != {'pid', 'pgid', 'sid', 'uid', 'ppid'} or not all(type(v) is int for v in worker.values()):
                raise RuntimeError('invalid worker identity')
            if set(lineage) != {'leader_pid', 'worker_pid'} or not all(type(v) is int for v in lineage.values()):
                raise RuntimeError('invalid direct-parent lineage')
            if self.worker_identity is not None:
                if worker != self.worker_identity or lineage != self.worker_lineage:
                    raise RuntimeError('immutable worker identity/lineage changed')
                return
            if (worker['pid'] <= 0 or worker['pid'] == leader or
                    lineage != {'leader_pid': leader, 'worker_pid': worker['pid']} or
                    worker['ppid'] != leader or worker['pgid'] != leader or worker['sid'] != leader or
                    worker['uid'] != self.identities[leader]['uid'] or worker['uid'] != os.getuid()):
                self.worker_state = 'CONTRADICTED'
                raise RuntimeError('worker report does not belong to registered leader')
            observed = {
                'pgid': self.operation('getpgid-worker-observe', worker['pid'], lambda: os.getpgid(worker['pid'])),
                'sid': self.operation('getsid-worker-observe', worker['pid'], lambda: os.getsid(worker['pid'])),
            }
            if observed != {'pgid': leader, 'sid': leader}:
                self.worker_state = 'CONTRADICTED'
                raise RuntimeError('worker kernel group/session mismatch')
            self.worker_identity = dict(worker)
            self.worker_lineage = dict(lineage)
            self.worker_state = 'VERIFIED'
            self.journal({'event': 'actual_worker_identity', 'worker_identity': worker,
                          'observed': observed, 'lineage': lineage,
                          'scope': 'child self-report PPID/UID plus leader Popen PID and kernel PGID/SID'})
            publish(root / 'worker-verified.json', dict(worker_pid=worker['pid'], leader_pid=leader))
        except BaseException as error:
            if self.worker_state != 'CONTRADICTED':
                self.worker_state = 'OBSERVATION_FAILED'
            self.journal({'event': 'worker_observation_failed', 'status': self.worker_state,
                          'error': repr(error)})
            raise

    def require_signal_ownership(self, pid):
        super().require_signal_ownership(pid)
        if self.worker_state != 'VERIFIED':
            self.journal({'event': 'signal_blocked_unverified_worker', 'pid': pid,
                          'worker_status': self.worker_state})
            raise RuntimeError('worker ownership not verified; no group probe/signal')
