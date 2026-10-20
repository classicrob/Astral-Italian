import json
from pathlib import Path
import signal
from types import SimpleNamespace
import tempfile
import unittest
from unittest.mock import patch

import dolphin_collect as collector


class LifecycleTests(unittest.TestCase):
    def test_termination_and_budget_signals_stop_owned_dolphin(self):
        for sig in (signal.SIGTERM, signal.SIGALRM):
            with self.subTest(signal=sig), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                args = SimpleNamespace(output=str(root / 'capture'), socket=str(root / 'gdb.sock'),
                                       dolphin=str(root / 'Dolphin'), disc=str(root / 'disc.iso'),
                                       user=str(root / 'user'), frames=300, input_script=None,
                                       gui=False, movie=None, cpu_core=4, connect_timeout=1,
                                       stop_timeout=1, wall_timeout=1)
                collector.prepare(args)
                Path(args.disc).write_bytes(b'SB4E01')
                class Stream:
                    def connect(self, _): pass
                    def settimeout(self, _): pass
                    def setsockopt(self, *_): pass
                    def close(self): pass
                class Remote:
                    def __init__(self, _): pass
                    def stop(self, *_args, **_kwargs):
                        signal.raise_signal(sig)
                class Process:
                    pid = 123
                    stopped = False
                    waited = False
                    def terminate(self): self.stopped = True
                    def wait(self, timeout): self.waited = True
                process = Process()
                previous = signal.getsignal(sig)
                with patch.object(collector, 'sha256', return_value='synthetic'), \
                     patch.object(collector.subprocess, 'Popen', return_value=process), \
                     patch.object(collector.socket, 'socket', return_value=Stream()), \
                     patch.object(collector, 'RSP', Remote):
                    with self.assertRaises(collector.ObserverError):
                        collector.collect(args)
                self.assertTrue(process.stopped)
                self.assertTrue(process.waited)
                manifest = json.loads((root / 'capture/manifest.json').read_text())
                self.assertEqual(manifest['status'], 'incomplete')
                self.assertIn(sig.name, manifest['error'])
                self.assertEqual(signal.getsignal(sig), previous)
                self.assertEqual(signal.getitimer(signal.ITIMER_REAL)[0], 0)
                self.assertFalse((root / 'capture/oracle.csv').exists())


if __name__ == '__main__':
    unittest.main()
