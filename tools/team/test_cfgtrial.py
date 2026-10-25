#!/usr/bin/env python3
"""Preservation guards and private command rewriting for config archaeology."""
import copy
import json
from pathlib import Path
import tempfile
import threading
from types import SimpleNamespace
import unittest
from unittest import mock

import cfgtrial as c


class Scores(unittest.TestCase):
    def report(self):
        return {'units': [{'functions': [
            {'name': 'same', 'size': '16', 'fuzzy_match_percent': 99, 'metadata': {'virtual_address': '100'}},
            {'name': 'same', 'size': '24', 'fuzzy_match_percent': 100, 'metadata': {'virtual_address': '200'}}],
            'sections': [{'name': '.text', 'size': '40', 'fuzzy_match_percent': 99},
                         {'name': '.data', 'size': '4', 'fuzzy_match_percent': 100}],
            'measures': {'matched_code': '24', 'matched_data': '4', 'fuzzy_match_percent': 99.8}}]}

    def test_duplicate_name_drop_is_not_hidden_by_another_occurrence(self):
        before = c.report_scores(self.report()); report = self.report()
        report['units'][0]['functions'][0]['fuzzy_match_percent'] = 100
        report['units'][0]['functions'][1]['fuzzy_match_percent'] = 99
        result = c.compare(before, c.report_scores(report))
        self.assertEqual(len(before['functions']), 2)
        self.assertEqual((result['functions_better'], result['functions_worse']), (1, 1))
        self.assertFalse(result['strict_gain'])
        self.assertEqual(result['exact_functions_lost'], ['same@200#1'])

    def test_identical_name_address_occurrences_are_separate(self):
        report = self.report(); report['units'][0]['functions'].append(copy.deepcopy(report['units'][0]['functions'][0]))
        self.assertEqual(len(c.report_scores(report)['functions']), 3)

    def test_unit_and_data_drops_each_reject_function_improvement(self):
        before = c.report_scores(self.report())
        for kind in ('unit', 'data', 'matched_data'):
            after = copy.deepcopy(before); after['functions']['same@100#1']['score'] = 100
            if kind == 'unit': after['unit_fuzzy'] = 99
            if kind == 'data': after['sections']['.data@#1']['score'] = 99
            if kind == 'matched_data': after['matched_data'] = 0
            self.assertFalse(c.compare(before, after)['strict_gain'])

    def test_equal_is_not_gain_and_all_dimensions_survive(self):
        before = c.report_scores(self.report()); after = copy.deepcopy(before)
        self.assertFalse(c.compare(before, after)['strict_gain'])
        after['functions']['same@100#1']['score'] = 100; after['matched_code'] = 40
        result = c.compare(before, after)
        self.assertTrue(result['strict_gain']); self.assertEqual(result['matched_code_gain'], 16)


class Commands(unittest.TestCase):
    def test_variant_is_after_all_configured_flags_and_output_private(self):
        edge = dict(argv=['build/tools/wibo', 'build/compilers/Wii/1.0/mwcceppc.exe',
                          '-O4,s', '-inline', 'auto', '-c', 'src/Game/Test.cpp', '-o', 'build/shared'], compiler_index=1)
        original = copy.deepcopy(edge)
        argv = c.variant_argv(edge, {'argv': ['-inline', 'nobottomup']}, Path('/tmp/private'), force13=True)
        self.assertEqual(edge, original)
        self.assertEqual(argv[1], 'build/compilers/Wii/1.3/mwcceppc.exe')
        self.assertEqual(argv[-1], '/tmp/private')
        self.assertEqual(argv[argv.index('-c')-4:argv.index('-c')], ['-inline', 'nobottomup', '-w', 'illpragmas,cmdline'])

    def test_prefix_required_for_source_pragma(self):
        edge = dict(argv=['compiler', '-c', 'a.cpp', '-o', 'out'], compiler_index=0)
        with self.assertRaises(ValueError):
            c.variant_argv(edge, {'pragma': 'auto_inline off'}, Path('/tmp/out'))
        argv = c.variant_argv(edge, {'pragma': 'auto_inline off'}, Path('/tmp/out'), Path('/tmp/prefix.h'))
        self.assertIn('Z:\\tmp\\prefix.h', argv)

    def test_variants_have_unique_names(self):
        variants = c.variants()
        self.assertEqual(len(variants), len({v['name'] for v in variants}))


class Scheduling(unittest.TestCase):
    def exercise(self, parallel, jobs=2):
        with tempfile.TemporaryDirectory() as directory:
            out = Path(directory)
            units = [dict(unit=dict(name='unit'+str(i))) for i in range(6)]
            (out / 'manifest.json').write_text(json.dumps(dict(units=units, workers=4, variants=[])))
            (out / 'acceptance.json').write_text('[]')
            if parallel:
                (out / 'parallel-enabled').touch()
            guard = threading.Lock()
            barrier = threading.Barrier(jobs) if parallel else None
            active = peak = 0
            serial_values = []

            def compile_one(manifest, item, variant, serial):
                nonlocal active, peak
                with guard:
                    active += 1
                    peak = max(peak, active)
                    serial_values.append(serial)
                if barrier:
                    barrier.wait(timeout=5)
                with guard:
                    active -= 1
                return dict(unit=item['unit']['name'], status='compile_failed')

            args = SimpleNamespace(out=out, variants=None, limit=None, jobs=jobs)
            with mock.patch.object(c, 'check'), mock.patch.object(c, 'compile_one', compile_one), \
                 mock.patch.object(c, 'summarize'), mock.patch.object(c, 'retrial_pins'), \
                 mock.patch.object(c.subprocess, 'run', side_effect=AssertionError('unexpected process/lock')):
                c.run(args)
            rows = [json.loads(line) for line in (out / 'baseline.jsonl').read_text().splitlines()]
            self.assertEqual([row['unit'] for row in rows], [item['unit']['name'] for item in units])
            self.assertEqual(serial_values, [not parallel] * 6)
            return peak

    def test_parallel_limit_does_not_take_heavy_lock(self):
        self.assertEqual(self.exercise(True), 2)

    def test_quiet_mode_stays_serial(self):
        self.assertEqual(self.exercise(False, jobs=4), 1)


if __name__ == '__main__':
    unittest.main()
