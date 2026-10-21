"""Asset-free synthetic ELF rejection controls; no candidate proof claims."""
import copy
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dispatchproof as dp


def elf(path, names, values, sizes, *, text_change=False, data_change=False,
        extra_reference=False, crossed=False, visibility=0, addend=0):
    strings = b'\0'
    offsets = []
    for name in [*names, 'unrelated']:
        offsets.append(len(strings))
        strings += name.encode() + b'\0'
    symbols = bytes(16)
    for i, (offset, value, size) in enumerate(zip(offsets, [*values, 32], [*sizes, 4])):
        symbols += struct.pack('>IIIBBH', offset, value, size, 0x12, visibility if i == 0 else 0, 1)
    text = bytearray(range(64))
    data = bytearray(32)
    if text_change:
        text[40] ^= 1
    if data_change:
        data[0] = 1
    references = [(8, 2 if crossed else 1, addend), (24, 1 if crossed else 2, 0)]
    if extra_reference:
        references.append((28, 1, 0))
    rela = b''.join(struct.pack('>IIi', offset, symbol << 8 | 1, extra) for offset, symbol, extra in references)
    section_names = b'\0.text\0.data\0.rela.data\0.symtab\0.strtab\0.shstrtab\0'
    blobs = [b'', bytes(text), bytes(data), rela, symbols, strings, section_names]
    offsets, content = [], b''
    for blob in blobs:
        offsets.append(0x34 + len(content))
        content += blob
        content += bytes(-len(content) % 4)
    headers = [(0,) * 10]
    for index, (name, typ, flags, link, info, align, entsize) in enumerate([
        ('.text', 1, 6, 0, 0, 4, 0), ('.data', 1, 3, 0, 0, 4, 0),
        ('.rela.data', 4, 0, 4, 2, 4, 12), ('.symtab', 2, 0, 5, 1, 4, 16),
        ('.strtab', 3, 0, 0, 0, 1, 0), ('.shstrtab', 3, 0, 0, 0, 1, 0)], 1):
        headers.append((section_names.index(name.encode() + b'\0'), typ, flags, 0,
                        offsets[index], len(blobs[index]), link, info, align, entsize))
    header = b'\x7fELF\x01\x02\x01' + bytes(9) + struct.pack(
        '>HHIIIIIHHHHHH', 1, 20, 1, 0, 0, 0x34 + len(content), 0, 0x34, 0, 0, 40, 7, 6)
    path.write_bytes(header + content + b''.join(struct.pack('>10I', *h) for h in headers))


class DispatchTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.make_case('TU13')

    def make_case(self, scope):
        self.scope = dp.SCOPES[scope]
        self.names = list(self.scope['symbols'])
        self.unit = self.scope['object']
        tree = {key: 'a' * 40 for key in ('src', 'include', 'libs', 'config')}
        self.plan = {'schema': 1, 'scope': scope,
            'bindings': {'base': 'b' * 40, 'tip': 'c' * 40,
                'source_trees': {'before': tree, 'after': tree.copy()},
                'source_files': {p: {self.scope['source']: 'd' * 64} for p in ('before', 'after')},
                'config_files': {p: {'config/SB4E01/symbols.txt': 'e' * 64} for p in ('before', 'after')},
                'toolchain': {'compiler': 'f' * 64}, 'generators': {'generator': '1' * 64},
                'graphs': {'before': '2' * 64, 'after': '3' * 64}, 'dol_sha256': dp.DOL_SHA},
            'units': [self.unit], 'symbols': {},
            'references': [[self.unit, '.data#0', 8, 1, 0, 'sym', self.names[0]],
                           [self.unit, '.data#0', 24, 1, 0, 'sym', self.names[1]]],
            'symbol_map': {}}
        for i, name in enumerate(self.names):
            maps = self.scope['symbols'][name]
            self.plan['symbols'][name] = {
                'before': [18, 0, '.text#0', i * 16, maps[0][1]],
                'after': [18, 0, '.text#0', (1 - i) * 16, maps[1][1]]}
            self.plan['symbol_map'][name] = dict(zip(('before', 'after'), map(list, maps)))
        self.pin = dp.plan_hash(self.plan)
        self.before = self.root / 'before.o'
        self.after = self.root / 'after.o'
        self.build('before')
        self.build('after')

    def build(self, phase, **kwargs):
        records = [self.plan['symbols'][name][phase] for name in self.names]
        values = kwargs.pop('values', [r[3] for r in records])
        sizes = kwargs.pop('sizes', [r[4] for r in records])
        elf(self.root / (phase + '.o'), self.names, values, sizes, **kwargs)

    def compare(self, observed=None):
        return dp.classify_objects(dp.read_tree({self.unit: self.before}),
            dp.read_tree({self.unit: self.after}), self.plan, self.pin,
            self.plan['bindings'] if observed is None else observed)

    def test_tu13_classifies_only_pinned_symbol_changes(self):
        result = self.compare()
        self.assertEqual(result['status'], 'COMPONENT_COMPARISON_ONLY')
        self.assertFalse(result['production_acceptance'])

    def test_tu14_uses_separate_size_and_address_records(self):
        self.make_case('TU14')
        self.assertFalse(self.compare()['production_acceptance'])
        self.build('after', sizes=[12, 8])
        with self.assertRaisesRegex(ValueError, 'actual execute'):
            self.compare()

    def test_wrong_target(self):
        self.build('after', crossed=True)
        with self.assertRaisesRegex(ValueError, 'reference'):
            self.compare()

    def test_extra_relocation(self):
        self.build('after', extra_reference=True)
        with self.assertRaisesRegex(ValueError, 'reference'):
            self.compare()

    def test_relocation_addend(self):
        self.build('after', addend=4)
        with self.assertRaisesRegex(ValueError, 'reference'):
            self.compare()

    def test_data_change(self):
        self.build('after', data_change=True)
        with self.assertRaisesRegex(ValueError, 'remainder'):
            self.compare()

    def test_body_change(self):
        self.build('after', text_change=True)
        with self.assertRaisesRegex(ValueError, 'remainder'):
            self.compare()

    def test_wrong_object_symbol_address(self):
        self.build('after', values=[20, 0])
        with self.assertRaisesRegex(ValueError, 'actual execute'):
            self.compare()

    def test_wrong_object_symbol_size(self):
        self.build('after', sizes=[12, 8])
        with self.assertRaisesRegex(ValueError, 'actual execute'):
            self.compare()

    def test_visibility(self):
        self.build('after', visibility=2)
        with self.assertRaisesRegex(ValueError, 'actual execute'):
            self.compare()

    def test_extra_external_reference(self):
        before = dp.read_tree({self.unit: self.before})
        after = dp.read_tree({self.unit: self.after})
        self.plan['units'].append('external.o')
        self.pin = dp.plan_hash(self.plan)
        row = {'relocations for .text#0': ((0, 10, 0, ('sym', self.names[0])),)}
        before['external.o'] = after['external.o'] = row
        with self.assertRaisesRegex(ValueError, 'reference'):
            dp.classify_objects(before, after, self.plan, self.pin, self.plan['bindings'])

    def test_missing_object(self):
        with self.assertRaisesRegex(ValueError, 'object'):
            dp.classify_objects({}, {}, self.plan, self.pin, self.plan['bindings'])

    def test_stale_bindings(self):
        for key in sorted(dp.BINDINGS):
            with self.subTest(binding=key):
                actual = copy.deepcopy(self.plan['bindings'])
                actual[key] = 'stale'
                with self.assertRaisesRegex(ValueError, 'stale'):
                    self.compare(actual)

    def test_wrong_plan_pin(self):
        with self.assertRaisesRegex(ValueError, 'plan hash'):
            dp.validate_plan(self.plan, '0' * 64)

    def test_unlisted_scope(self):
        self.plan['scope'] = 'TU*'
        with self.assertRaisesRegex(ValueError, 'scope'):
            dp.validate_plan(self.plan, dp.plan_hash(self.plan))

    def test_plan_cannot_authorize_wrong_map(self):
        self.plan['symbol_map'][self.names[0]]['after'][0] += 4
        with self.assertRaisesRegex(ValueError, 'address/size'):
            dp.validate_plan(self.plan, dp.plan_hash(self.plan))

    def map_lines(self, phase):
        return ''.join(f'{name} = .text:0x{row[phase][0]:08X}; // type:function size:0x{row[phase][1]:X}\n'
                       for name, row in self.plan['symbol_map'].items()) + 'unrelated mapping\n'

    def test_exact_config_delta(self):
        dp.symbol_map_check(self.map_lines('before'), self.map_lines('after'), self.plan)

    def test_wrong_config_size(self):
        with self.assertRaisesRegex(ValueError, 'address/size'):
            dp.symbol_map_check(self.map_lines('before'), self.map_lines('after').replace('size:0x8', 'size:0xC', 1), self.plan)

    def test_other_config_line_change(self):
        with self.assertRaisesRegex(ValueError, 'config line'):
            dp.symbol_map_check(self.map_lines('before'), self.map_lines('after') + 'extra\n', self.plan)

    def test_bad_restoration_and_noop_control(self):
        a = self.after.read_bytes()
        b = bytearray(a); b[0x40] ^= 1
        dp.restoration_check(a, bytes(b), a)
        with self.assertRaisesRegex(ValueError, 'byte-identical'):
            dp.restoration_check(a, bytes(b), bytes(b))
        with self.assertRaisesRegex(ValueError, 'did not perturb'):
            dp.restoration_check(a, a, a)

    def test_original_dol_hash(self):
        with self.assertRaisesRegex(ValueError, 'DOL hash'):
            dp.audit_tu13_dol(bytes(256))

    def test_no_passing_placeholder(self):
        status = dp.production_status()
        self.assertEqual(status['status'], 'PENDING')
        self.assertFalse(status['production_acceptance'])
        self.assertGreaterEqual(len(status['pending']), 5)


class DecoderTests(unittest.TestCase):
    def test_ppc_branch(self):
        self.assertEqual(dp.branch_target(0x48000020, 0x1000), 0x1020)
        self.assertEqual(dp.branch_target(0x4bffffe0, 0x1000), 0xfe0)
        with self.assertRaises(ValueError):
            dp.branch_target(0x48000021, 0x1000)
        with self.assertRaises(ValueError):
            dp.branch_target(0x60000000, 0x1000)

    def synthetic_dol(self):
        data = bytearray(0x150)
        for index, offset, address, size in ((0, 0x100, 0x80001000, 0x40), (7, 0x140, 0x80002000, 0x10)):
            struct.pack_into('>I', data, index * 4, offset)
            struct.pack_into('>I', data, 0x48 + index * 4, address)
            struct.pack_into('>I', data, 0x90 + index * 4, size)
        struct.pack_into('>II', data, 0x100, 0x80630000, 0x48000020)
        struct.pack_into('>I', data, 0x148, 0x80001000)
        return data

    def test_physical_edge_and_wrong_target_without_hash_rejection(self):
        data = self.synthetic_dol()
        edges = [(0x80002000, 0x80001000, 0x80001024)]
        dp.physical_edges(data, edges)
        struct.pack_into('>I', data, 0x104, 0x48000024)
        with self.assertRaisesRegex(ValueError, 'wrapper target'):
            dp.physical_edges(data, edges)

    def test_wrong_owning_slot_without_hash_rejection(self):
        data = self.synthetic_dol()
        struct.pack_into('>I', data, 0x148, 0x80001010)
        with self.assertRaisesRegex(ValueError, 'owning slot'):
            dp.physical_edges(data, [(0x80002000, 0x80001000, 0x80001024)])

    def test_dol_bounds_and_aliases(self):
        data = bytearray(0x110)
        struct.pack_into('>I', data, 0, 0x100)
        struct.pack_into('>I', data, 0x48, 0x80000000)
        struct.pack_into('>I', data, 0x90, 16)
        self.assertEqual(dp.dol_read(data, 0x80000000, 4), bytes(4))
        with self.assertRaises(ValueError):
            dp.dol_read(data, 0x8000000e, 4)
        struct.pack_into('>I', data, 4, 0x100)
        struct.pack_into('>I', data, 0x4c, 0x80000000)
        struct.pack_into('>I', data, 0x94, 16)
        with self.assertRaises(ValueError):
            dp.dol_read(data, 0x80000000, 4)


if __name__ == '__main__':
    unittest.main()
