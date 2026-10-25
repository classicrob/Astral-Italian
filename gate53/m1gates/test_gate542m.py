#!/usr/bin/env python3
import csv
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('gate542m', Path(__file__).with_name('gate542m.py'))
gate = importlib.util.module_from_spec(spec); spec.loader.exec_module(gate)


def event(name, frame, elapsed, complete=False, visible=False):
    return (f'[graphics preparation] event={name} version=1 frame={frame} clock=frame-csv-interval '
            f'source=ui-specialized-global elapsed_s={elapsed:.6f} polled=1 valid=1 '
            f'visible={int(visible)} complete={int(complete)} done={100 if complete else 10} total=100\n')


class Gate(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.root = Path(self.tmp.name)
        self.rows = [{'frame': n, 'phase': 'menu', 'interval_ms': 1000 / 60,
                      'presented_frame': max(0, n - 1), 'unfocused': '0'} for n in range(1, 2461)]
        self.text = event('initial', 1, 0) + event('shown', 2, .01, visible=True) + event('hidden', 601, 10)
        self.text += event('actual-global-100', 601, 10, complete=True)
        self.text += event('title-observation-complete', 2401, 40, complete=True)

    def tearDown(self): self.tmp.cleanup()

    def read(self, title=True, window=30, required=None):
        (self.root / 'app.log').write_text(self.text)
        with (self.root / 'frames.csv').open('w') as file:
            out = csv.DictWriter(file, fieldnames=self.rows[0].keys()); out.writeheader(); out.writerows(self.rows)
        return gate.score(self.root, title, window, required)

    def test_complete(self): self.assertTrue(self.read()['passed'])

    def test_historical(self):
        self.text = ''
        r = self.read(False)
        self.assertEqual(r['split_status'], 'split unscorable (no signal)')
        self.assertTrue(r['passed']) # only direct lines are applicable
        self.assertFalse(self.read(True)['passed'])

    def test_real100_required(self):
        self.text = self.text.replace('event=actual-global-100', 'event=tier_ready')
        self.assertFalse(self.read()['passed'])

    def test_false100_rejected(self):
        self.text = self.text.replace('done=100', 'done=99')
        self.assertFalse(self.read()['passed'])

    def test_250_all_phases(self):
        self.rows[10].update(phase='intro', interval_ms=250.001, unfocused='1')
        self.assertFalse(self.read(False)['checks']['no_non_gameplay_over250'])
        self.rows[10]['interval_ms'] = 250
        self.assertTrue(self.read(False)['checks']['no_non_gameplay_over250'])

    def test_post22_strict(self):
        for row in self.rows[601:]: row['interval_ms'] = 22.001
        self.assertFalse(self.read()['checks']['post100_p99_le22'])
        for row in self.rows[601:]: row['interval_ms'] = 22
        self.assertTrue(self.read()['checks']['post100_p99_le22'])

    def test_while_showing33_strict(self):
        for row in self.rows[1:600]: row['interval_ms'] = 33.001
        self.assertFalse(self.read()['checks']['showing_p99_le33'])
        for row in self.rows[1:600]: row['interval_ms'] = 33
        self.assertTrue(self.read()['checks']['showing_p99_le33'])

    def test_fixed_window_does_not_flatter_tail(self):
        a = self.read()['fixed_window']['stats']
        for n in range(2461, 12000): self.rows.append(dict(self.rows[-1], frame=n, interval_ms=1, presented_frame=n-1))
        b = self.read()['fixed_window']['stats']
        self.assertEqual(a, b)

    def test_missing_post(self):
        self.rows = self.rows[:600]
        r = self.read()
        self.assertFalse(r['passed']); self.assertEqual(r['split_status'], 'no post-100 samples')

    def test_required_deadline_admission_is_an_actual_gate(self):
        required={'FileSelect':69}
        self.assertFalse(self.read(required=required)['passed'])
        self.text+='[gx first-use deadlines] stage=FileSelect policy=frame-then-cost reason=validated configs=69\n'
        self.assertTrue(self.read(required=required)['passed'])
        self.text+='[gx first-use deadlines] stage=FileSelect policy=first-use-order reason=missing-unreadable configs=0\n'
        self.assertFalse(self.read(required=required)['passed'])

    def test_deadline_admission_fails_on_missing_log(self):
        self.assertFalse(gate.deadline_admissions('', {'FileSelect':69})['FileSelect']['passed'])

    def test_deadline_admission_requires_validated_policy_and_exact_count(self):
        good='[gx first-use deadlines] stage=FileSelect policy=frame-then-cost reason=validated configs=69'
        self.assertTrue(gate.deadline_admissions(good,{'FileSelect':69})['FileSelect']['passed'])
        for text in (good.replace('69','68'),good.replace('reason=validated','reason=missing-unreadable'),
                     good+'\n'+good.replace('frame-then-cost','first-use-order')):
            self.assertFalse(gate.deadline_admissions(text,{'FileSelect':69})['FileSelect']['passed'])

    def test_deadline_requirements_reject_ambiguous_spec(self):
        for spec in (['FileSelect:0'],['FileSelect:69','FileSelect:69'],['bad']):
            with self.assertRaises(ValueError):gate.deadline_requirements(spec)

    def test_timeout(self):
        self.text = self.text.replace('title-observation-complete', 'title-observation-timeout')
        self.assertFalse(self.read()['passed'])


if __name__ == '__main__': unittest.main()
