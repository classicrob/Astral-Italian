import json
from pathlib import Path
import tempfile
import unittest
from yoshi_replay import prefix, verify_export


class DriverTests(unittest.TestCase):
    def test_prefix_keeps_original_bytes_and_predecessor_edges(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); source = root / 'full.jsonl'; target = root / 'prefix.jsonl'
            lines = [json.dumps(dict(frame=i, samples=[dict(trig=0, release=2048, pos=1.75)])) + '\n' for i in range(3)]
            source.write_text(''.join(lines)); prefix(source, target, 2)
            self.assertEqual(target.read_text(), ''.join(lines[:2]))

    def test_missing_final_receipt_never_accepted(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); capture = root / 'capture'; capture.mkdir()
            (capture / 'manifest.json').write_text(json.dumps(dict(status='incomplete', frames=17)))
            with self.assertRaises(RuntimeError): verify_export(root, root / 'source.jsonl', 16)


if __name__ == '__main__': unittest.main()
