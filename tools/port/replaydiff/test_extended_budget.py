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

    def test_yoshi_scope_preserves_other_budgets(self):
        argv = ['dolphin_collect.py', 'collect', '--slot-held', '--output', '/tmp/unused',
                '--yoshi-replay-budget', '--wall-timeout', '10800', '--sync-stage', 'YosshiHomeGalaxy',
                '--sync-scenario', '1', '--sync-input-records', '/private/tape.jsonl',
                '--sync-input-records-sha256', '0' * 64, '--input-script', '/private/boot.json', '--frames', '30001']
        with patch.object(sys, 'argv', argv), patch.object(dolphin_collect, 'collect') as collect:
            self.assertEqual(dolphin_collect.main(), 0)
            self.assertEqual(collect.call_args.args[0].wall_timeout, 10800)
        for flag, value in (('--frames', '30002'), ('--wall-timeout', '10801'), ('--sync-stage', 'RedBlueExGalaxy')):
            changed = list(argv); changed[changed.index(flag) + 1] = value
            with patch.object(sys, 'argv', changed), contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                dolphin_collect.main()


if __name__ == '__main__':
    unittest.main()
