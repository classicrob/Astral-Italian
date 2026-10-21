"""Regression for the destructive unnormalized-debug-map comparison error."""
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import retention_paths as paths


class RetentionPathTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.original = self.root / 'current'; self.original.mkdir()
        self.retained = self.root / 'old'; self.retained.mkdir()

    def test_native_app_parent_components_keep_required_object(self):
        reference = str(self.original / 'native/app/../../CMakeFiles/game.dir/file.cpp.o')
        wanted = self.retained / 'CMakeFiles/game.dir/file.cpp.o'
        self.assertIn(str(wanted), paths.reference_closure([reference], self.original, self.retained))

    def test_missing_original_and_retained_tail_still_required(self):
        reference = str(self.original / 'native/app/../../missing/deep/file.o')
        wanted = self.retained / 'missing/deep/file.o'
        self.assertFalse(wanted.exists())
        self.assertEqual(paths.retained_reference(reference, self.original, self.retained), (wanted, None))

    def test_existing_and_missing_paths_have_same_identity(self):
        reference = str(self.original / 'native/app/../../CMakeFiles/file.o')
        first = paths.retained_reference(reference, self.original, self.retained)
        target = self.retained / 'CMakeFiles/file.o'; target.parent.mkdir(); target.write_bytes(b'fixture')
        self.assertEqual(paths.retained_reference(reference, self.original, self.retained), first)

    def test_archive_member_keeps_whole_archive(self):
        reference = str(self.original / 'native/app/../../lib.a') + '(member.o)'
        self.assertEqual(paths.retained_reference(reference, self.original, self.retained), (self.retained / 'lib.a', 'member.o'))

    def test_external_reference_is_not_rebased(self):
        external = self.root / 'external/object.o'
        self.assertEqual(paths.retained_reference(str(external), self.original, self.retained), (external, None))

    def test_symlink_root_and_missing_tail(self):
        alias = self.root / 'alias'; alias.symlink_to(self.original, target_is_directory=True)
        reference = str(alias / 'native/app/../../missing.o')
        self.assertEqual(paths.retained_reference(reference, self.original, self.retained), (self.retained / 'missing.o', None))

    def test_relative_paths_reject(self):
        with self.assertRaises(ValueError):
            paths.retained_reference('native/app/../../unknown.o', self.original, self.retained)


if __name__ == '__main__': unittest.main()
