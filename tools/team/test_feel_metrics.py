"""Asset-free counter/coverage controls for source-only feel extraction."""
import copy
import json
from unittest import mock
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import feel_metrics as fm


class FeelTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(); self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.launch = {'source': 'a' * 40, 'binary_sha256': 'b' * 64, 'environment': {}}
        self.context = {'source': 'a' * 40, 'binary_sha256': 'b' * 64, 'case_id': 'fixture'}

    def file(self, name, text):
        path = self.root / name; path.write_text(text); return path

    def test_missing_channels_never_zero_pass(self):
        result = fm.extract(self.launch, self.context)
        self.assertEqual(result['status'], 'UNQUALIFIED_FEEL_DIAGNOSTIC')
        self.assertEqual(result['qualification'], 'BLOCKED')
        self.assertIsNone(result['measured_qualified_frames'])
        for channel in result['channels'].values():
            self.assertEqual(channel['status'], 'NOTMEASURED')
        self.assertEqual(result['conditions']['contamination']['status'], 'UNKNOWN')

    def test_startup_gameplay_separate_histograms_and_strict25(self):
        path = self.file('frames.csv', 'frame,phase,interval_ms,present_ms,unfocused,occluded,efb_captures\n1,startup,100,,0,0,0\n2,gameplay,25,20,0,0,1\n3,gameplay,25.001,,1,0,0\n')
        data = fm.frames(path)['phases']
        self.assertEqual(data['startup']['timings']['interval_ms']['over_25ms'], 1)
        self.assertEqual(data['gameplay']['timings']['interval_ms']['over_25ms'], 1)
        self.assertEqual(sum(x['count'] for x in data['gameplay']['timings']['interval_ms']['histogram']), 2)
        self.assertEqual(data['startup']['timings']['present_ms']['status'], 'NOTMEASURED')
        self.assertEqual(data['gameplay']['timings']['present_ms']['samples'], 1)
        self.assertEqual(data['gameplay']['unfocused']['sum'], 1)
        self.assertEqual(data['gameplay']['efb_captures']['sum'], 1)

    def test_unknown_phase_not_relabelled_gameplay(self):
        path = self.file('f.csv', 'frame,phase,interval_ms\n0,,10\n')
        self.assertEqual(set(fm.frames(path)['phases']), {'UNKNOWN'})

    def test_empty_csv_not_measured(self):
        self.assertEqual(fm.frames(self.file('f.csv', 'frame,phase,interval_ms\n'))['status'], 'NOTMEASURED')

    def test_bad_frame_data_rejects(self):
        for text in ['1,startup,NaN\n', '1,startup,-1\n', '1,startup,1\n1,gameplay,1\n', '2,gameplay,1\n1,gameplay,1\n']:
            with self.subTest(data=text):
                path = self.file('f.csv', 'frame,phase,interval_ms\n' + text)
                with self.assertRaises(ValueError): fm.frames(path)

    def test_duplicate_columns_and_bad_rows_reject(self):
        for text in ('frame,phase,interval_ms,interval_ms\n', 'frame,phase,interval_ms\n1,x,2,extra\n'):
            with self.assertRaises(ValueError): fm.frames(self.file('f.csv', text))

    def test_stale_context_identity_rejects(self):
        for key in ('source', 'binary_sha256'):
            context = copy.deepcopy(self.context); context[key] = 'stale'
            with self.assertRaisesRegex(ValueError, 'identity mismatch'):
                fm.extract(self.launch, context)

    def test_audio_interval_counters_not_cumulative(self):
        text = '[audio] level 5/10 frames, underrun 3 frames, AI replayed 2 blocks (since previous report, 1.00 s\n[audio] level 5/10 frames, underrun 4 frames, AI replayed 1 blocks (since previous report, 1.50 s\n[audio] warning: sink slow\n'
        a = fm.log_metrics(text, {}, {})['audio']
        self.assertEqual(a['underrun_frames'], 7)
        self.assertEqual(a['reported_seconds'], 2.5)
        self.assertEqual(a['warning_count'], 1)
        self.assertEqual(a['real_output'], 'NOTMEASURED')
        self.assertEqual(a['dsp_counter_status'], 'NOTMEASURED')

    def test_warning_only_is_not_zero_underruns(self):
        a = fm.log_metrics('[audio] error: device failed\n', {}, {})['audio']
        self.assertEqual(a['warning_count'], 1)
        self.assertIsNone(a['underrun_frames'])
        self.assertEqual(a['underrun_counter_status'], 'NOTMEASURED')

    def test_pipeline_compile_latency_is_not_fresh_stall(self):
        text = '[gx pipeline compile] key=abc queue_ms=40 build_ms=5 status=ready\n'
        c = fm.log_metrics(text, {}, {})
        self.assertEqual(c['pipeline']['compile_timings']['queue_ms']['over_25ms'], 1)
        self.assertEqual(c['fresh_shader_stalls']['status'], 'NOTMEASURED')
        self.assertIn('not a measured gameplay stall', c['pipeline']['interpretation'])

    def test_fresh_blocking_channel_requires_bound_inventory(self):
        text = '[gx pipeline] blocking resolve 26 ms\n[gx pipeline] blocking resolve 25 ms\n'
        context = {'cache': {'classification': 'fresh', 'verified': True, 'inventory_sha256': 'c' * 64}}
        c = fm.log_metrics(text, {}, context)['fresh_shader_stalls']
        self.assertEqual(c['blocking_resolve_events'], 2)
        self.assertEqual(c['blocking_ms_total'], 51)
        self.assertEqual(c['over_25ms'], 1)
        context['cache']['classification'] = 'warm'
        self.assertEqual(fm.log_metrics(text, {}, context)['fresh_shader_stalls']['status'], 'NOTMEASURED')

    def test_empty_pipeline_log_needs_complete_coverage_to_count_zero(self):
        context = {'cache': {'classification': 'fresh', 'verified': True, 'inventory_sha256': 'c' * 64}}
        env = {'PETARI_PIPELINE_DIAG': '1'}
        text = '[gx pipeline] policy=async\n'
        self.assertEqual(fm.log_metrics(text, env, context)['pipeline']['status'], 'NOTMEASURED')
        context['log_complete'] = True
        self.assertEqual(fm.log_metrics(text, env, context)['fresh_shader_stalls']['blocking_resolve_events'], 0)

    def test_diagnostic_overhead_and_contamination_are_not_erased(self):
        self.launch['environment'] = {'PETARI_FRAME_STATS': '1', 'PETARI_AUDIO_DIAG': '1'}
        self.context['contamination'] = {'status': 'CONTAMINATED', 'jobs': ['fixture']}
        result = fm.extract(self.launch, self.context)
        self.assertEqual(result['conditions']['contamination']['status'], 'CONTAMINATED')
        self.assertEqual(result['conditions']['diagnostic_overhead']['status'], 'NOTQUANTIFIED')
        self.assertIn('PETARI_FRAME_STATS', result['conditions']['diagnostic_overhead']['enabled'])
        self.assertEqual(result['qualification'], 'BLOCKED')

    def test_histogram_exhaustive_zero_and_high_samples(self):
        data = fm.distribution([0, 16, 25, 33, 50, 100, 1000])
        self.assertEqual(sum(x['count'] for x in data['histogram']), 7)
        self.assertEqual(data['over_25ms'], 4)

    def test_malformed_pipeline_number_rejects(self):
        with self.assertRaises(ValueError):
            fm.log_metrics('[gx pipeline compile] queue_ms=NaN status=ready\n', {}, {})

    def test_cli_refuses_input_mutation_without_output(self):
        launch = self.file('launch.json', json.dumps(self.launch))
        context = self.file('context.json', json.dumps(self.context))
        frames = self.file('f.csv', 'frame,phase,interval_ms\n1,startup,10\n')
        out = self.root / 'out.json'
        original = fm.extract
        def changed(*args):
            result = original(*args)
            frames.write_text('frame,phase,interval_ms\n1,startup,11\n')
            return result
        argv = ['feel_metrics.py', '--launch', str(launch), '--context', str(context), '--frames', str(frames), '--output', str(out)]
        with mock.patch.object(sys, 'argv', argv), mock.patch.object(fm, 'extract', changed):
            with self.assertRaisesRegex(ValueError, 'input changed'):
                fm.main()
        self.assertFalse(out.exists())

    def test_cli_never_overwrites_prior_extraction(self):
        out = self.file('out.json', 'preserve')
        argv = ['feel_metrics.py', '--launch', str(self.root / 'missing'), '--context', str(self.root / 'missing2'), '--output', str(out)]
        with mock.patch.object(sys, 'argv', argv):
            with self.assertRaisesRegex(ValueError, 'output exists'):
                fm.main()
        self.assertEqual(out.read_text(), 'preserve')


if __name__ == '__main__': unittest.main()
