import json
import tempfile
import unittest
from pathlib import Path
import first_cause_packet as packet


class PacketTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def records(self, records):
        p = self.root/'inputs.jsonl'
        p.write_text(''.join(json.dumps(r)+'\n' for r in records))
        return p

    def row(self, frame, ordinal=0):
        return dict(frame=frame, read_ordinal=ordinal, channel=ordinal,
                    returned_count=2, samples=[{'trig': 1}, {'release': 2}])

    def test_keeps_samples_predecessor_digest_and_byte_offsets(self):
        p = self.records([self.row(100), self.row(101), self.row(101, 1)])
        r = packet.read_history(p, 101, 101, 100)
        self.assertEqual(r['records'], 3)
        self.assertEqual(r['groups'][1]['reads'], 2)
        self.assertEqual(r['window'][0]['affected_trace_frame'], 1)
        self.assertEqual(r['window'][0]['input_frame'], 0)
        self.assertEqual(r['window'][0]['record']['samples'], self.row(101)['samples'])
        raw = p.read_bytes()
        import hashlib
        self.assertEqual(r['preceding_bytes_sha256'], hashlib.sha256(raw[:r['preceding_bytes_end']]).hexdigest())
        w = r['window'][1]
        self.assertEqual(json.loads(raw[w['byte_start']:w['byte_end']]), self.row(101, 1))

    def test_zero_return_retains_slot_zero_history(self):
        row = self.row(0); row['returned_count'] = 0; row['samples'] = [{'trig': 9}]
        r = packet.read_history(self.records([row]), 0, 0)
        self.assertEqual(r['window'][0]['record']['samples'], [{'trig': 9}])

    def test_rejects_missing_history(self):
        r = self.row(0); r['samples'] = []
        with self.assertRaisesRegex(ValueError, 'history count'):
            packet.read_history(self.records([r]), 0, 0)

    def test_rejects_duplicate_or_reordered_reads(self):
        for records in [[self.row(1), self.row(1)], [self.row(1), self.row(0)]]:
            with self.assertRaisesRegex(ValueError, 'order'):
                packet.read_history(self.records(records), 0, 2)

    def test_rejects_ordinal_gap(self):
        with self.assertRaisesRegex(ValueError, 'ordinal gap'):
            packet.read_history(self.records([self.row(0), self.row(0, 2)]), 0, 1)
        with self.assertRaisesRegex(ValueError, 'ordinal zero'):
            packet.read_history(self.records([self.row(0, 1)]), 0, 1)

    def test_trace_raw_bits_and_exclusion_are_distinct(self):
        n, d = self.root/'n.csv', self.root/'d.csv'
        n.write_text('frame,x,x_bits,pending_state_id\n0,0,0x00000000,\n1,0,0x00000000,\n')
        d.write_text('frame,x,x_bits,pending_state_id\n0,0,0x00000000,unknown\n1,-0,0x80000000,\n')
        r = packet.traces(n, d, {'pending_state_id'}, 1, 1)
        self.assertEqual(r['first_by_field']['x_bits']['frame'], 1)
        self.assertNotIn('pending_state_id', r['first_by_field'])

    def test_rejects_dense_frame_and_length_errors(self):
        n, d = self.root/'n.csv', self.root/'d.csv'
        n.write_text('frame\n0\n2\n'); d.write_text('frame\n0\n1\n')
        with self.assertRaisesRegex(ValueError, 'dense'):
            packet.traces(n, d, {}, 0, 2)
        n.write_text('frame\n0\n')
        with self.assertRaisesRegex(ValueError, 'lengths'):
            packet.traces(n, d, {}, 0, 2)

    def test_source_inventory_pins_commits_and_missing_blobs(self):
        import subprocess
        def git(*args):
            return subprocess.check_output(['git', '-C', str(self.root), *args], stderr=subprocess.DEVNULL)
        git('init', '-q')
        git('config', 'user.name', 'Fixture'); git('config', 'user.email', 'fixture@example.test')
        f = self.root/'native/src/mtx.cpp'; f.parent.mkdir(parents=True); f.write_text('old')
        git('add', '.'); git('commit', '-qm', 'old'); old = git('rev-parse', 'HEAD').decode().strip()
        f.write_text('new'); git('commit', '-qam', 'new'); new = git('rev-parse', 'HEAD').decode().strip()
        out = self.root/'out'; out.mkdir()
        r = packet.source_inventory(self.root, old, new, out)
        self.assertEqual(r['original'], old)
        self.assertIsNone(r['selected_blob_sha256'][old]['native/include/petari/ppc_math.hpp'])
        self.assertNotEqual(r['selected_blob_sha256'][old]['native/src/mtx.cpp'], r['selected_blob_sha256'][new]['native/src/mtx.cpp'])
        import gzip
        self.assertIn(b'-old', gzip.decompress((out/'source-difference.patch.gz').read_bytes()))

    def test_wrong_acceptance_hash_creates_no_packet(self):
        (self.root/'acceptance.json').write_text('{}')
        out = self.root/'packet'
        with self.assertRaisesRegex(ValueError, 'acceptance hash'):
            packet.seal(self.root, out, '0'*64)
        self.assertFalse(out.exists())


if __name__ == '__main__':
    unittest.main()
