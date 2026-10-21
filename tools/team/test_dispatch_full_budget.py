"""Filesystem/source models only: no child, signal, timer or native lock."""
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import dispatch_full_budget as full


class ScanSourceModels(unittest.TestCase):
    def test_small_tree_and_symlink_not_followed(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root/'data').write_bytes(b'x'*4096)
            (root/'loop').symlink_to(root, target_is_directory=True)
            self.assertEqual(full.bounded_allocated(root), (root/'data').stat().st_blocks*512)

    def test_entry_limit_rejects(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            (root/'one').touch()
            (root/'two').touch()
            with self.assertRaises(full.accepted.DiskBudgetExceeded):
                full.bounded_allocated(root, entries=1)

    def test_elapsed_limit_rejects_without_sleep(self):
        with tempfile.TemporaryDirectory() as d:
            with patch.object(full.time, 'monotonic', side_effect=[0.0, 3.0]):
                with self.assertRaises(full.accepted.DiskBudgetExceeded):
                    full.bounded_allocated(Path(d), seconds=2.0)

    def test_generic_launch_blocked_before_popen(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d)
            job = full.FullBudgetJob(root, 10, root, 0, 4096, 0.25)
            with patch.object(full.accepted.p.subprocess, 'Popen') as spawn:
                with self.assertRaisesRegex(RuntimeError, 'ownership remains UNQUALIFIED'):
                    job.run(['DO_NOT_EXECUTE'])
                spawn.assert_not_called()


if __name__ == '__main__':
    unittest.main()
