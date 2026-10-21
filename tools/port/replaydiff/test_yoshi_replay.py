import json
from pathlib import Path
import tempfile
import unittest
from yoshi_replay import prefix, verify_export, bind_actual_suffix
from kpad_script import contract


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

    def test_connection_mismatch_has_delivery_classification_and_preserved_sample_bits(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); source = root / 'source.jsonl'
            sample = contract.idle_sample(True); sample['pos']['x'] = contract.floating(1.75)
            wanted = dict(frame=0, read_ordinal=0, channel=0, requested_length=120, returned_count=1, error=0,
                          connection=dict(probe_error=0, device_type=1), samples=[sample])
            actual = dict(wanted, frame=101, connection=dict(probe_error=-2, device_type=1))
            source.write_text(json.dumps(wanted) + '\n')
            (root / 'inputs.jsonl').write_text(json.dumps(actual) + '\n')
            with self.assertRaisesRegex(RuntimeError, 'input-delivery boundary'):
                bind_actual_suffix(root, source, 100, 1, False)
            (root / 'observed-inputs.jsonl.partial').unlink()
            result = bind_actual_suffix(root, source, 100, 1, True)
            self.assertEqual(result['connection_difference_reads'], 1)
            self.assertEqual(result['preconverted_native_comparison_through_trace_frame'], 0)
            exported = list(contract.rows_from_jsonl(Path(result['path'])))
            self.assertEqual(exported[0]['samples'], wanted['samples'])
            self.assertEqual(exported[0]['connection'], actual['connection'])
            self.assertEqual(result['first_connection_difference']['first_affected_trace_frame'], 1)

    def test_observed_mode_never_accepts_changed_sample_or_history(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); source = root / 'source.jsonl'
            wanted = dict(frame=0, read_ordinal=0, channel=0, requested_length=120, returned_count=1, error=0,
                          connection=dict(probe_error=0, device_type=1), samples=[contract.idle_sample(True)])
            source.write_text(json.dumps(wanted) + '\n')
            actual = json.loads(json.dumps(wanted)); actual['frame'] = 101; actual['samples'][0]['trig'] = 2048
            (root / 'inputs.jsonl').write_text(json.dumps(actual) + '\n')
            with self.assertRaisesRegex(RuntimeError, 'sample/read/history changed'):
                bind_actual_suffix(root, source, 100, 1, True)


if __name__ == '__main__': unittest.main()
