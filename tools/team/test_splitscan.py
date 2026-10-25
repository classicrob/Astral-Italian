#!/usr/bin/env python3
"""Boundary, provenance and extent fixtures for the read-only scanner."""
import struct
import tempfile
from pathlib import Path
from types import SimpleNamespace
import unittest

import splitscan as s


def dform(op,rt,ra,imm): return (op<<26)|(rt<<21)|(ra<<16)|(imm&0xffff)
def mr(dst,src): return (31<<26)|(src<<21)|(dst<<16)|(src<<11)|(444<<1)


def fake(words,relocations=None):
    symbol=s.ElfSymbol('subject',0,len(words)*4,1,2,1,0x80001000)
    table=('pool',0,0x10,2)
    elf=SimpleNamespace(data=[None,struct.pack('>'+str(len(words))+'I',*words),bytes(0x100)],
                        names=['','.text','.data'],relocations=relocations or {},origins={1:0x80001000,2:0x80648000},
                        tuples={table:[s.ElfSymbol('pool',0,0x10,2,1,1,0x80648000)]})
    return elf,symbol,table


def catalog():
    return s.Catalog('pool = .data:0x80648000; // type:object size:0x10 scope:local\nnext = .data:0x80648100; // type:object size:0x4\n')


class Provenance(unittest.TestCase):
    def anchored(self,tail):
        words=[dform(15,31,0,0),dform(14,31,31,0)]+tail
        elf,func,table=fake(words)
        elf.relocations={(1,2):(6,table,0),(1,6):(4,table,0)}
        return elf,func

    def test_signed_raw_lis_addi_and_load_width(self):
        elf,func,_=fake([dform(15,31,0,0x8065),dform(14,31,31,-0x8000),dform(50,0,31,8)])
        refs,_=s.references(elf,func,catalog(),True)
        load=next(r for r in refs if r.mode=='derived_memory')
        self.assertEqual((load.address,load.width),(0x80648008,8))

    def test_ha_carry_pair_recovers_full_address(self):
        elf,func=self.anchored([dform(32,3,31,12)])
        refs,_=s.references(elf,func,catalog(),True)
        load=next(r for r in refs if r.mode=='derived_memory')
        self.assertEqual((load.address,load.anchor),(0x8064800c,0x80648000))

    def test_low_half_without_compatible_high_does_not_invent_loaded_extent(self):
        elf,func,table=fake([dform(14,31,31,0),dform(32,3,31,12)])
        elf.relocations={(1,2):(4,table,0)}
        refs,_=s.references(elf,func,catalog(),True)
        self.assertFalse(any(r.mode=='derived_memory' for r in refs))

    def test_cfg_join_discards_disagreeing_pointer_and_earlier_observation(self):
        branch=(16<<26)|(12<<21)|(2<<16)|12
        elf,func=self.anchored([branch,dform(14,31,31,4),0x60000000,dform(32,3,31,0)])
        refs,_=s.references(elf,func,catalog(),True)
        self.assertFalse(any(r.mode=='derived_memory' for r in refs))

    def test_calls_preserve_nonvolatile_and_kill_volatile_pointer(self):
        elf,func=self.anchored([mr(3,31),0x48000001,dform(32,0,3,0),dform(32,0,31,4)])
        refs,_=s.references(elf,func,catalog(),True)
        loads=[r for r in refs if r.mode=='derived_memory']
        self.assertEqual([(r.site,r.address) for r in loads],[(20,0x80648004)])

    def test_stores_keep_base_unknown_writer_kills_it(self):
        elf,func=self.anchored([dform(36,3,31,4),dform(32,3,31,8),(31<<26)|(31<<21)|(339<<1),dform(32,3,31,12)])
        refs,_=s.references(elf,func,catalog(),True)
        self.assertEqual([r.address for r in refs if r.mode=='derived_memory'],[0x80648004,0x80648008])

    def test_sda21_resolves_without_known_sda_register(self):
        elf,func,table=fake([dform(48,0,2,0)])
        elf.relocations={(1,0):(109,table,4)}
        refs,_=s.references(elf,func,catalog(),True)
        self.assertEqual((refs[0].address,refs[0].width),(0x80648004,4))

    def test_dform_ra_zero_is_not_a_read_of_register_zero(self):
        elf,func=self.anchored([mr(0,31),dform(32,3,0,4)])
        refs,_=s.references(elf,func,catalog(),True)
        self.assertFalse(any(r.mode=='derived_memory' for r in refs))

    def test_none_relocation_is_annotation_not_rewritten_immediate(self):
        elf,func=self.anchored([dform(14,3,31,20)])
        table=('pool',0,0x10,2)
        elf.relocations[1,8]=(0,table,0)
        refs,_=s.references(elf,func,catalog(),True)
        self.assertTrue(any(r.mode=='derived_addi' and r.address==0x80648014 for r in refs))

    def test_loaded_initializer_edge_is_marked_hypothetical(self):
        elf,func,table=fake([dform(32,31,0,0),dform(32,3,31,4)])
        leaf=('leaf',0x20,4,2)
        elf.tuples[leaf]=[s.ElfSymbol('leaf',0x20,4,2,1,1,0x80648020)]
        elf.relocations={(1,0):(109,table,0),(2,0):(1,leaf,0)}
        refs,_=s.references(elf,func,catalog(),True)
        reached=next(r for r in refs if r.mode=='initializer_derived_memory')
        self.assertEqual(reached.address,0x80648024)
        self.assertTrue(reached.indirect)

    def test_unrelocated_mutable_data_is_not_assumed_to_be_a_pointer(self):
        elf,func,table=fake([dform(32,31,0,0),dform(32,3,31,4)])
        elf.data[2]=struct.pack('>I',0x80648020)+bytes(0xfc)
        elf.relocations={(1,0):(109,table,0)}
        refs,_=s.references(elf,func,catalog(),True)
        self.assertFalse(any('derived_memory' in r.mode for r in refs))

    def test_low_ori_without_high_does_not_invent_a_pointer(self):
        elf,func,table=fake([dform(24,31,31,0),dform(32,3,31,0)])
        elf.relocations={(1,2):(4,table,0)}
        refs,_=s.references(elf,func,catalog(),True)
        self.assertFalse(any('derived_memory' in r.mode for r in refs))


class RangesAndRanking(unittest.TestCase):
    def test_half_open_ownership_overlaps_and_gap(self):
        splits=s.Splits('Sections:\n .data type:data\nA.cpp:\n .data start:0x100 end:0x120\nB.c:\n .data start:0x110 end:0x130\nC.s:\n .text start:0x200 end:0x220\nD.cpp:\n .data start:0x140 end:0x150\n')
        self.assertEqual(splits.owners('.data',0x118),('A','B'))
        self.assertEqual(splits.owners('.data',0x130),())
        self.assertEqual(splits.expected('C','.data'),(0x130,0x140))

    def test_duplicate_symbol_names_fail_closed(self):
        c=s.Catalog('local = .data:0x100; // type:object size:0x4\nlocal = .data:0x200; // type:object size:0x4\n')
        self.assertIsNone(c.named('local'))
        self.assertEqual(c.named('local',0x200).address,0x200)

    def test_uniform_delta_is_data_only_and_sign_is_source_minus_target(self):
        a=[s.Ref('f',i*4,'.data',0x80000100,'base',0x80000000,4,'derived_memory',40+i,0x80630000|(40+i)) for i in range(3)]
        b=[s.Ref('f',i*4,'.data',0,'base',0,4,'derived_memory',8+i,0x80630000|(8+i)) for i in range(3)]
        hit=s.uniform_shifts(a,b)[0]
        self.assertEqual((hit['delta'],hit['sites']),(-32,3))
        changed=[s.Ref(**{**r.__dict__,'word':r.word^(1<<21)}) for r in b]
        self.assertFalse(s.uniform_shifts(a,changed))

    def test_function_bytes_deduplicates_and_separates_exact_unknown(self):
        scan=s.UnitScan('unit',functions={'a':dict(size=100,fuzzy=99),'b':dict(size=20,fuzzy=100),'c':dict(size=40,fuzzy=None)})
        got=s.function_bytes(scan,['a','a','b','c'])
        self.assertEqual((got['affected_unmatched_bytes'],got['already_matched_bytes'],got['total_affected_bytes']),(100,20,160))

    def test_near_boundary_and_shared_global_priority(self):
        splits=s.Splits('A.cpp:\n .text start:0x80000000 end:0x80000100\n .data start:0x100 end:0x120\nB.cpp:\n .text start:0x80000100 end:0x80000200\n .data start:0x120 end:0x140\n')
        c=s.Catalog('lbl_0000011C = .data:0x11C; // type:object size:0x4\n')
        ref=s.Ref('f',0,'.data',0x11c,'lbl_0000011C',0x11c,4,'relocation')
        a=s.UnitScan('B',refs=[ref],functions={'f':dict(size=32,fuzzy=99,address=0x80000100,offset=0)})
        rows,_=s.findings([a],splits,c,16)
        self.assertEqual((rows[0]['kind'],rows[0]['confidence'],rows[0]['distance']),('outside_neighbour','high',4))
        other=s.UnitScan('A',refs=[ref],functions=a.functions)
        rows,_=s.findings([a,other],splits,c,16)
        row=next(r for r in rows if r['unit']=='B')
        self.assertEqual((row['confidence'],row['shared_units']),('low',2))

    def test_merged_bss_and_allocation_padding_evidence(self):
        c=s.Catalog('first = .bss:0x100; // type:object size:0x18\nnext = .bss:0x118; // type:object size:0xA0\nend = .bss:0x1B8; // type:object size:0x4\n')
        target=SimpleNamespace(symbols=[s.ElfSymbol('first',0,0x18,1,1,1,0x100),s.ElfSymbol('next',0x18,0x84,1,1,1,0x118)],names=['','.bss'],origins={1:0x100})
        source=SimpleNamespace(symbols=[s.ElfSymbol('first',0,12,1,1,1,None),s.ElfSymbol('second',12,12,1,1,1,None),s.ElfSymbol('next',24,0xA0,1,1,1,None)],names=['','.bss'])
        issues=s.size_issues(target,source,c)
        self.assertEqual(len(issues),2)
        self.assertEqual({name for name,_,_ in issues[0]['children']},{'first','second'})
        self.assertEqual((issues[1]['target_size'],issues[1]['configured_size'],issues[1]['source_size']),(0x84,0xA0,0xA0))
        splits=s.Splits('unit.cpp:\n .bss start:0x100 end:0x1C0\n')
        rows,_=s.findings([s.UnitScan('unit',sizes=issues)],splits,c,0x400)
        stale=next(r for r in rows if r['symbol']=='next')
        self.assertEqual((stale['kind'],stale['confidence']),('stale_target_symbol_size','low'))

    def test_larger_source_extent_is_checked_against_dol_without_relocations(self):
        c=s.Catalog('rates = .rodata:0x80648000; // type:object size:0x3\nnext = .rodata:0x80648008; // type:object size:0x8\n')
        blob=struct.pack('>ff',50.0,0.0);dol=bytearray(0x108)
        for off,value in ((0,0x100),(0x48,0x80648000),(0x90,8)):struct.pack_into('>I',dol,off,value)
        dol[0x100:]=blob;c.dol=bytes(dol)
        target=SimpleNamespace(symbols=[s.ElfSymbol('rates',0,3,1,1,1,0x80648000)],names=['','.rodata'],origins={1:0x80648000})
        source=SimpleNamespace(symbols=[s.ElfSymbol('rates',0,8,1,1,1,None)],names=['','.rodata'],data=[None,blob],relocations={})
        self.assertTrue(s.size_issues(target,source,c)[0]['source_bytes_match_dol'])
        dol[-1]=1;c.dol=bytes(dol)
        self.assertFalse(s.size_issues(target,source,c)[0]['source_bytes_match_dol'])
        source.relocations[1,0]=(1,('other',0,4,1),0)
        self.assertIsNone(s.size_issues(target,source,c)[0]['source_bytes_match_dol'])


def elf_fixture(path,virtual=None):
    names=['','.text','.data','.symtab','.strtab','.shstrtab','.note.split']
    shnames=b'\0';offsets=[]
    for name in names:offsets.append(len(shnames));shnames+=name.encode()+b'\0'
    strings=b'\0subject\0pool\0'
    syms=b''.join(struct.pack('>IIIBBH',*row) for row in [(0,0,0,0,0,0),(1,0,4,18,0,1),(9,0,8,17,0,2)])
    desc=struct.pack('>'+str(len(virtual))+'I',*virtual) if virtual is not None else b''
    note=struct.pack('>III',6,len(desc),int.from_bytes(b'VIRT','big'))+b'Split\0\0\0'+desc if virtual is not None else b''
    data=[b'',struct.pack('>I',0x4e800020),bytes(8),syms,strings,shnames,note]
    raw=bytearray(52);sections=[]
    for i,blob in enumerate(data):
        while len(raw)%4:raw+=b'\0'
        off=len(raw);raw+=blob
        sections.append((offsets[i],2 if i==3 else 3 if i in (4,5) else 7 if i==6 else 1 if i else 0,0,0,off,len(blob),4 if i==3 else 0,0,4,16 if i==3 else 0))
    while len(raw)%4:raw+=b'\0'
    start=len(raw)
    for h in sections:raw+=struct.pack('>10I',*h)
    ident=b'\x7fELF\x01\x02\x01'+bytes(9)
    struct.pack_into('>16sHHIIIIIHHHHHH',raw,0,ident,1,20,1,0,0,start,0,52,0,0,40,len(sections),5)
    path.write_bytes(raw)


class OriginalAddresses(unittest.TestCase):
    def test_virt_note_is_per_symbol_not_per_section(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'target.o';elf_fixture(path,[0,0x80001000,0x80648000])
            elf=s.ELF(path,True)
            self.assertEqual(elf.functions()[0].absolute,0x80001000)
            self.assertEqual(elf.symbols[2].absolute,0x80648000)

    def test_invalid_virt_count_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/'target.o';elf_fixture(path,[0])
            with self.assertRaisesRegex(ValueError,'VIRT/symbol count'):s.ELF(path,True)


if __name__=='__main__':unittest.main()
