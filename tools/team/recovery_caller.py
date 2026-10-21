"""One allocated frozen GX recovery attempt, with child-before-restore ordering."""
import argparse
from contextlib import contextmanager
import fcntl
import json
import os
from pathlib import Path
import shlex
import shutil
import signal
import subprocess
import sys
import time

from recovery_transaction import (RecoveryTransaction, GitCheckout, atomic_json,
                                  directory_identity, qualify_image)
from retention_paths import canonical_path
from symbol_recovery import digest


class ChildNotQuiescent(RuntimeError):
    pass


def group_exists(pgid):
    try:
        os.killpg(pgid, 0)
        return True
    except ProcessLookupError:
        return False
    except PermissionError:
        # Existence is uncertain; never authorize restoration from EPERM.
        return True


@contextmanager
def exclusive_lock(path):
    fd = os.open(path, os.O_RDWR | os.O_CREAT, 0o600)
    try:
        fcntl.lockf(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        yield
    finally:
        os.close(fd)


class GroupSupervisor:
    """Adapt bounded_run's owned-session TERM/KILL/wait pattern with live guards."""
    def __init__(self, journal, check, stop_file, poll=0.2, term_seconds=2):
        self.journal, self.check = Path(journal), check
        self.stop_file = Path(stop_file)
        self.poll, self.term_seconds = poll, term_seconds
        self.children, self.events = [], []
        self.persist()

    def persist(self):
        atomic_json(self.journal, {'events': self.events, 'children': [
            {'pid': p.pid, 'pgid': p.pid, 'reaped': p.returncode is not None,
             'group_exists': group_exists(p.pid)} for p in self.children]})

    def stop_all(self):
        for child in self.children:
            if group_exists(child.pid):
                try:
                    os.killpg(child.pid, signal.SIGTERM)
                except ProcessLookupError:
                    pass
                except PermissionError as exc:
                    raise ChildNotQuiescent('cannot establish ownership/quiescence of group') from exc
                until = time.monotonic() + self.term_seconds
                while time.monotonic() < until:
                    child.poll()
                    if not group_exists(child.pid):
                        break
                    time.sleep(min(self.poll, 0.05))
                if group_exists(child.pid):
                    try:
                        os.killpg(child.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
                    except PermissionError as exc:
                        raise ChildNotQuiescent('cannot establish ownership/quiescence of group') from exc
            try:
                child.wait(timeout=2)
            except subprocess.TimeoutExpired as exc:
                raise ChildNotQuiescent('owned leader could not be reaped') from exc
            until = time.monotonic() + 2
            while group_exists(child.pid) and time.monotonic() < until:
                time.sleep(0.05)
            if group_exists(child.pid):
                self.persist()
                raise ChildNotQuiescent('owned group still present; DO NOT RESTORE')
        self.events.append('ALL_CHILD_GROUPS_GONE_AND_LEADERS_REAPED')
        self.persist()

    def run(self, argv, cwd, seconds, log, env=None):
        self.check()
        if self.stop_file.exists():
            raise RuntimeError('incoming/stop file present')
        if seconds <= 0:
            raise ValueError('positive phase timeout required')
        gate = [sys.executable, str(Path(__file__).resolve()), '--gated-child', json.dumps(argv)]
        with Path(log).open('xb') as output:
            child = subprocess.Popen(gate, cwd=cwd, env=env, stdin=subprocess.PIPE,
                                     stdout=output, stderr=subprocess.STDOUT,
                                     start_new_session=True)
            self.children.append(child)
            try:
                # The child blocks before exec; durable ownership precedes release.
                self.events.append({'SPAWNED_GATED': child.pid, 'argv': argv})
                self.persist()
                child.stdin.write(b'G'); child.stdin.flush(); child.stdin.close()
                until = time.monotonic() + seconds
                while True:
                    self.check()
                    if self.stop_file.exists():
                        raise RuntimeError('incoming/stop file present')
                    if time.monotonic() >= until:
                        raise TimeoutError('phase deadline exceeded')
                    rc = child.poll()
                    if rc is not None:
                        # A successful leader must not leave a worker writing paths.
                        self.stop_all()
                        if rc:
                            raise RuntimeError('child exit ' + str(rc))
                        return
                    time.sleep(self.poll)
            except BaseException:
                self.stop_all()
                raise


def restore_after_children(supervisor, transaction, receipt):
    supervisor.stop_all()
    if any(p.returncode is None or group_exists(p.pid) for p in supervisor.children):
        raise ChildNotQuiescent('refusing restoration with an unreaped or live owned group')
    atomic_json(receipt, {'all_children_gone_and_reaped': True})
    transaction.restore()


def verify_pins(pins):
    for path, expected in pins.items():
        if digest(path) != expected:
            raise ValueError('input/tool pin changed: ' + path)


def verify_generated(build, manifest):
    selected = {str(p.relative_to(build)) for p in (Path(build) / 'legacy-sjis').rglob('*')
                if p.is_file() and p.suffix in {'.cpp', '.hpp', '.h', '.inc'}}
    if selected != set(manifest) or len(selected) != 4931:
        raise ValueError('generated 4931-file closure differs')
    for relative, record in manifest.items():
        path = Path(build) / relative
        if path.stat().st_size != record['bytes'] or digest(path) != record['sha256']:
            raise ValueError('generated source differs: ' + relative)


def verify_additional_generated(build, manifest):
    for relative, record in manifest.items():
        path = canonical_path(Path(build) / relative)
        if not path.is_relative_to(canonical_path(build)):
            raise ValueError('generated source path escaped build')
        if path.stat().st_size != record['bytes'] or digest(path) != record['sha256']:
            raise ValueError('additional generated source differs: ' + relative)


def missing_object_measurement(build, contract, manifest):
    before = canonical_path(contract['preserved_build'])
    entries = []
    for row in manifest['unrecovered']:
        path = canonical_path(row['path'])
        if path.is_relative_to(before):
            actual = canonical_path(Path(build) / path.relative_to(before))
            sha = digest(actual) if actual.is_file() else None
            entries.append({'relative': str(path.relative_to(before)), 'expected_sha256': row['sha256'],
                            'actual_sha256': sha, 'exact': sha == row['sha256']})
    if len(entries) != 559:
        raise ValueError('reviewed build5.1 GX missing-object count changed')
    return entries


def verify_tokens(build, contract):
    commands = json.loads((Path(build) / 'compile_commands.json').read_text())
    actual = [shlex.split(c['command']) for c in commands]
    # Entry order may be irrelevant; argument order within each command is exact.
    if sorted(actual) != sorted(contract['compile_argv']):
        raise ValueError('compile argv drift')
    if any(not canonical_path(c['directory']).is_relative_to(canonical_path(build)) for c in commands):
        raise ValueError('compile cwd escaped canonical build')
    path = Path(build) / 'native/app/CMakeFiles/petari.dir/link.txt'
    if shlex.split(path.read_text()) != contract['link_argv']:
        raise ValueError('link argv drift')


def macho_uuid(path):
    import re
    data = subprocess.check_output(['/usr/bin/otool', '-l', str(path)], text=True)
    matches = re.findall(r'\buuid ([0-9a-fA-F-]+)', data)
    if len(matches) != 1:
        raise ValueError('unique Mach-O UUID required')
    return matches[0]


def validate_packet(packet, allocation, packet_path):
    if packet['schema'] != 1 or packet['target'] != 'build5.1 GX':
        raise ValueError('only the reviewed first target is supported')
    if allocation.get('status') != 'APPROVED' or allocation.get('packet_sha256') != digest(packet_path):
        raise ValueError('missing reviewed allocation/packet identity')
    if allocation.get('maximum_seconds') != 1560 or allocation.get('maximum_additional_bytes') != 8 * 1024**3:
        raise ValueError('allocation budget differs from reviewed first-target bound')
    if allocation['deadline'] > time.time() + 1560 or allocation['deadline'] <= time.time():
        raise ValueError('allocation wall deadline outside reviewed bound')
    required = {packet['snapshot'], packet['contract'], packet['generated_manifest'], packet['additional_generated_manifest'], packet['unrecovered_manifest']}
    required |= {str(Path(__file__).resolve().with_name(n)) for n in
                 ('recovery_caller.py', 'recovery_transaction.py', 'symbol_recovery.py', 'retention_paths.py')}
    if not required.issubset(packet['pins']):
        raise ValueError('mandatory input/caller pins missing')
    if packet['historical_source'] != '82434e85357f6b32c4943c8f30c657c511a9308d':
        raise ValueError('historical target source changed')
    if packet['native_lock'] != str(Path.home() / 'MacioGalaxy2-team/slots/native.lock'):
        raise ValueError('not the shared native POSIX lock')
    verify_pins(packet['pins'])
    snapshot = json.loads(Path(packet['snapshot']).read_text())
    if snapshot['actual_checkout']['head'] != '33e71ee880b426c0c7da03a724377200e874c417':
        raise ValueError('snapshot is not reviewed post-landing 33e71')
    if len(snapshot['images']) != 8 or len({x['path'] for x in snapshot['images']}) != 8 or len(snapshot['builds']) != 2:
        raise ValueError('both builds and all eight protected images required')
    if allocation.get('original_head') != snapshot['actual_checkout']['head'] or allocation.get('target_commit') != packet['historical_source']:
        raise ValueError('allocation checkout binding mismatch')
    if packet['preserve_builds'] != snapshot['builds'] or packet['preserve_images'] != snapshot['images']:
        raise ValueError('executable contract must preserve both builds and all eight images')
    if packet['expected_image'] != {'sha256': '1542894ffb4aceb90b4039680531cf656c10b11a8a9a273a13561b052c3c81a2',
                                    'uuid': '244128be-56b7-3adc-a689-a17526c8927c'}:
        raise ValueError('frozen target identity changed')
    return snapshot


def verify_build_snapshots(snapshot):
    for b in snapshot['builds']:
        if directory_identity(b['root']) != b['identity']:
            raise ValueError('current build directory identity changed')
        if digest(b['file_metadata_path']) != b['file_metadata_sha256']:
            raise ValueError('sealed build metadata changed')
        expected = {x['relative']: x for x in json.loads(Path(b['file_metadata_path']).read_text())}
        actual = {}
        for base, dirs, files in os.walk(b['root'], followlinks=False):
            for name in files:
                f = Path(base) / name
                stat = f.lstat()
                relative = str(f.relative_to(b['root']))
                actual[relative] = {'relative': relative, 'size': stat.st_size,
                                    'mtime_ns': stat.st_mtime_ns, 'inode': stat.st_ino,
                                    'symlink': os.readlink(f) if f.is_symlink() else None}
        if actual != expected:
            raise ValueError('current build files differ from sealed snapshot')


def execute(packet_path, allocation_path, output):
    packet_path, output = Path(packet_path), canonical_path(output)
    packet, allocation = (json.loads(Path(p).read_text()) for p in (packet_path, allocation_path))
    snapshot = validate_packet(packet, allocation, packet_path)
    root = canonical_path(packet['source_root'])
    contract = json.loads(Path(packet['contract']).read_text())
    if contract['source_root'] != str(root) or contract['configure_argv'] != packet['configure_argv']:
        raise ValueError('configure argv/source-root drift')
    for argv in (packet['configure_argv'], packet['build_argv']):
        program = shutil.which(argv[0])
        expected_program = packet['command_executables'].get(argv[0])
        if not program or canonical_path(program) != canonical_path(expected_program):
            raise ValueError('command executable resolution drift')
    if contract['build_argv'] != packet['build_argv'] or packet['build_argv'][-1] != '-j6':
        raise ValueError('build argv drift')
    if GitCheckout(root).read() != snapshot['actual_checkout']:
        raise ValueError('actual checkout differs from 33e71 snapshot')
    verify_build_snapshots(snapshot)
    for image in snapshot['images']:
        qualify_image(image['path'], image['sha256'], image['uuid'], macho_uuid)
    if output.exists() or output.is_relative_to(root) or root.is_relative_to(output):
        raise ValueError('output must be fresh and outside checkout')
    with exclusive_lock(packet['native_lock']):
        if GitCheckout(root).read() != snapshot['actual_checkout']:
            raise ValueError('checkout changed while acquiring lock')
        verify_build_snapshots(snapshot)
        output.mkdir(mode=0o700)
        atomic_json(output / 'launch-receipt.json', {'packet_sha256': digest(packet_path),
                    'allocation_sha256': digest(allocation_path), 'configure_argv': packet['configure_argv'],
                    'build_argv': packet['build_argv'], 'preserve_both_builds': snapshot['builds'],
                    'preserve_all_eight_images': snapshot['images']})
        transaction = RecoveryTransaction(output / 'transaction.json', GitCheckout(root))
        lease = dict(allocation, original_head=snapshot['actual_checkout']['head'],
                     source_root=str(root), target_commit=packet['historical_source'], native_lock_exclusive=True)
        transaction.prepare(root, snapshot['actual_checkout'], packet['historical_source'], lease,
                            {x['path']: x['sha256'] for x in snapshot['images']})
        def live_limits():
            free = shutil.disk_usage(root).free
            if free < 50 * 1024**3:
                raise RuntimeError('live disk floor crossed')
            if transaction.data['initial_free_bytes'] - free > 8 * 1024**3:
                raise RuntimeError('live additional-space budget exceeded')
            if time.time() >= allocation['deadline']:
                raise TimeoutError('total allocation deadline exceeded')
        supervisor = GroupSupervisor(output / 'children.json', live_limits, output / 'STOP')
        result = {'status': 'FAILED', 'real_recovery_attempt': True, 'promotion': False}
        try:
            with transaction.signal_unwind():
                try:
                    transaction.begin()
                    verify_pins(packet['pins'])
                    env = dict(os.environ, CMAKE_BUILD_PARALLEL_LEVEL='6')
                    supervisor.run(packet['configure_argv'], root, 180, output / 'configure.log', env)
                    build = Path(contract['canonical_build'])
                    verify_generated(build, json.loads(Path(packet['generated_manifest']).read_text()))
                    verify_additional_generated(build, json.loads(Path(packet['additional_generated_manifest']).read_text()))
                    verify_tokens(build, contract)
                    atomic_json(output / 'prebuild-qualification.json',
                                {'generated_files_exact': 4931, 'compile_and_link_tokens_exact': True})
                    supervisor.run(packet['build_argv'], root, 1200, output / 'build.log', env)
                    image = build / contract['image_relative']
                    # A mismatch never triggers timestamp rewriting, relink or substitution.
                    result['image'] = {'sha256': digest(image), 'uuid': macho_uuid(image)}
                    atomic_json(output / 'candidate-image.json', result['image'])
                    measured = missing_object_measurement(build, contract, json.loads(Path(packet['unrecovered_manifest']).read_text()))
                    atomic_json(output / 'missing-object-measurement.json', {'objects': measured})
                    qualify_image(image, packet['expected_image']['sha256'], packet['expected_image']['uuid'], macho_uuid)
                    if not all(x['exact'] for x in measured):
                        raise ValueError('missing symbol objects differ despite image match')
                    result['status'] = 'EXACT_IMAGE_REPRODUCED_SYMBOLIZATION_PENDING'
                finally:
                    # Mandatory ordering: no source/path restoration while any writer lives.
                    restore_after_children(supervisor, transaction, output / 'before-restore.json')
        except BaseException as exc:
            result['error'] = type(exc).__name__ + ': ' + str(exc)
            raise
        finally:
            result['transaction_phase'] = transaction.data['phase']
            result['historical_tools'] = 'METADATA_BOUND_ATTEMPT; HISTORICAL_FILE_HASHES_UNESTABLISHED'
            atomic_json(output / 'result.json', result)
        return result


def main():
    if len(sys.argv) == 3 and sys.argv[1] == '--gated-child':
        if os.read(0, 1) != b'G':
            raise SystemExit('unreleased owned child')
        if os.getpriority(os.PRIO_PROCESS, 0) != 19:
            os.setpriority(os.PRIO_PROCESS, 0, 19)
        argv = json.loads(sys.argv[2])
        os.execvpe(argv[0], argv, os.environ)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--packet', type=Path, required=True)
    parser.add_argument('--allocation', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    execute(args.packet, args.allocation, args.output)


if __name__ == '__main__':
    main()
