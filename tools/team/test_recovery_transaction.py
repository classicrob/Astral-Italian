import copy
import json
from pathlib import Path
import tempfile
import time
import os
import signal
import subprocess
import unittest

from recovery_transaction import RecoveryTransaction
from symbol_recovery import digest


class SimulatedDeath(BaseException):
    pass


class Checkout:
    def __init__(self, root, state):
        self.root, self.state = root, copy.deepcopy(state)
        self.select(state)

    def read(self):
        return copy.deepcopy(self.state)

    def select(self, state):
        self.state = copy.deepcopy(state)
        (self.root / 'tracked-source').write_text(state['head'])


class TransactionTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.base = Path(self.tmp.name).resolve()
        self.root = self.base / 'source'; self.root.mkdir()
        self.out = self.base / 'private'; self.out.mkdir()
        self.original = {'head': 'a' * 40, 'branch': 'wt/port', 'clean': True}
        self.target = 'b' * 40
        self.checkout = Checkout(self.root, self.original)
        self.builds = [self.root / 'build' / n for n in ('macos-arm64-release', 'macos-arm64-headless')]
        self.images = {}
        for i, b in enumerate(self.builds):
            b.mkdir(parents=True); p = b / 'image'; p.write_bytes(('original' + str(i)).encode())
            (b / 'object.o').write_bytes(b'preserved compiler object')
            self.images[str(p)] = digest(p)
        self.space = 80 * 1024**3
        self.lease = {'status': 'APPROVED', 'id': 'fixture', 'source_root': str(self.root),
                      'original_head': self.original['head'], 'target_commit': self.target,
                      'native_lock_exclusive': True, 'deadline': time.time() + 60}
        self.journal = self.out / 'journal.json'

    def transaction(self, failpoint=None, prepare=True):
        tx = RecoveryTransaction(self.journal, self.checkout, lambda p: self.space, failpoint)
        if prepare:
            tx.prepare(self.root, self.original, self.target, self.lease, self.images)
        return tx

    def assert_restored(self, tx):
        self.assertEqual(tx.data['phase'], 'RESTORED')
        self.assertEqual(self.checkout.read(), self.original)
        self.assertEqual((self.root / 'tracked-source').read_text(), self.original['head'])
        for b in self.builds:
            self.assertEqual(digest(b / 'image'), self.images[str(b / 'image')])
            self.assertEqual((b / 'object.o').read_bytes(), b'preserved compiler object')
        self.assertIsNone(tx.data['intent'])

    def test_success_restores_both_and_preserves_new_build(self):
        tx = self.transaction()
        def operation(tx):
            for b in self.builds:
                b.mkdir(); (b / 'image').write_bytes(b'new attempt')
            tx.checkpoint('ATTEMPT_LINKED')
            return 17
        self.assertEqual(tx.run_owned(operation), 17)
        self.assert_restored(tx)
        for b in tx.data['builds']:
            self.assertEqual((Path(b['attempt']) / 'image').read_bytes(), b'new attempt')
        tx.restore()
        self.assert_restored(tx)

    def test_operation_error_restores(self):
        tx = self.transaction()
        def operation(tx):
            raise RuntimeError('configure failed')
        with self.assertRaises(RuntimeError): tx.run_owned(operation)
        self.assert_restored(tx)

    def test_signal_unwind_restores(self):
        tx = self.transaction()
        def operation(tx):
            raise KeyboardInterrupt('owned signal')
        with self.assertRaises(KeyboardInterrupt): tx.run_owned(operation)
        self.assert_restored(tx)

    def test_restart_after_atomic_rename_before_commit(self):
        fired = []
        def fail(event):
            if event == 'AFTER_RENAME_BEFORE_COMMIT' and not fired:
                fired.append(event); raise SimulatedDeath()
        tx = self.transaction(fail)
        with self.assertRaises(SimulatedDeath): tx.begin()
        restarted = self.transaction(prepare=False)
        restarted.restore()
        self.assert_restored(restarted)

    def test_restart_before_atomic_rename(self):
        def fail(event):
            if event == 'RENAME_INTENT': raise SimulatedDeath()
        tx = self.transaction(fail)
        with self.assertRaises(SimulatedDeath): tx.begin()
        restarted = self.transaction(prepare=False); restarted.restore(); self.assert_restored(restarted)

    def test_restart_after_checkout_before_commit(self):
        def fail(event):
            if event == 'AFTER_CHECKOUT_BEFORE_COMMIT': raise SimulatedDeath()
        tx = self.transaction(fail)
        with self.assertRaises(SimulatedDeath): tx.begin()
        restarted = self.transaction(prepare=False); restarted.restore(); self.assert_restored(restarted)

    def test_restart_during_restore_preserves_attempt(self):
        tx = self.transaction(); tx.begin()
        self.builds[0].mkdir(); (self.builds[0] / 'failed').write_bytes(b'failure evidence')
        fired = []
        def fail(event):
            if event == 'AFTER_RENAME_BEFORE_COMMIT' and not fired:
                fired.append(event); raise SimulatedDeath()
        tx.failpoint = fail
        with self.assertRaises(SimulatedDeath): tx.restore()
        restarted = self.transaction(prepare=False); restarted.restore(); self.assert_restored(restarted)
        self.assertEqual((Path(restarted.data['builds'][0]['attempt']) / 'failed').read_bytes(), b'failure evidence')

    def test_no_overwrite_of_occupied_attempt(self):
        tx = self.transaction(); tx.begin(); self.builds[0].mkdir()
        dest = Path(tx.data['builds'][0]['attempt']); dest.mkdir(); (dest / 'keep').write_bytes(b'unrelated')
        with self.assertRaises(ValueError): tx.restore()
        self.assertEqual((dest / 'keep').read_bytes(), b'unrelated')
        self.assertTrue(Path(tx.data['builds'][0]['protected']).exists())

    def test_mismatched_protected_image_rejects_before_move(self):
        tx = self.transaction(); (self.builds[0] / 'image').write_bytes(b'changed')
        with self.assertRaises(ValueError): tx.begin()
        self.assertTrue(self.builds[0].exists())

    def test_dirty_or_foreign_checkout_rejects(self):
        tx = self.transaction(); tx.begin()
        self.checkout.state['clean'] = False
        with self.assertRaises(ValueError): tx.restore()
        self.assertTrue(Path(tx.data['builds'][0]['protected']).exists())

    def test_changed_directory_identity_rejects(self):
        tx = self.transaction(); tx.begin()
        b = tx.data['builds'][0]; Path(b['protected']).rename(self.out / 'held-original')
        Path(b['protected']).mkdir()
        with self.assertRaises(ValueError): tx.restore()
        self.assertTrue((self.out / 'held-original/image').exists())

    def test_disk_floor_unwinds_without_deleting(self):
        tx = self.transaction()
        def operation(tx):
            self.space = 49 * 1024**3
            tx.checkpoint('BUILDING')
        with self.assertRaises(ValueError): tx.run_owned(operation)
        self.assert_restored(tx)

    def test_expired_lease_rejects_begin_but_allows_restore(self):
        tx = self.transaction(); tx.data['lease']['deadline'] = time.time() - 1; tx.save()
        with self.assertRaises(ValueError): tx.begin()
        tx.restore(); self.assert_restored(tx)

    def test_missing_allocation_rejects_prepare(self):
        self.lease['status'] = 'UNALLOCATED'
        with self.assertRaises(ValueError): self.transaction()
        self.assertFalse(self.journal.exists())

    def test_actual_owned_sigterm_unwinds_and_restores_handler(self):
        from recovery_transaction import RecoveryInterrupted
        old = signal.getsignal(signal.SIGTERM)
        tx = self.transaction()
        def operation(tx):
            os.kill(os.getpid(), signal.SIGTERM)
        with self.assertRaises(RecoveryInterrupted): tx.run_owned(operation)
        self.assert_restored(tx)
        self.assertEqual(signal.getsignal(signal.SIGTERM), old)

    def test_final_SHA_and_UUID_are_both_required(self):
        from recovery_transaction import qualify_image
        image = self.builds[0] / 'image'
        with self.assertRaises(ValueError): qualify_image(image, '0' * 64, 'uuid', lambda p: 'uuid')
        with self.assertRaises(ValueError): qualify_image(image, digest(image), 'expected', lambda p: 'wrong')
        result = qualify_image(image, digest(image), 'expected', lambda p: 'expected')
        self.assertTrue(result['exact_image_sha256_and_UUID'])
        self.assertEqual(result['symbolization_qualification'], 'PENDING')

    def test_real_git_adapter_restores_synthetic_checkout(self):
        from recovery_transaction import GitCheckout
        def git(*args):
            return subprocess.check_output(['git', '-C', str(self.root), *args],
                                           text=True, stderr=subprocess.DEVNULL).strip()
        git('init', '-q')
        git('config', 'user.name', 'Recovery Fixture')
        git('config', 'user.email', 'fixture@example.invalid')
        git('config', 'commit.gpgsign', 'false')
        hooks = self.out / 'empty-hooks'; hooks.mkdir()
        git('config', 'core.hooksPath', str(hooks))
        (self.root / '.gitignore').write_text('build/\n')
        git('add', '.gitignore', 'tracked-source'); git('commit', '-q', '-m', 'original')
        original_head = git('rev-parse', 'HEAD')
        (self.root / 'tracked-source').write_text('historical source')
        git('commit', '-qam', 'historical'); target = git('rev-parse', 'HEAD')
        git('checkout', '-qb', 'wt/port', original_head)
        adapter = GitCheckout(self.root); original = adapter.read()
        self.lease.update(original_head=original_head, target_commit=target)
        tx = RecoveryTransaction(self.journal, adapter, lambda p: self.space)
        tx.prepare(self.root, original, target, self.lease, self.images)
        def operation(tx):
            self.assertEqual((self.root / 'tracked-source').read_text(), 'historical source')
        tx.run_owned(operation)
        self.assertEqual(adapter.read(), original)
        self.assertEqual((self.root / 'tracked-source').read_text(), self.original['head'])
        self.assertEqual(tx.data['phase'], 'RESTORED')

    def test_space_budget_and_terminal_journal_reject_reuse(self):
        tx = self.transaction()
        def operation(tx):
            self.space = 71 * 1024**3
            tx.checkpoint('SPACE_CHECK')
        with self.assertRaises(ValueError): tx.run_owned(operation)
        self.assert_restored(tx)
        self.space = 80 * 1024**3
        with self.assertRaises(ValueError): tx.begin()

    def test_tampered_intent_or_paths_reject(self):
        tx = self.transaction(); tx.data['builds'][0]['canonical'] = '/unowned'
        with self.assertRaises(ValueError): tx.restore()
        self.assertTrue(self.builds[0].exists())


if __name__ == '__main__':
    unittest.main()
