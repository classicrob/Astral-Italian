"""DISABLED full-context integration of accepted disposable lifecycle component.

No generic compiler ownership gate is implemented/qualified here. run() refuses
before spawning. This file is a review candidate, not execution authorization.
"""
import os
import json
import hashlib
import sys
import stat
from pathlib import Path
import shutil
import time

import dispatch_budget as accepted
import dispatch_exec_gate as gate

SCAN_ENTRIES = 200000
SCAN_SECONDS = 2.0
GENERIC_OWNERSHIP_STATUS = 'SOURCE_IMPLEMENTED_ACTUAL_CONTEXT_UNQUALIFIED'
FULL_ROOT = Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-full-v9-r2'
FIXED_PATH = '/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin'


def launch_context(root):
    """Data only: pin exact resolution; do not create paths or run commands."""
    targets = {}
    for name in ('git', 'ninja', sys.executable):
        found = shutil.which(name, path=FIXED_PATH)
        if found is None:
            raise RuntimeError('missing chosen launch tool: '+name)
        path = Path(found).resolve()
        targets[name] = {'path':str(path), 'sha256':gate.digest(path)}
    runtime = {}
    for name in ('python3', '/usr/bin/env', '/bin/sh'):
        found = shutil.which(name, path=FIXED_PATH)
        if found is None:
            raise RuntimeError('missing shell/shebang resolution: '+name)
        runtime[name] = {'path':found,'resolved':str(Path(found).resolve()),'sha256':gate.digest(found)}
    return {'python':str(Path(sys.executable).resolve()),
            'python_sha256':gate.digest(sys.executable),
            'gate':str(Path(gate.__file__).resolve()), 'gate_sha256':gate.digest(gate.__file__),
            'targets':targets, 'runtime':runtime,
            'env':{'HOME':str(Path.home()), 'PATH':FIXED_PATH, 'LANG':'C', 'LC_ALL':'C',
                   'TMPDIR':str(Path(root)/'tmp'), 'PYTHONCOERCECLOCALE':'0',
                   'PYTHONUTF8':'1', 'PYTHONNOUSERSITE':'1'}}


def bounded_allocated(root, *, entries=SCAN_ENTRIES, seconds=SCAN_SECONDS):
    """Finite traversal, no symlink following. Syscall latency remains sampled."""
    start = time.monotonic()
    count = 0
    total = 0
    todo = [Path(root)]
    if not stat.S_ISDIR(Path(root).lstat().st_mode):
        raise accepted.DiskBudgetExceeded('allocation root is not a real directory')
    while todo:
        if time.monotonic()-start >= seconds:
            raise accepted.DiskBudgetExceeded('allocation scan time bound')
        directory = todo.pop()
        try:
            with os.scandir(directory) as items:
                for item in items:
                    count += 1
                    if count > entries or time.monotonic()-start >= seconds:
                        raise accepted.DiskBudgetExceeded('allocation scan entry/time bound')
                    try:
                        entry_stat = item.stat(follow_symlinks=False)
                        if item.is_symlink():
                            continue
                        total += entry_stat.st_blocks*512
                        if item.is_dir(follow_symlinks=False):
                            todo.append(Path(item.path))
                    except FileNotFoundError:
                        continue
        except FileNotFoundError:
            if directory == Path(root):
                raise
            continue
    return total


class FullBudgetJob(accepted.LiveBudgetJob):
    def __init__(self, *args, launch=None, **kwargs):
        super().__init__(*args, **kwargs)
        self.launch = launch
        self.launch_number = 0
        self.active_launch = None
        self.launches = []
        self.in_cleanup = False

    def __enter__(self):
        if self.baseline is None:
            self.baseline = bounded_allocated(self.root)
        return super().__enter__()

    def check(self):
        self.remaining()
        self.observe_fixture()
        start = time.monotonic()
        free = shutil.disk_usage(self.root).free
        try:
            size = bounded_allocated(self.root)
        except accepted.DiskBudgetExceeded as error:
            self.failure = str(error)
            raise
        self.remaining()
        elapsed = time.monotonic()-start
        self.max_scan_seconds = max(self.max_scan_seconds, elapsed)
        if elapsed > SCAN_SECONDS:
            self.failure = 'combined disk/scan sampled time bound'
            raise accepted.DiskBudgetExceeded(self.failure)
        growth = max(0, size-self.baseline)
        self.samples += 1
        self.peak_growth = max(self.peak_growth, growth)
        self.lowest_free = free if self.lowest_free is None else min(self.lowest_free, free)
        if free < self.min_free:
            self.failure = 'free-space floor crossed'
            raise accepted.DiskBudgetExceeded(self.failure)
        if growth > self.max_growth:
            self.failure = 'owned allocated-byte growth crossed'
            raise accepted.DiskBudgetExceeded(self.failure)

    def observe_fixture(self):
        if self.active_launch is None:
            if self.groups:
                raise RuntimeError('owned group without a registered launch request')
            return
        if not self.groups:
            return
        if self.fixture_identity_path.is_symlink():
            for pid in self.groups:
                self.ownership_state(pid, 'CONTRADICTED', reason='launch identity symlink')
            raise RuntimeError('launch identity symlink')
        super().observe_fixture()
        if len(self.groups) != 1:
            raise RuntimeError('serial proof launcher requires exactly one group')
        pid = next(iter(self.groups))
        if self.ownership.get(pid, {}).get('status') != 'VERIFIED':
            return  # Unverified gate self-expires; no acknowledgement or signals.
        row = self.active_launch
        try:
            if self.identities[pid]['uid'] != os.getuid():
                raise RuntimeError('launch actual UID differs from controller')
            report = row['directory']/'target.json'
            if not report.exists():
                if row['acknowledged']:
                    raise RuntimeError('acknowledged launch target evidence disappeared')
                return
            expected = {'pid':pid, 'request_sha256':row['pin'],
                        'target':row['request']['target'],
                        'target_sha256':row['request']['target_sha256'],
                        'cwd':row['request']['cwd']}
            if report.is_symlink() or json.loads(report.read_text()) != expected:
                raise RuntimeError('actual gate target/request mismatch')
            if (row['directory']/'request.json').is_symlink() or gate.digest(row['directory']/'request.json') != row['pin']:
                raise RuntimeError('parent launch request drift')
            if not row['acknowledged'] and not self.in_cleanup:
                self.remaining()
                gate.publish(row['directory']/'ack.json', {'pid':pid,'request_sha256':row['pin']})
                row['acknowledged'] = True
                self.journal({'event':'launch_acknowledged','pid':pid,
                              'request_sha256':row['pin'],'target':expected['target'],
                              'actual_identity':self.identities[pid]})
        except (accepted.DiskBudgetExceeded, accepted.ProofInterrupted):
            raise
        except BaseException as error:
            self.ownership_state(pid, 'CONTRADICTED', reason=repr(error))
            raise

    def run(self, argv, cwd=None, log='commands.log'):
        if self.launch is None:
            raise RuntimeError('generic compiler launch ownership remains UNQUALIFIED')
        if self.groups or self.active_launch is not None:
            raise RuntimeError('overlapping top-level proof commands unsupported')
        if dict(os.environ) != self.launch['env']:
            raise RuntimeError('parent execution environment differs from pinned context')
        if launch_context(self.root) != self.launch:
            raise RuntimeError('launch/shell/shebang target bytes or resolution drift')
        if argv[0] not in self.launch['targets']:
            raise RuntimeError('unsupported proof command target')
        target = self.launch['targets'][argv[0]]
        self.launch_number += 1
        if self.launch_number > 128:
            raise RuntimeError('bounded proof launch inventory exceeded')
        directory = self.work/('launch-%04d'%self.launch_number)
        directory.mkdir(mode=0o700)
        request = {'nonce':os.urandom(16).hex(),'argv':list(argv),'cwd':str(Path(cwd or Path.cwd()).resolve()),
                   'env':self.launch['env'],'target':target['path'],
                   'target_sha256':target['sha256'],'gate_sha256':self.launch['gate_sha256'],
                   'python_sha256':self.launch['python_sha256']}
        gate.publish(directory/'request.json', request)
        pin = gate.digest(directory/'request.json')
        self.identities.clear();self.ownership.clear()
        self.fixture_identity_path = directory/'identity.json'
        self.active_launch = {'directory':directory,'request':request,'pin':pin,'acknowledged':False}
        row = self.active_launch
        try:
            # Same accepted OwnedJob.run atomic spawn/register/mask and retirement,
            # with only gate prefix and journaled normal wait/status added.
            self.check();self.remaining()
            p = accepted.p
            with (self.work/log).open('ab') as stream:
                if p.threading.active_count() != 1:
                    raise RuntimeError('preexec mask restoration requires single thread')
                mask = p.signal.pthread_sigmask(p.signal.SIG_BLOCK,set(self.previous))
                try:
                    proc = p.subprocess.Popen([self.launch['python'],self.launch['gate'],str(directory/'request.json'),pin],
                        cwd=cwd,stdout=stream,stderr=p.subprocess.STDOUT,start_new_session=True,
                        preexec_fn=lambda:p.signal.pthread_sigmask(p.signal.SIG_SETMASK,mask))
                    self.groups[proc.pid] = proc
                    row['registered_pid'] = proc.pid
                finally:p.signal.pthread_sigmask(p.signal.SIG_SETMASK,mask)
                code = self.operation('wait-reap-normal',proc.pid,lambda:proc.wait(timeout=self.remaining()))
                row['tool_exit_status'] = code
                if code:raise p.subprocess.CalledProcessError(code,argv)
                if self.alive(proc.pid):raise RuntimeError('leader exited with surviving owned group')
                del self.groups[proc.pid]
                self.events.append({'pgid':proc.pid,'leader_reaped_and_group_absent':True})
            self.remaining()
        finally:
            pid = row.get('registered_pid')
            self.launches.append({'request_sha256':pin,'request':request,
                'registered_pid':pid,'acknowledged':row['acknowledged'],
                'identity':self.identities.get(pid),'ownership':self.ownership.get(pid),
                'tool_exit_status':row.get('tool_exit_status'),
                'retired':pid is not None and pid not in self.groups})
            # On failure cleanup still needs this exact request/identity state.
            if not self.groups:
                self.active_launch = None
                self.fixture_identity_path = None

    def __exit__(self, typ, value, tb):
        self.in_cleanup = True  # May verify/stop gate, but never admit new exec.
        try:
            return super().__exit__(typ,value,tb)
        finally:
            # Bounded final provenance, after accepted cleanup. Completion hashes
            # these final bytes; no status/error is replaced with a success.
            path = self.work/'disk-budget.json'
            if path.exists():
                report = json.loads(path.read_text())
                journal = self.work/'syscall-journal.jsonl'
                if journal.exists() and (not stat.S_ISREG(journal.lstat().st_mode) or journal.stat().st_size>8*1024**2):
                    raise RuntimeError('journal is not bounded regular metadata')
                raw = journal.read_bytes() if journal.exists() else b''
                if len(raw)>8*1024**2:raise RuntimeError('bounded journal metadata exceeded')
                report['launch_provenance'] = {'launches':self.launches,
                    'journal_sha256':hashlib.sha256(raw).hexdigest(),
                    'journal':[json.loads(line) for line in raw.splitlines()]}
                accepted.p.dump(path,report)
