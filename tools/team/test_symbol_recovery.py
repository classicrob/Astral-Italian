import copy
from pathlib import Path
import tempfile
import unittest

from symbol_recovery import configure_argv, patched_sdl, recovery_plan


class RecoveryGuards(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'source'
        self.root.mkdir()
        self.contract = {'source_root': str(self.root),
                         'canonical_build': str(self.root / 'build/macos-arm64-release')}
        self.args = dict(source_commit='1' * 40, expected={'sha256': '2' * 64,
                         'uuid': '12345678-1234-1234-1234-123456789abc'},
                         attempt=Path(self.temp.name) / 'fresh', available_bytes=80 * 1024**3,
                         source_clean=True)

    def test_no_execution_or_filesystem_mutation(self):
        result = recovery_plan(self.contract, **self.args)
        self.assertFalse(result['execution_allowed'])
        self.assertFalse(result['executor_implemented'])
        self.assertFalse(Path(self.args['attempt']).exists())
        self.assertEqual(result['restore_in_order'][-1]['action'],
                         'reverify_original_status_and_all_protected_hashes_and_UUIDs')

    def test_dirty_source_rejects(self):
        self.args['source_clean'] = False
        with self.assertRaises(ValueError): recovery_plan(self.contract, **self.args)

    def test_low_space_rejects(self):
        self.args['available_bytes'] = 64 * 1024**3
        with self.assertRaises(ValueError): recovery_plan(self.contract, **self.args)

    def test_overlap_rejects_after_normalization(self):
        self.args['attempt'] = self.root / 'build/macos-arm64-release/native/app/../../attempt'
        with self.assertRaises(ValueError): recovery_plan(self.contract, **self.args)

    def test_foreign_build_path_rejects(self):
        self.contract['canonical_build'] = str(self.root / 'build/approximate')
        with self.assertRaises(ValueError): recovery_plan(self.contract, **self.args)

    def test_stale_attempt_rejects(self):
        self.args['attempt'].mkdir()
        with self.assertRaises(ValueError): recovery_plan(self.contract, **self.args)

    def test_incomplete_identity_rejects(self):
        for key, value in [('source_commit', '1234'), ('expected', {'sha256': 'bad', 'uuid': 'bad'})]:
            args = copy.deepcopy(self.args); args[key] = value
            with self.assertRaises(ValueError): recovery_plan(self.contract, **args)

    def test_extracts_only_configure_argv(self):
        script = 'rm -rf forbidden\ncmake --preset gx \\\n -DFOO="$HOME/test space" > "$out/configure.log" 2>&1\nmake\n'
        self.assertEqual(configure_argv(script, '/home/a'),
                         ['cmake', '--preset', 'gx', '-DFOO=/home/a/test space'])

    def test_shell_substitution_rejects(self):
        for text in ['cmake --preset gx -DFOO=$(date)', 'cmake --preset gx -DFOO=$UNKNOWN']:
            with self.assertRaises(ValueError): configure_argv(text, '/home/a')

    def test_patch_unknown_syntax_or_input_rejects(self):
        with self.assertRaises(ValueError):
            patched_sdl(b'abc', 'src/joystick/SDL_gamepad.c',
                        {'apply-sdl3-android-nintendo-auto-mapping.cmake': 'changed syntax'})


class RestorePreflight(unittest.TestCase):
    def test_restore_rejects_unowned_and_changed_protection(self):
        from symbol_recovery import restore_preflight, digest
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp) / 'source'; root.mkdir()
            build = root / 'build/macos-arm64-release'
            protected = build.with_name(build.name + '.recovery-protected-test')
            protected.mkdir(parents=True); image = protected / 'image'; image.write_bytes(b'original')
            journal = {'schema': 1, 'kind': 'FROZEN_SYMBOL_RECOVERY',
                       'source_root': str(root), 'canonical_build': str(build),
                       'protected_build': str(protected), 'attempt_output': str(Path(tmp) / 'attempt'),
                       'attempt_commit': '1' * 40, 'original_head': '2' * 40,
                       'original_branch': 'wt/port', 'protected_files': {'image': digest(image)}}
            self.assertEqual(len(restore_preflight(journal, '1' * 40, True)), 4)
            self.assertTrue(protected.exists())
            with self.assertRaises(ValueError): restore_preflight(journal, '3' * 40, True)
            with self.assertRaises(ValueError): restore_preflight(journal, '1' * 40, False)
            image.write_bytes(b'changed')
            with self.assertRaises(ValueError): restore_preflight(journal, '1' * 40, True)


if __name__ == '__main__':
    unittest.main()
