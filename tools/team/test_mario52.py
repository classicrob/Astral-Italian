"""Synthetic ELF controls for the narrowly approved function-growth envelope."""
import struct
import sys
from pathlib import Path
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import mario52 as m


def fixture(path, after=False):
    names = ['.text'] * 9 + ['.rodata', '.rela.text', '.symtab', '.strtab', '.shstrtab']
    shstr = b'\0' + b''.join(n.encode() + b'\0' for n in dict.fromkeys(names))
    strings = b'\0' + m.SYMBOL.encode() + b'\0other\0'
    size = 268 if after else 260
    text = bytes(0x2200) + bytes([2 if after else 1]) * size + bytes(0x2310 - 0x2200 - size) + b'NEXT'
    symbols = bytes(16) + struct.pack('>IIIBBH', 1, 0x2200, size, 0x12, 0, 9)
    symbols += struct.pack('>IIIBBH', len(m.SYMBOL) + 2, 0x2310, 4, 0x12, 0, 9)
    relocations = struct.pack('>IIi', 0x2200, (2 << 8) | 10, 1 if after else 0)
    relocations += struct.pack('>IIi', 0x2310, (1 << 8) | 10, 0)
    blobs = [b''] + [b'TEXT'] * 8 + [text, b'POOL', relocations, symbols, strings, shstr]
    headers = [(0,) * 10]
    data = b''
    for index, (name, blob) in enumerate(zip(names, blobs[1:]), 1):
        offset = 0x34 + len(data)
        data += blob + bytes(-len(blob) % 4)
        typ, flags, link, info, align, entry = 1, 6 if index <= 9 else 2, 0, 0, 4, 0
        if name == '.rela.text':
            typ, flags, link, info, entry = 4, 0, 12, 9, 12
        elif name == '.symtab':
            typ, flags, link, info, entry = 2, 0, 13, 1, 16
        elif name in ('.strtab', '.shstrtab'):
            typ, flags, align = 3, 0, 1
        headers.append((shstr.index(name.encode() + b'\0'), typ, flags, 0, offset, len(blob), link, info, align, entry))
    shoff = 0x34 + len(data)
    header = b'\x7fELF\x01\x02\x01' + bytes(9)
    header += struct.pack('>HHIIIIIHHHHHH', 1, 20, 1, 0, 0, shoff, 0, 0x34, 0, 0, 40, len(headers), 14)
    path.write_bytes(header + data + b''.join(struct.pack('>10I', *row) for row in headers))
    return headers


class Mario52Tests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        self.before, self.after = self.root / 'before.o', self.root / 'after.o'
        fixture(self.before)
        self.headers = fixture(self.after, after=True)

    def tearDown(self):
        self.tmp.cleanup()

    def test_approved_in_place_growth_and_body_relocations(self):
        m.envelope(self.before, self.after)

    def test_four_actual_outside_envelope_controls(self):
        self.assertEqual(len(m.negative_controls(self.before, self.after)), 4)

    def test_remaining_padding_is_not_waived(self):
        data = bytearray(self.after.read_bytes())
        data[self.headers[9][4] + 0x230d] = 1
        self.after.write_bytes(data)
        with self.assertRaisesRegex(ValueError, 'padding'):
            m.envelope(self.before, self.after)

    def test_approved_symbol_visibility_not_waived(self):
        data = bytearray(self.after.read_bytes())
        data[self.headers[12][4] + 16 + 13] = 2
        self.after.write_bytes(data)
        with self.assertRaisesRegex(ValueError, 'outside-envelope'):
            m.envelope(self.before, self.after)

    def test_unreviewed_whole_object_rejected_before_envelope(self):
        with self.assertRaisesRegex(ValueError, 'unreviewed full object'):
            m.verify_objects(self.before, self.after)

    def test_private_reference_bytes_are_pinned(self):
        dol = self.root / 'main.dol'
        dol.write_bytes(b'not the private reference')
        with self.assertRaisesRegex(ValueError, 'DOL hash'):
            m.verify_reference(dol, self.before, self.after)


if __name__ == '__main__':
    unittest.main()
