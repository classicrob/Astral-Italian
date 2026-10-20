import contextlib
import io
import sys
import unittest
from unittest.mock import patch
import dolphin_collect


class ExtendedBudgetTests(unittest.TestCase):
    def invoke(self, timeout, extended=False):
        argv = ['dolphin_collect.py', 'collect', '--slot-held', '--output', '/tmp/unused',
                '--wall-timeout', str(timeout)] + (['--extended-wall-budget'] if extended else [])
        with patch.object(sys, 'argv', argv), patch.object(dolphin_collect, 'collect') as collect:
            dolphin_collect.main()
            return collect.call_args.args[0]

    def test_default_stays_480(self):
        self.assertEqual(self.invoke(480).wall_timeout, 480)
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.invoke(481)

    def test_explicit_extended_boundary(self):
        self.assertEqual(self.invoke(1500, True).wall_timeout, 1500)
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.invoke(1501, True)


if __name__ == '__main__':
    unittest.main()
