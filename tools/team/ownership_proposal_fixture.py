"""Author-invented byte fixture, materialized from the sealed declarative example.

No original DOL, retained reconstruction ELF or external fixture is read.
Metadata is written directly from explicit records, not the checker ELF decoder.
"""
import copy
import json
import struct
from pathlib import Path
import nervemap_ownership_proposal as checker

TEMPLATE = json.loads((Path(__file__).parent / 'testdata/ownership-proposal/association-template.json').read_bytes())


def walk(value):
    yield value
    if isinstance(value, dict):
        for child in value.values():
            yield from walk(child)
    elif isinstance(value, list):
        for child in value:
            yield from walk(child)


class Fixture:
    def __init__(self):
        self.envelope = copy.deepcopy(TEMPLATE)
        self.blobs = {}
        self.relation = r = self.envelope['relations'][0]
        self.context = c = self.envelope['context']
        self.put(c['tool'], checker.BASELINE_BYTES)
        self.put(c['design_spec'], checker.DESIGN_BYTES)
        self.elf = self.make_elf(r['reconstruction'])
        self.refresh_object()
        self.image = self.make_image()
        self.symbols = ('invented_ready_singleton = .sbss:0x81002000; // type:object size:0x4\n'
                        'invented_startup = .text:0x81001000; // type:function size:0xC\n'
                        'invented_sinit = .text:0x81001020; // type:function size:0x18\n'
                        'invented_ctor = .text:0x81001040; // type:function size:0x10\n'
                        'invented_vtable = .data:0x81003000; // type:object size:0x10\n'
                        'invented_execute = .text:0x81001060; // type:function size:0x8\n').encode()
        self.splits = b'Game/Invented.cpp:\n    .text start:0x81001000 end:0x81001100\n    .ctors start:0x81004000 end:0x81004004\n'
        self.put(c['full_splits'], self.splits)
        self.refresh_symbols()
        for node in walk(r):
            if isinstance(node, dict) and set(node) == {'path', 'sha256'} :
                self.put(node, self.blobs.get(node['path'], b'{}'))
        self.startup = copy.deepcopy(r['original_initializer']['initializer'])
        self.startup.update(address='0x81001000', size=12)
        self.refresh_image()
        self.source_context()

    def put(self, pin, data):
        pin['sha256'] = checker.digest(data)
        self.blobs[pin['path']] = bytes(data)
        return pin

    def put_json(self, pin, value):
        return self.put(pin, checker.canonical(value))

    def refresh_object(self):
        r = self.relation['reconstruction']
        old = r['object']['sha256']
        self.put(r['object'], self.elf)
        for node in walk(r):
            if isinstance(node, dict) and node.get('object_sha256') == old:
                node['object_sha256'] = r['object']['sha256']

    def refresh_symbols(self):
        self.put(self.context['full_symbols'], self.symbols)
        self.relation['mapped_record']['map_sha256'] = self.context['full_symbols']['sha256']

    def refresh_image(self):
        self.put(self.context['original_image'], self.image)
        claim = self.relation['original_initializer']
        claim['image_sha256'] = self.context['original_image']['sha256']
        for span in [claim[k] for k in ('initializer', 'constructor', 'ctors_entry', 'vtable_observation')] + [self.startup]:
            addr, size = int(span['address'], 16), span['size']
            off = 0x100 + addr - 0x81001000 if addr < 0x81003000 else 0x200 + addr - 0x81003000 if addr < 0x81004000 else 0x220 + addr - 0x81004000
            span['sha256'] = checker.digest(self.image[off:off + size])
            span['image_sha256'] = claim['image_sha256']
        self.put_json(claim['register_and_cfg_witness'], dict(schema=1, kind='synthetic-initializer-cfg',
                      startup_chain=[self.startup], initializer_invocation_domain='registered_ctor_table_with_derived_startup_bases'))
        for role in self.relation['unit_roles']:
            if role['role'] == 'original_initializer_contribution':
                role['evidence'] = copy.deepcopy(claim['register_and_cfg_witness'])

    def source_context(self):
        r, c = self.relation['reconstruction'], self.context
        binding = self.relation['source_binding']
        source_pins = []
        header = b'namespace NrvInvented {\nstruct Ready {}; }\n'
        source = b'// invented source context only\n' * 9 + b'namespace NrvInvented {\nReady ready; }\n'
        for row in [binding['declaration'], binding['static_definition']] + binding['relevant_headers']:
            self.put(row, source if row['path'].startswith('src/') else header)
            self.put_json(row['preprocessing_witness'], dict(schema=1, kind='synthetic-source-anchor',
                          source_sha256=row['sha256'], **{k: row[k] for k in ('line', 'end_line', 'role', 'qualified_name', 'declared_type')}))
            pin = {k: row[k] for k in ('path', 'sha256')}
            if pin not in source_pins:
                source_pins.append(pin)
        self.put_json(r['build_recipe'], dict(schema=1, kind='synthetic-reconstruction-context',
                      object_sha256=r['object']['sha256'], original_selection_grant=False, source_pins=source_pins,
                      **{k: r[k] for k in ('source_commit', 'source_tree', 'source_unit_path', 'logical_split_unit')}))
        self.put_json(r['audited_source_bridge'], dict(schema=1, kind='synthetic-source-context-bridge',
                      reconstructed_source_commit=r['source_commit'], reconstructed_source_tree=r['source_tree'],
                      audited_source_commit=c['audited_source_commit'], audited_source_tree=c['audited_source_tree'], source_pins=source_pins))

    def check(self):
        evidence = checker.SyntheticEvidence(self.blobs)
        return checker.check_proposal(self.envelope, evidence)

    @staticmethod
    def make_image():
        raw = bytearray(0x224)
        for index, off, addr, size in ((0, 0x100, 0x81001000, 0x100), (7, 0x200, 0x81003000, 0x20), (8, 0x220, 0x81004000, 4)):
            for base, value in ((0, off), (0x48, addr), (0x90, size)):
                struct.pack_into('>I', raw, base + index * 4, value)
        struct.pack_into('>III', raw, 0xd8, 0x81002000, 0x100, 0x81001000)
        for off, words in ((0x100, [0x3da08100, 0x39ad2000, 0x4e800020]),
                           (0x120, [0x386d0000, 0x4800001d, 0x4e800020, 0x60000000, 0x60000000, 0x60000000]),
                           (0x140, [0x3d808100, 0x398c3000, 0x91830000, 0x4e800020]),
                           (0x160, [0x4e800020, 0x60000000]),
                           (0x200, [0, 0, 0x81001060, 0]), (0x220, [0x81001020])):
            struct.pack_into('>' + 'I' * len(words), raw, off, *words)
        return raw

    @staticmethod
    def make_elf(reconstruction):
        sections = {}
        symbols = {}
        for node in walk(reconstruction):
            if isinstance(node, dict) and 'index' in node and 'occurrence' in node:
                sections[node['index']] = node
            if isinstance(node, dict) and 'symbol_index' in node:
                symbols[node['symbol_index']] = node
        raw = bytearray(5376 + 27 * 40)
        raw[:16] = b'\x7fELF\x01\x02\x01' + bytes(9)
        struct.pack_into('>HHIIIIIHHHHHH', raw, 16, 1, 20, 1, 0, 0, 5376, 0, 52, 0, 0, 40, 27, 26)
        for index in range(4, 128):
            struct.pack_into('>IIIBBH', raw, 1024 + index * 16, 0, 0, 0, 0x10, 0, 0)
        for index, sym in symbols.items():
            struct.pack_into('>IIIBBH', raw, 1024 + index * 16, sym['string_name_offset'], sym['value'], sym['size'],
                             sym['binding'] * 16 + sym['type'], sym['other'], sym['section_index'])
            text = sym['name'].encode() + b'\0'
            raw[3072 + sym['string_name_offset']:3072 + sym['string_name_offset'] + len(text)] = text
        for relocation in reconstruction['relocations']:
            struct.pack_into('>IIi', raw, relocation['entry_file_offset'], relocation['source_offset'],
                             relocation['raw_r_info'], relocation['explicit_signed_addend'])
        for index, sec in sections.items():
            text = sec['name'].encode() + b'\0'
            raw[4352 + sec['name_offset']:4352 + sec['name_offset'] + len(text)] = text
            struct.pack_into('>10I', raw, 5376 + index * 40, *(sec[k] for k in
                             ('name_offset', 'type', 'flags', 'address', 'file_offset', 'size', 'sh_link', 'sh_info', 'alignment', 'entry_size')))
        raw[4352 + 26 * 32:4352 + 26 * 32 + 10] = b'.shstrtab\0'
        struct.pack_into('>10I', raw, 5376 + 26 * 40, 26 * 32, 3, 0, 0, 4352, 1024, 0, 0, 1, 0)
        return raw
