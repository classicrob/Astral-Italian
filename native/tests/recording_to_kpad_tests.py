#!/usr/bin/env python3
import csv
import importlib.util
import io
import json
from pathlib import Path
import struct
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('recording_to_kpad', ROOT/'native/tools/recording_to_kpad.py')
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)


def source_row(frame, **overrides):
    row = {key: '0' for key in m.REQUIRED | m.POINTER}
    row.update(frame=str(frame), stage='YosshiHomeGalaxy', scenario='1', valid='1',
               x='-500', y='-1140', z='7400')
    row.update({key: str(value) for key, value in overrides.items()})
    return row


class ConversionTests(unittest.TestCase):
    def test_shared_encoder_preserves_outside_pointer_and_shoot(self):
        rows = [source_row(0, shoot=1, pointer_valid=1, pointer_x=1.25, pointer_y=-1.5),
                source_row(1, shoot=1, pointer_valid=1, pointer_x=1.5, pointer_y=-1.75),
                source_row(2, shoot=0, pointer_valid=0)]
        records = list(m.input_rows(rows, 0, True))
        first, second = records[0]['samples'][0], records[4]['samples'][0]
        self.assertEqual((first['hold'], first['trig'], first['release']), (1024, 0, 0))
        self.assertEqual(first['pos']['x']['value'], 1.5)
        self.assertEqual(first['pos']['y']['value'], -1.75)
        self.assertEqual(first['vec']['x']['value'], .25)
        self.assertEqual(first['vec']['y']['value'], -.25)
        self.assertEqual(second['release'], 1024)
        self.assertEqual(second['dpd_valid_fg'], 0)
        out = io.BytesIO(); m.kpad.encode_rows(records, 2, out)
        self.assertEqual(out.getvalue()[:8], b'SMG2KPD1')
        self.assertEqual(len(records), 8)

    def test_button_press_at_seam(self):
        rows = [source_row(0), source_row(1, shoot=1, jump=1)]
        sample = next(m.input_rows(rows, 0, True))['samples'][0]
        self.assertEqual(sample['trig'], 1024 | 2048)

    def test_first_pointer_after_invalid_has_zero_derivative(self):
        sample = m.sample_for(source_row(1, pointer_valid=1, pointer_x=-2), source_row(0), True)
        self.assertEqual(sample['pos']['x']['value'], -2)
        self.assertEqual(sample['vec']['x']['value'], 0)

    def test_legacy_is_labeled_not_rejected(self):
        rows = [source_row(0), source_row(1)]
        for row in rows:
            for key in m.POINTER: del row[key]
        receipt = m.provenance(rows, False, 0, 'YosshiHomeGalaxy', 1, 1)
        self.assertFalse(receipt['pointer_shoot_columns_recorded'])
        self.assertIn('NO pointer/shoot recorded', receipt['captured'])
        sample = next(m.input_rows(rows, 0, False))['samples'][0]
        self.assertEqual((sample['hold'], sample['dpd_valid_fg']), (0, 0))

    def test_no_stage_transition_trimming_and_explicit_postroll(self):
        rows = [source_row(0), source_row(1, shoot=1), source_row(2, stage='MarioFaceShipGalaxy', scenario=2)]
        receipt = m.provenance(rows, True, 0, 'YosshiHomeGalaxy', 1, 3)
        self.assertEqual((receipt['recorded_input_frames'], receipt['explicit_neutral_postroll_frames']), (2, 1))
        self.assertEqual(len(list(m.input_rows(rows, 0, True, 3))), 12)
        with self.assertRaises(ValueError): list(m.input_rows(rows, 0, True, 1))

    def test_unrepresentable_pointer_flagged_no_clamp(self):
        rows = [source_row(0), source_row(1, pointer_valid=1, pointer_x='1e100')]
        receipt = m.provenance(rows, True, 0, 'YosshiHomeGalaxy', 1, 1)
        self.assertEqual(receipt['unrepresentable_pointer_source_frames'], [1])
        with self.assertRaises((ValueError, OverflowError, struct.error)): list(m.input_rows(rows, 0, True))

    def test_csv_strictness_and_complete_provenance(self):
        with tempfile.TemporaryDirectory() as tmp:
            tmp = Path(tmp); source = tmp/'record.csv'; rows = [source_row(0), source_row(1, shoot=1, pointer_valid=1, pointer_x=1.1)]
            with source.open('w') as f:
                writer = csv.DictWriter(f, fieldnames=list(rows[0])); writer.writeheader(); writer.writerows(rows)
            self.assertEqual(m.main(['--recording', str(source), '--output', str(tmp/'out'), '--stage', 'YosshiHomeGalaxy', '--scenario', '1']), 0)
            receipt = json.loads((tmp/'out/manifest.json').read_text())
            self.assertEqual(receipt['pointer_outside_unit_square_source_frames'], [1])
            self.assertEqual(receipt['source_shoot_frames'], 1)
            self.assertFalse(receipt['anchor_is_exact_ready_seam'])
            self.assertTrue(receipt['synthesized'])
            self.assertEqual(receipt['input_source_start'], 1)
            with self.assertRaises(FileExistsError): m.main(['--recording', str(source), '--output', str(tmp/'out'), '--stage', 'YosshiHomeGalaxy', '--scenario', '1'])
            rows[1]['shoot'] = '2'
            with source.open('w') as f:
                writer = csv.DictWriter(f, fieldnames=list(rows[0])); writer.writeheader(); writer.writerows(rows)
            with self.assertRaises(ValueError): m.checked_rows(source)


if __name__ == '__main__': unittest.main()
