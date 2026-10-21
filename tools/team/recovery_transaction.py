"""Durable preservation/restore transaction; execution requires an allocated caller.

No configure/build or command-line mutation entry point is supplied. Tests use
synthetic directories and a checkout adapter. A reviewed caller supplies the
exclusive lease and owns the native lock for the entire real transaction.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
from contextlib import contextmanager
import time
import uuid

from retention_paths import canonical_path
from symbol_recovery import digest


def atomic_json(path, data):
    path = Path(path)
    temp = path.with_name(path.name + '.pending-' + uuid.uuid4().hex)
    encoded = (json.dumps(data, sort_keys=True, indent=2) + '\n').encode()
    with temp.open('xb') as f:
        f.write(encoded)
        f.flush()
        os.fsync(f.fileno())
    os.replace(temp, path)
    fd = os.open(path.parent, os.O_RDONLY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


def directory_identity(path):
    path = Path(path)
    if path.is_symlink() or not path.is_dir():
        raise ValueError('expected real directory')
    s = path.stat()
    return [s.st_dev, s.st_ino]


class RecoveryTransaction:
    def __init__(self, journal, checkout, free_bytes=None, failpoint=None):
        self.path = Path(journal)
        self.checkout = checkout
        self.free_bytes = free_bytes or (lambda p: shutil.disk_usage(p).free)
        self.failpoint = failpoint or (lambda event: None)
        self.data = json.loads(self.path.read_text()) if self.path.exists() else None

    def save(self):
        atomic_json(self.path, self.data)

    def record(self, event):
        self.data['events'].append({'event': event, 'time': time.time()})
        self.save()
        self.failpoint(event)

    def prepare(self, root, expected_checkout, target_commit, lease, protected_files):
        """Snapshot before mutation; lease approval is explicit caller input."""
        if self.data is not None or self.path.exists():
            raise ValueError('journal already exists')
        root = canonical_path(root)
        output = canonical_path(self.path.parent)
        if output.is_relative_to(root) or root.is_relative_to(output):
            raise ValueError('journal must be outside source/build paths')
        if not output.is_dir():
            raise ValueError('private output directory must already exist')
        if self.checkout.read() != expected_checkout or not expected_checkout['clean']:
            raise ValueError('checkout differs from approved snapshot')
        if lease.get('status') != 'APPROVED' or lease.get('original_head') != expected_checkout['head']:
            raise ValueError('missing allocation or stale original HEAD')
        if lease.get('target_commit') != target_commit or lease.get('source_root') != str(root):
            raise ValueError('allocation target mismatch')
        if lease.get('native_lock_exclusive') is not True or lease.get('deadline', 0) <= time.time():
            raise ValueError('exclusive allocation expired or missing')
        if self.free_bytes(root) < 65 * 1024**3:
            raise ValueError('disk reserve below 65GiB')
        tag = lease['id']
        if not tag or not all(c.isalnum() or c == '-' for c in tag):
            raise ValueError('invalid allocation id')
        builds = []
        for name in ('macos-arm64-release', 'macos-arm64-headless'):
            canonical = root / 'build' / name
            protected = canonical.with_name(name + '.recovery-protected-' + tag)
            attempt = canonical.with_name(name + '.recovery-attempt-' + tag)
            if protected.exists() or attempt.exists():
                raise ValueError('preservation or attempt destination occupied')
            builds.append({'canonical': str(canonical), 'protected': str(protected),
                           'attempt': str(attempt), 'identity': directory_identity(canonical)})
        images = {}
        for path, expected in protected_files.items():
            p = canonical_path(path)
            if digest(p) != expected:
                raise ValueError('protected file hash mismatch')
            images[str(p)] = expected
        if not images:
            raise ValueError('empty protected hash closure')
        self.data = {'schema': 1, 'kind': 'FROZEN_SYMBOL_RECOVERY', 'root': str(root),
                     'original_checkout': expected_checkout, 'target_commit': target_commit,
                     'lease': lease, 'builds': builds, 'images': images,
                     'phase': 'PREPARED', 'intent': None, 'events': [],
                     'initial_free_bytes': self.free_bytes(root)}
        self.record('PREPARED')
        return self.data

    def validate(self):
        d = self.data
        if d is None or d.get('schema') != 1 or d.get('kind') != 'FROZEN_SYMBOL_RECOVERY':
            raise ValueError('invalid transaction journal')
        root = canonical_path(d['root'])
        if canonical_path(self.path.parent).is_relative_to(root):
            raise ValueError('journal moved into protected root')
        for b, name in zip(d['builds'], ('macos-arm64-release', 'macos-arm64-headless')):
            base = root / 'build' / name
            if b['canonical'] != str(base):
                raise ValueError('canonical path changed')
            if b['protected'] != str(base.with_name(name + '.recovery-protected-' + d['lease']['id'])):
                raise ValueError('protected path changed')
            if b['attempt'] != str(base.with_name(name + '.recovery-attempt-' + d['lease']['id'])):
                raise ValueError('attempt path changed')
        if len(d['builds']) != 2:
            raise ValueError('both build paths required')

    def move(self, source, dest, identity):
        source, dest = Path(source), Path(dest)
        if not source.exists() or dest.exists() or dest.is_symlink():
            raise ValueError('rename requires existing source and absent destination')
        if directory_identity(source) != identity:
            raise ValueError('directory identity mismatch')
        self.data['intent'] = {'kind': 'rename', 'source': str(source),
                               'dest': str(dest), 'identity': identity}
        self.record('RENAME_INTENT')
        os.rename(source, dest)
        self.failpoint('AFTER_RENAME_BEFORE_COMMIT')
        self.data['intent'] = None
        self.record('RENAME_COMMITTED')

    def recover_intent(self):
        """Reconcile an interrupted atomic rename without overwriting anything."""
        intent = self.data['intent']
        if not intent:
            return
        if intent['kind'] != 'rename':
            raise ValueError('unknown journal intent')
        source, dest = Path(intent['source']), Path(intent['dest'])
        allowed = {(b['canonical'], b['protected']) for b in self.data['builds']}
        allowed |= {(b['canonical'], b['attempt']) for b in self.data['builds']}
        allowed |= {(b['protected'], b['canonical']) for b in self.data['builds']}
        if (str(source), str(dest)) not in allowed:
            raise ValueError('rename intent escaped declared paths')
        if source.exists() == dest.exists():
            raise ValueError('ambiguous interrupted rename')
        actual = source if source.exists() else dest
        if directory_identity(actual) != intent['identity']:
            raise ValueError('interrupted rename directory identity mismatch')
        self.data['intent'] = None
        self.record('RENAME_RECONCILED')

    def verify_protected(self):
        for original, expected in self.data['images'].items():
            p = Path(original)
            for b in self.data['builds']:
                base = Path(b['canonical'])
                if p.is_relative_to(base):
                    stored = Path(b['protected'])
                    if stored.exists():
                        if directory_identity(stored) != b['identity']:
                            raise ValueError('original directory identity changed')
                        p = stored / p.relative_to(base)
                    break
            if not p.is_file() or digest(p) != expected:
                raise ValueError('original protected file changed')

    def begin(self):
        self.validate()
        self.recover_intent()
        if self.data['phase'] == 'RESTORED':
            raise ValueError('completed transaction cannot be reused')
        if time.time() >= self.data['lease']['deadline']:
            raise ValueError('allocation expired')
        if self.checkout.read() != self.data['original_checkout']:
            raise ValueError('source changed before begin')
        if self.free_bytes(self.data['root']) < 65 * 1024**3:
            raise ValueError('disk reserve below 65GiB')
        self.verify_protected()
        for b in self.data['builds']:
            if Path(b['protected']).exists():
                if directory_identity(b['protected']) != b['identity']:
                    raise ValueError('protected directory identity changed')
            else:
                self.move(b['canonical'], b['protected'], b['identity'])
        self.data['phase'] = 'CHECKOUT_INTENT'
        self.record('CHECKOUT_INTENT')
        self.checkout.select({'head': self.data['target_commit'], 'branch': None, 'clean': True})
        self.failpoint('AFTER_CHECKOUT_BEFORE_COMMIT')
        if self.checkout.read() != {'head': self.data['target_commit'], 'branch': None, 'clean': True}:
            raise ValueError('historical checkout failed identity check')
        self.data['phase'] = 'ACTIVE'
        self.record('ACTIVE')

    def checkpoint(self, phase):
        self.validate()
        if self.free_bytes(self.data['root']) < 50 * 1024**3:
            raise ValueError('disk floor crossed; restore required')
        if self.data['initial_free_bytes'] - self.free_bytes(self.data['root']) > self.data['lease'].get('maximum_additional_bytes', 8 * 1024**3):
            raise ValueError('allocation space budget exceeded; restore required')
        if time.time() >= self.data['lease']['deadline']:
            raise ValueError('allocation expired; restore required')
        self.verify_protected()
        self.data['phase'] = phase
        self.record(phase)

    def restore(self):
        """Restore is allowed after lease expiry; it never discards failed builds."""
        self.validate()
        self.recover_intent()
        actual = self.checkout.read()
        allowed = [self.data['original_checkout'],
                   {'head': self.data['target_commit'], 'branch': None, 'clean': True}]
        if actual not in allowed:
            raise ValueError('unexpected or dirty checkout: manual recovery required')
        self.verify_protected()
        self.data['phase'] = 'RESTORE_CHECKOUT_INTENT'
        self.record('RESTORE_CHECKOUT_INTENT')
        self.checkout.select(self.data['original_checkout'])
        if self.checkout.read() != self.data['original_checkout']:
            raise ValueError('original checkout was not restored exactly')
        for b in self.data['builds']:
            source, protected, attempt = (Path(b[k]) for k in ('canonical', 'protected', 'attempt'))
            if protected.exists():
                if directory_identity(protected) != b['identity']:
                    raise ValueError('original directory replaced')
                if source.exists():
                    self.move(source, attempt, directory_identity(source))
                self.move(protected, source, b['identity'])
            elif directory_identity(source) != b['identity']:
                raise ValueError('original directory missing')
        self.verify_protected()
        self.data['phase'] = 'RESTORED'
        self.record('RESTORED')

    @contextmanager
    def signal_unwind(self):
        old = {}
        def interrupted(signum, frame):
            raise RecoveryInterrupted(signum)
        try:
            for sig in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
                old[sig] = signal.signal(sig, interrupted)
            yield
        finally:
            for sig, handler in old.items():
                signal.signal(sig, handler)

    def run_owned(self, operation):
        """Signal/error handlers can request unwind; abrupt death uses restore()."""
        with self.signal_unwind():
            try:
                self.begin()
                return operation(self)
            finally:
                if self.data is not None:
                    self.restore()


class RecoveryInterrupted(BaseException):
    pass


class GitCheckout:
    """Actual adapter, uninvoked by synthetic qualification or read-only snapshots."""
    def __init__(self, root):
        self.root = canonical_path(root)

    def git(self, *args):
        return subprocess.check_output(['git', '-C', str(self.root), *args], text=True).strip()

    def read(self):
        branch = self.git('symbolic-ref', '--short', '-q', 'HEAD') if self._attached() else None
        return {'head': self.git('rev-parse', 'HEAD'), 'branch': branch,
                'clean': not self.git('status', '--porcelain')}

    def _attached(self):
        return subprocess.run(['git', '-C', str(self.root), 'symbolic-ref', '-q', 'HEAD'],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0

    def select(self, state):
        if not self.read()['clean']:
            raise ValueError('refusing checkout with local changes')
        head = state['head']
        if len(head) != 40 or any(c not in '0123456789abcdef' for c in head):
            raise ValueError('full checkout commit required')
        if state['branch']:
            branch = state['branch']
            subprocess.run(['git', 'check-ref-format', '--branch', branch], check=True,
                           stdout=subprocess.DEVNULL)
            if self.git('rev-parse', 'refs/heads/' + branch) != head:
                raise ValueError('original branch ref moved; manual recovery required')
            argv = ['git', '-C', str(self.root), 'checkout', branch]
        else:
            argv = ['git', '-C', str(self.root), 'checkout', '--detach', head]
        subprocess.run(argv, check=True)
        if self.read() != state:
            raise ValueError('checkout postcondition failed')


def qualify_image(path, expected_sha256, expected_uuid, uuid_reader):
    if digest(path) != expected_sha256:
        raise ValueError('rebuilt image SHA differs from frozen image')
    if uuid_reader(path).lower() != expected_uuid.lower():
        raise ValueError('rebuilt image UUID differs from frozen image')
    return {'exact_image_sha256_and_UUID': True, 'symbolization_qualification': 'PENDING'}
