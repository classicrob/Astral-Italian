"""Disabled, synthetic-only association checker; never supplies an effective owner.

Standalone callable and invented controls; independent exact-checkpoint review required.
The sealed design defines the claim shape. This module checks consistency against
explicit in-memory invented evidence. It grants no original allocation/selection,
sourceclass, API, dispatch, lifetime, live integration or sweep acceptance.
"""
from __future__ import annotations
import argparse
import base64
import collections
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

import nervemap as baseline

HERE = Path(__file__).parent
SCHEMA_BYTES = (HERE / 'nervemap_ownership_proposal.schema.json').read_bytes()
DESIGN_BYTES = (HERE / 'nervemap-ownership-design.md').read_bytes()
BASELINE_BYTES = Path(baseline.__file__).read_bytes()
SCHEMA_SHA = hashlib.sha256(SCHEMA_BYTES).hexdigest()
DESIGN_SHA = hashlib.sha256(DESIGN_BYTES).hexdigest()
BASELINE_SHA = hashlib.sha256(BASELINE_BYTES).hexdigest()
CHECKER_SHA = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
SCHEMA = json.loads(SCHEMA_BYTES)
assert SCHEMA_SHA == '954903ba6045c7c457930b9ad77ef41b344e00a550b097dd6be354392ecd872f'
assert BASELINE_SHA == '4df422b232f9fe23984ca952cb7fd0bbd12672a40fb984a9e5a44e0bf7b852bc'
OBLIGATIONS = dict(original_allocation='UNKNOWN', selected_original_weak_provider='UNKNOWN',
                   unassigned_tail='UNKNOWN', sourceclass='NOT_GRANTED', API='NOT_GRANTED', lifetime='NOT_GRANTED')


class Refused(ValueError):
    pass


def require(condition, reason):
    if not condition:
        raise Refused(reason)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False, allow_nan=False).encode()


def parse_json(data):
    def unique(pairs):
        result = {}
        for key, value in pairs:
            require(key not in result, 'duplicate_JSON_key:' + key)
            result[key] = value
        return result
    def no_float(value):
        raise Refused('noninteger_JSON_number')
    return json.loads(data, object_pairs_hook=unique, parse_float=no_float, parse_constant=no_float)


class SyntheticEvidence:
    """Opaque IDs only. Pin paths are never passed to any filesystem API."""
    def __init__(self, blobs):
        require(isinstance(blobs, dict), 'evidence_requires_dict')
        require(all(type(k) is str and type(v) is bytes for k, v in blobs.items()), 'evidence_requires_byte_blobs')
        self.blobs = dict(blobs)
        self.reads = []

    def read(self, pin):
        require(type(pin) is dict and set(pin) == {'path', 'sha256'}, 'invalid_evidence_pin')
        require(type(pin['path']) is str and type(pin['sha256']) is str, 'invalid_pin_types')
        self.reads.append(pin['path'])
        require(pin['path'] in self.blobs, 'missing_blob:' + pin['path'])
        value = self.blobs[pin['path']]
        require(len(value) <= 16 * 1024 * 1024, 'synthetic_blob_limit')
        require(digest(value) == pin['sha256'], 'blob_hash_mismatch:' + pin['path'])
        return value

    def json(self, pin):
        return parse_json(self.read(pin))


def shape(value, rule, path='$'):
    """Only the closed vocabulary used by the exact sealed schema, no new deps."""
    if '$ref' in rule:
        return shape(value, SCHEMA['$defs'][rule['$ref'][8:]], path)
    if 'oneOf' in rule:
        errors = [shape(value, child, path) for child in rule['oneOf']]
        return [] if sum(not e for e in errors) == 1 else [path + ':oneOf']
    errors = []
    types = {'object': dict, 'array': list, 'string': str, 'integer': int, 'boolean': bool, 'null': type(None)}
    if 'type' in rule and type(value) is not types[rule['type']]:
        return [path + ':type_' + rule['type']]
    if 'const' in rule and (type(value) is not type(rule['const']) or value != rule['const']):
        errors.append(path + ':const')
    if 'enum' in rule and not any(type(value) is type(x) and value == x for x in rule['enum']):
        errors.append(path + ':enum')
    if type(value) is dict and rule.get('type') == 'object':
        errors += [path + '/' + k + ':required' for k in rule.get('required', []) if k not in value]
        props = rule.get('properties', {})
        for key, child in value.items():
            if key not in props:
                if rule.get('additionalProperties') is False:
                    errors.append(path + '/' + str(key) + ':unexpected')
            else:
                errors.extend(shape(child, props[key], path + '/' + key))
    if type(value) is list:
        if len(value) < rule.get('minItems', 0):
            errors.append(path + ':minItems')
        for index, child in enumerate(value):
            errors.extend(shape(child, rule['items'], path + '/' + str(index)))
    if type(value) is str:
        if len(value) < rule.get('minLength', 0) or ('pattern' in rule and not re.search(rule['pattern'], value)):
            errors.append(path + ':string_constraint')
    if type(value) is int:
        if ('minimum' in rule and value < rule['minimum']) or ('maximum' in rule and value > rule['maximum']):
            errors.append(path + ':number_constraint')
    return errors


def interval(start, size, bound=0x100000000):
    require(type(start) is int and type(size) is int and start >= 0 and size > 0 and start + size <= bound,
            'invalid_or_overflowing_extent')
    return start, start + size


def address(text):
    return int(text, 16)


class Elf:
    """ELF32BE metadata decoder. Every table/entry occurrence stays distinct."""
    def __init__(self, data):
        self.data, self.sha = data, digest(data)
        require(len(data) >= 52 and data[:7] == b'\x7fELF\x01\x02\x01', 'ELF32BE_header')
        fields = struct.unpack_from('>HHIIIIIHHHHHH', data, 16)
        typ, machine, version, _, _, shoff, flags, ehsize, _, _, shsize, count, names = fields
        require((typ, machine, version, ehsize, shsize) == (1, 20, 1, 52, 40), 'ELF_ABI_or_section_header')
        require(0 < count <= 4096 and names < count, 'ELF_section_count')
        interval(shoff, count * 40, len(data))
        raw = [struct.unpack_from('>10I', data, shoff + index * 40) for index in range(count)]
        require(raw[0] == (0,) * 10, 'ELF_reserved_null_section')
        require(raw[names][1] == 3, 'ELF_section_name_table')
        strings = self.slice(raw[names][4], raw[names][5])
        self.flags, self.sections, self.symbols, self.relocations = flags, [], {}, {}
        occurrences = collections.Counter()
        ranges = []
        for index, row in enumerate(raw):
            nameoff, kind, sflags, addr, offset, size, link, info, align, entsize = row
            name = self.string(strings, nameoff)
            occurrence = name + '#' + str(occurrences[name]); occurrences[name] += 1
            require(align == 0 or align & (align - 1) == 0, 'ELF_alignment')
            if size and kind != 8:
                interval(offset, size, len(data))
                require(not any(offset < hi and lo < offset + size for lo, hi in ranges), 'ELF_overlapping_file_sections')
                ranges.append((offset, offset + size))
            self.sections.append(dict(index=index, name=name, occurrence=occurrence, type=kind, flags=sflags,
                                      alignment=align, size=size, file_offset=offset, address=addr,
                                      name_offset=nameoff, entry_size=entsize, sh_link=link, sh_info=info))
        for sec in self.sections:
            if sec['type'] != 2:
                continue
            require(sec['entry_size'] == 16 and sec['size'] % 16 == 0, 'ELF_symtab_entry_size')
            require(sec['sh_link'] < count and self.sections[sec['sh_link']]['type'] == 3, 'ELF_symtab_string_link')
            stringsec = self.sections[sec['sh_link']]
            strings = self.slice(stringsec['file_offset'], stringsec['size'])
            total = sec['size'] // 16
            require(0 < sec['sh_info'] <= total and total > 0, 'ELF_symtab_local_boundary')
            require(self.slice(sec['file_offset'], 16) == bytes(16), 'ELF_reserved_null_symbol')
            for index in range(total):
                off, value, size, info, other, shndx = struct.unpack_from('>IIIBBH', data, sec['file_offset'] + index * 16)
                binding, stype = info >> 4, info & 15
                require((index < sec['sh_info']) == (binding == 0), 'ELF_symbol_local_order')
                defining = self.sections[shndx] if shndx < count else None
                kind = 'UNDEFINED' if shndx == 0 else 'COMMON' if shndx == 0xfff2 else 'SECTION' if defining else 'RESERVED'
                row = dict(object_sha256=self.sha, symbol_table_section_index=sec['index'], symbol_index=index,
                           symbol_table_entry_size=16, string_table_section_index=sec['sh_link'], string_name_offset=off,
                           name=self.string(strings, off), section_index=shndx,
                           section_occurrence=defining['occurrence'] if defining else None,
                           value=value, size=size, binding=binding, type=stype, other=other, definition_kind=kind,
                           symbol_table_section=sec, string_table_section=stringsec, defining_section=defining)
                self.symbols[(sec['index'], index)] = row
        for sec in self.sections:
            if sec['type'] not in (4, 9):
                continue
            require(sec['type'] == 4 and sec['entry_size'] == 12 and sec['size'] % 12 == 0, 'ELF_RELA_required')
            require(sec['sh_link'] < count and self.sections[sec['sh_link']]['type'] == 2 and
                    sec['sh_info'] < count, 'ELF_RELA_links')
            for index in range(sec['size'] // 12):
                off = sec['file_offset'] + index * 12
                location, info, addend = struct.unpack_from('>IIi', data, off)
                require((sec['sh_link'], info >> 8) in self.symbols, 'ELF_RELA_target_index')
                self.relocations[(sec['index'], index)] = dict(relocation_section=sec, entry_index=index,
                    entry_file_offset=off, encoding='RELA', entry_size=12, sh_link=sec['sh_link'], sh_info=sec['sh_info'],
                    source_section=self.sections[sec['sh_info']], source_offset=location, raw_r_info=info,
                    relocation_type=info & 255, explicit_signed_addend=addend,
                    target=self.symbols[(sec['sh_link'], info >> 8)], object_sha256=self.sha)

    def slice(self, offset, size):
        require(offset >= 0 and size >= 0 and offset + size <= len(self.data), 'ELF_file_bounds')
        return self.data[offset:offset + size]

    @staticmethod
    def string(strings, offset):
        require(offset < len(strings), 'ELF_string_offset')
        end = strings.find(b'\0', offset)
        require(end >= 0, 'ELF_unterminated_string')
        return strings[offset:end].decode('utf-8')

    def symbol(self, record, *, section_source=False):
        key = (record['symbol_table_section_index'], record['symbol_index'])
        require(self.symbols.get(key) == record, 'ELF_symbol_identity')
        require(record['definition_kind'] == 'SECTION', 'ELF_defined_symbol_required')
        require(record['symbol_index'] != 0, 'ELF_reserved_null_symbol')
        if section_source and record['type'] == 3 and record['size'] == 0:
            require(record['value'] == 0, 'ELF_section_symbol_value')
        else:
            require(record['size'] > 0, 'ELF_symbol_extent')
            interval(record['value'], record['size'], record['defining_section']['size'])
        return key


class Image:
    """In-memory synthetic DOL; reuse reviewed byte reads and CFG interpreter."""
    read, word, span = baseline.Dol.read, baseline.Dol.word, baseline.Dol.span
    def __init__(self, data):
        self.data, self.path, self.segments = data, 'SYNTHETIC_IN_MEMORY_IMAGE', []
        require(len(data) >= 0x100, 'image_header')
        for index in range(18):
            off, start, size = (struct.unpack_from('>I', data, p + index * 4)[0] for p in (0, 0x48, 0x90))
            if not size:
                continue
            interval(off, size, len(data)); interval(start, size)
            require(off >= 0x100, 'image_segment_header_overlap')
            require(not any(start < hi and lo < start + size or off < oldoff + hi - lo and oldoff < off + size
                            for lo, hi, oldoff, _ in self.segments), 'image_overlapping_segments')
            self.segments.append((start, start + size, off, 'text' if index < 7 else 'data'))
        self.entrypoint = struct.unpack_from('>I', data, 0xe0)[0]
        bss, size = struct.unpack_from('>II', data, 0xd8)
        self.bss = interval(bss, size) if size else None
        require(not self.bss or not any(lo < self.bss[1] and self.bss[0] < hi
                                      for lo, hi, _, _ in self.segments), 'image_BSS_overlaps_loaded_span')


def full_maps(evidence, context):
    rows = []
    for line, text in enumerate(evidence.read(context['full_symbols']).decode().splitlines(), 1):
        m = re.match(r'(.+?)\s*=\s*(\.\w+):0x([\da-fA-F]+);(.*)', text)
        if not m:
            continue
        size = re.search(r'\bsize:0x([\da-fA-F]+)', m[4]); typ = re.search(r'\btype:(\w+)', m[4])
        rows.append(dict(name=m[1].strip().strip('"'), section=m[2], address='0x%08x' % int(m[3], 16),
                         size_explicit=size is not None, declared_size=int(size[1], 16) if size else 0,
                         line=line, record_index=len(rows), symbol_type=typ[1] if typ else None,
                         map_sha256=context['full_symbols']['sha256']))
    splits = object.__new__(baseline.Splits); splits.rows = []; splits.path = 'SYNTHETIC_SPLITS'
    unit = None
    for text in evidence.read(context['full_splits']).decode().splitlines():
        if text and not text[0].isspace() and text.endswith(':'):
            unit = text[:-1]
        m = re.match(r'\s+(\.\w+)\s+start:0x([\da-fA-F]+)\s+end:0x([\da-fA-F]+)', text)
        if m and unit:
            start, end = int(m[2], 16), int(m[3], 16)
            require(start < end <= 0x100000000, 'split_extent')
            splits.rows.append((start, end, m[1], unit))
    return rows, splits


def backed_witness(evidence, pin, kind, fields):
    value = evidence.json(pin)
    require(type(value) is dict and value.get('kind') == kind and type(value.get('schema')) is int and value['schema'] == 1,
            'aux_witness_kind_or_schema')
    for key, expected in fields.items():
        require(type(value.get(key)) is type(expected) and value[key] == expected, 'aux_witness_field:' + key)
    return value


def source_context(evidence, context, relation, reconstruction):
    binding = relation['source_binding']
    require(binding['audited_source_commit'] == context['audited_source_commit'] and
            binding['audited_source_tree'] == context['audited_source_tree'] and
            binding['class_name'] == relation['class_name'], 'source_context_mismatch')
    sources = [binding['declaration'], binding['static_definition']] + binding['relevant_headers']
    required = set()
    for row in sources:
        pin = {key: row[key] for key in ('path', 'sha256')}
        data = evidence.read(pin); required.add((pin['path'], pin['sha256']))
        require(1 <= row['line'] <= row['end_line'] <= len(data.decode().splitlines()), 'source_anchor_extent')
        backed_witness(evidence, row['preprocessing_witness'], 'synthetic-source-anchor',
                       {key: row[key] for key in ('line', 'end_line', 'role', 'qualified_name', 'declared_type')} |
                       dict(source_sha256=row['sha256']))
    recipe = backed_witness(evidence, reconstruction['build_recipe'], 'synthetic-reconstruction-context',
                            {key: reconstruction[key] for key in ('source_commit', 'source_tree', 'source_unit_path', 'logical_split_unit')} |
                            dict(object_sha256=reconstruction['object']['sha256'], original_selection_grant=False))
    require(reconstruction['audited_source_bridge'].get('state') != 'UNKNOWN', 'missing_source_context_bridge')
    bridge = backed_witness(evidence, reconstruction['audited_source_bridge'], 'synthetic-source-context-bridge',
                            dict(reconstructed_source_commit=reconstruction['source_commit'],
                                 reconstructed_source_tree=reconstruction['source_tree'],
                                 audited_source_commit=context['audited_source_commit'], audited_source_tree=context['audited_source_tree']))
    require(reconstruction['source_tree'] == context['audited_source_tree'], 'source_tree_equivalence_unclosed')
    for witness in (recipe, bridge):
        require(type(witness.get('source_pins')) is list, 'source_pins_required')
        keys = []
        for pin in witness['source_pins']:
            evidence.read(pin); keys.append((pin['path'], pin['sha256']))
        require(len(keys) == len(set(keys)) and required <= set(keys), 'source_pin_coverage_or_duplicates')
        require(any(path == reconstruction['source_unit_path'] for path, _ in keys), 'producing_TU_source_pin_missing')


def reconstruction_identity(evidence, reconstruction):
    elf = Elf(evidence.read(reconstruction['object']))
    require(reconstruction['elf_flags'] == elf.flags, 'ELF_flags')
    for field in ('singleton_section', 'ctors_section'):
        sec = reconstruction[field]
        require(sec['index'] < len(elf.sections) and elf.sections[sec['index']] == sec, 'ELF_section_identity')
    keys = {field: elf.symbol(reconstruction[field]) for field in ('singleton', 'initializer', 'constructor', 'vtable', 'execute')}
    singleton = reconstruction['singleton']
    require((singleton['binding'], singleton['type'], singleton['size']) == (2, 1, 4), 'reconstructed_weak_4B_object_required')
    require(singleton['defining_section'] == reconstruction['singleton_section'] and
            reconstruction['singleton_section']['type'] == 8, 'singleton_NOBITS_section')
    for otherkey, other in elf.symbols.items():
        if otherkey != keys['singleton'] and other['type'] != 3 and other['name'] and other['definition_kind'] == 'SECTION':
            require(not (other['section_index'] == singleton['section_index'] and
                         other['value'] < singleton['value'] + 4 and singleton['value'] < other['value'] + max(1, other['size'])),
                    'competing_reconstructed_definition')
    roles = dict(ctors_initializer=(1, None, 'initializer'), sinit_singleton=(109, 'initializer', 'singleton'),
                 sinit_constructor=(10, 'initializer', 'constructor'), ctor_vtable_hi=(6, 'constructor', 'vtable'),
                 ctor_vtable_lo=(4, 'constructor', 'vtable'), vtable_execute=(1, 'vtable', 'execute'))
    seen, rolecounts = set(), collections.Counter()
    for claim in reconstruction['relocations']:
        key = (claim['relocation_section']['index'], claim['entry_index'])
        require(key not in seen, 'duplicate_relocation_occurrence'); seen.add(key)
        actual = elf.relocations.get(key)
        require(actual is not None and all(claim[k] == v for k, v in actual.items()), 'ELF_RELA_identity')
        elf.symbol(claim['source_symbol'], section_source=claim['role'] == 'ctors_initializer')
        typ, source, target = roles[claim['role']]; rolecounts[claim['role']] += 1
        require(claim['relocation_type'] == typ and claim['explicit_signed_addend'] == 0 and
                elf.symbol(claim['target']) == keys[target], 'relocation_role_target_or_addend')
        if source is not None:
            require(elf.symbol(claim['source_symbol']) == keys[source], 'relocation_source_symbol')
        else:
            require(claim['source_section'] == reconstruction['ctors_section'], 'ctors_relocation_section')
        width = 2 if typ in (4, 6) else 4
        symbol = claim['source_symbol']; offset = claim['source_offset']
        limit = claim['source_section']['size'] if claim['role'] == 'ctors_initializer' and symbol['type'] == 3 and symbol['size'] == 0 else symbol['value'] + symbol['size']
        require(symbol['section_index'] == claim['source_section']['index'] and
                symbol['value'] <= offset and offset + width <= limit, 'relocation_source_extent')
        interval(offset, width, claim['source_section']['size'])
        if claim['role'] == 'vtable_execute':
            require(offset == reconstruction['vtable']['value'] + 8, 'vtable_execute_slot')
    require(rolecounts == collections.Counter({role: 1 for role in roles}), 'required_relocation_roles')
    # Cover full overlapping relocation widths, including the selected .ctors cell.
    contributions = [(reconstruction[field]['section_index'], reconstruction[field]['value'],
                      reconstruction[field]['value'] + reconstruction[field]['size'])
                     for field in ('initializer', 'constructor', 'vtable')]
    contributions += [(claim['source_section']['index'], claim['source_offset'], claim['source_offset'] + 4)
                      for claim in reconstruction['relocations'] if claim['role'] == 'ctors_initializer']
    for key, actual in elf.relocations.items():
        width = {1: 4, 4: 2, 6: 2, 10: 4, 109: 4}.get(actual['relocation_type'])
        for section, lo, hi in contributions:
            if actual['sh_info'] != section:
                continue
            require(width is not None, 'unsupported_relocation_width_in_selected_section')
            if actual['source_offset'] < hi and lo < actual['source_offset'] + width:
                require(key in seen, 'unlisted_relocation_in_selected_contribution')
    return elf


def span_binding(evidence, image, rows, splits, context, claim, *, function=False, unowned=False):
    require(claim['image_sha256'] == context['original_image']['sha256'], 'span_image_context')
    start, end = interval(address(claim['address']), claim['size'])
    require(digest(image.read(start, claim['size'])) == claim['sha256'], 'span_hash')
    owners = splits.owners(start, claim['section'])
    require(claim['existing_owner_units'] == owners, 'span_owner_record')
    if owners:
        require(len(owners) == 1 and splits.owns_extent(start, end - start, claim['section'], owners[0]), 'span_full_ownership')
    else:
        require(unowned, 'span_owner_missing')
    if claim['ownership_witness'].get('state') != 'UNKNOWN':
        evidence.read(claim['ownership_witness'])  # Identity only; never replaces the actual split test.
    if function:
        candidates = [row for row in rows if address(row['address']) == start]
        require(len(candidates) == 1 and candidates[0]['section'] == claim['section'] and
                candidates[0]['symbol_type'] == 'function' and candidates[0]['size_explicit'] and
                candidates[0]['declared_size'] == claim['size'] and start % 4 == 0 and claim['size'] % 4 == 0,
                'function_map_identity_or_extent')
        return dict(address=start, size=claim['size'], section=claim['section'], name=candidates[0]['name'])
    return None


def reachable(image, function):
    start, end = function['address'], function['address'] + function['size']
    pending, seen = [start], set()
    while pending:
        pc = pending.pop()
        require(start <= pc < end and pc % 4 == 0, 'reachable_CFG_extent')
        if pc in seen:
            continue
        seen.add(pc); require(len(seen) <= 4096, 'reachable_CFG_limit')
        word = image.word(pc); op = word >> 26
        if word == 0x4e800020:
            continue
        if op in (16, 18) and not word & 1:
            target = baseline.branch(word, pc)
            if op == 18:
                if start <= target < end:
                    pending.append(target)
                continue
            pending.append(target)
        if pc + 4 < end:
            pending.append(pc + 4)
    return seen


def original_initializer(evidence, context, relation, image, rows, splits):
    claim = relation['original_initializer']
    require(claim.get('state') != 'UNKNOWN', 'original_initializer_gap')
    require(claim['image_sha256'] == context['original_image']['sha256'], 'initializer_image_context')
    witness = backed_witness(evidence, claim['register_and_cfg_witness'], 'synthetic-initializer-cfg',
                            dict(initializer_invocation_domain='registered_ctor_table_with_derived_startup_bases'))
    chain = witness.get('startup_chain')
    require(type(chain) is list and 0 < len(chain) <= 16, 'startup_chain_required')
    require(address(chain[0]['address']) == image.entrypoint, 'startup_entrypoint')
    bases = {}
    for index, span in enumerate(chain):
        require(not shape(span, SCHEMA['$defs']['span']), 'startup_span_shape')
        function = span_binding(evidence, image, rows, splits, context, span, function=True)
        events, issues = baseline.trace(image, function, {})
        require(not issues, 'startup_CFG_unclosed')
        require(not any(event['kind'] in ('store', 'unsupported_store') for event in events),
                'startup_memory_effects_unclosed')
        if index + 1 < len(chain):
            calls = [event for event in events if event['kind'] == 'call']
            require(len(calls) == 1 and calls[0]['target'] == address(chain[index + 1]['address']), 'startup_chain_call')
            # A span-wide call census does not prove all startup paths invoke it.
            # This deliberately narrow interprocedural mode accepts only a linear
            # prefix to BL and a transparent return epilogue. Arbitrary caller CFG
            # or r13 writes after the helper remain unsupported, hence UNKNOWN.
            callpc = calls[0]['pc']
            for pc in range(function['address'], callpc, 4):
                word = image.word(pc)
                require(word >> 26 not in (16, 18, 19), 'startup_call_not_linear_dominating')
            require(image.word(callpc) >> 26 == 18 and image.word(callpc) & 1, 'startup_call_requires_direct_BL')
            returned = False
            for pc in range(callpc + 4, function['address'] + function['size'], 4):
                word = image.word(pc)
                if word == 0x4e800020:
                    returned = True
                    break
                require(word == 0x60000000 or word & 0xfc1fffff == 0x7c0803a6,
                        'startup_postcall_r13_preservation_unclosed')
            require(returned, 'startup_linear_return_missing')
        else:
            require(not any(event['kind'] == 'call' for event in events), 'startup_helper_calls_unclosed')
            returns = [event for event in events if event['kind'] == 'return']
            require(returns and len({event['r13'] for event in returns}) == 1 and returns[0]['r13'] is not None,
                    'startup_r13_not_derived')
            bases[13] = returns[0]['r13']
    span_binding(evidence, image, rows, splits, context, claim['ctors_entry'])
    require(claim['ctors_entry']['section'] == '.ctors' and claim['ctors_entry']['size'] == 4 and
            image.word(address(claim['ctors_entry']['address'])) == address(claim['initializer']['address']),
            'initializer_registration')
    initializer = span_binding(evidence, image, rows, splits, context, claim['initializer'], function=True)
    ctor = span_binding(evidence, image, rows, splits, context, claim['constructor'], function=True)
    require(claim['initializer_unit'] in claim['initializer']['existing_owner_units'] and
            claim['initializer_unit'] in claim['ctors_entry']['existing_owner_units'], 'initializer_unit_context')
    require(initializer['name'] == relation['reconstruction']['initializer']['name'] and
            ctor['name'] == relation['reconstruction']['constructor']['name'], 'initializer_or_ctor_symbol_identity')
    obj, vt = address(relation['singleton_address']), address(claim['vptr_address'])
    require(address(claim['singleton_address']) == obj, 'initializer_singleton_context')
    loadpc, callpc, storepc = (address(claim[key]) for key in ('singleton_load_pc', 'constructor_call_pc', 'vptr_store_pc'))
    events, issues = baseline.trace(image, initializer, bases, tracked_object=obj)
    require(not issues and loadpc in reachable(image, initializer), 'initializer_CFG_unclosed')
    word = image.word(loadpc)
    require(word >> 26 == 14 and (word >> 21) & 31 == 3 and (word >> 16) & 31 == 13 and
            (bases[13] + baseline.signed(word & 0xffff, 16)) & 0xffffffff == obj, 'singleton_load_role_or_address')
    calls = [event for event in events if event['kind'] == 'call']
    require(len(calls) == 1 and calls[0]['pc'] == callpc and calls[0]['target'] == ctor['address'] and
            calls[0]['r3'] == obj and image.word(callpc) >> 26 == 18 and image.word(callpc) & 1,
            'constructor_call_role_receiver_or_census')
    # Instruction decoding uses immutable supplied image bytes. Without a memory
    # effect model, initializer stores could alter .ctors, vtables or later code.
    require(not any(event['kind'] in ('store', 'unsupported_store') for event in events),
            'initializer_memory_effects_unclosed')
    events, issues = baseline.trace(image, ctor, {**bases, 3: obj}, tracked_object=obj)
    require(not issues and not any(event['kind'] == 'call' for event in events), 'constructor_CFG_or_effect_unclosed')
    returns = [event for event in events if event['kind'] == 'return']
    require(returns and all(event.get('vptr') == vt for event in returns), 'final_vptr_unclosed')
    stores = [event for event in events if event['kind'] == 'store' and event['pc'] == storepc and
              event['address'] == obj and event['value'] == vt]
    require(len(stores) == 1, 'declared_vptr_store_missing')
    require(not any(event['kind'] == 'unsupported_store' or
                    event['kind'] == 'store' and event.get('address') != obj for event in events),
            'constructor_non_vptr_memory_effects_unclosed')
    span_binding(evidence, image, rows, splits, context, claim['vtable_observation'], unowned=True)
    require(address(claim['vtable_observation']['address']) == vt and claim['vtable_observation']['size'] >= 12,
            'vtable_observation_span')
    candidates = [row for row in rows if address(row['address']) == vt]
    require(len(candidates) == 1 and candidates[0]['name'] == relation['reconstruction']['vtable']['name'], 'vtable_map_identity')
    require(image.word(vt + 8) == address(claim['execute_address']), 'original_execute_slot')
    image.read(address(claim['execute_address']), 4)
    return dict(startup_r13='0x%08x' % bases[13], constructor_call_reachable=True,
                registration_address=claim['ctors_entry']['address'], final_vptr=claim['vptr_address'],
                execute_slot=8, callback_source_correctness='NOT_EVALUATED')


def extents_and_selection(evidence, context, relation, image):
    ext = relation['extents']; obj = address(relation['singleton_address'])
    record, reconstruction = relation['mapped_record'], relation['reconstruction']
    require(ext['mapped_span']['address'] == relation['singleton_address'] == record['address'] and
            record['size_explicit'] and ext['mapped_span']['size'] == record['declared_size'], 'mapped_extent_identity')
    lo, hi = interval(obj, ext['mapped_span']['size'])
    require(ext['reconstructed_symbol_size'] == reconstruction['singleton']['size'] == 4, 'reconstructed_extent_identity')
    require(ext['observed_access_extent'] == dict(address=relation['singleton_address'], size=4) and hi >= obj + 4,
            'observed_access_extent')
    require(image.bss and image.bss[0] <= lo and hi <= image.bss[1], 'BSS_containment_only')
    cursor = obj + 4
    for tail in ext['unassigned_ranges']:
        start, end = interval(address(tail['range']['address']), tail['range']['size'])
        require(start == cursor and end <= hi, 'unassigned_tail_geometry'); cursor = end
    require(cursor == hi, 'unassigned_tail_missing_or_inflated')
    original = ext['original_allocation_extent']
    if original.get('state') != 'UNKNOWN':
        start, end = interval(address(original['range']['address']), original['range']['size'])
        require(start <= obj and obj + 4 <= end and end <= hi, 'claimed_original_extent_geometry')
        evidence.read(original['witness']); evidence.read(original['original_image_binding'])
    selection = relation['original_selection']
    if selection.get('state') != 'UNKNOWN':
        providers = selection['providers']; selected = selection['selected_provider']
        require(sum(provider == selected for provider in providers) == 1, 'selected_provider_membership')
        require(selected['disposition'] == 'selected' and sum(p['disposition'] == 'selected' for p in providers) == 1 and
                not any(p['disposition'] == 'ambiguous' for p in providers), 'original_selection_disposition')
        keys = []
        for provider in providers:
            blob = evidence.read(provider['original_input_object']); elf = Elf(blob)
            elf.symbol(provider['definition'])
            section = provider['section']; require(elf.sections[section['index']] == section, 'provider_section_identity')
            keys.append((provider['original_input_object']['sha256'], provider['definition']['symbol_table_section_index'],
                         provider['definition']['symbol_index'], provider['input_order'], provider['archive_member_identity']))
            evidence.read(provider['resolution_witness'])
            start, end = interval(address(provider['contribution']['address']), provider['contribution']['size'])
            if provider == selected:
                require(start <= obj and obj + 4 <= end, 'original_contribution_excludes_object')
                if original.get('state') != 'UNKNOWN':
                    require(start <= address(original['range']['address']) and
                            address(original['range']['address']) + original['range']['size'] <= end, 'original_contribution_extent')
        require(len(set(keys)) == len(keys), 'duplicate_original_provider')
        for key in ('witness', 'original_image_binding', 'original_link_recipe', 'all_provider_inventory', 'selection_rule_witness'):
            evidence.read(selection[key])
    for role in relation['unit_roles']:
        if role['evidence'].get('state') == 'UNKNOWN':
            require(role['unit'] == 'UNKNOWN', 'unit_claim_without_evidence')
        else:
            evidence.read(role['evidence'])
    return dict(geometry='CONSISTENT', original_allocation='UNKNOWN', selected_original_weak_provider='UNKNOWN',
                tail_status='UNKNOWN', BSS_evidence='CONTAINMENT_ONLY')


def check_proposal(envelope, evidence):
    """No filesystem/network/subprocess access; supplied pin.path values stay opaque."""
    report = dict(status='INPUT_REFUSED', production_acceptance=False, effective_owners=[],
                  structural_errors=[], relations=[], obligations=dict(OBLIGATIONS),
                  checker_sha256=CHECKER_SHA, baseline_trace_sha256=BASELINE_SHA,
                  sourceclass_semantic_proof='NOT_EVALUATED', actual_original_data_claims=False)
    if type(envelope) is not dict or envelope.get('evidence_domain') != 'SYNTHETIC':
        report['structural_errors'] = ['synthetic_domain_required_before_evidence_access']
        return report
    errors = shape(envelope, SCHEMA)
    if errors:
        report['structural_errors'] = errors
        return report
    if not isinstance(evidence, SyntheticEvidence):
        report['structural_errors'] = ['explicit_SyntheticEvidence_required']
        return report
    try:
        context = envelope['context']
        require(context['proposal_schema_sha256'] == SCHEMA_SHA and context['design_spec']['sha256'] == DESIGN_SHA,
                'schema_or_design_context')
        require(context['tool']['sha256'] == BASELINE_SHA and evidence.read(context['tool']) == BASELINE_BYTES,
                'baseline_tool_context')
        require(evidence.read(context['design_spec']) == DESIGN_BYTES, 'design_spec_backing')
        allowed = context['allowed_subject_ids']; ids = [r['subject_id'] for r in envelope['relations']]
        require(len(allowed) == len(set(allowed)) and len(ids) == len(set(ids)) and set(ids) <= set(allowed),
                'duplicate_or_nonallowlisted_subject')
        review = envelope['review']
        if review is not None:
            covered = {key: value for key, value in envelope.items() if key != 'review'}
            require(review['claim_sha256'] == digest(canonical(covered)), 'review_context_or_scope_hash')
            receipt = evidence.json(review['receipt'])
            require(all(type(receipt.get(key)) is type(review[key]) and receipt[key] == review[key]
                        for key in ('status', 'scope', 'reviewer', 'claim_sha256')), 'review_receipt_identity')
        image = Image(evidence.read(context['original_image']))
        rows, splits = full_maps(evidence, context)
    except (Refused, ValueError, KeyError, TypeError, UnicodeError, struct.error) as error:
        report['status'] = 'UNKNOWN'; report['structural_errors'] = ['context_evidence:' + str(error)]
        return report
    for relation in envelope['relations']:
        item = dict(subject_id=relation['subject_id'], status='UNKNOWN', reasons=[], components={},
                    obligations=dict(OBLIGATIONS), production_acceptance=False, effective_owners=[])
        try:
            require(relation['reconstruction'].get('state') != 'UNKNOWN', 'reconstruction_gap')
            record = relation['mapped_record']
            require(record in rows and record['map_sha256'] == context['full_symbols']['sha256'], 'mapped_record_context')
            require(record['name'] == relation['reconstruction']['singleton']['name'], 'mapped_singleton_symbol_identity')
            lo, hi = interval(address(record['address']), record['declared_size'])
            require(not any(other is not record and other != record and other['section'] == record['section'] and
                            address(other['address']) < hi and lo < address(other['address']) + max(1, other['declared_size'])
                            for other in rows), 'competing_original_map_record')
            source_context(evidence, context, relation, relation['reconstruction'])
            item['components']['source_context_pins'] = 'CONSISTENT_IDENTITY_ONLY'
            elf = reconstruction_identity(evidence, relation['reconstruction'])
            item['components']['reconstructed_definition'] = dict(status='BOUND_COMPONENT_ONLY', object_sha256=elf.sha)
            item['components']['original_initializer'] = original_initializer(evidence, context, relation, image, rows, splits)
            item['components']['extents'] = extents_and_selection(evidence, context, relation, image)
            item['status'] = 'ASSOCIATION_CANDIDATE_ONLY'
        except (Refused, ValueError, KeyError, TypeError, IndexError, UnicodeError, struct.error) as error:
            item['reasons'].append(str(error))
        report['relations'].append(item)
    report['status'] = 'ASSOCIATION_CANDIDATE_ONLY' if report['relations'] and all(
        r['status'] == 'ASSOCIATION_CANDIDATE_ONLY' for r in report['relations']) else 'UNKNOWN'
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--synthetic-bundle', type=Path, required=True,
                        help='one explicit envelope+base64 blobs JSON file; pin paths never resolve')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    try:
        bundle = parse_json(args.synthetic_bundle.read_bytes())
        require(set(bundle) == {'envelope', 'blobs_base64'}, 'synthetic_bundle_shape')
        blobs = {key: base64.b64decode(value, validate=True) for key, value in bundle['blobs_base64'].items()}
        result = check_proposal(bundle['envelope'], SyntheticEvidence(blobs))
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(2, 'ownership proposal: input refused: ' + str(error) + '\n')
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n')
    return 0 if result['status'] == 'ASSOCIATION_CANDIDATE_ONLY' else 2


if __name__ == '__main__':
    raise SystemExit(main())
