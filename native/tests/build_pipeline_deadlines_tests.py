#!/usr/bin/env python3
import csv
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import build_pipeline_deadlines as builder
import pipeline_deadlines as deadlines
import seedpack


class Build(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(); self.root = Path(self.tmp.name)
        self.seeds = self.root / 'seeds'; self.seeds.mkdir()
        import sqlite3
        self.rows = [(1, 11, 13, 3, b'abc', 82), (1, 12, 13, 3, b'def', 83)]
        for name in ('__global__', 'A'):
            with sqlite3.connect(self.seeds / (name + '.db')) as db:
                db.executescript(seedpack.SCHEMA); db.execute('INSERT INTO aurora_schema VALUES(1)')
                db.executemany('INSERT INTO pipeline_cache VALUES(?,?,?,?,?,?)', self.rows)
        self.batch = self.root / 'batch'; self.batch.mkdir()
        self.arms = []
        for index in (1, 2):
            name = f'warm-{index}'; folder = self.batch / 'A' / name; folder.mkdir(parents=True)
            capture = [{'event': 'header', 'version': 1, 'gx_config_version': 13,
                        'clock': 'fifo-renderer-frame-id', 'source': 'find_pipeline-before-memo'}]
            capture += [{'event': 'first_use', 'stage': 'A', 'visit': 1, 'origin_frame': 480, 'frame': 480+n,
                         'relative_frame': n, 'config': f'{key:016x}'} for n, key in enumerate((11, 12))]
            capture += [{'event': 'end', 'complete': True, 'records': 2}]
            (folder / 'first-use.jsonl').write_text(''.join(json.dumps(r)+'\n' for r in capture))
            (folder / 'app.log').write_text('[input-script] done after 970 frames\n')
            (folder / 'frames.csv').write_text('frame,pipeline_wait_ms\n1,0.000\n')
            (folder / 'launch.json').write_text(json.dumps({'exe_sha256': '4'*64, 'renderer_key': '3'*64, 'environment': {}}))
            self.arms.append({'passed': True, 'name': name, 'recording_exe_sha256': '4'*64, 'renderer_key': '3'*64,
                              'checks': dict(valid_capture=True, no_pipeline_waits=True, no_pop_in=True)})
            self.rehash(index)
        self.data = {'version': 1, 'recording_only': True, 'stages_planned': ['A'],
                     'recording_build_metadata': {'source': '1'*40, 'binary_sha256': '2'*64, 'renderer_key': '3'*64},
                     'recording_build_metadata_sha256': 'unit-metadata', 'stage_results': [
                         {'stage': 'A', 'passed': True, 'warm_disagreements': {}, 'cold_disagreements': None, 'arms': self.arms}]}

    def tearDown(self): self.tmp.cleanup()

    def rehash(self, index):
        arm = self.arms[index-1]; folder = self.batch / 'A' / arm['name']
        for name, field in [('first-use.jsonl', 'capture_sha256'), ('app.log', 'app_log_sha256'), ('launch.json', 'launch_sha256')]:
            arm[field] = builder.sha(folder / name)

    def run_build(self, expected=None):
        path = self.batch / 'PROVENANCE.json'; path.write_text(json.dumps(self.data))
        return builder.build(self.seeds, [path], self.root / 'out', self.root / 'report.json', expected)

    def test_exact_pair(self):
        result = self.run_build(); self.assertTrue(result['passed'])
        self.assertEqual(deadlines.read(self.root / 'out/A.first-use.bin', 'A', self.rows), {11: 0, 12: 1})
        self.assertTrue(result['all_database_rows_unchanged'])

    def test_edited_raw(self):
        (self.batch / 'A/warm-2/app.log').write_text('changed')
        self.assertFalse(self.run_build()['passed'])

    def test_raw_wait_overrides_claim(self):
        (self.batch / 'A/warm-2/frames.csv').write_text('frame,pipeline_wait_ms\n1,0.001\n')
        self.assertFalse(self.run_build()['passed'])

    def test_disagreement_overrides_claim(self):
        path = self.batch / 'A/warm-2/first-use.jsonl'
        rows = [json.loads(line) for line in path.read_text().splitlines()]
        rows[2].update(frame=482, relative_frame=2)
        path.write_text(''.join(json.dumps(r)+'\n' for r in rows)); self.rehash(2)
        result = self.run_build(); self.assertFalse(result['passed'])
        self.assertTrue(result['stage_results'][0]['disagreements'])

    def test_unknown_config(self):
        for index in (1, 2):
            path = self.batch / f'A/warm-{index}/first-use.jsonl'
            path.write_text(path.read_text().replace('000000000000000c', '00000000000003e7')); self.rehash(index)
        self.assertFalse(self.run_build()['passed'])

    def test_partial_batch_cannot_pass_complete_sweep(self):
        result = self.run_build(['A', 'B'])
        self.assertFalse(result['passed'])
        self.assertEqual(result['unrecorded_stages'], ['B'])
        self.assertEqual(result['missing_or_failed_stages'], ['B'])

    def test_renderer_must_match_recording_build(self):
        self.data['recording_build_metadata']['renderer_key'] = '5'*64
        self.assertFalse(self.run_build()['passed'])


if __name__ == '__main__': unittest.main()
