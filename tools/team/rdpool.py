"""Compare complete referenced pool objects against DOL bytes and ELF relocations.

Only constant, file-backed, read-only reaches are supported. This supplies
additional evidence to rdchecked's string-base guard, never a standalone proof.
"""
import collections
import hashlib
from pathlib import Path
import re
import struct


class PoolUnproved(ValueError):
    pass


class PoolExtentUnproved(PoolUnproved):
    pass


class PoolELF:
    def __init__(self, path):
        raw = Path(path).read_bytes()
        if len(raw) < 52 or raw[:6] != b'\x7fELF\x01\x02':
            raise PoolUnproved('pool object is not big-endian ELF32')
        header = struct.unpack_from('>16sHHIIIIIHHHHHH', raw)
        start, stride, count = header[6], header[11], header[12]
        if stride != 40 or start + count * stride > len(raw):
            raise PoolUnproved('invalid pool ELF section table')
        sections = [struct.unpack_from('>10I', raw, start + i * stride) for i in range(count)]
        self.data, self.types = [], []
        for section in sections:
            self.types.append(section[1])
            if section[1] == 8:
                self.data.append(None)
            else:
                off, size = section[4:6]
                if off + size > len(raw):
                    raise PoolUnproved('truncated pool ELF section')
                self.data.append(raw[off:off + size])
        shstrings = self.data[header[13]]
        self.names = [self.string(shstrings, s[0]) for s in sections]
        self.objects = collections.defaultdict(list)
        tables = {}
        for si, section in enumerate(sections):
            if section[1] != 2:
                continue
            strings = self.data[section[6]]
            if section[9] != 16 or section[5] % 16:
                raise PoolUnproved('invalid pool ELF symbol table')
            table = []
            for off in range(0, section[5], 16):
                name, address, size, info, _, sec = struct.unpack_from('>IIIBBH', self.data[si], off)
                symbol = (self.string(strings, name), address, size, sec)
                table.append(symbol)
                if info & 15 == 1 and size and 0 < sec < count:
                    self.objects[sec].append((address, size))
            tables[si] = table
        self.relocations = {}
        for si, section in enumerate(sections):
            if section[1] != 4:
                continue
            if section[9] != 12 or section[5] % 12:
                raise PoolUnproved('invalid pool ELF relocation table')
            for off in range(0, section[5], 12):
                address, info, addend = struct.unpack_from('>IIi', self.data[si], off)
                key = (section[7], address)
                if key in self.relocations:
                    raise PoolUnproved('overlapping pool relocations')
                self.relocations[key] = (info & 255, tables[section[6]][info >> 8], addend)

    @staticmethod
    def string(data, offset):
        if data is None or not 0 <= offset < len(data):
            raise PoolUnproved('invalid ELF string offset')
        end = data.find(b'\0', offset)
        if end < 0:
            raise PoolUnproved('unterminated ELF string')
        return data[offset:end].decode('utf-8')


class PoolCatalog(dict):
    pass


def pool_catalog(path):
    result = PoolCatalog()
    result.extents = collections.defaultdict(set)
    for line in Path(path).read_text().splitlines():
        obj = re.match(r'\S+ = \.\w+:0x([0-9A-Fa-f]+);.*type:object.*size:0x([0-9A-Fa-f]+)', line)
        if obj and int(obj[2], 16):
            result.extents[int(obj[1], 16)].add(int(obj[2], 16))
        match = re.match(r'(\S+) = \.\w+:0x([0-9A-Fa-f]+);', line)
        if match:
            name, address = match[1], int(match[2], 16)
            if name in result and result[name] != address:
                result[name] = None
            else:
                result.setdefault(name, address)
    return result


class PoolComparator:
    def __init__(self, read_target, elf, catalog, target_origins, source_origins, target_extents=None):
        self.read_target, self.elf, self.catalog = read_target, elf, catalog
        self.target_origins, self.source_origins = target_origins, source_origins
        self.target_extents = getattr(catalog, 'extents', {}) if target_extents is None else target_extents
        self.cache, self.active, self.evidence = set(), set(), []
        self.active_pool_ranges = []

    def source(self, sec, offset, size):
        if not 0 <= sec < len(self.elf.data) or self.elf.data[sec] is None:
            raise PoolUnproved('pool reference is not file-backed')
        data = self.elf.data[sec]
        if size <= 0 or offset < 0 or offset + size > len(data):
            raise PoolUnproved('pool reference exceeds source section')
        return data[offset:offset + size]

    def extent(self, sec, offset):
        objects = [(start, size) for start, size in self.elf.objects.get(sec, [])
                   if start <= offset < start + size]
        exact = [(start, size) for start, size in objects if start == offset]
        candidates = exact or objects
        if not candidates:
            raise PoolUnproved('address-taken pool reference lacks a complete object extent')
        smallest = min(size for _, size in candidates)
        candidates = set((start, size) for start, size in candidates if size == smallest)
        if len(candidates) != 1:
            raise PoolUnproved('ambiguous pool object extent')
        return next(iter(candidates))

    def verify(self, token, displacement, width=None):
        if not isinstance(displacement, int):
            raise PoolUnproved('unknown pool displacement')
        targets = set(self.target_origins.get(token, ()))
        sources = set(self.source_origins.get(token, ()))
        if len(targets) != 1 or len(sources) != 1:
            raise PoolUnproved('missing/ambiguous original string-base origins')
        target_base = next(iter(targets))
        name, source_base = next(iter(sources))
        if self.elf.names.count(name) != 1:
            raise PoolUnproved('ambiguous source pool section')
        sec = self.elf.names.index(name)
        delta = target_base - source_base
        offset = source_base + displacement
        if width is None:
            start, size = self.extent(sec, offset)
        else:
            if width not in (1, 2, 4, 8):
                raise PoolUnproved('unsupported pool load width')
            start, size = offset, width
        self.source(sec, start, size)
        try:
            self.compare(delta + start, sec, start, size, {sec: delta}, width is None)
        except PoolExtentUnproved:
            # Coarse target labels can group several source literals. A whole
            # pool match compares those bytes too, rather than discarding a tail.
            data = self.elf.data[sec]
            if data is None or not data:
                raise
            self.compare(delta, sec, 0, len(data), {sec: delta}, False, True)
        # A loaded internal pointer retains its pool provenance for later reaches.
        if width is not None:
            rels = [(roff, rel) for (rsec, roff), rel in self.elf.relocations.items()
                    if rsec == sec and roff < offset + width and offset < roff + 4]
            if rels:
                if width != 4 or len(rels) != 1 or rels[0][0] != offset:
                    return ('opaque-pointer', None)
                _, (kind, symbol, addend) = rels[0]
                name, value, _, symsec = symbol
                if 0 < symsec < len(self.elf.data) and self.elf.names[symsec] in ('.data', '.rodata', '.sdata', '.sdata2'):
                    raw = self.read_target(target_base + displacement, 4)
                    if raw is None or len(raw) != 4:
                        raise PoolUnproved('loaded pool pointer is not DOL-backed')
                    destination = struct.unpack('>I', raw)[0]
                    source_destination = value + addend
                    pointer_token = f'POOLPTR[{symsec}:{source_destination:x}:{destination:x}]'
                    self.target_origins[pointer_token] = {destination}
                    self.source_origins[pointer_token] = {(self.elf.names[symsec], source_destination)}
                    return (pointer_token, 0)
        return None

    def compare(self, address, sec, offset, size, section_deltas, full_object=False, whole_pool=False):
        key = (address, sec, offset, size, tuple(sorted(section_deltas.items())), full_object, whole_pool)
        if key in self.cache or key in self.active:
            return
        if len(self.active) >= 32:
            raise PoolUnproved('pool pointer recursion limit')
        source = bytearray(self.source(sec, offset, size))
        target = self.read_target(address, size)
        if target is None or len(target) != size:
            raise PoolUnproved('pool reference is not DOL-backed')
        if full_object:
            extents = set(self.target_extents.get(address, ()))
            if len(extents) != 1:
                raise PoolExtentUnproved('missing/ambiguous complete target pool object extent')
            target_size = next(iter(extents))
            if target_size < size:
                raise PoolExtentUnproved('target pool object is shorter than source object')
            if target_size > size:
                padding = self.read_target(address + size, target_size - size)
                if padding is None or len(padding) != target_size - size or any(padding):
                    raise PoolExtentUnproved('target pool object extra tail is not zero padding')
        if whole_pool:
            if len(self.target_extents.get(address, ())) != 1:
                raise PoolUnproved('missing/ambiguous target pool start boundary')
            end = address + size
            known_end = address
            for start, extents in self.target_extents.items():
                for extent in extents:
                    if start < end and address < start + extent:
                        known_end = max(known_end, start + extent)
                        if start < address:
                            raise PoolUnproved('target pool object begins before checked source pool')
                        if start + extent > end:
                            extra = self.read_target(end, start + extent - end)
                            if extra is None or len(extra) != start + extent - end or any(extra):
                                raise PoolUnproved('target object extends beyond checked source pool')
            if known_end < end and any(source[known_end - address:]):
                raise PoolUnproved('source pool has non-padding bytes past known target boundary')
            self.active_pool_ranges.append((sec, address - offset, offset, offset + size))
        self.active.add(key)
        relocated = []
        try:
            for (rsec, roff), (kind, symbol, addend) in self.elf.relocations.items():
                if rsec != sec or roff + 4 <= offset or roff >= offset + size:
                    continue
                if kind != 1 or roff < offset or roff + 4 > offset + size:
                    raise PoolUnproved('unsupported/partial pool relocation')
                name, value, extent, symsec = symbol
                if name in self.catalog and self.catalog[name] is None:
                    raise PoolUnproved('ambiguous pool relocation identity: ' + name)
                mapped = self.catalog.get(name)
                internal = mapped is None and symsec in section_deltas
                if mapped is None and not internal:
                    raise PoolUnproved('unresolved pool relocation identity: ' + name)
                expected = ((section_deltas[symsec] + value if internal else mapped) + addend) & 0xffffffff
                actual = struct.unpack_from('>I', target, roff - offset)[0]
                if actual != expected:
                    raise PoolUnproved('different pool relocation identity: ' + name)
                struct.pack_into('>I', source, roff - offset, expected)
                relocated.append((roff, name, addend, expected))
                defined_data = (0 < symsec < len(self.elf.data) and
                                self.elf.names[symsec] in ('.data', '.rodata', '.sdata', '.sdata2'))
                if internal or defined_data:
                    objstart, objsize = self.extent(symsec, value + addend)
                    pointee_delta = expected - (value + addend)
                    pointee_deltas = {**section_deltas, symsec: pointee_delta}
                    covered = any(psec == symsec and pdelta == pointee_delta and
                                  first <= objstart and objstart + objsize <= last
                                  for psec, pdelta, first, last in self.active_pool_ranges)
                    if not covered:
                        self.compare(pointee_delta + objstart, symsec, objstart,
                                     objsize, pointee_deltas, True)
            if bytes(source) != target:
                raise PoolUnproved('different referenced pool bytes')
            self.cache.add(key)
            self.evidence.append({'target_address': hex(address), 'source_section': self.elf.names[sec],
                                  'source_offset': offset, 'size': size, 'whole_pool': whole_pool,
                                  'resolved_relocations': relocated,
                                  'target_sha256': hashlib.sha256(target).hexdigest(),
                                  'source_relocated_sha256': hashlib.sha256(source).hexdigest()})
        finally:
            self.active.remove(key)
            if whole_pool:
                self.active_pool_ranges.pop()


def read_pool_dol(data, address, size):
    if size <= 0:
        return None
    for i in range(18):
        off, base, extent = (struct.unpack_from('>I', data, start + i * 4)[0]
                             for start in (0, 0x48, 0x90))
        if extent and base <= address and address + size <= base + extent:
            position = off + address - base
            return data[position:position + size] if position + size <= len(data) else None
    return None
