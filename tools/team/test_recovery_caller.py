import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

from recovery_caller import GroupSupervisor, group_exists, verify_tokens, verify_generated, exclusive_lock, restore_after_children, ChildNotQuiescent


class CallerTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name).resolve()
        self.supervisor = GroupSupervisor(self.root / 'children.json', lambda: None,
                                          self.root / 'STOP', poll=0.02, term_seconds=0.1)
        self.addCleanup(self.supervisor.stop_all)

    def test_timeout_group_killed_reaped_before_restore(self):
        unrelated = subprocess.Popen([sys.executable, '-c', 'import time; time.sleep(30)'], start_new_session=True)
        try:
            # Both leader and worker ignore TERM; KILL must finish the whole group.
            code = ('import signal,subprocess,sys,time; '
                    'signal.signal(signal.SIGTERM,signal.SIG_IGN); '
                    'subprocess.Popen([sys.executable,"-c",'
                    '"import signal,time; signal.signal(signal.SIGTERM,signal.SIG_IGN); time.sleep(30)"]); '
                    'time.sleep(30)')
            with self.assertRaises(TimeoutError):
                self.supervisor.run([sys.executable, '-c', code], self.root, 0.4, self.root / 'timeout.log')
            self.assertTrue(all(p.returncode is not None and not group_exists(p.pid)
                                for p in self.supervisor.children))
            self.assertIsNone(unrelated.poll())
            report = json.loads((self.root / 'children.json').read_text())
            self.assertEqual(report['events'][-1], 'ALL_CHILD_GROUPS_GONE_AND_LEADERS_REAPED')
        finally:
            os.killpg(unrelated.pid, signal.SIGKILL); unrelated.wait()

    def test_real_transaction_restores_only_after_disposable_child_termination(self):
        from recovery_transaction import RecoveryTransaction
        from symbol_recovery import digest
        source = self.root / 'source'; source.mkdir()
        output = self.root / 'private'; output.mkdir()
        original = {'head': 'a' * 40, 'branch': 'wt/port', 'clean': True}
        class Checkout:
            state = dict(original)
            def read(self): return dict(self.state)
            def select(self, state):
                if state == original:
                    if any(group_exists(p.pid) or p.returncode is None for p in self_supervisor.children):
                        raise AssertionError('checkout restoration while child alive')
                self.state = dict(state)
        checkout = Checkout(); self_supervisor = self.supervisor
        images = {}
        for name in ('macos-arm64-release', 'macos-arm64-headless'):
            b = source / 'build' / name; b.mkdir(parents=True)
            (b / 'image').write_bytes(name.encode()); images[str(b / 'image')] = digest(b / 'image')
        tx = RecoveryTransaction(output / 'transaction.json', checkout, lambda p: 80 * 1024**3)
        lease = {'status': 'APPROVED', 'id': 'fixture', 'original_head': original['head'],
                 'source_root': str(source), 'target_commit': 'b' * 40,
                 'native_lock_exclusive': True, 'deadline': time.time() + 60}
        tx.prepare(source, original, 'b' * 40, lease, images); tx.begin()
        try:
            with self.assertRaises(TimeoutError):
                self.supervisor.run([sys.executable, '-c', 'import time; time.sleep(30)'],
                                    source, 0.15, output / 'child.log')
        finally:
            restore_after_children(self.supervisor, tx, output / 'before-restore.json')
        self.assertEqual(checkout.read(), original)
        self.assertEqual(tx.data['phase'], 'RESTORED')
        for path, sha in images.items(): self.assertEqual(digest(path), sha)
        self.assertTrue(json.loads((output / 'before-restore.json').read_text())['all_children_gone_and_reaped'])
        receipt = os.environ.get('RECOVERY_TEST_RECEIPT')
        if receipt:
            evidence = {'scope': 'DISPOSABLE_SOURCE_ONLY_CONTROL',
                        'test': 'real transaction restores only after disposable child termination',
                        'children': [{'pid': p.pid, 'wait_returncode': p.returncode,
                                      'group_exists_after_cleanup': group_exists(p.pid)} for p in self.supervisor.children],
                        'before_restore_gate': json.loads((output / 'before-restore.json').read_text()),
                        'transaction_phase': tx.data['phase'], 'checkout_restored': checkout.read() == original,
                        'both_build_images_restored': all(digest(p) == sha for p, sha in images.items()),
                        'events': tx.data['events']}
            with Path(receipt).open('x') as f:
                json.dump(evidence, f, indent=2); f.write('\n')

    def test_unquiescent_child_refuses_restore(self):
        class Unsafe:
            def stop_all(self): raise ChildNotQuiescent('still alive')
        class Transaction:
            restored = False
            def restore(self): self.restored = True
        tx = Transaction()
        with self.assertRaises(ChildNotQuiescent):
            restore_after_children(Unsafe(), tx, self.root / 'must-not-exist.json')
        self.assertFalse(tx.restored)
        self.assertFalse((self.root / 'must-not-exist.json').exists())

    def test_child_nice_is_exactly_19(self):
        path = self.root / 'nice.log'
        self.supervisor.run([sys.executable, '-c', 'import os; print(os.getpriority(os.PRIO_PROCESS,0))'],
                            self.root, 5, path)
        self.assertEqual(path.read_text().strip(), '19')

    def test_live_budget_failure_stops_child_before_restore(self):
        calls = []
        def check():
            calls.append(1)
            if len(calls) > 4:
                raise RuntimeError('disk floor crossed')
        self.supervisor.check = check
        with self.assertRaisesRegex(RuntimeError, 'disk floor'):
            self.supervisor.run([sys.executable, '-c', 'import time; time.sleep(30)'],
                                self.root, 5, self.root / 'budget.log')
        self.assertTrue(all(not group_exists(p.pid) and p.returncode is not None
                            for p in self.supervisor.children))

    def test_incoming_stop_file_stops_child(self):
        stop = self.root / 'STOP'
        code = 'import pathlib,time; pathlib.Path(' + repr(str(stop)) + ').write_text("incoming"); time.sleep(30)'
        with self.assertRaisesRegex(RuntimeError, 'incoming'):
            self.supervisor.run([sys.executable, '-c', code], self.root, 5, self.root / 'incoming.log')
        self.assertTrue(all(not group_exists(p.pid) for p in self.supervisor.children))

    def test_signal_exception_stops_child(self):
        calls = []
        def check():
            calls.append(1)
            if len(calls) > 4:
                os.kill(os.getpid(), signal.SIGTERM)
        self.supervisor.check = check
        def interrupted(signum, frame):
            raise KeyboardInterrupt()
        old = signal.signal(signal.SIGTERM, interrupted)
        try:
            with self.assertRaises(KeyboardInterrupt):
                self.supervisor.run([sys.executable, '-c', 'import time; time.sleep(30)'],
                                    self.root, 5, self.root / 'signal.log')
        finally:
            signal.signal(signal.SIGTERM, old)
        self.assertTrue(all(not group_exists(p.pid) for p in self.supervisor.children))

    def test_successful_leader_does_not_leave_writer(self):
        code = 'import subprocess,sys; subprocess.Popen([sys.executable,"-c","import time; time.sleep(30)"])'
        self.supervisor.run([sys.executable, '-c', code], self.root, 5, self.root / 'success.log')
        self.assertTrue(all(p.returncode is not None and not group_exists(p.pid)
                            for p in self.supervisor.children))

    def test_compile_link_drift_rejects_before_build(self):
        path = self.root / 'native/app/CMakeFiles/petari.dir'; path.mkdir(parents=True)
        cmd = {'directory': str(self.root), 'command': 'c++ -O2 -c source.cpp'}
        (self.root / 'compile_commands.json').write_text(json.dumps([cmd]))
        (path / 'link.txt').write_text('c++ source.o -o Petari')
        contract = {'compile_argv': [['c++', '-O2', '-c', 'source.cpp']],
                    'link_argv': ['c++', 'source.o', '-o', 'Petari']}
        verify_tokens(self.root, contract)
        cmd['command'] = 'c++ -O3 -c source.cpp'
        (self.root / 'compile_commands.json').write_text(json.dumps([cmd]))
        with self.assertRaisesRegex(ValueError, 'compile argv'): verify_tokens(self.root, contract)
        cmd['command'] = 'c++ -O2 -c source.cpp'
        (self.root / 'compile_commands.json').write_text(json.dumps([cmd]))
        (path / 'link.txt').write_text('c++ source.o -framework Surprise -o Petari')
        with self.assertRaisesRegex(ValueError, 'link argv'): verify_tokens(self.root, contract)

    def test_stale_packet_tool_snapshot_or_target_rejects(self):
        import recovery_caller
        from symbol_recovery import digest
        snapshot = self.root / 'snapshot.json'
        snapshot.write_text(json.dumps({'actual_checkout': {'head': '33e71ee880b426c0c7da03a724377200e874c417'},
                                       'images': [{'path': str(i)} for i in range(8)], 'builds': [{}, {}]}))
        packet = {'schema': 1, 'target': 'build5.1 GX', 'snapshot': str(snapshot),
                  'historical_source': '82434e85357f6b32c4943c8f30c657c511a9308d',
                  'preserve_builds': [{}, {}], 'preserve_images': [{'path': str(i)} for i in range(8)],
                  'native_lock': str(Path.home() / 'MacioGalaxy2-team/slots/native.lock'),
                  'expected_image': {'sha256': '1542894ffb4aceb90b4039680531cf656c10b11a8a9a273a13561b052c3c81a2',
                                     'uuid': '244128be-56b7-3adc-a689-a17526c8927c'}}
        files = [snapshot]
        for key in ('contract', 'generated_manifest', 'additional_generated_manifest', 'unrecovered_manifest'):
            path = self.root / (key + '.json'); path.write_text('{}'); packet[key] = str(path); files.append(path)
        files += [Path(recovery_caller.__file__).with_name(n) for n in
                  ('recovery_caller.py', 'recovery_transaction.py', 'symbol_recovery.py', 'retention_paths.py')]
        packet['pins'] = {str(p): digest(p) for p in files}
        path = self.root / 'packet.json'
        def allocation():
            path.write_text(json.dumps(packet))
            return {'status': 'APPROVED', 'packet_sha256': digest(path), 'maximum_seconds': 1560,
                    'maximum_additional_bytes': 8 * 1024**3, 'deadline': time.time() + 60,
                    'original_head': '33e71ee880b426c0c7da03a724377200e874c417',
                    'target_commit': packet['historical_source']}
        lease = allocation(); recovery_caller.validate_packet(packet, lease, path)
        lease['packet_sha256'] = '0' * 64
        with self.assertRaises(ValueError): recovery_caller.validate_packet(packet, lease, path)
        lease = allocation(); snapshot.write_text('{}')
        with self.assertRaises(ValueError): recovery_caller.validate_packet(packet, lease, path)
        snapshot.write_text(json.dumps({'actual_checkout': {'head': 'wrong'}}))
        packet['pins'][str(snapshot)] = digest(snapshot); lease = allocation()
        with self.assertRaises(ValueError): recovery_caller.validate_packet(packet, lease, path)


    def test_exact_4931_source_manifest_rejects_changed_content(self):
        import hashlib
        root = self.root / 'legacy-sjis'; root.mkdir()
        data = b'fixture source'
        manifest = {}
        for i in range(4931):
            path = root / (str(i) + '.hpp'); path.write_bytes(data)
            manifest[str(path.relative_to(self.root))] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
        verify_generated(self.root, manifest)
        (root / '100.hpp').write_bytes(b'different source')
        with self.assertRaisesRegex(ValueError, 'source differs'): verify_generated(self.root, manifest)

    def test_build_metadata_drift_rejects(self):
        from recovery_caller import verify_build_snapshots
        from recovery_transaction import directory_identity
        from symbol_recovery import digest
        build = self.root / 'build'; build.mkdir(); path = build / 'image'; path.write_bytes(b'original')
        stat = path.lstat(); rows = [{'relative': 'image', 'size': stat.st_size,
                                    'mtime_ns': stat.st_mtime_ns, 'inode': stat.st_ino, 'symlink': None}]
        manifest = self.root / 'file-metadata.json'; manifest.write_text(json.dumps(rows))
        snapshot = {'builds': [{'root': str(build), 'identity': directory_identity(build),
                               'file_metadata_path': str(manifest), 'file_metadata_sha256': digest(manifest)}]}
        verify_build_snapshots(snapshot)
        path.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'files differ'): verify_build_snapshots(snapshot)


    def test_permission_uncertainty_never_means_group_gone(self):
        with patch('recovery_caller.os.killpg', side_effect=PermissionError()):
            self.assertTrue(group_exists(12345))


    def test_generated_missing_closure_rejects(self):
        (self.root / 'legacy-sjis').mkdir()
        with self.assertRaisesRegex(ValueError, '4931'): verify_generated(self.root, {})

    def test_POSIX_lock_is_nonblocking_and_does_not_truncate(self):
        path = self.root / 'native.lock'; path.write_bytes(b'preserve lock contents')
        with exclusive_lock(path):
            code = ('import fcntl,sys; f=open(sys.argv[1],"r+"); '
                    'fcntl.lockf(f,fcntl.LOCK_EX|fcntl.LOCK_NB)')
            child = subprocess.run([sys.executable, '-c', code, str(path)], capture_output=True)
            self.assertNotEqual(child.returncode, 0)
        self.assertEqual(path.read_bytes(), b'preserve lock contents')
        with exclusive_lock(path): pass


if __name__ == '__main__':
    unittest.main()
