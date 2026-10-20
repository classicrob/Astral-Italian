import unittest
from unittest.mock import patch

import dolphin_stage_entry as entry
from dolphin_collect import ObserverError


class Remote:
    def __init__(self):
        self.registers = dict.fromkeys(range(71), 0)
        self.registers.update({1: 0x81700000, 64: entry.ENTRY, 67: entry.ENTRY})
        self.saved = '00' * 400
        self.calls = []
        self.writes = []

    def read(self, address, size):
        if address == entry.STAGE_STRING:
            return b'RedBlueExGalaxy\0'
        return bytes(size)

    def command(self, packet):
        if packet.startswith('p'):
            number = int(packet[1:], 16)
            width = 16 if 32 <= number < 64 else 8
            return f'{self.registers[number]:0{width}x}'
        raise AssertionError(packet)

    def ok(self, packet):
        self.writes.append(packet)
        if packet.startswith('P'):
            register, value = packet[1:].split('=')
            self.registers[int(register, 16)] = int(value, 16)
        else:
            raise AssertionError(packet)

    def reg(self, number):
        return self.registers[number]

    def stop(self, packet):
        self.calls.append(dict(self.registers))
        self.registers[64] = entry.ENTRY


class Memory:
    def __init__(self, remote):
        pass

    def pointer(self, address):
        return address + 0x1000

    def string(self, address, limit):
        return 'user1'

    def boolean(self, address):
        return 0

    def u32(self, address):
        return 0x80251F00


class EntryTests(unittest.TestCase):
    def make_entry(self):
        with patch.object(entry, 'dol_function', side_effect=lambda path, address, size: bytes(size)):
            return entry.StageEntry('unused')

    def test_exact_three_calls_restore_registers_and_skip_original_destination(self):
        stage = self.make_entry()
        remote = Remote()
        with patch.object(entry.collector, 'Memory', Memory):
            stage.enter(remote, 999)
        self.assertEqual([call[64] for call in remote.calls], [entry.FUNCTIONS[name][0] for name in ('reset', 'request', 'set_in_stage')])
        self.assertEqual({key: remote.calls[1][key] for key in (3, 4, 5, 6)}, {3: entry.STAGE_STRING, 4: 1, 5: 1, 6: 0})
        self.assertEqual(remote.calls[2][4], entry.STAGE_STRING)
        self.assertEqual(remote.registers[64], entry.EPILOGUE)
        self.assertTrue(stage.receipt['destination_branch_skipped'])
        self.assertFalse(stage.receipt['direct_guest_memory_writes'])
        self.assertEqual(stage.receipt['source_frame'], 999)
        self.assertFalse(any(packet.startswith('M') for packet in remote.writes))
        with self.assertRaisesRegex(ObserverError, 'one-shot'):
            stage.enter(remote, 1000)

    def test_corrupt_file_rejected_before_calls(self):
        stage = self.make_entry()
        remote = Remote()
        with patch.object(entry.collector, 'Memory', Memory), patch.object(Memory, 'boolean', return_value=1):
            with self.assertRaisesRegex(ObserverError, 'file 1'):
                stage.enter(remote, 999)
        self.assertEqual(remote.calls, [])

    def test_retail_signature_must_match_before_calls(self):
        stage = self.make_entry()
        stage.expected['request'] = b'X'
        remote = Remote()
        with self.assertRaisesRegex(ObserverError, 'signature mismatch'):
            stage.enter(remote, 999)
        self.assertEqual(remote.calls, [])


if __name__ == '__main__':
    unittest.main()
