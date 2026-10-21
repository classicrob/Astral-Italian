import hashlib
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from visual_window import capture_window


class FakeWindows:
    def __init__(self, executable):
        self.executable = str(executable.resolve())
        self.identities = 0
        self.permitted = True
        self.rows = [dict(pid=123, window_id=45, bounds=dict(X=0, Y=0, Width=32, Height=32))]
        self.changed = False

    def identity(self, pid):
        if pid != 123:
            raise AssertionError('unexpected process')
        self.identities += 1
        return dict(executable=self.executable,
                    start_time='changed' if self.changed and self.identities > 1 else 'same')

    def permission(self):
        return self.permitted

    def windows(self, pid):
        return self.rows


class WindowTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.executable = self.root / 'synthetic-executable'
        self.executable.write_bytes(b'synthetic executable, never launched')
        self.sha = hashlib.sha256(self.executable.read_bytes()).hexdigest()
        self.backend = FakeWindows(self.executable)
        self.request = dict(capture_source_frame=60, source_scene_epoch=0)

    def call(self, **kwargs):
        with patch('visual_window.private_path', side_effect=Path):
            return capture_window(123, self.executable, self.sha, self.root / 'image.png',
                                  self.request, backend=self.backend, **kwargs)

    def screenshot(self, command, **kwargs):
        self.assertEqual(command[:5], ['/usr/sbin/screencapture', '-x', '-o', '-l', '45'])
        Path(command[-1]).write_bytes(b'\x89PNG\r\n\x1a\n' + b'\0' * 8 + struct.pack('>II', 32, 24))
        return subprocess.CompletedProcess(command, 0, '', '')

    def test_owned_capture_keeps_presentation_unknown(self):
        with patch('visual_window.subprocess.run', side_effect=self.screenshot):
            receipt = self.call()
        self.assertEqual(receipt['dimensions'], [32, 24])
        self.assertEqual(receipt['status'], 'UNQUALIFIED_WINDOW_CAPTURE')
        self.assertIsNone(receipt['rendered_frame_id'])
        self.assertIsNone(receipt['render_fence_complete'])
        self.assertEqual(receipt['request'], self.request)

    def test_identity_permission_hash_ambiguity_fail_before_capture(self):
        for mode in ('identity', 'permission', 'hash', 'ambiguous', 'missing'):
            with self.subTest(mode=mode):
                self.backend = FakeWindows(self.executable)
                self.sha = hashlib.sha256(self.executable.read_bytes()).hexdigest()
                if mode == 'identity': self.backend.executable = '/another/executable'
                elif mode == 'permission': self.backend.permitted = False
                elif mode == 'hash': self.sha = '0' * 64
                elif mode == 'ambiguous': self.backend.rows *= 2
                elif mode == 'missing': self.backend.rows = []
                with patch('visual_window.subprocess.run') as run:
                    with self.assertRaises(RuntimeError): self.call()
                    run.assert_not_called()

    def test_explicit_window_id_cannot_select_other_process(self):
        with patch('visual_window.subprocess.run') as run:
            with self.assertRaises(RuntimeError): self.call(window_id=99)
            run.assert_not_called()

    def test_pid_reuse_or_capture_failure_retains_failure(self):
        self.backend.changed = True
        with patch('visual_window.subprocess.run', side_effect=self.screenshot):
            with self.assertRaisesRegex(RuntimeError, 'identity changed'): self.call()
        (self.root / 'image.png').unlink()
        self.backend.changed = False
        with patch('visual_window.subprocess.run', return_value=subprocess.CompletedProcess([], 1, '', 'failed')):
            with self.assertRaisesRegex(RuntimeError, 'capture failed'): self.call()


if __name__ == '__main__': unittest.main()
