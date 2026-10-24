#!/usr/bin/env python3
"""No-build capture intake: explicit frames only; malformed/unfinished observations fail closed."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from record_pipeline_deadlines import observations, disagreements, seed_identity


class Captures(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.path = Path(self.tmp.name) / 'capture.jsonl'
        self.rows = [{'event': 'header', 'version': 1, 'gx_config_version': 13,
                      'clock': 'fifo-renderer-frame-id', 'source': 'find_pipeline-before-memo'},
                     {'event': 'first_use', 'stage': 'A', 'visit': 1, 'origin_frame': 480,
                      'frame': 480, 'relative_frame': 0, 'config': '0000000000000001'},
                     {'event': 'end', 'complete': True, 'records': 1}]

    def tearDown(self):
        self.tmp.cleanup()

    def read(self):
        self.path.write_text(''.join(json.dumps(r) + '\n' for r in self.rows))
        return observations(self.path, 'A')

    def test_exact(self):
        self.assertEqual(self.read(), {'0000000000000001': 0})

    def test_missing_footer(self):
        self.rows.pop()
        with self.assertRaises(ValueError): self.read()

    def test_wrong_origin(self):
        self.rows[1]['relative_frame'] = 1
        with self.assertRaises(ValueError): self.read()

    def test_duplicate(self):
        self.rows.insert(2, dict(self.rows[1])); self.rows[-1]['records'] = 2
        with self.assertRaises(ValueError): self.read()

    def test_wrong_clock(self):
        self.rows[0]['clock'] = 'host-time'
        with self.assertRaises(ValueError): self.read()

    def test_incomplete(self):
        self.rows[-1]['complete'] = False
        with self.assertRaises(ValueError): self.read()

    def test_disagreement_retained(self):
        self.assertEqual(disagreements({'a': 0, 'b': 1}, {'a': 1, 'c': 0}),
                         {'a': [0, 1], 'b': [1, None], 'c': [None, 0]})

    def test_missing_stage_seed_keeps_recording_case(self):
        import sqlite3
        app = Path(self.tmp.name) / 'Petari.app'
        seeds = app / 'Contents/Resources/pipeline-seeds'
        seeds.mkdir(parents=True)
        with sqlite3.connect(seeds / '__global__.db') as db:
            db.execute('CREATE TABLE petari_cache_build(build_key TEXT)')
            db.execute('INSERT INTO petari_cache_build VALUES(?)', ('a' * 64,))
        result = seed_identity(app, 'StaffRollGalaxy')
        self.assertTrue(result['stage_seed_missing'])
        self.assertIsNone(result['stage_seed_sha256'])
        self.assertEqual(result['renderer_key'], 'a' * 64)
        self.assertEqual(result['identity_seed'], '__global__.db')
        self.assertFalse((seeds / 'StaffRollGalaxy.db').exists())


if __name__ == '__main__': unittest.main()
