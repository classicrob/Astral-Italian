"""Synthetic stage-sync metadata checks; no game files or emulator needed."""
import contextlib
import csv
import io
import json
from pathlib import Path
import tempfile
import unittest

import tracediff as td
from sync_validation import validate_sync


class SyncValidationTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.root = Path(temp.name)
        self.paths = [self.root / name for name in ('a.csv', 'b.csv')]
        self.origin = dict(schema_version=1, phase='post-endFrame', stage='FixtureGalaxy', scenario=1,
                           source_frame=100, source_scene_epoch=4, source_ticks=None, source_vi_count=None,
                           rng_valid=True, rng_seed=19, rng_forced=False)
        self.policy = dict(tick_hz=60750000, rate_numerator=60000, rate_denominator=1001,
                           epoch_ticks=0, fpcr_fz=True, audio_output='off', audio_processing='virtual-ai-dsp')
        self.rows = [dict(schema_version='1', frame=str(frame), scene_epoch='0', scene_valid='1',
                          scene='Game', stage='FixtureGalaxy', scenario='1', init_state='6',
                          rng_valid='1', rng_seed='19', player_valid='0', x='100') for frame in range(2)]
        for path in self.paths:
            self.write_trace(path, self.rows)
            self.write_json(Path(str(path) + '.sync.json'), self.origin)
            self.write_json(Path(str(path) + '.manifest.json'), dict(timing_policy=self.policy))

    @staticmethod
    def write_json(path, data):
        path.write_text(json.dumps(data))

    @staticmethod
    def write_trace(path, rows):
        with path.open('w', newline='') as stream:
            writer = csv.DictWriter(stream, rows[0].keys())
            writer.writeheader()
            writer.writerows(rows)

    def check(self):
        return validate_sync(self.paths, [td.read_trace(path) for path in self.paths], require=True)

    def change_origin(self, **changes):
        self.write_json(Path(str(self.paths[1]) + '.sync.json'), dict(self.origin, **changes))

    def test_compatible_without_player_gate_and_different_source_origin(self):
        self.change_origin(source_frame=900, source_scene_epoch=9, source_ticks=1234, rng_forced=True)
        report = self.check()
        self.assertTrue(report['compatible'], report['errors'])
        self.assertEqual(report['sides']['b']['sync']['data']['source_ticks'], 1234)

    def test_missing_sidecar_is_not_a_legacy_comparison(self):
        Path(str(self.paths[0]) + '.sync.json').unlink()
        report = validate_sync(self.paths, [td.read_trace(path) for path in self.paths])
        self.assertFalse(report['compatible'])
        self.assertTrue(report['enabled'])

    def test_incompatible_phase_target_seed(self):
        for changes in (dict(phase='pre-loop'), dict(stage='OtherGalaxy'), dict(scenario=2), dict(rng_seed=22)):
            with self.subTest(changes=changes):
                self.change_origin(**changes)
                self.assertFalse(self.check()['compatible'])

    def test_strict_origin_types_and_observed_rng(self):
        for changes in (dict(source_frame=-1), dict(source_ticks=''), dict(rng_valid=1),
                        dict(rng_seed=2**32), dict(rng_forced='false'), dict(scenario=True)):
            with self.subTest(changes=changes):
                self.change_origin(**changes)
                self.assertFalse(self.check()['compatible'])

    def test_row_zero_must_match_declared_seam(self):
        for changes in (dict(scene_epoch='4'), dict(init_state='5'), dict(scene='Logo'),
                        dict(stage='OtherGalaxy'), dict(rng_seed='20'), dict(rng_valid='0')):
            with self.subTest(changes=changes):
                rows = [dict(row) for row in self.rows]
                rows[0].update(changes)
                self.write_trace(self.paths[1], rows)
                self.assertFalse(self.check()['compatible'])

    def test_absent_and_unknown_policies_are_unqualified(self):
        for metadata in ({}, dict(timing_policy=dict(self.policy, rate_numerator=None)),
                         dict(timing_policy=dict(self.policy, fpcr_fz=None))):
            with self.subTest(metadata=metadata):
                self.write_json(Path(str(self.paths[1]) + '.manifest.json'), metadata)
                self.assertFalse(self.check()['compatible'])

    def test_unrecorded_clock_epoch_is_unqualified(self):
        policy = {key: value for key, value in self.policy.items() if key != 'epoch_ticks'}
        self.write_json(Path(str(self.paths[1]) + '.manifest.json'), dict(timing_policy=policy))
        self.assertFalse(self.check()['compatible'])

    def test_policy_mismatches_fail_including_epoch(self):
        for changes in (dict(epoch_ticks=1), dict(rate_denominator=1000), dict(fpcr_fz=False),
                        dict(audio_output='on'), dict(audio_processing='Dolphin DSP HLE')):
            with self.subTest(changes=changes):
                self.write_json(Path(str(self.paths[1]) + '.manifest.json'), dict(timing_policy=dict(self.policy, **changes)))
                self.assertFalse(self.check()['compatible'])

    def test_basis_descriptions_are_retained_provenance(self):
        metadata = dict(timing_policy=dict(self.policy, tick_hz_basis='static source inspection'))
        self.write_json(Path(str(self.paths[1]) + '.manifest.json'), metadata)
        report = self.check()
        self.assertTrue(report['compatible'], report['errors'])
        self.assertIn('tick_hz_basis', report['sides']['b']['timing_policy'])

    def test_incomplete_manifest_rejected(self):
        self.write_json(Path(str(self.paths[1]) + '.manifest.json'), dict(status='incomplete', timing_policy=self.policy))
        self.assertFalse(self.check()['compatible'])

    def test_duplicate_json_keys_rejected(self):
        Path(str(self.paths[1]) + '.sync.json').write_text('{"rng_seed":19,"rng_seed":20}')
        self.assertTrue(any('duplicate' in error for error in self.check()['errors']))

    def test_parent_manifest_and_explicit_override(self):
        for path in self.paths:
            Path(str(path) + '.manifest.json').unlink()
        manifest = self.root / 'manifest.json'
        self.write_json(manifest, dict(timing_policy=self.policy))
        self.assertTrue(self.check()['compatible'])
        manifest.rename(self.root / 'explicit.json')
        report = validate_sync(self.paths, [td.read_trace(path) for path in self.paths],
                               require=True, manifests=(self.root / 'explicit.json', self.root / 'explicit.json'))
        self.assertTrue(report['compatible'], report['errors'])

    def test_cli_retains_state_diff_on_unqualified_metadata(self):
        self.write_json(Path(str(self.paths[1]) + '.manifest.json'), {})
        rows = [dict(row) for row in self.rows]
        rows[0]['x'] = '110'
        self.write_trace(self.paths[1], rows)
        output = io.StringIO()
        with contextlib.redirect_stdout(output):
            code = td.main([*[str(path) for path in self.paths], '--require-sync', '--json', '-'])
        report = json.loads(output.getvalue())
        self.assertEqual(code, 2)
        self.assertEqual(report['first_divergence']['frame'], 0)
        self.assertFalse(report['initial_non_rng_state']['equal_within_policy'])
        self.assertEqual(report['summary']['diverging_frames'], 1)

    def test_legacy_csvs_are_explicitly_not_sync_qualified(self):
        for path in self.paths:
            Path(str(path) + '.sync.json').unlink()
        report = validate_sync(self.paths, [td.read_trace(path) for path in self.paths])
        self.assertEqual(report['status'], 'not-requested')
        self.assertFalse(report['compatible'])
        self.assertFalse(report['errors'])


if __name__ == '__main__':
    unittest.main()
