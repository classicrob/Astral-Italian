#!/usr/bin/env python3
import importlib.util
from pathlib import Path
import tempfile
import time
import unittest

spec = importlib.util.spec_from_file_location('startup542', Path(__file__).with_name('startup542.py'))
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)


class Startup(unittest.TestCase):
    def test_route_flush_bounds_contain_native_event_at_boundary(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); (root/'app.log').write_text('')
            (root/'route.csv').write_text('frame,stage,valid\n')
            observer = m.Observer(root, 'Yoshi'); observer.spawned(123)
            with (root/'route.csv').open('a') as out:
                out.writelines(f'{i},Yoshi,0\n' for i in range(60)); out.flush()
            time.sleep(.02)
            native = m.clocks()
            with (root/'app.log').open('a') as out:
                out.write(f"[native first valid] stage=Yoshi route_row=60 monotonic_ns={native['monotonic_ns'] + 10**15} wall_ns={native['wall_ns']}\n")
            with (root/'route.csv').open('a') as out:
                out.writelines(f'{i},Yoshi,1\n' for i in range(60, 120)); out.flush()
            with (root/'app.log').open('a') as out: out.write('[play ready] frame 121 wall_ms=1000\n')
            observer.thread.join(timeout=1); result = observer.finish()
            self.assertTrue(result['observer_joined'])
            self.assertTrue(result['native_first_valid_inside_external_bounds'])
            self.assertIn('not assumed equal', result['native_bound_validation_clock'])
            self.assertEqual(result['first_valid']['prior_flush_row_count'], 60)
            self.assertEqual(result['first_valid']['route_row'], 60)
            self.assertGreaterEqual(result['first_valid']['elapsed_ms_bounds'][1], result['first_valid']['elapsed_ms_bounds'][0])

    def test_title_shared_native_event(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp); (root/'app.log').write_text('')
            observer = m.Observer(root); observer.spawned(123)
            with (root/'app.log').open('a') as out: out.write('[gx uber async] event=title_first_draw phase=recording elapsed_ms=5\n')
            observer.thread.join(timeout=1); result = observer.finish()
            self.assertIn('title_first_draw', result)
            self.assertNotIn('first_valid', result)

    def test_missing_endpoint_is_not_invented(self):
        with tempfile.TemporaryDirectory() as tmp:
            observer = m.Observer(tmp, 'Yoshi'); result = observer.finish()
            self.assertNotIn('first_valid', result)
            self.assertNotIn('play_ready_observed', result)


if __name__ == '__main__': unittest.main()
