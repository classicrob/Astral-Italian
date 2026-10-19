#!/usr/bin/env python3
"""Fail-closed data obligations for semclass. Read-only; no compiler/build calls.

All functions, including code-100 functions, are visited. datacontent's scoped
resolver/tracer is reused as discovery, never as an equivalence certificate.
Mutable identities and pointer referents remain obligations even for equal bytes.
"""
import argparse
from collections import Counter
import hashlib
import importlib.util
import json
import re
from pathlib import Path
import struct
import sys

VERSION = 'SB4E01'
SDA = {2: 0x807DECA0, 13: 0x807D7320}  # SB4E01 original linker anchors
_CACHE = {}


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def finding(reason, target_offset=None, source_offset=None, **facts):
    return dict(subclass='data', target_offset=target_offset,
                source_offset=source_offset, reason=reason, **facts)


class Session:
    def __init__(self, root):
        self.root = Path(root).resolve()
        path = self.root/'tools/team/datacontent.py'
        key = '_semdata_dc_' + hashlib.sha256(str(path).encode()).hexdigest()[:12]
        spec = importlib.util.spec_from_file_location(key, path)
        dc = importlib.util.module_from_spec(spec)
        sys.modules[key] = dc
        spec.loader.exec_module(dc)
        self.dc = dc
        self.world = dc.World(self.root)
        original = self.root/f'orig/{VERSION}/sys/main.dol'
        # Never certify using the rebuilt image as the original oracle.
        self.world.image = original.read_bytes()
        self.world.dol_sections = [(dc.word(self.world.image, i*4),
                                    dc.word(self.world.image, 0x48+i*4),
                                    dc.word(self.world.image, 0x90+i*4)) for i in range(18)]
        self.config = {v['name'].removeprefix('main/'): v for v in
                       json.loads((self.root/'objdiff.json').read_text())['units']}
        self.pins = {str(p): digest(p) for p in [original, path,
                    self.root/f'config/{VERSION}/symbols.txt',
                    self.root/f'config/{VERSION}/splits.txt', self.root/'objdiff.json']}
        self.providers = {}
        self._owner_cache = {}
        self._identity_cache = {}
        self._data_at = {}
        for row in self.world.data_symbols:
            self._data_at.setdefault((row['section'],row['address']),[]).append(row)
        self._role_pairs = set()
        self._padding_ok = set()
        self._resolved_address = self.world.resolve_address
        self.world.resolve_address = self.resolve_address


    def dol(self, address, size):
        for offset, begin, extent in self.world.dol_sections:
            if begin <= address and address+size <= begin+extent:
                at = offset+address-begin
                if at+size <= len(self.world.image):
                    return self.world.image[at:at+size]
        return None

    def load_provider(self, unit, side):
        key = (unit, side)
        if key not in self.providers:
            config = self.config.get(unit, {})
            name = config.get('target_path' if side == 'target' else 'base_path')
            if not name:
                raise ValueError('provider_object_unconfigured')
            path = self.root/name
            self.providers[key] = self.dc.Elf(path)
            self.pins[str(path)] = hashlib.sha256(self.providers[key].data).hexdigest()
        return self.providers[key]

    def owner(self, record):
        key = record['section'],record['address'],record['size']
        if key not in self._owner_cache:
            sec,start,size = key
            stop = start+max(1,size)
            spans = [(u,lo,hi) for (u,s),(lo,hi) in self.world.ranges.items()
                     if s==sec and lo<stop and start<hi]
            owners = {u for u,lo,hi in spans}
            complete = [u for u,lo,hi in spans if lo<=start and stop<=hi]
            self._owner_cache[key] = (complete[0] if len(owners)==1 and len(complete)==1 else None)
        return self._owner_cache[key]

    @staticmethod
    def aligned_extent(target_size, source_size):
        return target_size==source_size or (0<source_size<target_size and any(
            (source_size+a-1)//a*a==target_size for a in (4,8)))

    @staticmethod
    def qualified_data_name(name):
        # Bounded CodeWarrior qualified DATA names. No bare/local/@ names;
        # conservatively reject encodings this small parser cannot validate.
        match = re.fullmatch(r'([A-Za-z_]\w*)__(.+)',name)
        if not match:return False
        tail=match[2]
        if tail.startswith('Q'):
            if len(tail)<2 or not tail[1].isdigit() or tail[1]=='0':return False
            count,tail=int(tail[1]),tail[2:]
        else:count=1
        for _ in range(count):
            m=re.match(r'([1-9][0-9]*)([A-Za-z_].*)',tail)
            if not m:return False
            size=int(m[1]);tail=m[2]
            if len(tail)<size or not re.fullmatch(r'[A-Za-z_]\w*',tail[:size]):return False
            tail=tail[size:]
        return not tail

    def orphan_BSS(self, symbol, section=None):
        if symbol.binding==0 or not self.qualified_data_name(symbol.name):return None
        rows=self.world.symbols.get(symbol.name,[])
        if len(rows)!=1:return None
        row=rows[0]
        if row['local'] or row['function'] or row['section'] not in ('.bss','.sbss') or not row['size']:
            return None
        if section is not None and row['section']!=section:return None
        start,stop=row['address'],row['address']+row['size']
        bss,extent=struct.unpack_from('>II',self.world.image,0xd8)
        if not bss<=start<stop<=bss+extent:return None
        # An adjacent/overlapping TU owns its bytes even if this TU has no split.
        if any(lo<stop and start<hi for lo,hi in self.world.ranges.values()):return None
        return row

    def resolve_address(self, elf, unit, pointer):
        address,error=self._resolved_address(elf,unit,pointer)
        if not error:return address,None
        symbol=pointer.relocation.symbol
        section=elf.names[symbol.section] if elf.local(symbol) else None
        row=self.orphan_BSS(symbol,section)
        if row is None:return address,error
        delta=pointer.relocation.addend+pointer.delta
        if not 0<=delta<row['size']:return None,'orphan_BSS_effective_address_outside_object'
        return row['address']+delta,None

    def records(self, symbol, unit, section, size=None):
        records = self.world.symbols.get(symbol.name, [])
        if symbol.binding == 0:
            records = [v for v in records if v['section'] == section
                       and self.world.owns(unit, section, v['address'])
                       and self.world.owns(unit, section, v['address']+max(1,v['size'])-1)]
        else:
            records = [v for v in records if not v['local'] and v['section'] == section]
        if size is not None:
            records = [v for v in records if v['size'] == size]
        ids = {(v['section'], v['address'], v['size'], v['function']) for v in records}
        return records if len(ids) == 1 else []

    def source_definition(self, elf, unit, pointer):
        dc = self.dc
        symbol = pointer.relocation.symbol
        if elf.local(symbol):
            return elf, unit, pointer, None
        if symbol.binding == 0:
            return None, None, None, 'undefined_source_local'
        records = [v for v in self.world.symbols.get(symbol.name, []) if not v['local']]
        ids = {(v['section'],v['address'],v['size'],v['function']) for v in records}
        if len(ids) != 1:
            return None, None, None, 'source_external_identity_unknown_or_ambiguous'
        owner = self.owner(records[0])
        if owner is None:
            return None, None, None, 'source_external_owner_unknown'
        try:
            provider = self.load_provider(owner, 'source')
        except (OSError, ValueError) as error:
            return None, None, None, 'source_external_provider_unavailable:'+str(error)
        definitions = [v for v in provider.symbols if v.name == symbol.name
                       and provider.local(v) and v.binding != 0]
        unique = {(v.address,v.section,v.size,v.kind) for v in definitions}
        if len(unique) != 1:
            return None, None, None, 'source_external_definition_unknown_or_ambiguous'
        resolved = dc.Pointer(dc.Relocation(pointer.relocation.kind, definitions[0],
                          pointer.relocation.addend), pointer.delta, pointer.high, pointer.dynamic)
        return provider, owner, resolved, None

    def source_object(self, elf, unit, pointer, limit):
        elf, unit, pointer, error = self.source_definition(elf, unit, pointer)
        if error:
            return None, None, None, None, error
        obj, error = elf.bound(pointer, limit)
        if error == 'runtime_NOBITS_data':
            sym = pointer.relocation.symbol
            at = sym.address+pointer.relocation.addend+pointer.delta
            choices = [v for v in elf.symbols if v.section == sym.section and v.kind == 1
                       and v.size and v.address <= at < v.address+v.size]
            ids = {(v.name,v.address,v.size) for v in choices}
            if len(ids) != 1:
                return elf, unit, pointer, None, 'NOBITS_source_extent_ambiguous'
            selected = choices[0]
            if selected.size > limit or selected.address+selected.size > elf.sections[sym.section][5]:
                return elf, unit, pointer, None, 'NOBITS_source_extent_outside_section_or_limit'
            obj = dict(symbol=selected.name, section=elf.names[sym.section],
                       section_index=sym.section, start=selected.address, size=selected.size,
                       reference_offset=at-selected.address, extent_proof='ELF_NOBITS_symbol_size',
                       nobits=True)
            error = None
        return elf, unit, pointer, obj, error

    def target_object(self, elf, unit, pointer, limit):
        obj, error = self.world.target_bound(elf, unit, pointer, limit)
        if error != 'target_object_not_file_backed':
            if obj:
                obj=dict(obj,owning_unit=self.owner(dict(section=obj['section'],address=obj['start'],size=obj['size'])))
            return obj,error
        address, failure = self.world.resolve_address(elf, unit, pointer)
        if failure:
            return None, failure
        choices = [v for v in self.world.data_symbols if v['size']
                   and v['address'] <= address < v['address']+v['size']
                   and (not v['local'] or self.world.owns(unit,v['section'],v['address']))]
        if not choices:
            return None, error
        obj = min(choices,key=lambda v:(v['size'],v['address'],v['name']))
        bss, extent = struct.unpack_from('>II', self.world.image, 0xd8)
        if obj['section'] not in ('.bss','.sbss') or not (bss <= obj['address'] and
                       obj['address']+obj['size'] <= bss+extent):
            return None, error
        owner = self.owner(obj)
        orphan=self.orphan_BSS(pointer.relocation.symbol,obj['section'])
        if (owner is None and orphan is None) or obj['size'] > limit:
            return None, 'NOBITS_target_owner_or_extent_unknown'
        return dict(symbol=obj['name'],section=obj['section'],start=obj['address'],
                    size=obj['size'],reference_offset=address-obj['address'],
                    extent_proof='original_DOL_BSS_extent_and_scoped_metadata',
                    owning_unit=owner,nobits=True,orphan_BSS=orphan is not None), None

    def source_identity(self, elf, unit, obj):
        if not hasattr(elf,'_semdata_identity_sha'):
            elf._semdata_identity_sha=hashlib.sha256(elf.data).hexdigest()
        key=(str(elf.path),elf._semdata_identity_sha,unit,obj['section_index'],obj['start'],obj['size'],obj['symbol'],obj.get('nobits',False))
        if key not in self._identity_cache:
            self._identity_cache[key]=self._source_identity(elf,unit,obj)
        return self._identity_cache[key]

    def _source_identity(self, elf, unit, obj):
        symbols=[v for v in elf.symbols if v.name==obj['symbol'] and
                 v.section==obj['section_index'] and v.address==obj['start'] and
                 v.size==obj['size'] and v.kind==1]
        if not symbols:return None,'source_object_identity_missing'
        logical=[]
        for symbol in symbols:
            # C3 is strictly a unique fully-qualified global linker binding.
            orphan=self.orphan_BSS(symbol,obj['section']) if obj.get('nobits') else None
            if orphan is not None:
                if not self.aligned_extent(orphan['size'],obj['size']):
                    return None,'orphan_BSS_extent_incompatible'
                logical.append((orphan['section'],orphan['address'],obj['size']))
                continue
            base=self.world.bases.get((unit,obj['section']))
            if base is None:return None,'source_owning_section_unknown'
            address=base+obj['start']
            # A canonical name known elsewhere must never be silently relabelled
            # as the object that happens to occupy its new physical offset.
            known=self.world.symbols.get(symbol.name,[])
            anonymous=(symbol.name.startswith('@') or symbol.name.startswith('...') or
                       re.fullmatch(r'lbl_[0-9A-Fa-f]{8}',symbol.name))
            if known and not anonymous:
                records=self.records(symbol,unit,obj['section'])
                records=[v for v in records if not v['function'] and v['address']==address and
                         self.aligned_extent(v['size'],obj['size']) and self.owner(v)==unit]
            else:
                # C1: unique owning TU + section + exact offset + bounded extent,
                # independently of @ / lbl_ spelling. Never byte/name guessing.
                if not anonymous:
                    return None,'canonical_named_object_absent_from_catalog'
                records=[v for v in self._data_at.get((obj['section'],address),[]) if
                         self.aligned_extent(v['size'],obj['size']) and self.owner(v)==unit]
            ids={(v['section'],v['address'],v['size']) for v in records}
            if len(ids)!=1:return None,'source_TU_section_offset_extent_identity_unknown_or_ambiguous'
            record=records[0]
            # Multiple live objects/aliases spanning a proposed alignment tail
            # are checked separately by padding_pair; identity alone waives none.
            logical.append((record['section'],record['address'],obj['size']))
        if len(set(logical))!=1:return None,'source_object_identity_ambiguous'
        return logical[0],None

    def padding_pair(self, te, se, unit, provider, owner, to, so):
        """C2 only: zero/unallocated alignment tail, no aliased data or pointers.

        Per-function access coverage is a separate prerequisite collected before
        pairing. Every load/store must fit the source payload. Address escapes
        have no consumer extent proof and keep alignment padding UNKNOWN.
        """
        if to['size']==so['size']:return to,None
        if not self.aligned_extent(to['size'],so['size']):return to,'alignment_extent_incompatible'
        if to['reference_offset']!=so['reference_offset']:return to,'alignment_reference_offsets_differ'
        if to['reference_offset']<0 or to['reference_offset']>=so['size']:
            return to,'alignment_reference_outside_source_extent'
        start,tail,stop=to['start'],to['start']+so['size'],to['start']+to['size']
        record=dict(section=to['section'],address=start,size=to['size'])
        target_owner=self.owner(record)
        orphan=to.get('orphan_BSS',False)
        if target_owner is None and not orphan:return to,'alignment_target_whole_extent_owner_unknown'
        aliases=[v for v in self.world.data_symbols if v['section']==to['section'] and
                 v['address']<stop and tail<v['address']+max(v['size'],1) and
                 not(v['address']==start and v['size']==to['size'])]
        if aliases:return to,'alignment_tail_aliases_another_target_object'
        if to.get('nobits') or so.get('nobits'):
            if not(to.get('nobits')and so.get('nobits')):return to,'alignment_NOBITS_contract_differs'
        else:
            if to['extent_proof']!='config_symbol_size_DOL':return to,'alignment_tail_not_original_DOL_bytes'
            if any(bytes.fromhex(to['hex'])[so['size']:]):return to,'alignment_tail_nonzero'
            try:pe=self.load_provider(target_owner,'target')
            except(OSError,ValueError):return to,'alignment_target_provider_unknown'
            sec=[i for i,n in enumerate(pe.names)if n==to['section']]
            if len(sec)!=1:return to,'alignment_target_section_ambiguous'
            base=self.world.bases.get((target_owner,to['section']))
            if base is None:return to,'alignment_target_section_base_unknown'
            local_start,local_tail,local_stop=start-base,tail-base,stop-base
            if local_start<0 or local_stop>pe.sections[sec[0]][5]:return to,'alignment_tail_outside_target_ELF'
            if any(s.section==sec[0] and s.kind==1 and s.size and s.address<local_stop and
                   local_tail<s.address+s.size and not(s.address==local_start and s.size==to['size'])
                   for s in pe.symbols):return to,'alignment_tail_aliases_target_ELF_object'
            if any(section==sec[0] and at<local_stop and local_tail<at+4
                   for section,at in pe.relocations):return to,'alignment_tail_contains_pointer_relocation'
        key=(to['section'],to['start'],to['size'],so['size'])
        if key not in self._padding_ok:return to,'alignment_all_function_accesses_not_proven'
        out=dict(to,original_size=to['size'],size=so['size'],alignment_padding_bytes=to['size']-so['size'])
        if 'hex'in out:out['hex']=out['hex'][:so['size']*2]
        out['pointers']={k:v for k,v in out.get('pointers',{}).items()if k<so['size']}
        return out,None

    def padding_accesses(self, te, se, unit, tr, sr, ti, si, pairs, limit):
        """No partial-use waiver: inspect ALL traced accesses on BOTH sides."""
        candidates={}
        for t,s,_ in pairs:
            to,terr=self.target_object(te,unit,t['pointer'],limit)
            pe,pu,pp,so,serr=self.source_object(se,unit,s['pointer'],limit)
            if terr or serr or not self.aligned_extent(to['size'],so['size']) or to['size']==so['size']:continue
            key=(to['section'],to['start'],to['size'],so['size'])
            candidates[key]=(to,pe,pu,so)
        safe=set()
        for key,(to,pe,pu,so)in candidates.items():
            okay=not(ti or si)
            for side,elf,refs in [('target',te,tr),('source',se,sr)]:
                for ref in refs:
                    if side=='target':obj,error=self.target_object(elf,unit,ref['pointer'],limit)
                    else:
                        ep,eu,ptr,obj,error=self.source_object(elf,unit,ref['pointer'],limit)
                    if error:
                        okay=False;continue
                    related=(obj['section']==to['section'] and obj['start']==to['start']) if side=='target' else (
                        ep.path==pe.path and obj['section']==so['section'] and obj['start']==so['start'])
                    if not related:continue
                    op=ref['opcode'];width=ref['width']or(self.dc.memory_width(op)if 32<=op<=55 else 0)
                    address_only=ref['kind']in('relocation','address')and op in(14,15)
                    # Width0 does not bound an escaped pointer's consumers.
                    # Until an explicit per-callee extent contract exists, an
                    # address materialization keeps this padding proof open.
                    if ref['dynamic_index']or not width:okay=False
                    if address_only:okay=False
                    if obj['reference_offset']<0 or obj['reference_offset']+max(width,1)>so['size']:okay=False
            if okay:safe.add(key)
        return safe

    def pointer_identity(self, elf, unit, rel, side):
        dc = self.dc
        if side == 'target':
            address,error = self.world.resolve_address(elf,unit,dc.Pointer(rel))
            return address,error
        provider,owner,ptr,error = self.source_definition(elf,unit,dc.Pointer(rel))
        if error:
            return None,error
        symbol = ptr.relocation.symbol
        sec = provider.names[symbol.section]
        if provider.sections[symbol.section][2] & 4:
            if symbol.kind != 2 or ptr.relocation.addend:
                return None,'interior_code_pointer_identity_unproven'
            records = self.records(symbol,owner,sec)
            if not records or not records[0]['function']:
                return None,'code_pointer_identity_unproven'
            return records[0]['address'],None
        provider,owner,ptr,obj,error = self.source_object(provider,owner,ptr,65536)
        if error:
            return None,error
        identity,error = self.source_identity(provider,owner,obj)
        return (identity[1]+obj['reference_offset'],None) if identity else (None,error)

    def pointer_maps(self, elf, unit, obj, side):
        pointers = {}
        if obj.get('nobits'):
            return pointers,None
        if side == 'target':
            # Referents may be outside this TU. Use their actual owning target ELF,
            # never omit pointer relocations just because the caller is foreign.
            record = dict(section=obj['section'], address=obj['start'],size=obj['size'])
            owner = self.owner(record)
            if owner is None:
                return None,'target_data_provider_owner_unknown'
            try:
                provider = self.load_provider(owner,'target')
            except (OSError,ValueError) as error:
                return None,'target_data_provider_unavailable:'+str(error)
            base = self.world.bases.get((owner,obj['section']))
            sections = [i for i,n in enumerate(provider.names) if n == obj['section']]
            if base is None or len(sections) != 1:
                return None,'target_data_provider_section_unknown'
            sec, start, owner_elf, owner_unit = sections[0],obj['start']-base,provider,owner
        else:
            sec,start,owner_elf,owner_unit = obj['section_index'],obj['start'],elf,unit
        for (section,at),rel in owner_elf.relocations.items():
            if section != sec or not start <= at < start+obj['size']:
                continue
            offset = at-start
            if rel.kind not in (1,26) or offset+4 > obj['size']:
                return None,'unsupported_or_cross_extent_pointer_relocation'
            identity,error = self.pointer_identity(owner_elf,owner_unit,rel,side)
            if error:
                return None,'pointer_identity_unknown:'+error
            if side == 'target':
                expected = identity if rel.kind == 1 else (identity-(obj['start']+offset))&0xffffffff
                if int.from_bytes(bytes.fromhex(obj['hex'])[offset:offset+4],'big') != expected:
                    return None,'target_pointer_relocation_not_bound_to_DOL'
            pointee_kind = None
            if side == 'source':
                pe,pu,pp,perror = self.source_definition(owner_elf,owner_unit,self.dc.Pointer(rel))
                if not perror:
                    pointee_kind = ('code' if pe.sections[pp.relocation.symbol.section][2]&4 else 'data')
            pointers[offset] = dict(identity=identity,relocation_kind=rel.kind,
                                    symbol=rel.symbol.name,addend=rel.addend,
                                    pointee_kind=pointee_kind)
        return pointers,None

    def bind_function(self, elf, unit, function):
        dc = self.dc
        base = self.world.bases.get((unit,elf.names[function.section]))
        if base is None:
            return 'target_text_section_unknown'
        address = base+function.address
        actual = self.dol(address,function.size)
        if actual is None or function.size%4:
            return 'target_function_not_bounded_in_original_DOL'
        for off in range(0,function.size,4):
            at = function.address+off
            w = dc.word(elf.contents[function.section],at)
            rel = elf.instruction_relocation(function.section,at)
            if rel:
                dest,error = self.world.resolve_address(elf,unit,dc.Pointer(rel))
                if error:
                    return 'target_instruction_relocation_unresolved:'+error
                if rel.kind == 10:
                    w = w&0xfc000003 | ((dest-(address+off))&0x3fffffc)
                elif rel.kind in (4,5,6):
                    part = dest if rel.kind == 4 else dest>>16 if rel.kind == 5 else (dest+0x8000)>>16
                    w = w&0xffff0000 | part&0xffff
                elif rel.kind == 109:
                    if elf.local(rel.symbol):
                        section = elf.names[rel.symbol.section]
                    else:
                        matches = [v for v in self.world.symbols.get(rel.symbol.name,[]) if not v['local']]
                        sections = {v['section'] for v in matches}
                        if len(sections) != 1:
                            return 'target_SDA_section_ambiguous'
                        section = next(iter(sections))
                    reg = 2 if section in ('.sdata2','.sbss2') else 13
                    value = dest-SDA[reg]
                    if not -32768 <= value < 32768:
                        return 'target_SDA_displacement_outside_signed16'
                    w = w&~0x1fffff | reg<<16 | value&0xffff
                elif rel.kind in (1,26):
                    w = dest if rel.kind == 1 else (dest-address-off)&0xffffffff
                else:
                    return 'unsupported_target_instruction_relocation:'+str(rel.kind)
            if w != dc.word(actual,off):
                return 'target_instruction_not_bound_to_original_DOL:'+hex(off)
        return None

    def compare_pair(self, te, se, unit, tr, sr, limit):
        dc = self.dc
        site = dict(target_offset=tr['site'],source_offset=sr['site'])
        role_proven=(tr['site'],sr['site'])in self._role_pairs
        findings = []
        to,terror = self.target_object(te,unit,tr['pointer'],limit)
        provider,owner,sp,so,serror = self.source_object(se,unit,sr['pointer'],limit)
        refs = dict(target_referent=to or tr['pointer'].relocation.symbol.name,
                    source_referent=so or sr['pointer'].relocation.symbol.name)
        if terror or serror:
            return [finding('object_resolution_unknown',**site,target_reason=terror,
                            source_reason=serror,**refs)],None
        to,padding_error=self.padding_pair(te,se,unit,provider,owner,to,so)
        if padding_error:
            findings.append(finding('alignment_padding_unknown',**site,detail=padding_error))
        # Keep outputs compact: blobs are pinned by input objects, not duplicated.
        refs = dict(target_referent={k:v for k,v in to.items() if k not in ('hex','pointers','decode_words')},
                    source_referent={k:v for k,v in so.items() if k not in ('hex','pointers','decode_words')})
        if to['section'] != so['section']:
            findings.append(finding('different_data_sections',**site,certainty='definite' if role_proven else 'unknown',**refs))
        widths = [ref['width'] or (dc.memory_width(ref['opcode']) if
                  ref['opcode'] in (32,33,34,35,36,37,38,39,40,41,42,43,44,45,48,49,50,51,52,53,54,55)
                  else 0) for ref in (tr,sr)]
        if widths[0] != widths[1]:
            findings.append(finding('different_data_access_width',**site,
                                    target_width=widths[0],source_width=widths[1],certainty='definite' if role_proven else 'unknown',**refs))
        for side,obj,width in [('target',to,widths[0]),('source',so,widths[1])]:
            if width and (obj['reference_offset'] < 0 or
                          obj['reference_offset']+width > obj['size']):
                findings.append(finding('memory_access_exceeds_referent_extent',**site,
                         side=side,access_width=width,**refs))
        if tr['dynamic_index'] or sr['dynamic_index']:
            findings.append(finding('dynamic_index_extent_unproven',**site,**refs))
        if to['size'] != so['size'] or to['reference_offset'] != so['reference_offset']:
            findings.append(finding('different_object_extent_or_reference_offset',**site,**refs))
        if so['extent_proof'] not in ('ELF_symbol_size','ELF_NOBITS_symbol_size'):
            findings.append(finding('source_extent_not_symbol_proven',**site,**refs))
        identity,error = self.source_identity(provider,owner,so)
        expected = (to['section'],to['start'],to['size'])
        same = identity == expected
        if to.get('nobits') or so.get('nobits'):
            if not (same and to.get('nobits') and so.get('nobits')):
                findings.append(finding('mutable_NOBITS_identity_unproven_or_different',
                                        **site,identity_reason=error,**refs))
            return findings, ('owned-NOBITS',expected,to['reference_offset']) if not findings else None
        if to['extent_proof'] != 'config_symbol_size_DOL':
            findings.append(finding('target_initializer_not_original_DOL_file_bytes',**site,**refs))
        tp,terr = self.pointer_maps(te,unit,to,'target')
        spp,serr = self.pointer_maps(provider,owner,so,'source')
        if terr or serr:
            findings.append(finding('pointer_or_provider_identity_unknown',**site,
                                    target_reason=terr,source_reason=serr,**refs))
            # Scalar bytes remain useful independent negatives. Do not mask a
            # single unsupported or unresolved pointer as equality evidence.
            tp = to.get('pointers',{})
            spp = so.get('pointers',{})
        result = dc.compare_bytes(bytes.fromhex(to['hex']),bytes.fromhex(so['hex']),
                                  to['reference_offset'],so['reference_offset'],tp,spp)
        for diff in result['scalar_differences']:
            details = dict(diff)
            details['target_data_offset'] = details.pop('target_offset')
            details['source_data_offset'] = details.pop('source_offset')
            findings.append(finding('different_data_bytes',**site,**details,**refs))
        for annotation in result['pointer_annotations']:
            if annotation['status'] != 'equal':
                findings.append(finding('different_or_unresolved_pointer_identity',**site,
                                        pointer=annotation,certainty='definite' if annotation['status']=='different' and (same or role_proven) else 'unknown',**refs))
            elif annotation['source'].get('pointee_kind') != 'code':
                # Same pointerword is NOT a payload proof. Loaded pointer GPRs
                # are intentionally forgotten by the discovery tracer. Until
                # transitive data-content closure is established, keep this open.
                findings.append(finding('data_pointee_contents_unverified',**site,
                                        pointer=annotation,**refs))
        for advisory in result['advisories']:
            findings.append(finding('data_extent_advisory',**site,advisory=advisory,**refs))
        # Byte equality is not identity. Only a whole, nonescaping direct loaded
        # scalar in two truly readonly sections has a by-value literal contract.
        width = tr['width'] or (dc.memory_width(tr['opcode']) if tr['opcode'] in
                                (32,34,40,42,48,50) else 0)
        target_readonly = False
        record = dict(section=to['section'],address=to['start'],size=to['size'])
        target_owner = self.owner(record)
        if target_owner is not None:
            try:
                target_provider = self.load_provider(target_owner,'target')
                target_sections = [i for i,n in enumerate(target_provider.names) if n == to['section']]
                target_readonly = (len(target_sections) == 1 and
                    not target_provider.sections[target_sections[0]][2]&1 and
                    target_provider.sections[target_sections[0]][1] != 8)
            except (OSError,ValueError):
                pass
        readonly = (target_readonly and not provider.sections[so['section_index']][2]&1
                    and to['section'] in ('.rodata','.sdata2'))
        literal = (role_proven and readonly and tr['kind'] in ('relocation','memory') and
                   tr['opcode'] in (32,34,40,42,48,50) and
                   not tp and not spp and to['size'] == so['size'] == width
                   and not tr['dynamic_index'] and not sr['dynamic_index'] and
                   to['section'] == so['section'] and
                   to['reference_offset'] == so['reference_offset'] == 0)
        if not same and not literal:
            findings.append(finding('different_or_unproven_data_object_identity',**site,
                                    source_identity=identity,identity_reason=error,
                                    certainty='definite' if identity is not None and role_proven else 'unknown',**refs))
        for entry in findings:
            if entry['reason'] == 'different_data_bytes':
                entry['certainty'] = 'definite' if same or literal else 'unknown-contract'
                entry['byte_comparison_contract'] = ('scoped-object-identity' if same else
                    'readonly-loaded-value' if literal else 'unproven-referent-correspondence')
        if not same and not literal:
            findings.append(finding('data_consumer_correspondence_unknown',**site,**refs))
        token = ('owned-data',expected,to['reference_offset']) if same else (
                 'readonly-loaded-value',to['section'],width,to['hex'])
        return findings,token if not findings else None

    def audit_unit(self, unit, target_path, source_path, limit=65536):
        dc = self.dc
        te,se = dc.Elf(target_path),dc.Elf(source_path)
        pins = {str(Path(p).resolve()):hashlib.sha256(e.data).hexdigest()
                for p,e in [(target_path,te),(source_path,se)]}
        functions,tokens = {},{'target':{},'source':{}}
        for name,tf in te.functions.items():
            if name.startswith('gap_'):
                continue
            ff = functions[name] = dict(findings=[],complete=False,
                                       target_size=tf.size,source_size=None)
            problems = ff['findings']
            error = self.bind_function(te,unit,tf)
            if error:
                problems.append(finding('original_DOL_binding_failed',detail=error))
            self._role_pairs=set()
            self._padding_ok=set()
            sf = se.functions.get(name)
            if sf is None:
                problems.append(finding('unpaired_target_function'))
                continue
            ff['source_size'] = sf.size
            tr,ti = dc.trace(te,tf,unit,self.world)
            sr,si = dc.trace(se,sf,unit,self.world)
            # Gekko paired-single loads/stores can consume up to8B (W=0) or
            #4B (W=1) regardless of runtime quantization. datacontent does not
            # trace these; account for known static bases and bound the maximum.
            for e,f,refs,issues in [(te,tf,tr,ti),(se,sf,sr,si)]:
                flow = dc.register_flow(e,f)
                for off in range(0,f.size,4):
                    raw = dc.word(e.contents[f.section],f.address+off)
                    opcode,ra = raw>>26,(raw>>16)&31
                    if opcode not in (56,57,60,61):
                        continue
                    state = flow.get(off)
                    base = state.get(ra) if state else None
                    if opcode in (57,61) and refs:
                        issues.append(dict(site=off,reason='paired_single_update_flow_unproven'))
                    if isinstance(base,dc.Pointer) and not base.high:
                        imm = raw&0xfff
                        imm -= 0x1000 if imm&0x800 else 0
                        ptr = dc.Pointer(base.relocation,base.delta+imm,dynamic=base.dynamic)
                        if self.world.is_data(e,ptr):
                            refs.append(dict(site=off,kind='paired_memory',opcode=opcode,
                                shape=raw&0xffff0000,width=4 if raw&0x8000 else 8,
                                dynamic_index=ptr.dynamic,pointer=ptr,
                                advisory='max_quantized_access_extent'))
                        else:
                            issues.append(dict(site=off,reason='static_code_as_data_extent_unproven'))
            for side,issues in [('target',ti),('source',si)]:
                for issue in issues:
                    problems.append(finding('data_trace_unknown',
                        target_offset=issue.get('site') if side=='target'else None,
                        source_offset=issue.get('site') if side=='source'else None,
                        side=side,trace_issue=issue))
            # The advisory tracer can stop at opaque indirect branches. Account
            # for EVERY static data relocation, including code100/unvisited rows.
            for side,e,f,refs in [('target',te,tf,tr),('source',se,sf,sr)]:
                seen = {ref['site'] for ref in refs if ref['kind']=='relocation'}
                indexed_seen = {ref['site'] for ref in refs if ref['kind'] in
                                {v[0] for v in dc.INDEXED.values()}}
                issue_seen = {v['site'] for v in (ti if side=='target'else si)}
                states = dc.register_flow(e,f)
                for off in range(0,f.size,4):
                    raw = dc.word(e.contents[f.section],f.address+off)
                    opcode,xo,ra = raw>>26,(raw>>1)&1023,(raw>>16)&31
                    unknown = None
                    if opcode==31 and xo in dc.INDEXED and off not in indexed_seen|issue_seen:
                        unknown = 'untraced_indexed_access'
                    elif refs and off not in states and (32<=opcode<=55 or opcode in (56,57,60,61)) and ra != 1:
                        unknown = 'untraced_memory_after_opaque_control_flow'
                    elif refs and opcode in (46,47):
                        unknown = 'multi_register_memory_flow_unproven'
                    if unknown:
                        problems.append(finding(unknown,
                            target_offset=off if side=='target'else None,
                            source_offset=off if side=='source'else None,side=side))
                    rel = e.instruction_relocation(f.section,f.address+off)
                    if rel and rel.kind not in (10,26) and self.world.is_data(e,dc.Pointer(rel)) and off not in seen:
                        problems.append(finding('untraced_static_data_relocation',
                          target_offset=off if side=='target'else None,
                          source_offset=off if side=='source'else None,
                          side=side,symbol=rel.symbol.name))
            tw,sw=dc.function_words(te,tf),dc.function_words(se,sf)
            if tw==sw and error is None:
                self._role_pairs={(off,off)for off in range(0,tf.size,4)}
            alignment = dc.instruction_alignment(tw,sw)
            pairs,issues = dc.pair_references(tr,sr,alignment)
            for issue in issues:
                problems.append(finding('data_reference_pairing_unproven',
                   target_offset=issue.get('site') if issue['side']=='target'else None,
                   source_offset=issue.get('site') if issue['side']=='source'else None,
                   pairing_issue=issue))
            self._padding_ok=self.padding_accesses(te,se,unit,tr,sr,ti,si,pairs,limit) if not problems else set()
            ft,fs = {},{}
            for tref,sref,_ in pairs:
                entries,token = self.compare_pair(te,se,unit,tref,sref,limit)
                problems.extend(entries)
                if token is not None:
                    ft[tref['site']] = token
                    fs[sref['site']] = token
            ff['reference_counts'] = dict(target=len(tr),source=len(sr),paired=len(pairs))
            ff['complete'] = not problems
            # No token escapes when ANY obligation in the function is open.
            if ff['complete']:
                tokens['target'][name],tokens['source'][name] = ft,fs
        for name in se.functions.keys()-te.functions.keys():
            functions[name] = dict(complete=False,findings=[finding('unpaired_source_function')],
                                   target_size=None,source_size=se.functions[name].size)
        allpins = dict(self.pins,**pins)
        changed = [p for p,h in allpins.items() if digest(p)!=h]
        if changed:
            for value in functions.values():
                value['complete'] = False
                value['findings'].append(finding('audit_input_changed',paths=changed))
            tokens = {'target':{},'source':{}}
        definite = {n for n,v in functions.items() if any(x.get('certainty')=='definite' for x in v['findings'])}
        unknown = {n for n,v in functions.items() if any(x.get('certainty')!='definite' for x in v['findings'])}
        byte_difference = {n for n,v in functions.items() if any(x['reason']=='different_data_bytes' for x in v['findings'])}
        definite_bytes = {n for n,v in functions.items() if any(x['reason']=='different_data_bytes' and
                          x.get('certainty')=='definite' for x in v['findings'])}
        return dict(schema_version=3,unit=unit,functions=functions,pins=allpins,
                    relocation_tokens=tokens,stats=dict(functions=len(functions),
                    complete=sum(v['complete'] for v in functions.values()),
                    semantic=sum(bool(v['findings']) for v in functions.values()),
                    definite_semantic_functions=len(definite),
                    observed_byte_difference_functions=len(byte_difference),
                    definite_byte_difference_functions=len(definite_bytes),
                    unknown_functions=len(unknown),unknown_only_functions=len(unknown-definite),
                    mixed_definite_unknown_functions=len(unknown&definite)),
                    contract='No unknown/advisory is ABI evidence; all data findings gate semantic. '
                             'Code semantics and dynamic heap/field alias contracts remain semclass obligations.')


def audit_unit(root, unit, target_path, source_path):
    """semclass adapter: no compiler, metadata, source, or proof writes."""
    root = Path(root).resolve()
    # Cheap cache invalidation plus content pins/reread in Session.audit_unit.
    watched = [root/'tools/team/datacontent.py',root/'objdiff.json',
               root/f'config/{VERSION}/symbols.txt',root/f'config/{VERSION}/splits.txt',
               root/f'orig/{VERSION}/sys/main.dol']
    key = (str(root),tuple((str(p),p.stat().st_size,p.stat().st_mtime_ns) for p in watched))
    if key not in _CACHE:
        _CACHE.clear()
        _CACHE[key] = Session(root)
    return _CACHE[key].audit_unit(unit,Path(target_path),Path(source_path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path.cwd())
    parser.add_argument('--unit',required=True)
    parser.add_argument('--target-object',type=Path,required=True)
    parser.add_argument('--source-object',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True)
    args = parser.parse_args()
    result = audit_unit(args.root,args.unit,args.target_object,args.source_object)
    args.out.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result['stats']))


if __name__ == '__main__':
    main()
