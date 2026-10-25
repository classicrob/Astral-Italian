#!/usr/bin/env python3
"""Read-only SB4E01 data-boundary triage from existing target/source objects.

No compiler, Ninja, configure or split is invoked. References are observations,
not ownership proofs: shared globals, pooled strings and allocation padding can
legitimately cross a named symbol's extent. Findings preserve that distinction.
"""
from __future__ import annotations

import argparse
from bisect import bisect_right
from collections import Counter, defaultdict, deque
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field, replace
from functools import lru_cache
import csv
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess
import time

from rdpool import PoolELF, PoolUnproved, read_pool_dol

DATA = {'.data', '.rodata', '.sdata', '.sdata2', '.bss', '.sbss', '.sbss2'}
CODE = {'.text', '.init'}
SCRATCH = Path.home() / 'MacioGalaxy2-team/scratch-decomp2/rules'


def norm(name):
    return re.sub(r'\.(?:cpp|c|s|S)$', '', name.removeprefix('main/').removeprefix('src/'))


def digest(data): return hashlib.sha256(data).hexdigest()
def signed(value, bits): return value - (1 << bits) if value & (1 << (bits-1)) else value
def hx(value): return '' if value is None else f'0x{value:08X}'


@dataclass(frozen=True)
class Symbol:
    name: str
    section: str
    address: int
    size: int = 0
    kind: str = ''
    scope: str = ''
    line: int = 0


class Catalog:
    def __init__(self, text):
        self.names, self.sections = defaultdict(list), defaultdict(list)
        self.dol = None
        for line, raw in enumerate(text.splitlines(), 1):
            m = re.match(r'(\S+)\s*=\s*(\S+):0x([0-9a-fA-F]+);(.*)', raw)
            if not m: continue
            attrs = dict(re.findall(r'(\w+):([^\s;]+)', m[4]))
            sym = Symbol(m[1], m[2], int(m[3], 16), int(attrs.get('size', '0'), 0), attrs.get('type', ''), attrs.get('scope', ''), line)
            self.names[sym.name].append(sym)
            self.sections[sym.section].append(sym)
        self.starts = {}
        self.bounds = {}
        self.objects = {}
        self.object_starts = {}
        for section, syms in self.sections.items():
            syms.sort(key=lambda s: (s.address, s.name))
            self.starts[section] = [s.address for s in syms]
            self.objects[section] = [s for s in syms if s.kind=='object']
            self.object_starts[section] = [s.address for s in self.objects[section]]
            if section in DATA and syms:
                self.bounds[section] = (syms[0].address, max(s.address+max(s.size, 1) for s in syms))

    def named(self, name, address=None):
        found = self.names.get(name, [])
        if address is not None: found = [s for s in found if s.address == address]
        return found[0] if len(found) == 1 else None

    def section_at(self, address):
        found = [s for s, (a, b) in self.bounds.items() if a <= address < b]
        return found[0] if len(found) == 1 else None

    def covering(self, section, address):
        syms = self.sections.get(section, [])
        i = bisect_right(self.starts.get(section, []), address)-1
        if i < 0: return None
        start = syms[i].address
        candidates = []
        while i >= 0 and syms[i].address == start:
            s = syms[i]
            if s.kind == 'object' and s.size and address < s.address+s.size: candidates.append(s)
            i -= 1
        return min(candidates, key=lambda s: s.size) if candidates else None

    def next_object(self, section, address):
        starts=self.object_starts.get(section,[])
        i=bisect_right(starts,address)
        return starts[i] if i<len(starts) else None


class Splits:
    def __init__(self, text):
        self.units, self.lines, current = {}, {}, None
        for line, raw in enumerate(text.splitlines(), 1):
            if raw and not raw[0].isspace() and raw.rstrip().endswith(':'):
                current = None if raw.strip() == 'Sections:' else norm(raw.strip()[:-1])
                if current is not None:
                    if current in self.units: raise ValueError('duplicate split unit '+current)
                    self.units[current] = {}; self.lines[current] = line
            elif current is not None:
                m = re.match(r'\s+(\S+)\s+start:0x([0-9a-fA-F]+)\s+end:0x([0-9a-fA-F]+)', raw)
                if m:
                    a, b = int(m[2], 16), int(m[3], 16)
                    if b < a: raise ValueError('reversed split range')
                    self.units[current][m[1]] = (a, b)
        self.order = {u: i for i, u in enumerate(self.units)}
        self.ranges = defaultdict(list)
        for u, sections in self.units.items():
            for s, (a, b) in sections.items(): self.ranges[s].append((a, b, u))
        for ranges in self.ranges.values(): ranges.sort()
        self.starts = {s: [r[0] for r in rs] for s, rs in self.ranges.items()}
        self.max_ends = {}
        for section, ranges in self.ranges.items():
            maximum=0; ends=[]
            for _,end,_ in ranges: maximum=max(maximum,end);ends.append(maximum)
            self.max_ends[section]=ends

    @lru_cache(maxsize=131072)
    def owners(self, section, address):
        # Overlaps are retained, rather than choosing one owner arbitrarily.
        ranges=self.ranges.get(section,[]);maximum=self.max_ends.get(section,[])
        i=bisect_right(self.starts.get(section,[]),address)-1;found=[]
        while i>=0 and maximum[i]>address:
            if ranges[i][1]>address: found.append(ranges[i][2])
            i-=1
        return tuple(sorted(found))

    @lru_cache(maxsize=None)
    def expected(self, unit, section):
        own = self.units[unit].get(section)
        if own: return own
        before, after = [], []
        rank = self.order[unit]
        for u, sections in self.units.items():
            if section not in sections: continue
            if self.order[u] < rank: before.append((self.order[u], sections[section][1]))
            elif self.order[u] > rank: after.append((self.order[u], sections[section][0]))
        return (max(before)[1] if before else None, min(after)[1] if after else None)


@dataclass(frozen=True)
class ElfSymbol:
    name: str
    value: int
    size: int
    section: int
    kind: int
    binding: int
    absolute: int | None


class ELF(PoolELF):
    """Reuse PoolELF's checked data/RELA parser; expose symbols and VIRT notes."""
    def __init__(self, path, target=False):
        before = path.stat()
        super().__init__(path)
        raw = path.read_bytes()
        after = path.stat()
        if (before.st_mtime_ns, before.st_size) != (after.st_mtime_ns, after.st_size):
            raise ValueError('object changed while reading: '+str(path))
        self.sha256 = digest(raw)
        header = struct.unpack_from('>16sHHIIIIIHHHHHH', raw)
        self.headers = [struct.unpack_from('>10I', raw, header[6]+i*40) for i in range(header[12])]
        self.sizes = [h[5] for h in self.headers]
        virtual = None
        if target and '.note.split' in self.names:
            note = self.data[self.names.index('.note.split')]; pos = 0
            while pos+12 <= len(note):
                namesz, descsz, kind = struct.unpack_from('>III', note, pos); pos += 12
                if pos+((namesz+3)&~3)+((descsz+3)&~3) > len(note): raise ValueError('truncated .note.split')
                name = note[pos:pos+namesz]; pos += (namesz+3)&~3
                desc = note[pos:pos+descsz]; pos += (descsz+3)&~3
                if name == b'Split\0' and kind == int.from_bytes(b'VIRT', 'big'):
                    if descsz % 4: raise ValueError('invalid VIRT note')
                    virtual = struct.unpack('>'+str(descsz//4)+'I', desc)
        self.symbols = []
        for si, h in enumerate(self.headers):
            if h[1] != 2: continue
            count = h[5]//16
            if virtual is not None and len(virtual) != count: raise ValueError('VIRT/symbol count mismatch')
            for index, off in enumerate(range(0, h[5], 16)):
                name, value, size, info, _, sec = struct.unpack_from('>IIIBBH', self.data[si], off)
                absolute = virtual[index] if virtual is not None and virtual[index] else None
                self.symbols.append(ElfSymbol(self.string(self.data[h[6]], name), value, size, sec, info&15, info>>4, absolute))
        self.origins = {}
        for sym in self.symbols:
            if sym.kind == 3 and sym.absolute is not None and 0 < sym.section < len(self.names):
                self.origins[sym.section] = sym.absolute-sym.value
        self.tuples = defaultdict(list)
        for s in self.symbols: self.tuples[s.name, s.value, s.size, s.section].append(s)

    def functions(self):
        return [s for s in self.symbols if s.kind == 2 and s.size and 0 < s.section < len(self.names)
                and self.names[s.section] in CODE and not s.name.startswith('gap_')]


@dataclass(frozen=True)
class Value:
    value: int
    section: str = ''
    symbol: str = ''
    anchor: int = 0
    high: bool = False
    external: bool = False
    indirect: bool = False


@dataclass(frozen=True)
class Ref:
    function: str
    site: int
    section: str
    address: int
    symbol: str
    anchor: int
    width: int
    mode: str
    immediate: int | None = None
    word: int = 0
    external: bool = False
    indirect: bool = False


def resolve(elf, rel, catalog, target):
    kind, symbol, addend = rel
    name, value, size, sec = symbol
    # Function/branch relocations are not data references.
    if kind in (10, 11, 12, 13, 18, 23, 26): return None
    if 0 < sec < len(elf.names):
        section = elf.names[sec]
        if section not in DATA: return None
        if target:
            candidates = {s.absolute for s in elf.tuples.get(symbol, []) if s.absolute is not None}
            if len(candidates) == 1: address = next(iter(candidates))
            elif sec in elf.origins: address = elf.origins[sec]+value
            else:
                configured = catalog.named(name)
                if configured is None: return None
                address = configured.address
        else: address = value
        return Value(address+addend, section, name, address, external=False)
    sym = catalog.named(name)
    if sym is None or sym.section not in DATA: return None
    return Value(sym.address+addend, sym.section, name, sym.address, external=not target)


WIDTHS = {32:4,33:4,34:1,35:1,36:4,37:4,38:1,39:1,40:2,41:2,42:2,43:2,
          44:2,45:2,48:4,49:4,50:8,51:8,52:4,53:4,54:8,55:8}


def initializer_pointer(elf,address,catalog,target):
    """Follow a pointer's ELF initializer, explicitly retaining its uncertainty.

    Mutable memory may hold a different pointer at runtime. These edges expose
    initial data ownership only; they never establish a runtime extent proof.
    """
    if address.external: return None
    sections=[i for i,name in enumerate(elf.names) if name==address.section]
    if len(sections)!=1:return None
    section=sections[0]
    if elf.data[section] is None:return None
    if target:
        if section not in elf.origins:return None
        offset=address.value-elf.origins[section]
    else:offset=address.value
    if offset<0 or offset%4 or offset+4>len(elf.data[section]):return None
    relocation=elf.relocations.get((section,offset))
    if not relocation or relocation[0] not in (1,24):return None
    value=resolve(elf,relocation,catalog,target)
    return replace(value,indirect=True) if value else None


def references(elf, func, catalog, target):
    """Conservative forward constant/provenance analysis, meeting at CFG joins.

    Unsupported GPR writers kill values. Calls kill volatile GPRs. A join keeps
    a value only when every reachable predecessor agrees; loops converge by
    losing facts. Only relocatable pointer initializers are followed, marked as
    hypothetical initializer edges. Unknown indexed addresses are not followed.
    """
    blob = elf.data[func.section]
    if blob is None or func.value+func.size > len(blob) or func.size%4: return [], 1
    words = list(struct.unpack('>'+str(func.size//4)+'I', blob[func.value:func.value+func.size]))
    n = len(words); incoming = {0:{}}; queue = deque([0]); observed = {}; visits = Counter()
    unresolved = set()
    def literal(value):
        section = catalog.section_at(value) if target else None
        sym = catalog.covering(section, value) if section else None
        return Value(value, section or '', sym.name if sym else '', sym.address if sym else value)
    def emit(i, value, width, mode, immediate=None):
        if value.section in DATA and not value.high:
            if value.indirect:mode='initializer_'+mode
            observed[i, mode] = Ref(func.name, func.value+i*4, value.section, value.value, value.symbol,
                                    value.anchor, width, mode, immediate, words[i], value.external,value.indirect)
    while queue:
        i = queue.popleft(); visits[i] += 1
        if visits[i] > 64:
            raise ValueError('CFG convergence limit in '+func.name)
        state = dict(incoming[i]); word = words[i]; op = word>>26
        rt, ra, rb = (word>>21)&31, (word>>16)&31, (word>>11)&31
        imm = signed(word&0xffff,16); off = func.value+i*4
        rel = elf.relocations.get((func.section,off+2)) or elf.relocations.get((func.section,off))
        value = resolve(elf,rel,catalog,target) if rel else None
        annotation = value if rel and rel[0]==0 else None
        if annotation is not None: value=None
        if rel and value is None and rel[0] not in (10,11,12,13,18,23,26): unresolved.add(off)
        # Remove a prior optimistic observation when a join loses its provenance.
        for key in [k for k in observed if k[0] == i]: del observed[key]
        if value: emit(i,value,WIDTHS.get(op,0),'relocation')
        if op == 15:
            if value:
                high=(value.value+(0x8000 if rel[0]==6 else 0))&0xffff0000
                state[rt] = Value(high,value.section,value.symbol,value.anchor,True,value.external)
            elif ra == 0: state[rt] = Value((imm<<16)&0xffffffff, high=True)
            else: state.pop(rt,None)
        elif op == 14:
            if value:
                base=state.get(ra)
                expected=(value.value+0x8000)&0xffff0000
                if rel[0]==109 or (base is not None and base.high and base.value==expected): state[rt] = value
                else: state.pop(rt,None)
            elif ra == 0: state[rt] = literal(imm&0xffffffff)
            elif ra in state:
                base = state[ra]
                number = (base.value+imm)&0xffffffff
                if base.high and base.section: state.pop(rt,None)
                else:
                    result = literal(number) if not base.section else Value(number,base.section,base.symbol,base.anchor,external=base.external,indirect=base.indirect)
                    state[rt] = result; emit(i,result,0,'derived_addi',imm)
            else: state.pop(rt,None)
        elif op in WIDTHS or op in (46,47,56,57,60,61):
            width = WIDTHS.get(op, (32-rt)*4 if op in (46,47) else 4 if word&(1<<15) else 8)
            displacement = signed(word&0xfff,12) if op in (56,57,60,61) else imm
            access=value
            if not value and ra!=0 and ra in state and not state[ra].high:
                base = state[ra]
                access=Value((base.value+displacement)&0xffffffff,base.section,base.symbol,base.anchor,external=base.external,indirect=base.indirect)
                emit(i,access,width,'derived_memory',displacement)
            load = op in (32,33,34,35,40,41,42,43,46)
            if load:
                for register in range(rt,32) if op==46 else [rt]: state.pop(register,None)
            if op in (32,33) and access is not None:
                pointer=initializer_pointer(elf,access,catalog,target)
                if pointer is not None:
                    state[rt]=pointer;emit(i,pointer,0,'pointer')
            if op in (33,35,37,39,41,43,45,49,51,53,55,57,61): state.pop(ra,None)
        elif op == 24:
            if value:
                base=state.get(rt)
                if base is not None and base.high and base.value==(value.value&0xffff0000):state[ra]=value
                else:state.pop(ra,None)
            elif rt in state and not state[rt].section:
                state[ra] = literal((state[rt].value | (word&0xffff))&0xffffffff)
                emit(i,state[ra],0,'raw_lis_ori',word&0xffff)
            else: state.pop(ra,None)
        elif op == 31:
            xo = (word>>1)&1023
            if xo == 444 and rt == rb:
                if rt in state: state[ra] = state[rt]
                else: state.pop(ra,None)
            elif xo not in (0,32,144,467,598,854):
                # Unknown register-form writer: over-kill instead of inventing a reach.
                state.pop(rt,None); state.pop(ra,None)
        elif op in (7,8,12,13): state.pop(rt,None)
        elif op in (20,21,23,25,26,27,28,29): state.pop(ra,None)
        elif op not in (10,11,16,17,18,19,59,63):
            state.clear()
        successors = [i+1] if i+1<n else []
        if op in (16,18):
            if word&1:
                for reg in (0,*range(3,13)): state.pop(reg,None)
            else:
                delta = signed(word&0xfffc,16) if op==16 else signed(word&0x3fffffc,26)
                destination = delta if word&2 else off+delta
                local = (destination-func.value)//4
                successors = ([i+1] if op==16 and i+1<n else []) + ([local] if func.value<=destination<func.value+func.size and destination%4==0 else [])
        elif op == 19:
            xo = (word>>1)&1023
            if xo in (16,528):
                if word&1:
                    for reg in (0,*range(3,13)): state.pop(reg,None)
                else:
                    # Conditional return retains only its fall-through path.
                    successors = [i+1] if ((word>>21)&20)!=20 and i+1<n else []
        for nxt in set(successors):
            if nxt not in incoming:
                incoming[nxt] = dict(state); queue.append(nxt)
            else:
                merged = {r:v for r,v in incoming[nxt].items() if state.get(r)==v}
                if merged != incoming[nxt]: incoming[nxt] = merged; queue.append(nxt)
    return list(observed.values()), len(unresolved)


@dataclass
class UnitScan:
    unit: str
    refs: list = field(default_factory=list)
    functions: dict = field(default_factory=dict)
    sizes: list = field(default_factory=list)
    shifts: list = field(default_factory=list)
    target_hash: str = ''
    source_hash: str = ''
    unresolved: int = 0
    error: str = ''
    source_missing: bool = False


def size_issues(target, source, catalog):
    ours = defaultdict(list)
    for s in source.symbols:
        if s.kind==1 and 0<s.section<len(source.names) and source.names[s.section] in DATA:
            ours[s.name].append(s)
    issues = []
    for s in target.symbols:
        if s.kind!=1 or not s.size or not 0<s.section<len(target.names) or target.names[s.section] not in DATA: continue
        absolute = s.absolute
        if absolute is None and s.section in target.origins: absolute=target.origins[s.section]+s.value
        configured = catalog.named(s.name,absolute)
        if configured is None or configured.kind!='object': continue
        if absolute is None:absolute=configured.address
        matches = ours.get(s.name,[])
        if len(matches)!=1: continue
        other=matches[0]
        if other.size==configured.size and s.size==configured.size: continue
        delta=absolute-other.value
        children=sorted([(o.name,o.value-other.value,o.size) for values in ours.values() for o in values
                  if o.section==other.section and other.value<=o.value<other.value+configured.size],key=lambda c:(c[1],c[0]))
        section=target.names[s.section]
        next_address=catalog.next_object(section,absolute)
        byte_match=None;byte_reason='no file-backed source/DOL extent'
        source_data=getattr(source,'data',[])
        if catalog.dol is not None and other.section<len(source_data) and source_data[other.section] is not None and 0<other.size<=0x10000:
            blob=source_data[other.section][other.value:other.value+other.size]
            relocated=any(sec==other.section and off<other.value+other.size and off+4>other.value for sec,off in source.relocations)
            target_bytes=read_pool_dol(catalog.dol,absolute,other.size)
            if relocated:byte_reason='source extent contains relocations; raw bytes not comparable'
            elif target_bytes is not None and len(blob)==other.size:
                byte_match=blob==target_bytes;byte_reason='full source extent compared with original DOL bytes'
        issues.append(dict(section=section,symbol=s.name,address=absolute,target_size=s.size,
                           configured_size=configured.size,source_size=other.size,children=children,
                           source_offset=other.value,next_symbol_address=next_address,
                           source_end=absolute+other.size,config_line=configured.line,
                           source_bytes_match_dol=byte_match,byte_check=byte_reason))
    return issues


def uniform_shifts(target_refs, source_refs):
    left={(r.function,r.site):r for r in target_refs if r.immediate is not None}
    right={(r.function,r.site):r for r in source_refs if r.immediate is not None}
    groups=defaultdict(list)
    for key,a in left.items():
        b=right.get(key)
        if b is None or a.section!=b.section or (a.word&0xffff0000)!=(b.word&0xffff0000): continue
        if a.immediate==b.immediate: continue
        groups[a.section].append((b.immediate-a.immediate,a,b))
    results=[]
    for section,rows in groups.items():
        counts=Counter(d for d,_,_ in rows)
        for delta,count in counts.most_common():
            if count<2 or count/len(rows)<0.6: continue
            selected=[(a,b) for d,a,b in rows if d==delta]
            results.append(dict(section=section,delta=delta,sites=len(selected),different_sites=len(rows),
                                functions=sorted({a.function for a,_ in selected}),
                                samples=[dict(function=a.function,site=a.site,target_immediate=a.immediate,
                                              source_immediate=b.immediate,target_address=a.address,source_offset=b.address,
                                              target_symbol=a.symbol,source_symbol=b.symbol) for a,b in selected[:12]]))
    return results


def scan_unit(unit,target_path,source_path,catalog,report):
    result=UnitScan(unit)
    try:
        target=ELF(target_path,True);result.target_hash=target.sha256
        target_funcs=target.functions()
        for f in target_funcs:
            score=report.get(f.name,{}).get('fuzzy_match_percent')
            absolute=f.absolute if f.absolute is not None else target.origins.get(f.section,0)+f.value
            result.functions[f.name]=dict(size=f.size,fuzzy=score,address=absolute,offset=f.value,section=target.names[f.section])
            refs,unresolved=references(target,f,catalog,True);result.refs.extend(refs);result.unresolved+=unresolved
        if source_path is None or not source_path.is_file(): result.source_missing=True;return result
        source=ELF(source_path);result.source_hash=source.sha256
        result.sizes=size_issues(target,source,catalog)
        source_funcs={f.name:f for f in source.functions()}
        aligned_target,aligned_source=[],[]
        for f in target_funcs:
            other=source_funcs.get(f.name)
            if other is None or other.size!=f.size: continue
            refs,_=references(source,other,catalog,False)
            # Sites are relative to the selected function, not the translation unit.
            aligned_source.extend(replace(r,site=r.site-other.value) for r in refs)
            aligned_target.extend(replace(r,site=r.site-f.value) for r in result.refs if r.function==f.name)
        result.shifts=uniform_shifts(aligned_target,aligned_source)
    except (ValueError,OSError,struct.error,PoolUnproved) as error: result.error=str(error)
    return result


def function_bytes(scan, names):
    funcs={n:scan.functions[n] for n in set(names) if n in scan.functions}
    return dict(affected_functions=len(funcs),affected_unmatched_bytes=sum(f['size'] for f in funcs.values() if f['fuzzy'] is not None and f['fuzzy']<100),
                already_matched_bytes=sum(f['size'] for f in funcs.values() if f['fuzzy']==100),
                total_affected_bytes=sum(f['size'] for f in funcs.values()),
                functions=[dict(name=n,**f) for n,f in sorted(funcs.items())])


def findings(scans,splits,catalog,near):
    users=defaultdict(set)
    for scan in scans:
        for r in scan.refs:
            users[r.section,r.anchor].add(scan.unit)
            users[r.section,r.address].add(scan.unit)
    rows=[];far=0
    for scan in scans:
        if scan.error or scan.unit not in splits.units: continue
        grouped=defaultdict(list)
        for r in scan.refs:
            own=splits.units[scan.unit].get(r.section)
            inside=own and own[0]<=r.address<own[1]
            if inside and (not r.width or r.address+r.width<=own[1]): continue
            expected=splits.expected(scan.unit,r.section)
            a,b=expected
            distance=max((a-r.address if a is not None and r.address<a else 0),
                         (r.address-b if b is not None and r.address>=b else 0))
            if distance>near: far+=1;continue
            owner=splits.owners(r.section,r.address)
            kind='range_overrun' if inside else ('outside_neighbour' if owner else 'outside_gap')
            if own is None: kind='missing_section_'+('neighbour' if owner else 'gap')
            grouped[kind,r.section,r.symbol,r.anchor,tuple(owner)].append((r,distance))
        for (kind,section,symbol,anchor,owners),items in grouped.items():
            refs=[r for r,_ in items]; shared=users[section,anchor]
            sym=catalog.named(symbol,anchor)
            anonymous=bool(re.match(r'(?:lbl_|@|\.\.\.)',symbol)) or bool(sym and sym.scope=='local')
            confidence='high' if len(shared)==1 and anonymous else 'low'
            if confidence=='low' and len(shared)==1 and not owners: confidence='medium'
            if confidence=='high' and all(r.indirect for r in refs):confidence='medium'
            if all(not r.width for r in refs) and min(d for _,d in items)==0: confidence='low'
            window=splits.expected(scan.unit,section)
            anchor_distance=max(window[0]-anchor if window[0] is not None and anchor<window[0] else 0,
                                anchor-window[1] if window[1] is not None and anchor>=window[1] else 0)
            row=dict(kind=kind,confidence=confidence,unit=scan.unit,section=section,symbol=symbol,address=hx(anchor),
                     configured_start=hx(splits.units[scan.unit].get(section,(None,None))[0]),
                     configured_end=hx(splits.units[scan.unit].get(section,(None,None))[1]),
                     owner=','.join(owners) or 'UNOWNED',distance=anchor_distance,shared_units=len(shared),
                     min_reference_distance=min(d for _,d in items),max_reference_distance=max(d for _,d in items),
                     delta='',sites=len(refs),evidence=f'{len(refs)} reaches ({sum(r.indirect for r in refs)} initializer-derived); target {hx(min(r.address for r in refs))}..{hx(max(r.address+max(r.width,1) for r in refs))}; expected {hx(splits.expected(scan.unit,section)[0])}..{hx(splits.expected(scan.unit,section)[1])}',
                     samples=[dict(function=r.function,site=hx(r.site-scan.functions[r.function]['offset']+scan.functions[r.function]['address']),address=hx(r.address),width=r.width,mode=r.mode) for r in refs[:12]],
                     referencing_units=sorted(shared),**function_bytes(scan,[r.function for r in refs]))
            rows.append(row)
        for issue in scan.sizes:
            a=issue['address'];b=a+max(issue['configured_size'],issue['source_size'])
            refs=[r for r in scan.refs if r.section==issue['section'] and (a<=r.address<b or r.anchor==a)]
            children=issue['children'];merged=len({offset for _,offset,_ in children})>1 and issue['source_size']<issue['configured_size']
            confidence='high' if (merged and issue['section'] in ('.bss','.sbss','.sbss2')) or issue['source_end']==issue['next_symbol_address'] or issue['target_size']!=issue['configured_size'] else 'medium'
            if issue['source_bytes_match_dol'] is False:confidence='medium'
            stale=issue['source_size']==issue['configured_size'] and issue['target_size']!=issue['configured_size']
            if stale:confidence='low'
            prefix='Configured/source sizes agree; target snapshot needs refresh, not another config edit. ' if stale else ''
            rows.append(dict(kind='stale_target_symbol_size' if stale else 'merged_target_symbol' if merged else 'symbol_size_disagreement',confidence=confidence,
                             unit=scan.unit,section=issue['section'],symbol=issue['symbol'],address=hx(a),
                             configured_start=hx(a),configured_end=hx(a+issue['configured_size']),owner=scan.unit,distance=0,
                             shared_units=len(users[issue['section'],a]),delta='',sites=len(refs),
                             evidence=prefix+f"configured=0x{issue['configured_size']:X}, target ELF=0x{issue['target_size']:X}, source ELF=0x{issue['source_size']:X}; source objects inside configured extent: "+', '.join(f'{n}+0x{o:X}(0x{s:X})' for n,o,s in children[:8])+f"; next object={hx(issue['next_symbol_address'])}; source/DOL bytes={issue['source_bytes_match_dol']}; symbols.txt:{issue['config_line']}",
                             size_evidence=issue,**function_bytes(scan,[r.function for r in refs])))
        # A derived access beyond its anchor is evidence about a pool reach, not
        # automatically an instruction to enlarge that named object.
        extents=defaultdict(list)
        for r in scan.refs:
            if not r.width or r.address<r.anchor: continue
            sym=catalog.named(r.symbol,r.anchor)
            if sym and sym.size and r.address+r.width>sym.address+sym.size:
                extents[sym.name,sym.address,sym.section,sym.size].append(r)
        for (name,address,section,size),refs in extents.items():
            end=max(r.address+r.width for r in refs)
            rows.append(dict(kind='referenced_extent_exceeds_symbol',confidence='low',unit=scan.unit,section=section,
                             symbol=name,address=hx(address),configured_start=hx(address),configured_end=hx(address+size),
                             owner=','.join(splits.owners(section,address)) or 'UNOWNED',distance=0,
                             shared_units=len(users[section,address]),delta='',sites=len(refs),
                             evidence=f'declared 0x{size:X}; observed base-relative access end +0x{end-address:X}; may be a legitimate pool base, not an object-size proof',
                             samples=[dict(function=r.function,site=hx(r.site-scan.functions[r.function]['offset']+scan.functions[r.function]['address']),address=hx(r.address),width=r.width) for r in refs[:12]],
                             **function_bytes(scan,[r.function for r in refs])))
        for shift in scan.shifts:
            rows.append(dict(kind='uniform_offset_delta',confidence='medium',unit=scan.unit,section=shift['section'],symbol='',address='',
                             configured_start=hx(splits.units[scan.unit].get(shift['section'],(None,None))[0]),
                             configured_end=hx(splits.units[scan.unit].get(shift['section'],(None,None))[1]),owner='',distance='',shared_units='',
                             delta=f"{shift['delta']:+#x}",sites=shift['sites'],
                             evidence=f"source minus target immediate {shift['delta']:+#x} at {shift['sites']}/{shift['different_sites']} differing data reaches; equal-sized functions, identical opcode/register fields; data layout signal, not a proven split fix",
                             samples=shift['samples'],**function_bytes(scan,shift['functions'])))
    rank={'high':0,'medium':1,'low':2}
    rows.sort(key=lambda r:(r['affected_unmatched_bytes']==0,rank[r['confidence']],-r['affected_unmatched_bytes'],-r['total_affected_bytes'],r['unit'],r['kind'],r['symbol']))
    for i,row in enumerate(rows,1): row['rank']=i
    return rows,far


def unit_candidates(rows):
    grouped=defaultdict(list)
    for row in rows:
        if row['confidence']!='low' and row['affected_unmatched_bytes']:grouped[row['unit']].append(row)
    out=[]
    for unit,items in grouped.items():
        functions={f['name']:f for row in items for f in row['functions'] if f['fuzzy'] is not None and f['fuzzy']<100}
        signals=[]
        for row in items:
            issue=row.get('size_evidence',{})
            strong=(row['kind']=='uniform_offset_delta' or (row['kind']=='merged_target_symbol' and row['section'] in ('.bss','.sbss','.sbss2'))
                    or (issue.get('source_size',0)>issue.get('configured_size',0) and issue.get('source_end')==issue.get('next_symbol_address')))
            if strong:signals.append(row)
        signal_functions={f['name']:f for row in signals for f in row['functions'] if f['fuzzy'] is not None and f['fuzzy']<100}
        out.append(dict(unit=unit,layout_or_size_bytes=sum(f['size'] for f in signal_functions.values()),
                        affected_unmatched_bytes=sum(f['size'] for f in functions.values()),affected_functions=len(functions),
                        findings=len(items),kinds=','.join(sorted({r['kind'] for r in items})),
                        sections=','.join(sorted({r['section'] for r in items})),
                        delta_signals='; '.join(r['section']+':'+r['delta'] for r in signals if r['kind']=='uniform_offset_delta'),
                        example_ranks=','.join(str(r['rank']) for r in (signals or items)[:5])))
    out.sort(key=lambda r:(-r['layout_or_size_bytes'],-r['affected_unmatched_bytes'],r['unit']))
    return out


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=Path.cwd(),help='configured tool/target worktree')
    p.add_argument('--config-root',type=Path,help='read splits/symbols from this checkout (default root)')
    p.add_argument('--base-root',type=Path,help='read compiled source objects from this checkout (default root)')
    p.add_argument('--report',type=Path,default=SCRATCH.parent/'report-base.json')
    p.add_argument('--out',type=Path,default=SCRATCH)
    p.add_argument('--near',type=lambda s:int(s,0),default=0x400,help='maximum distance outside expected section window')
    p.add_argument('--workers',type=int,default=2)
    p.add_argument('--prefix',default='')
    args=p.parse_args()
    if not 1<=args.workers<=4 or args.near<0:p.error('workers 1..4 and nonnegative --near required')
    started=time.monotonic();root=args.root.resolve();config=(args.config_root or root).resolve();base=(args.base_root or root).resolve()
    paths=[config/'config/SB4E01/splits.txt',config/'config/SB4E01/symbols.txt',args.report.resolve(),root/'objdiff.json',Path(__file__).resolve(),Path(__file__).with_name('rdpool.py').resolve()]
    dol_path=root/'orig/SB4E01/sys/main.dol'
    if dol_path.is_file():paths.append(dol_path)
    content=[path.read_bytes() for path in paths];pins={str(path):digest(data) for path,data in zip(paths,content)}
    splits=Splits(content[0].decode());catalog=Catalog(content[1].decode());report=json.loads(content[2])
    if len(content)>6:catalog.dol=content[6]
    for sec,ranges in splits.ranges.items():
        if sec in DATA and ranges:
            lo=min(r[0] for r in ranges);hi=max(r[1] for r in ranges)
            old=catalog.bounds.get(sec,(lo,hi));catalog.bounds[sec]=(min(lo,old[0]),max(hi,old[1]))
    reports={norm(u['name']):{f['name']:f for f in u.get('functions',[])} for u in report['units']}
    configured=json.loads(content[3])['units']
    objects={norm(u['name']):u for u in configured}
    tasks=[];missing=[]
    inventory=list(splits.units)+[unit for unit in objects if unit not in splits.units]
    for unit in inventory:
        if not unit.startswith(norm(args.prefix)):continue
        u=objects.get(unit,{})
        target=root/u.get('target_path',f'build/SB4E01/obj/{unit}.o')
        source=base/u['base_path'] if u.get('base_path') else None
        if not target.is_file():missing.append(unit);continue
        tasks.append((unit,target,source,catalog,reports.get(unit,{})))
    with ThreadPoolExecutor(args.workers) as executor:scans=list(executor.map(lambda a:scan_unit(*a),tasks))
    rows,far=findings(scans,splits,catalog,args.near)
    candidates=unit_candidates(rows)
    changed=[path for path,value in pins.items() if digest(Path(path).read_bytes())!=value]
    if changed:raise ValueError('configuration/report changed during scan: '+', '.join(changed))
    out=args.out.resolve();out.mkdir(parents=True,exist_ok=True)
    columns=['rank','confidence','kind','unit','section','symbol','address','configured_start','configured_end','owner','distance','shared_units','affected_functions','affected_unmatched_bytes','already_matched_bytes','total_affected_bytes','delta','sites','evidence']
    with (out/'splitscan.tsv').open('w') as f:
        w=csv.DictWriter(f,fieldnames=columns,extrasaction='ignore',delimiter='\t',lineterminator='\n');w.writeheader();w.writerows(rows)
    with (out/'splitscan-details.jsonl').open('w') as f:
        for row in rows:f.write(json.dumps(row)+'\n')
    counts=Counter((r['confidence'],r['kind']) for r in rows)
    with (out/'splitscan-summary.tsv').open('w') as f:
        w=csv.writer(f,delimiter='\t',lineterminator='\n');w.writerow(['confidence','kind','findings'])
        w.writerows((c,k,n) for (c,k),n in sorted(counts.items()))
    with (out/'splitscan-units.tsv').open('w') as f:
        columns=['unit','layout_or_size_bytes','affected_unmatched_bytes','affected_functions','findings','kinds','sections','delta_signals','example_ranks']
        w=csv.DictWriter(f,fieldnames=columns,delimiter='\t',lineterminator='\n');w.writeheader();w.writerows(candidates)
    def revision(path):
        r=subprocess.run(['git','rev-parse','HEAD'],cwd=path,text=True,capture_output=True)
        return r.stdout.strip() if r.returncode==0 else None
    meta=dict(root=str(root),config_root=str(config),base_root=str(base),seconds=time.monotonic()-started,
              tool_revision=revision(root),tool_sha256=digest(Path(__file__).read_bytes()),config_revision=revision(config),base_revision=revision(base),input_sha256=pins,
              near=args.near,units_scanned=len(scans),missing_target_units=missing,missing_source_units=[s.unit for s in scans if s.source_missing],
              explicitly_split_units=sum(s.unit in splits.units for s in scans),automatic_reference_units=[s.unit for s in scans if s.unit not in splits.units and s.functions],
              errors=[dict(unit=s.unit,error=s.error) for s in scans if s.error],
              references=sum(len(s.refs) for s in scans),nondata_or_unresolved_nonbranch_relocations=sum(s.unresolved for s in scans),
              distant_external_reaches_omitted=far,findings=len(rows),
              object_inventory_sha256=digest(json.dumps([(s.unit,s.target_hash,s.source_hash) for s in scans]).encode()),
              ranking='Known unmatched-function findings first, then confidence, unique affected unmatched-function bytes and all affected-function bytes; already_matched_bytes counts only 100% functions. Automatic units contribute reference-sharing evidence.',
              limitations=['No ownership or source-equivalence proof; no automatic edits.',
                           'CFG tracks constant addresses and direct data relocations. Pointer initializer edges are hypothetical: mutable memory may differ at runtime. Other loaded pointers and unknown indexed reaches are not followed.',
                           'Uniform deltas require equal function sizes and identical opcode/register fields at the same instruction offset.',
                           'Source object sizes and pooled-base access extents are evidence, not ABI or symbol-size proofs.'])
    (out/'splitscan-meta.json').write_text(json.dumps(meta,indent=2)+'\n')
    lines=['# SB4E01 split scan', '',f"{len(scans)} target units ({meta['explicitly_split_units']} explicitly configured), {meta['references']} code/data reaches, {len(rows)} observations, {len(meta['errors'])} parse errors; {meta['seconds']:.2f}s.", '',
           'Read-only: existing objects only. No config, game source or build output is changed. `splitscan.tsv` contains ranked observations; `splitscan-details.jsonl` contains sites, affected functions and source-object extent witnesses. `splitscan-units.tsv` deduplicates functions within each unit.', '',
           'The first byte column below counts unmatched functions with a uniform data-immediate shift, merged BSS objects, or a larger source object ending exactly at the next configured object. The second includes ownership-only observations. These are affected code sizes, not promised matched-code gains. Direct literal/SDA relocation identities can be ignored by the report, so ownership alone need not explain a code mismatch. Summing observation rows double-counts functions.', '',
           '| Unit | Layout/size evidence bytes | All affected unmatched bytes | Offset deltas (source minus target) |', '|---|---:|---:|---|']
    for row in candidates[:15]:lines.append(f"| `{row['unit']}` | {row['layout_or_size_bytes']} | {row['affected_unmatched_bytes']} | {row['delta_signals']} |")
    lines+=['', 'Shared/externally named references, pooled-base extent overruns, pointer initializer edges and possible allocation padding require inspection. Mutable pointers can differ from their ELF initializers. Stale target-symbol sizes are labelled separately when configured/source sizes already agree. Automatic units contribute to sharing counts; they have no explicit ownership range to correct.', '',
            f"Config checkout: `{config}` @ `{meta['config_revision']}`; exact input and object-inventory hashes are in `splitscan-meta.json`. Existing source objects come from `{base}`. Unknown indexed loads, general loaded pointers and structurally different function pairs are not covered by the delta detector."]
    (out/'splitscan-summary.md').write_text('\n'.join(lines)+'\n')
    print(f"{len(scans)} units; {meta['references']} data reaches; {len(rows)} findings; {len(meta['errors'])} errors; {meta['seconds']:.2f}s")
    print('rank\tconfidence\tunmatched_bytes\tunit\tkind\tevidence')
    for row in rows[:20]:print('\t'.join(str(row[k]) for k in ('rank','confidence','affected_unmatched_bytes','unit','kind','evidence')))
    return 0


if __name__=='__main__':
    try:raise SystemExit(main())
    except (ValueError,OSError,PoolUnproved) as error:raise SystemExit('splitscan: '+str(error))
