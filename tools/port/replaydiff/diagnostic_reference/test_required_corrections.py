"""Pure controls for DR1/DR2; no child, emulator or host helper is invoked."""
import ast
import importlib.util
from pathlib import Path
import unittest

SOURCE = Path(__file__).with_name('stopped_reference.py')
spec = importlib.util.spec_from_file_location('reference_source', SOURCE)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class RequiredCorrections(unittest.TestCase):
    def test_current_ready_positive(self):
        row = dict(frame=616, scene_valid=1, scene='Game', stage='YosshiHomeGalaxy',
                   scenario=1, init_state=6, player_valid=1)
        module.require_current_ready(row)

    def test_current_observation_mismatches_refuse(self):
        row = dict(frame=616, scene_valid=1, scene='Game', stage='YosshiHomeGalaxy',
                   scenario=1, init_state=6, player_valid=1)
        for key, value in [('frame',615), ('scene_valid',0), ('scene','Intermission'),
                           ('stage','RedBlueExGalaxy'), ('scenario',2),
                           ('init_state',0), ('player_valid',0)]:
            with self.subTest(key=key), self.assertRaises(ValueError):
                module.require_current_ready(dict(row, **{key:value}))
        for key in row:
            missing = row.copy()
            del missing[key]
            with self.subTest(missing=key), self.assertRaises(ValueError):
                module.require_current_ready(missing)

    def test_main_status_propagates_without_running_main(self):
        tree = ast.parse(SOURCE.read_text())
        guard = tree.body[-1]
        self.assertIsInstance(guard, ast.If)
        self.assertIsInstance(guard.body[0], ast.Raise)
        fragment = ast.Module(body=guard.body, type_ignores=[])
        code = compile(ast.fix_missing_locations(fragment), '<entry-status-control>', 'exec')
        for status in (None,0,2):
            with self.subTest(status=status), self.assertRaises(SystemExit) as caught:
                exec(code, {'main':lambda:status, 'SystemExit':SystemExit})
            self.assertEqual(caught.exception.code,status)


if __name__ == '__main__':
    unittest.main()
