import json
import io
from pathlib import Path
import tempfile
import unittest

from dolphin_collect import ObserverError
from kpad_script import ENTRY, RETURN, ScriptedInput, contract, expand, load_script


class ScriptTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.path = self.root / 'script.json'
        self.path.write_text(json.dumps(dict(version=1, frames=20, segments=[
            dict(start=2, end=5, buttons=['A'], pointer=[-.5, .25], stick=[0, 1])
        ])))
        self.input = ScriptedInput(self.path, 20, self.root / 'inputs.jsonl')

    def tearDown(self):
        self.input.close(None)
        self.tmp.cleanup()

    def test_edges_are_per_read_and_release_is_explicit(self):
        self.assertEqual(self.input.sample(1, 0)['hold'], 0)
        pressed = self.input.sample(2, 0)
        self.assertEqual((pressed['hold'], pressed['trig']), (2048, 2048))
        self.assertEqual(self.input.sample(2, 0)['trig'], 0)
        self.assertEqual(self.input.sample(5, 0)['release'], 2048)
        self.assertEqual(self.input.sample(5, 0)['release'], 0)

    def test_native_sample_encoder_preserves_pointer_and_nunchuk(self):
        sample = self.input.sample(2, 0)
        encoded = contract.encode_sample(sample)
        self.assertEqual(len(encoded), 240)
        self.assertEqual(encoded[32:40].hex(), 'bf0000003e800000')
        self.assertEqual(encoded[92:96].hex(), '01000205')
        self.assertEqual(encoded[96:104].hex(), '000000003f800000')
        self.assertEqual(self.input.sample(2, 1)['dev_type'], 253)

    def test_bad_script_cannot_silently_change_frame_contract(self):
        with self.assertRaises(ObserverError):
            load_script(self.path, 21)
        data = json.loads(self.path.read_text())
        data['segments'].append(dict(start=4, end=6))
        self.path.write_text(json.dumps(data))
        with self.assertRaises(ObserverError):
            load_script(self.path, 20)

    def test_public_script_expands_to_a_strict_native_tape(self):
        output = self.root / 'expanded.jsonl'
        expand(self.path, output)
        rows = list(contract.rows_from_jsonl(output))
        self.assertEqual(len(rows), 80)
        self.assertEqual([(row['read_ordinal'], row['channel']) for row in rows[:4]], [(i, i) for i in range(4)])
        self.assertEqual(rows[8]['samples'][0]['trig'], 2048)
        self.assertEqual(rows[12]['samples'][0]['trig'], 0)
        self.assertEqual(rows[20]['samples'][0]['release'], 2048)
        tape = io.BytesIO()
        contract.encode_rows(rows, 20, tape)
        self.assertEqual(len(tape.getvalue()), 24 + 80 * (contract.RECORD.size + 240))
        with self.assertRaises(ObserverError):
            expand(self.path, output)

    def test_live_edits_cannot_rewrite_executed_inputs(self):
        self.input.live = True
        self.input.sample(6, 0)
        data = json.loads(self.path.read_text())
        data['segments'].append(dict(start=10, end=12, buttons=['B']))
        self.path.write_text(json.dumps(data))
        self.input.modified = -1
        self.input.sample(7, 0)
        self.assertEqual(len(self.input.revisions), 1)
        data['segments'][0]['buttons'] = ['B']
        self.path.write_text(json.dumps(data))
        self.input.modified = -1
        with self.assertRaises(ObserverError):
            self.input.sample(8, 0)

    def test_hook_preserves_real_read_then_exports_native_contract(self):
        class Remote:
            def __init__(self):
                self.registers = {3: 0, 4: 0x80001000, 5: 120, 6: 0x80002000, 64: ENTRY, 67: RETURN}
                self.memory = {}
                self.commands = []

            def reg(self, number):
                return self.registers[number]

            def ok(self, command):
                self.commands.append(command)
                if command.startswith('M'):
                    location, data = command[1:].split(':')
                    address, length = (int(part, 16) for part in location.split(','))
                    raw = bytes.fromhex(data)
                    assert len(raw) == length
                    self.memory[address] = raw
                elif command.startswith('P3='):
                    self.registers[3] = int(command[3:], 16)

            def stop(self, command):
                assert command == 'c'
                self.registers[64] += 4

            def read(self, address, length):
                return self.memory[address][:length]

        remote = Remote()
        self.assertTrue(self.input.handle(remote, ENTRY, 2))
        self.assertEqual(self.input.reads, 0)
        self.assertIsNotNone(self.input.pending)
        self.assertFalse(any(command.startswith(('M', 'P')) for command in remote.commands))
        remote.registers[64] = RETURN
        remote.registers[3] = 9
        self.assertTrue(self.input.handle(remote, RETURN, 2))
        self.assertEqual(remote.reg(3), 1)
        self.input.file.flush()
        row = json.loads((self.root / 'inputs.jsonl.partial').read_text())
        self.assertEqual((row['frame'], row['read_ordinal'], row['channel']), (2, 0, 0))
        self.assertEqual(row['samples'][0]['trig'], 2048)
        tape = io.BytesIO()
        contract.encode_rows([row], 20, tape)
        self.assertTrue(tape.getvalue().startswith(b'SMG2KPD1'))
        self.input.end_frame()
        self.assertEqual(self.input.ordinal, 0)

    def test_write_is_bounded_to_mapped_ram_and_sample_or_error_width(self):
        class Remote:
            def ok(self, _):
                raise AssertionError('invalid write was sent')
        for address, data in ((0x817ffffc, bytes(240)), (0x80000000, bytes(8)), (0, bytes(4))):
            with self.assertRaises(ObserverError):
                self.input.write(Remote(), address, data)


if __name__ == '__main__':
    unittest.main()
