import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
from native_capture_png import run_fixture


class CheckedPngReuseGateTests(unittest.TestCase):
    def test_no_reuse_or_lifecycle_qualification(self):
        gate=run_fixture.reuse_gate()
        self.assertEqual(gate['status'],'FAIL_CLOSED')
        for field in ('spawned','lock_acquired','output_created','lifecycle_qualified'):
            self.assertIs(gate[field],False)
        self.assertEqual(gate['visual_qualification'],'BLOCKED')
        self.assertIsNone(gate['qualified_frames'])

    def test_cli_fails_before_any_spawn_lock_or_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/'not-created'
            stream=io.StringIO()
            with patch('subprocess.Popen',side_effect=AssertionError('unexpected spawn')), \
                 patch('fcntl.lockf',side_effect=AssertionError('unexpected lock')), \
                 patch('pathlib.Path.open',side_effect=AssertionError('unexpected file open')), \
                 contextlib.redirect_stdout(stream):
                self.assertEqual(run_fixture.main(['--output',str(path)]),2)
            self.assertFalse(path.exists())
            self.assertEqual(json.loads(stream.getvalue())['status'],'FAIL_CLOSED')

    def test_no_bypass_argument(self):
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):run_fixture.main(['--output','unused','--force'])


if __name__=='__main__':unittest.main()
