#!/usr/bin/env python3
"""Audit full bounded data objects referenced by existing PPC function objects.

Read-only candidate finder, not an equivalence certificate. No compilation occurs.
Example: datacontent.py Game/Ride Game/AreaObj Game/Boss --out /tmp/data.json
Use --unit and --source-object to compare a frozen scratch candidate. Unknown
addresses, bounds, reference pairing and split/merged extents remain advisories.
Only known allocated ELF bytes or mapped target DOL bytes are read. In particular,
source undefined symbols never silently fall back to target data.
"""
import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
from dataclasses import dataclass
from difflib import SequenceMatcher
import json
import math
from pathlib import Path
import re
import struct

VERSION = 'SB4E01'


def word(data, at=0):
    return struct.unpack_from('>I', data, at)[0]


def signed16(value):
    return value - 65536 if value & 32768 else value


def decode(data):
    result = {'hex': data.hex()}
    if len(data) == 4:
        floating = struct.unpack('>f', data)[0]
        result.update(u32=word(data), i32=struct.unpack('>i', data)[0],
                      f32=floating if math.isfinite(floating) else str(floating))
    elif len(data) == 2:
        result.update(u16=int.from_bytes(data, 'big'), i16=int.from_bytes(data, 'big', signed=True))
    elif len(data) == 1:
        result['u8'] = data[0]
    return result


def compare_bytes(target, source, target_reference_offset=0, source_reference_offset=0,
                  target_pointers=None, source_pointers=None):
    """Compare whole objects; differing extents never produce an equality result.

    Reference offsets align objects that were split/merged by the compiler. Such
    comparisons report only their aligned overlap as scalar candidates, while
    retaining both complete byte strings and an explicit extent advisory.
    Pointer maps use object-relative byte offsets and optional resolved identity.
    A relocation masks exactly four bytes; unknown pointers are not scalar data.
    """
    target_pointers = target_pointers or {}
    source_pointers = source_pointers or {}
    shift = target_reference_offset - source_reference_offset
    begin, end = max(0, shift), min(len(target), len(source) + shift)
    advisories, differences, pointers = [], [], []
    if len(target) != len(source) or shift:
        advisories.append({'reason': 'different_object_extents_or_reference_offsets',
                           'target_size': len(target), 'source_size': len(source),
                           'target_reference_offset': target_reference_offset,
                           'source_reference_offset': source_reference_offset})
    if end <= begin:
        advisories.append({'reason': 'no_aligned_object_overlap'})
    masked = set()
    pointer_difference = False
    for t_at in sorted(set(target_pointers) | {s_at + shift for s_at in source_pointers}):
        s_at = t_at - shift
        t_pointer, s_pointer = target_pointers.get(t_at), source_pointers.get(s_at)
        ti = t_pointer.get('identity') if t_pointer else None
        si = s_pointer.get('identity') if s_pointer else None
        status = 'equal' if ti is not None and ti == si else 'unresolved'
        if ti is not None and si is not None and ti != si:
            status = 'different'
            pointer_difference = True
        annotations = [p for p in (t_pointer, s_pointer) if p is not None]
        kinds = {p.get('relocation_kind', 1) for p in annotations}
        # Only full-width ADDR32/REL32 relocations are pointer words. Recognized
        # halfword fields are masked precisely and remain an explicit advisory.
        widths = {4 if kind in (1, 26) else 2 if kind in (3, 4, 5, 6, 32) else 0 for kind in kinds}
        width = max(widths, default=0)
        if not kinds <= {1, 26}:
            advisories.append({'reason': 'unsupported_pointer_relocation_kind',
                               'target_offset': t_at, 'source_offset': s_at,
                               'relocation_kinds': sorted(kinds), 'masked_bytes': width})
        pointers.append({'target_offset': t_at, 'source_offset': s_at,
                         'target': t_pointer, 'source': s_pointer, 'status': status})
        masked.update(range(t_at, t_at + width))
    # Keep changed words intact for useful float/int decodes, but never include a
    # pointer byte in a scalar mismatch. Unaligned pointer slots can split words.
    for at in range(max(0, begin), max(0, end), 4):
        stop = min(at + 4, end)
        spans = []
        for index in range(at, stop):
            if index in masked:
                continue
            if spans and spans[-1][-1] + 1 == index:
                spans[-1].append(index)
            else:
                spans.append([index])
        for span in spans:
            start, stop_at = span[0], span[-1] + 1
            t_bytes = target[start:stop_at]
            s_start = start - shift
            s_bytes = source[s_start:s_start + len(t_bytes)]
            if t_bytes != s_bytes:
                differences.append({'target_offset': start, 'source_offset': s_start,
                                    'target_hex': t_bytes.hex(), 'source_hex': s_bytes.hex(),
                                    'target_decode': decode(t_bytes), 'source_decode': decode(s_bytes)})
    status = ('scalar_difference' if differences else 'pointer_difference' if pointer_difference
              else 'advisory_extent' if advisories else 'pointer_unresolved'
              if any(p['status'] == 'unresolved' for p in pointers) else 'equal')
    return {'status': status, 'scalar_differences': differences,
            'pointer_annotations': pointers, 'advisories': advisories,
            'target_hex': target.hex(), 'source_hex': source.hex(),
            'target_size': len(target), 'source_size': len(source),
            'aligned_range': [begin, max(begin, end)]}


@dataclass(frozen=True)
class Symbol:
    name: str
    address: int
    size: int
    section: int
    kind: int
    binding: int


@dataclass(frozen=True)
class Relocation:
    kind: int
    symbol: Symbol
    addend: int


@dataclass(frozen=True)
class Pointer:
    relocation: Relocation
    delta: int = 0
    high: bool = False
    dynamic: bool = False


def cstring(data, offset):
    end = data.find(b'\0', offset)
    if end < 0:
        raise ValueError('unterminated ELF string')
    return data[offset:end].decode(errors='replace')


class Elf:
    """Small stdlib ELF32 BE reader, derived from pooled-string-audit/dataref."""
    def __init__(self, path):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        if self.data[:6] != b'\x7fELF\x01\x02':
            raise ValueError(f'expected big-endian ELF32: {path}')
        header = struct.unpack_from('>16sHHIIIIIHHHHHH', self.data)
        self.sections = [struct.unpack_from('>IIIIIIIIII', self.data, header[6] + i * header[11])
                         for i in range(header[12])]
        self.contents = [b'' if s[1] == 8 else self.data[s[4]:s[4] + s[5]] for s in self.sections]
        names = self.contents[header[13]]
        self.names = [cstring(names, s[0]) for s in self.sections]
        self.symbols, self.functions, tables = [], {}, {}
        self.relocations = {}
        for index, section in enumerate(self.sections):
            if section[1] != 2:
                continue
            strings, symbols = self.contents[section[6]], []
            for at in range(0, section[5], section[9]):
                name, address, size, info, _, owner = struct.unpack_from('>IIIBBH', self.contents[index], at)
                symbol = Symbol(cstring(strings, name), address, size, owner, info & 15, info >> 4)
                symbols.append(symbol)
                self.symbols.append(symbol)
                if symbol.kind == 2 and size and 0 < owner < len(self.sections):
                    self.functions[symbol.name] = symbol
            tables[index] = symbols
        for index, section in enumerate(self.sections):
            if section[1] != 4:
                continue
            for at in range(0, section[5], section[9]):
                address, info, addend = struct.unpack_from('>IIi', self.contents[index], at)
                self.relocations[section[7], address] = Relocation(info & 255, tables[section[6]][info >> 8], addend)

    def instruction_relocation(self, section, address):
        return self.relocations.get((section, address)) or self.relocations.get((section, address + 2))

    def local(self, symbol):
        return 0 < symbol.section < len(self.sections)

    def data_symbol(self, symbol):
        return self.local(symbol) and self.sections[symbol.section][2] & 2 and not self.sections[symbol.section][2] & 4

    def bound(self, pointer, limit):
        symbol = pointer.relocation.symbol
        section = symbol.section
        if not self.data_symbol(symbol):
            return None, 'undefined_or_nondata_source_symbol'
        if self.sections[section][1] == 8:
            return None, 'runtime_NOBITS_data'
        at = symbol.address + pointer.relocation.addend + pointer.delta
        # A compiler section-base relocation denotes a pooled allocation, not the
        # first scalar symbol. Reading just that scalar misses indexed array tails.
        section_base = symbol.kind == 3 or symbol.name.startswith('...')
        if section_base:
            start, size, name, proof = 0, self.sections[section][5], symbol.name, 'ELF_section_pool'
        else:
            candidates = [s for s in self.symbols if s.section == section and s.kind not in (2, 3)
                          and s.size and s.address <= at < s.address + s.size]
            if candidates:
                obj = min(candidates, key=lambda s: (s.size, s.address, s.name))
                start, size, name, proof = obj.address, obj.size, obj.name, 'ELF_symbol_size'
            else:
                labels = [s for s in self.symbols if s.section == section and s.kind not in (2, 3)
                          and s.name and not s.name.startswith('...')]
                preceding = [s for s in labels if s.address <= at]
                if not preceding:
                    return None, 'unbounded_source_object'
                obj = max(preceding, key=lambda s: s.address)
                following = [s.address for s in labels if s.address > obj.address]
                if not following:
                    return None, 'source_label_has_no_upper_bound'
                start, size, name, proof = obj.address, min(following) - obj.address, obj.name, 'label_to_next_label_advisory'
        if size > limit:
            return None, 'object_exceeds_limit'
        if size <= 0 or start < 0 or start + size > len(self.contents[section]) or not start <= at < start + size:
            return None, 'source_object_outside_section_or_reference_outside_object'
        blob = self.contents[section][start:start + size]
        pointers = {}
        for (owner, where), rel in self.relocations.items():
            if owner == section and start <= where < start + size:
                pointers[where - start] = {'symbol': rel.symbol.name, 'addend': rel.addend,
                                         'relocation_kind': rel.kind}
        return {'symbol': name, 'section': self.names[section], 'section_index': section,
                'start': start, 'size': size, 'reference_offset': at - start,
                'extent_proof': proof, 'hex': blob.hex(), 'pointers': pointers,
                'decode_words': [dict(offset=i, **decode(blob[i:i + 4])) for i in range(0, len(blob), 4)]}, None


class World:
    def __init__(self, tree):
        self.tree = Path(tree)
        self.image = (self.tree / f'build/{VERSION}/main.dol').read_bytes()
        self.dol_sections = [(word(self.image, i * 4), word(self.image, 0x48 + i * 4),
                              word(self.image, 0x90 + i * 4)) for i in range(18)]
        self.symbols, self.bases, self.ranges = defaultdict(list), {}, {}
        for line in (self.tree / f'config/{VERSION}/symbols.txt').read_text().splitlines():
            match = re.match(r'(.+?) = (\.\w+):0x([\da-fA-F]+);(.*)', line)
            if match:
                name, section, address, meta = match.groups()
                size = re.search(r'\bsize:0x([\da-fA-F]+)', meta)
                self.symbols[name].append({'name': name, 'section': section, 'address': int(address, 16),
                                          'size': int(size[1], 16) if size else 0,
                                          'function': 'type:function' in meta,
                                          'local': bool(re.search(r'\bscope:local\b', meta))})
        self.data_symbols = sorted((s for records in self.symbols.values() for s in records if not s['function']), key=lambda s: s['address'])
        self.data_starts = [s['address'] for s in self.data_symbols]
        self.data_max_end = []
        maximum = 0
        for symbol in self.data_symbols:
            maximum = max(maximum, symbol['address'] + symbol['size'])
            self.data_max_end.append(maximum)
        unit = None
        for line in (self.tree / f'config/{VERSION}/splits.txt').read_text().splitlines():
            if line and not line[0].isspace() and line.endswith(':'):
                unit = line[:-1].removesuffix('.cpp').removesuffix('.c')
            match = re.search(r'^\s+(\.\w+)\s+start:0x([\da-fA-F]+)\s+end:0x([\da-fA-F]+)', line)
            if match:
                options = re.split(r'//|#', line[match.end():], maxsplit=1)[0].split()
                if 'skip' in options:
                    continue
                self.bases[unit, match[1]] = int(match[2], 16)
                self.ranges[unit, match[1]] = (int(match[2], 16), int(match[3], 16))

    def owns(self, unit, section, address):
        bounds = self.ranges.get((unit, section))
        return bounds is not None and bounds[0] <= address < bounds[1]

    def resolve_address(self, elf, unit, pointer):
        rel, symbol = pointer.relocation, pointer.relocation.symbol
        if elf.local(symbol):
            # Target ELF section offsets, plus this unit's split, identify local
            # symbols even when other TUs use the same spelling. Never fall back
            # to a globally indexed name if the owning section is unavailable.
            section = elf.names[symbol.section]
            base = self.bases.get((unit, section))
            if base is None:
                return None, 'target_owning_section_unknown'
            address = base + symbol.address
            if not self.owns(unit, section, address):
                return None, 'target_symbol_outside_owning_section'
            effective = address + rel.addend + pointer.delta
            if not self.owns(unit, section, effective):
                return None, 'target_effective_address_outside_owning_section'
            return effective, None
        records = self.symbols.get(symbol.name, [])
        if symbol.binding == 0:
            return None, 'target_local_symbol_requires_owning_section'
        # An undefined relocation cannot identify another TU's local object by
        # spelling. Duplicate global identities are also unresolved, not last-win.
        globals_ = [record for record in records if not record['local']]
        identities = {(record['section'], record['address']) for record in globals_}
        if len(identities) == 1:
            return next(iter(identities))[1] + rel.addend + pointer.delta, None
        if len(identities) > 1:
            return None, 'ambiguous_target_symbol_identity'
        return None, ('target_local_symbol_requires_owning_section' if records
                      else 'target_address_unknown')

    def address(self, elf, unit, pointer):
        return self.resolve_address(elf, unit, pointer)[0]

    def is_data(self, elf, pointer):
        symbol = pointer.relocation.symbol
        if elf.local(symbol):
            return bool(elf.data_symbol(symbol))
        known = [record for record in self.symbols.get(symbol.name, []) if not record['local']]
        # Unknown/ambiguous references remain visible for a resolution advisory.
        identities = {(record['section'], record['address']) for record in known}
        return (symbol.binding == 0 or len(identities) != 1
                or any(not record['function'] for record in known))

    def target_bound(self, elf, unit, pointer, limit):
        address, error = self.resolve_address(elf, unit, pointer)
        if error:
            return None, error
        candidates = []
        index = bisect_right(self.data_starts, address) - 1
        while index >= 0 and self.data_max_end[index] > address:
            symbol = self.data_symbols[index]
            if (symbol['size'] and symbol['address'] <= address < symbol['address'] + symbol['size']
                    and (not symbol['local'] or self.owns(unit, symbol['section'], symbol['address']))):
                candidates.append(symbol)
            index -= 1
        if not candidates:
            return None, 'target_object_bound_unknown'
        obj = min(candidates, key=lambda s: (s['size'], s['address'], s['name']))
        start, size = obj['address'], obj['size']
        if obj['local'] and not self.owns(unit, obj['section'], start + size - 1):
            return None, 'target_local_object_extent_outside_owning_section'
        if size > limit:
            return None, 'object_exceeds_limit'
        blob = None
        for offset, begin, extent in self.dol_sections:
            if begin <= start and start + size <= begin + extent:
                position = offset + start - begin
                if position + size <= len(self.image):
                    blob = self.image[position:position + size]
                    break
        # The explicitly named read-only zero section has initialized zero data;
        # arbitrary unmapped addresses and mutable BSS must never become zeros.
        proof = 'config_symbol_size_DOL'
        if blob is None and obj['section'] == '.sbss2':
            blob, proof = bytes(size), 'config_readonly_sbss2_zero'
        if blob is None:
            return None, 'target_object_not_file_backed'
        pointers = {}
        for (owner, where), rel in elf.relocations.items():
            if not (0 < owner < len(elf.sections)):
                continue
            base = self.bases.get((unit, elf.names[owner]))
            if base is not None and start <= base + where < start + size:
                pointers[base + where - start] = {'symbol': rel.symbol.name, 'addend': rel.addend,
                                                 'relocation_kind': rel.kind}
        return {'symbol': obj['name'], 'section': obj['section'], 'start': start,
                'owning_unit': unit if obj['local'] else None,
                'size': size, 'reference_offset': address - start,
                'extent_proof': proof, 'hex': blob.hex(), 'pointers': pointers,
                'decode_words': [dict(offset=i, **decode(blob[i:i + 4])) for i in range(0, len(blob), 4)]}, None


# Indexed PPC memory operations: XO -> (mnemonic, width, store, integer-result).
INDEXED = {23: ('lwzx', 4, False, True), 55: ('lwzux', 4, False, True),
           87: ('lbzx', 1, False, True), 119: ('lbzux', 1, False, True),
           279: ('lhzx', 2, False, True), 311: ('lhzux', 2, False, True),
           343: ('lhax', 2, False, True), 375: ('lhaux', 2, False, True),
           535: ('lfsx', 4, False, False), 567: ('lfsux', 4, False, False),
           599: ('lfdx', 8, False, False), 631: ('lfdux', 8, False, False),
           151: ('stwx', 4, True, False), 215: ('stbx', 1, True, False),
           407: ('sthx', 2, True, False), 663: ('stfsx', 4, True, False),
           727: ('stfdx', 8, True, False)}


def memory_width(opcode):
    if opcode in (34, 35, 38, 39):
        return 1
    if 40 <= opcode <= 45:
        return 2
    return 8 if opcode in (50, 51, 54, 55) else 4


def register_flow(elf, function):
    """Meet identical register values at CFG joins; calls kill volatile GPRs."""
    from collections import deque
    sec, start, size = function.section, function.address, function.size
    inputs = {0: {}}
    pending = deque([0])
    queued = {0}
    while pending:
        off = pending.popleft()
        queued.discard(off)
        regs = dict(inputs[off])
        w = word(elf.contents[sec], start + off)
        op, rt, ra, rb = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        imm, xo = signed16(w & 65535), (w >> 1) & 1023
        rel = elf.instruction_relocation(sec, start + off)
        successors = [off + 4] if off + 4 < size else []
        if rel and rel.kind == 10:
            for reg in [0] + list(range(3, 13)):
                regs.pop(reg, None)
            if not w & 1:
                successors = []
        elif op in (16, 18) and w & 1:
            for reg in [0] + list(range(3, 13)):
                regs.pop(reg, None)
        elif op in (16, 18):
            displacement = signed16(w & 0xfffc) if op == 16 else (w & 0x3fffffc) - (0x4000000 if w & 0x2000000 else 0)
            destination = displacement - start if w & 2 else off + displacement
            successors = ([off + 4] if op == 16 and off + 4 < size else [])
            if 0 <= destination < size and destination % 4 == 0:
                successors.append(destination)
        elif op == 19:
            if xo in (16, 528):  # bclr/bcctr (possibly a linking indirect call)
                if w & 1:
                    for reg in [0] + list(range(3, 13)):
                        regs.pop(reg, None)
                elif (w >> 21) & 0x14 == 0x14:
                    successors = []  # unconditional return or indirect branch
                # A conditional return/branch can still fall through.
            elif xo == 50:  # rfi
                successors = []
            # CR logical operations (including crclr) and isync preserve GPR flow.
        elif op == 15:
            regs.pop(rt, None)
            if rel and rel.kind in (5, 6):
                regs[rt] = Pointer(rel, high=True)
        elif rel and rel.kind in (4, 109):
            if op == 14:
                regs[rt] = Pointer(rel)
            elif 32 <= op <= 55:
                if op <= 43:
                    regs.pop(rt, None)
                if op & 1:
                    regs.pop(ra, None)
        elif op == 14:
            base = regs.get(ra) if ra else None
            if isinstance(base, Pointer) and not base.high:
                regs[rt] = Pointer(base.relocation, base.delta + imm, dynamic=base.dynamic)
            else:
                regs.pop(rt, None)
        elif 32 <= op <= 55:
            if op <= 43:
                regs.pop(rt, None)
            if op & 1:
                regs.pop(ra, None)
        elif op == 31 and xo in INDEXED:
            if INDEXED[xo][3]:
                regs.pop(rt, None)
            if xo in (55, 119, 311, 375, 567, 631):
                regs.pop(ra, None)
        elif op == 31 and xo == 444 and rt == rb:
            if rt in regs:
                regs[ra] = regs[rt]
            else:
                regs.pop(ra, None)
        elif op == 31 and xo == 266:
            bases = [regs[r] for r in (ra, rb) if isinstance(regs.get(r), Pointer) and not regs[r].high]
            if len(bases) == 1:
                base = bases[0]
                regs[rt] = Pointer(base.relocation, base.delta, dynamic=True)
            else:
                regs.pop(rt, None)
        elif op in (10, 11, 36, 38, 44, 47, 48, 50, 52, 54, 59, 63):
            pass
        elif op in (20, 21, 23, 24, 25, 26, 27, 28, 29):
            regs.pop(ra, None)
        elif op == 31:
            regs.pop(rt, None)
            regs.pop(ra, None)
        else:
            regs.pop(rt, None)
        for destination in successors:
            previous = inputs.get(destination)
            merged = regs if previous is None else {r: value for r, value in previous.items() if regs.get(r) == value}
            if previous is None or previous != merged:
                inputs[destination] = dict(merged)
                if destination not in queued:
                    pending.append(destination)
                    queued.add(destination)
    return inputs


def trace(elf, function, unit, world):
    """Track relocated bases and constant offsets; unknown paths are advisory.

    Every data relocation is emitted even when a complete effective-address
    chain cannot be proven. Indexed events record unknown indexes explicitly.
    They request the complete base object, never a guessed indexed element.
    """
    regs, refs, issues = {}, [], []
    states = register_flow(elf, function)
    sec, start = function.section, function.address
    joins = set()
    for off in range(0, function.size, 4):
        w = word(elf.contents[sec], start + off)
        op = w >> 26
        if op in (16, 18) and not elf.instruction_relocation(sec, start + off):
            displacement = signed16(w & 0xfffc) if op == 16 else (w & 0x3fffffc) - (0x4000000 if w & 0x2000000 else 0)
            destination = displacement if w & 2 else off + displacement
            if 0 <= destination < function.size:
                joins.add(destination)

    def emit(off, w, pointer, kind, width=0, note=None):
        if world.is_data(elf, pointer):
            refs.append({'site': off, 'kind': kind, 'opcode': w >> 26,
                         'shape': w & (0xffff0000 if w >> 26 != 31 else 0xffffffff),
                         'width': width, 'dynamic_index': pointer.dynamic,
                         'pointer': pointer, 'advisory': note})

    for off in range(0, function.size, 4):
        if off not in states:
            continue
        regs = dict(states[off])
        w = word(elf.contents[sec], start + off)
        op, rt, ra, rb = w >> 26, (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        imm, xo = signed16(w & 65535), (w >> 1) & 1023
        rel = elf.instruction_relocation(sec, start + off)
        if rel and rel.kind not in (10, 26):
            emit(off, w, Pointer(rel), 'relocation', note='relocation_association_not_effective_address_proof')
        if rel and rel.kind == 10:
            for reg in [0] + list(range(3, 13)):
                regs.pop(reg, None)
            continue
        if op in (16, 18, 19):
            regs.clear()
            continue
        if op == 15:
            regs.pop(rt, None)
            if rel and rel.kind in (5, 6):
                regs[rt] = Pointer(rel, high=True)
            continue
        if rel and rel.kind in (4, 109):
            pointer = Pointer(rel)
            if op == 14:
                regs[rt] = pointer
                emit(off, w, pointer, 'address')
            elif 32 <= op <= 55:
                emit(off, w, pointer, 'memory', memory_width(op))
                if op <= 43:
                    regs.pop(rt, None)
            continue
        if op == 14:
            base = regs.get(ra) if ra else None
            if isinstance(base, Pointer) and not base.high:
                regs[rt] = Pointer(base.relocation, base.delta + imm, dynamic=base.dynamic)
                emit(off, w, regs[rt], 'address')
            else:
                regs.pop(rt, None)
            continue
        if 32 <= op <= 55:
            base = regs.get(ra)
            if isinstance(base, Pointer) and not base.high:
                emit(off, w, Pointer(base.relocation, base.delta + imm, dynamic=base.dynamic), 'memory',
                     memory_width(op))
            if op <= 43:
                regs.pop(rt, None)
            if op & 1:
                regs.pop(ra, None)
            continue
        if op == 31 and xo in INDEXED:
            mnemonic, width, store, integer = INDEXED[xo]
            bases = [regs[r] for r in (ra, rb) if isinstance(regs.get(r), Pointer) and not regs[r].high]
            if len(bases) == 1:
                base = bases[0]
                emit(off, w, Pointer(base.relocation, base.delta, dynamic=True), mnemonic, width,
                     'index_value_unknown_compare_whole_base_object')
            else:
                issues.append({'site': off, 'reason': 'indexed_base_unknown_or_ambiguous', 'mnemonic': mnemonic})
            if integer:
                regs.pop(rt, None)
            if xo in (55, 119, 311, 375, 567, 631):
                regs.pop(ra, None)
            continue
        if op == 31 and xo == 444 and rt == rb:
            if rt in regs:
                regs[ra] = regs[rt]
            else:
                regs.pop(ra, None)
            continue
        if op in (10, 11, 16, 18, 19, 36, 38, 44, 47, 48, 50, 52, 54, 59, 63):
            continue
        if op in (24, 25, 26, 27, 28, 29, 20, 21, 23):
            regs.pop(ra, None)
        elif op == 31:
            # Unknown extended operations can write RT or RA. Forget both.
            regs.pop(rt, None)
            regs.pop(ra, None)
        else:
            regs.pop(rt, None)
    return refs, issues


def instruction_alignment(target_words, source_words):
    """Map instruction offsets; nonexact bodies need unique three-word context.

    Relocation-masked words alone are not a same-offset pairing certificate when
    earlier instructions were inserted/outlined. Ambiguous runs stay unpaired.
    """
    if target_words == source_words:
        return {i * 4: i * 4 for i in range(len(target_words))}
    target_contexts = Counter(tuple(target_words[i:i + 3]) for i in range(len(target_words) - 2))
    source_contexts = Counter(tuple(source_words[i:i + 3]) for i in range(len(source_words) - 2))
    aligned = {}
    for t_start, s_start, size in SequenceMatcher(None, target_words, source_words, autojunk=False).get_matching_blocks():
        for delta in range(size - 2):
            context = tuple(target_words[t_start + delta:t_start + delta + 3])
            if target_contexts[context] == source_contexts[context] == 1:
                for within in range(3):
                    aligned[(t_start + delta + within) * 4] = (s_start + delta + within) * 4
    return aligned


def function_words(elf, function):
    result = []
    for offset in range(0, function.size, 4):
        at = function.address + offset
        value = word(elf.contents[function.section], at)
        rel = elf.instruction_relocation(function.section, at)
        if rel:
            value &= 0xfc000003 if rel.kind in (10, 26) else 0xffff0000
        result.append((value, rel.kind if rel else None,
                       rel.symbol.name if rel and rel.kind == 10 else None))
    return result


def pair_references(target, source, alignment):
    pairs, issues = [], []
    source_index = defaultdict(list)
    for index, ref in enumerate(source):
        source_index[ref['site'], ref['kind'], ref['opcode'], ref['shape']].append(index)
    used = set()
    for ref in target:
        key = alignment.get(ref['site']), ref['kind'], ref['opcode'], ref['shape']
        candidates = source_index.get(key, [])
        if len(candidates) == 1 and candidates[0] not in used:
            index = candidates[0]
            used.add(index)
            pairs.append((ref, source[index], 'aligned_instruction_context_kind_shape'))
        else:
            issues.append({'side': 'target', 'site': ref['site'], 'kind': ref['kind'],
                           'symbol': ref['pointer'].relocation.symbol.name, 'reason': 'reference_pairing_unproven'})
    for index, ref in enumerate(source):
        if index not in used:
            issues.append({'side': 'source', 'site': ref['site'], 'kind': ref['kind'],
                           'symbol': ref['pointer'].relocation.symbol.name, 'reason': 'reference_pairing_unproven'})
    return pairs, issues


def audit(tree, prefixes, unit_filter=None, source_override=None, target_override=None, max_object=65536):
    tree, world = Path(tree), World(tree)
    if isinstance(prefixes, str):
        prefixes = [prefixes]
    prefixes = [p.removeprefix('main/') for p in prefixes]
    configs = json.loads((tree / 'objdiff.json').read_text())['units']
    comparisons, advisories, stats, by_dir = [], [], Counter(), defaultdict(Counter)
    for config in configs:
        unit = config['name'].removeprefix('main/')
        if not any(unit.startswith(p) for p in prefixes) or unit_filter and unit != unit_filter:
            continue
        directory = '/'.join(unit.split('/')[:2])
        counts = by_dir[directory]
        if not config.get('base_path') or not config.get('target_path'):
            advisories.append({'unit': unit, 'reason': 'missing_object_config'})
            counts['units_missing'] += 1
            continue
        target_path = Path(target_override) if target_override else tree / config['target_path']
        source_path = Path(source_override) if source_override else tree / config['base_path']
        if not target_path.exists() or not source_path.exists():
            advisories.append({'unit': unit, 'reason': 'missing_existing_object'})
            counts['units_missing'] += 1
            continue
        target, source = Elf(target_path), Elf(source_path)
        counts['units'] += 1
        seen = {}
        for name, tf in target.functions.items():
            if name.startswith('gap_'):
                continue
            counts['target_functions'] += 1
            sf = source.functions.get(name)
            if sf is None:
                counts['unpaired_target_functions'] += 1
                advisories.append({'unit': unit, 'function': name, 'reason': 'unpaired_target_function'})
                continue
            counts['paired_functions'] += 1
            tr, ti = trace(target, tf, unit, world)
            sr, si = trace(source, sf, unit, world)
            counts['target_references'] += len(tr)
            counts['source_references'] += len(sr)
            counts['target_indexed_references'] += sum(r['kind'] in {x[0] for x in INDEXED.values()} for r in tr)
            target_words, source_words = function_words(target, tf), function_words(source, sf)
            alignment = instruction_alignment(target_words, source_words)
            pairs, failures = pair_references(tr, sr, alignment)
            for side, issues in [('target', ti), ('source', si), ('pair', failures)]:
                advisories.extend(dict(unit=unit, function=name, trace_side=side, **issue) for issue in issues)
            for t_ref, s_ref, evidence in pairs:
                t_obj, t_error = world.target_bound(target, unit, t_ref['pointer'], max_object)
                s_obj, s_error = source.bound(s_ref['pointer'], max_object)
                caller = {'function': name, 'target_site': t_ref['site'], 'source_site': s_ref['site'],
                          'reference_kind': t_ref['kind'], 'width': t_ref['width'],
                          'dynamic_index': t_ref['dynamic_index'], 'pairing': evidence}
                if t_error or s_error:
                    advisories.append({'unit': unit, **caller, 'reason': 'object_resolution_failed',
                                       'target_reason': t_error, 'source_reason': s_error,
                                       'target_symbol': t_ref['pointer'].relocation.symbol.name,
                                       'source_symbol': s_ref['pointer'].relocation.symbol.name})
                    continue
                caller['target_object_offset'] = t_obj['reference_offset']
                caller['source_object_offset'] = s_obj['reference_offset']
                key = (name, t_obj['start'], t_obj['size'],
                       s_obj['section_index'], s_obj['start'], s_obj['size'],
                       t_obj['reference_offset'] - s_obj['reference_offset'])
                if key in seen:
                    seen[key]['references'].append(caller)
                    continue
                result = compare_bytes(bytes.fromhex(t_obj['hex']), bytes.fromhex(s_obj['hex']),
                                       t_obj['reference_offset'], s_obj['reference_offset'],
                                       t_obj['pointers'], s_obj['pointers'])
                row = {'unit': unit, 'function': name, 'target_object_file': str(target_path),
                       'source_object_file': str(source_path), 'target_object': t_obj,
                       'source_object': s_obj, 'references': [caller], **result}
                seen[key] = row
                comparisons.append(row)
                counts['object_comparisons'] += 1
                counts[result['status']] += 1
                counts['scalar_different_words'] += len(result['scalar_differences'])
        # Source-only functions are also enumerated, never silently certified.
        for name in source.functions.keys() - target.functions.keys():
            counts['unpaired_source_functions'] += 1
            advisories.append({'unit': unit, 'function': name, 'reason': 'unpaired_source_function'})
    for counts in by_dir.values():
        stats.update(counts)
    stats['advisories'] = len(advisories)
    return {'warning': 'Advisory existing-object audit, not a correctness/equivalence certificate. '
                       'Same-site pairing can be incomplete after codegen changes. Runtime mutable data is initial image only. '
                       'Unknown bounds/addresses/pointers are never equality evidence.',
            'prefixes': prefixes, 'stats': dict(stats), 'by_directory': {k: dict(v) for k, v in by_dir.items()},
            'mismatches': [r for r in comparisons if r['scalar_differences'] or r['status'] == 'pointer_difference'],
            'comparisons': comparisons, 'advisories': advisories}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('prefixes', nargs='+', help='Unit prefixes, e.g. Game/Ride Game/AreaObj')
    parser.add_argument('--tree', type=Path, default=Path.cwd())
    parser.add_argument('--unit', dest='unit_filter')
    parser.add_argument('--source-object', type=Path)
    parser.add_argument('--target-object', type=Path)
    parser.add_argument('--max-object', type=int, default=65536)
    parser.add_argument('--out', type=Path, help='JSON report path (otherwise stdout)')
    parser.add_argument('--mismatches-only', action='store_true', help='Omit complete comparison list')
    args = parser.parse_args()
    if (args.source_object or args.target_object) and not args.unit_filter:
        parser.error('object overrides require --unit')
    try:
        result = audit(args.tree.resolve(), args.prefixes, args.unit_filter,
                       args.source_object, args.target_object, args.max_object)
        if args.mismatches_only:
            result.pop('comparisons')
        text = json.dumps(result, indent=2, allow_nan=False) + '\n'
        if args.out:
            args.out.write_text(text)
            print(json.dumps({'out': str(args.out), 'stats': result['stats'], 'by_directory': result['by_directory']}))
        else:
            print(text, end='')
    except (OSError, ValueError, KeyError, IndexError, struct.error) as error:
        parser.exit(1, f'datacontent: {error}\n')


if __name__ == '__main__':
    main()
