import copy
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from kpad_records import RawSuffix, digest
from kpad_script import contract, ScriptedInput, RETURN
from dolphin_collect import ObserverError


class RawSuffixTests(unittest.TestCase):
    def make(self, root, frames=2):
        rows = []
        for frame in range(frames):
            for channel in range(4):
                sample = contract.idle_sample(channel == 0)
                if channel == 0:
                    sample.update(hold=1024, trig=0, release=2048)
                    sample['pos']['x'] = contract.floating(1.75)
                    sample['vec']['y'] = contract.floating(-0.0)
                    sample['acc_speed'] = contract.floating(0.125)
                rows.append(dict(frame=frame, read_ordinal=channel, channel=channel,
                                 requested_length=120, returned_count=1 if channel == 0 else 0,
                                 error=0 if channel == 0 else -1,
                                 connection=dict(probe_error=0 if channel == 0 else -1,
                                                 device_type=1 if channel == 0 else 253), samples=[sample]))
        path = root / 'input.jsonl'
        path.write_text(''.join(json.dumps(row) + '\n' for row in rows))
        return path, rows

    def test_preserves_history_and_bits_and_phase(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path, rows = self.make(root)
            suffix = RawSuffix(path, digest(path), 2, root, contract)
            suffix.start(201)
            for frame in range(2):
                for channel in range(4):
                    row = suffix.take(201 + frame, channel, channel, 120)
                    self.assertEqual(row, dict(rows[frame * 4 + channel], frame=201 + frame))
                    self.assertEqual(contract.encode_sample(row['samples'][0]),
                                     contract.encode_sample(rows[frame * 4 + channel]['samples'][0]))
                suffix.end_frame(201 + frame)
            suffix.close(True)
            self.assertEqual(suffix.receipt['status'], 'captured')

    def test_raw_injection_checks_actual_connection_and_writes_exact_sample(self):
        class Remote:
            def __init__(self): self.memory = {}; self.registers = {3: 9, 64: RETURN}
            def ok(self, command):
                if command.startswith('M'):
                    target, raw = command[1:].split(':')
                    self.memory[int(target.split(',')[0], 16)] = bytes.fromhex(raw)
                elif command.startswith('P3='): self.registers[3] = int(command[3:], 16)
            def reg(self, number): return self.registers[number]
            def read(self, address, size): return self.memory[address][:size]
            def stop(self, command): self.registers[64] += 4
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); path, rows = self.make(root, 1)
            suffix = RawSuffix(path, digest(path), 1, root, contract); suffix.start(201)
            script = root / 'boot.json'; script.write_text(json.dumps(dict(version=1, frames=300, segments=[])))
            inputs = ScriptedInput(script, 300, root / 'export.jsonl'); inputs.raw_suffix = suffix
            remote = Remote()
            for channel in range(4):
                remote.registers[64] = RETURN
                inputs.pending = (channel, 0x80001000, 120, 0x80002000)
                with patch('kpad_observe.read_connection', return_value=rows[channel]['connection']):
                    self.assertTrue(inputs.handle(remote, RETURN, 201))
                self.assertEqual(remote.read(0x80001000, 240), bytes(contract.encode_sample(rows[channel]['samples'][0])))
            inputs.end_frame(201); inputs.close(None, True)
            actual = list(contract.rows_from_jsonl(root / 'export.jsonl'))
            self.assertEqual(actual, [dict(row, frame=201) for row in rows])

    def test_connection_mismatch_fails_before_sample_write(self):
        class Remote:
            def ok(self, command): raise AssertionError('must not write mismatched controller data')
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); path, _ = self.make(root, 1)
            suffix = RawSuffix(path, digest(path), 1, root, contract); suffix.start(201)
            script = root / 'boot.json'; script.write_text(json.dumps(dict(version=1, frames=300, segments=[])))
            inputs = ScriptedInput(script, 300, root / 'export.jsonl'); inputs.raw_suffix = suffix
            inputs.pending = (0, 0x80001000, 120, 0x80002000)
            with patch('kpad_observe.read_connection', return_value=dict(probe_error=-1, device_type=253)):
                with self.assertRaises(ObserverError): inputs.handle(Remote(), RETURN, 201)
            inputs.close(None)
            self.assertFalse((root / 'export.jsonl').exists())

    def test_observed_connection_is_exported_without_changing_source_samples(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); path, rows = self.make(root, 1)
            suffix = RawSuffix(path, digest(path), 1, root, contract, observed_connection=True)
            suffix.start(201)
            original = copy.deepcopy(rows[0])
            row = suffix.take(201, 0, 0, 120)
            observed = dict(probe_error=-2, device_type=1)
            exported = suffix.connection(row, observed)
            self.assertEqual(exported['connection'], observed)
            self.assertEqual(exported['samples'], original['samples'])
            self.assertEqual(row['connection'], original['connection'])
            self.assertEqual(suffix.receipt['connection_difference_reads'], 1)
            self.assertEqual(suffix.receipt['first_connection_difference']['classification'],
                             'input-delivery boundary; not gameplay divergence')
            suffix.close(False)

    def test_missing_read_and_incomplete_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path, _ = self.make(root)
            suffix = RawSuffix(path, digest(path), 2, root, contract)
            suffix.start(10)
            with self.assertRaises(ObserverError):
                suffix.take(10, 0, 1, 120)
            with self.assertRaises(ObserverError):
                suffix.end_frame(10)
            with self.assertRaises(ObserverError):
                suffix.close(True)
            self.assertEqual(suffix.receipt['status'], 'incomplete')

    def test_changed_source_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path, _ = self.make(root, 1)
            suffix = RawSuffix(path, digest(path), 1, root, contract)
            suffix.start(1)
            for channel in range(4):
                suffix.take(1, channel, channel, 120)
            suffix.end_frame(1)
            path.write_text(path.read_text() + '\n')
            with self.assertRaises(ObserverError):
                suffix.close(True)

    def test_schema_validation_rejects_duplicate_and_bad_hash(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path, _ = self.make(root, 1)
            with self.assertRaises(ObserverError):
                RawSuffix(path, '0' * 64, 1, root, contract)
            path.write_text('{"frame":0,"frame":0}\n')
            with self.assertRaises(ValueError):
                RawSuffix(path, digest(path), 1, root, contract)


if __name__ == '__main__':
    unittest.main()
