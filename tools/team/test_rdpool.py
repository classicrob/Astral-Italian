import collections
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest

from rdchecked import checked_equal, checked_equal_mem, checked_literal_bases
from rdpool import PoolComparator, PoolELF, PoolUnproved, pool_catalog, read_pool_dol

TOKEN = 'STR[53616d65]'


def asm(text):
    return [part.strip() for part in text.split(';') if part.strip()]


def fixture(source=b'Same\0\0\0\0Later\0\0\0', target=None, objects=None, relocations=None, catalog=None):
    target = source if target is None else target
    elf = SimpleNamespace(data=[b'', source], names=['', '.data'],
                          objects={1: objects or [(0, 5), (8, len(source) - 8)]},
                          relocations=relocations or {})
    memory = {0x1000 + i: value for i, value in enumerate(target)}
    def read(address, size):
        return bytes(memory[address+i] for i in range(size)) if all(address+i in memory for i in range(size)) else None
    return PoolComparator(read, elf, catalog or {}, {TOKEN: {0x1000}}, {TOKEN: {('.data', 0)}},
                          {0x1000 + start: {size} for start, size in elf.objects[1]}), memory


class PoolSpanTests(unittest.TestCase):
    def test_full_later_string_positive(self):
        pool, _ = fixture()
        pool.verify(TOKEN, 8)
        self.assertEqual(pool.evidence[0]['size'], 8)

    def test_same_first_string_different_later_string(self):
        pool, _ = fixture(target=b'Same\0\0\0\0Other\0\0\0')
        with self.assertRaisesRegex(PoolUnproved, 'different referenced'):
            pool.verify(TOKEN, 8)

    def test_same_first_word_different_table_tail(self):
        a=b'Same\0\0\0\0'+bytes.fromhex('1122334455667788')
        b=b'Same\0\0\0\0'+bytes.fromhex('1122334499667788')
        pool, _ = fixture(a,b)
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8)
        pool.verify(TOKEN,8,4)
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8,8)

    def test_negative_constant_reach_still_checks_full_object(self):
        pool, _ = fixture()
        pool.source_origins[TOKEN]={('.data',8)}
        pool.target_origins[TOKEN]={0x1008}
        pool.verify(TOKEN,-8)
        self.assertEqual(pool.evidence[0]['target_address'],'0x1000')

    def test_target_extra_tail_only_zero_padding(self):
        source=b'Same\0\0\0\0'+bytes.fromhex('1122334455667788')
        for tail, ok in ((bytes(4),True),(b'Other',False)):
            pool,_=fixture(source,source+tail)
            pool.target_extents[0x1008]={8+len(tail)}
            if ok:pool.verify(TOKEN,8)
            else:
                with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8)

    def test_missing_binary_target_extent_fails_closed(self):
        source=b'Same\0\0\0\0'+bytes.fromhex('1122334455667788')
        pool,_=fixture(source)
        pool.target_extents.clear()
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8)

    def test_missing_object_extent(self):
        pool, _ = fixture(objects=[(0,5)])
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8)

    def test_section_bounds_and_unknown_offset(self):
        pool, _ = fixture()
        for off in (-1,100,None):
            with self.assertRaises(PoolUnproved):pool.verify(TOKEN,off,4)

    def test_ambiguous_base_origin(self):
        pool, _ = fixture()
        pool.target_origins[TOKEN].add(0x2000)
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8)

    def test_callback_relocation_identity(self):
        source=b'Same\0\0\0\0'+bytes(4)
        target=source[:8]+struct.pack('>I',0x80001234)
        rel={(1,8):(1,('callback',0,0,0),0)}
        pool,_=fixture(source,target,relocations=rel,catalog={'callback':0x80001234})
        pool.verify(TOKEN,8)
        self.assertEqual(pool.evidence[0]['resolved_relocations'][0][1],'callback')

    def test_wrong_callback_not_same_symbol(self):
        source=b'Same\0\0\0\0'+bytes(4)
        target=source[:8]+struct.pack('>I',0x80009999)
        pool,_=fixture(source,target,relocations={(1,8):(1,('callback',0,0,0),0)},catalog={'callback':0x80001234})
        with self.assertRaisesRegex(PoolUnproved,'relocation identity'):pool.verify(TOKEN,8)

    def test_unknown_and_partial_relocations(self):
        source=b'Same\0\0\0\0'+bytes(4)
        for kind,name,catalog,width in [(1,'missing',{},4),(6,'callback',{'callback':0},4),(1,'callback',{'callback':0},2)]:
            pool,_=fixture(source,relocations={(1,8):(kind,(name,0,0,0),0)},catalog=catalog)
            with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8,width)

    def test_internal_pointer_recursively_checks_pointee(self):
        source=b'Same\0\0\0\0'+bytes(4)+b'Good\0'
        target=source[:8]+struct.pack('>I',0x100c)+b'Good\0'
        rel={(1,8):(1,('@later',12,5,1),0)}
        objects=[(0,5),(8,4),(12,5)]
        pool,_=fixture(source,target,objects,rel)
        pointer=pool.verify(TOKEN,8,4)
        self.assertEqual(pointer[1],0)
        self.assertEqual(len(pool.evidence),2)
        bad=target[:12]+b'Evil\0'
        pool,_=fixture(source,bad,objects,rel)
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8,4)

    def test_named_defined_pointee_is_not_skipped(self):
        source=b'Same\0\0\0\0'+bytes(4)+b'Good\0'
        target=source[:8]+struct.pack('>I',0x100c)+b'Evil\0'
        pool,_=fixture(source,target,[(0,5),(8,4),(12,5)],{(1,8):(1,('named',12,5,1),0)},{'named':0x100c})
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8,4)

    def test_ambiguous_catalog_entry_fails_closed(self):
        source=b'Same\0\0\0\0'+bytes(4)+b'Good\0'
        target=source[:8]+struct.pack('>I',0x100c)+b'Good\0'
        pool,_=fixture(source,target,[(0,5),(8,4),(12,5)],{(1,8):(1,('ambiguous',12,5,1),0)},{'ambiguous':None})
        with self.assertRaises(PoolUnproved):pool.verify(TOKEN,8,4)

    def test_positive_guard_and_both_dag_dispatches(self):
        pool,_=fixture()
        lines=asm(f'lis r11, {TOKEN}@ha; addi r11, r11, {TOKEN}@l; addi r4, r11, 8; blr')
        self.assertFalse(checked_literal_bases(lines,lines)[0])
        self.assertTrue(checked_literal_bases(lines,lines,pool.verify)[0])
        for checker in (checked_equal,checked_equal_mem):
            self.assertTrue(checker(lines,lines,pool.verify)[0])

    def test_bad_full_span_fails_every_guard_dispatch(self):
        pool,_=fixture(target=b'Same\0\0\0\0Other\0\0\0')
        lines=asm(f'lis r11, {TOKEN}@ha; addi r11, r11, {TOKEN}@l; addi r4, r11, 8; blr')
        for checker in (checked_literal_bases,checked_equal,checked_equal_mem):
            self.assertFalse(checker(lines,lines,pool.verify)[0])

    def test_float_load_width_and_store_rejection(self):
        pool,_=fixture()
        prefix=f'lis r11, {TOKEN}@ha; addi r11, r11, {TOKEN}@l; '
        load=asm(prefix+'lfs f0, 8(r11); blr')
        store=asm(prefix+'stfs f0, 8(r11); blr')
        self.assertTrue(checked_literal_bases(load,load,pool.verify)[0])
        self.assertFalse(checked_literal_bases(store,store,pool.verify)[0])

    def test_indexed_or_returned_unknown_offsets_stay_real(self):
        pool,_=fixture()
        prefix=f'lis r3, {TOKEN}@ha; addi r3, r3, {TOKEN}@l; '
        for tail in ('lwzx r4, r3, r5; blr','bl strchr; lwz r4, 8(r3); blr'):
            lines=asm(prefix+tail)
            self.assertFalse(checked_literal_bases(lines,lines,pool.verify)[0])

    def test_loaded_data_pointer_keeps_provenance(self):
        source=b'Same\0\0\0\0'+bytes(4)+b'Good\0'
        target=source[:8]+struct.pack('>I',0x100c)+b'Good\0'
        pool,_=fixture(source,target,[(0,5),(8,4),(12,5)],{(1,8):(1,('@later',12,5,1),0)})
        lines=asm(f'lis r11, {TOKEN}@ha; addi r11, r11, {TOKEN}@l; lwz r10, 8(r11); lbz r3, 0(r10); blr')
        self.assertTrue(checked_literal_bases(lines,lines,pool.verify)[0])
        lines=asm(f'lis r11, {TOKEN}@ha; addi r11, r11, {TOKEN}@l; lwz r10, 8(r11); lwz r3, 100(r10); blr')
        self.assertFalse(checked_literal_bases(lines,lines,pool.verify)[0])


class PoolInputTests(unittest.TestCase):
    def test_unknown_dol_sections_not_bss_zeroes(self):
        data=bytearray(256)
        struct.pack_into('>III',data,0xD8,0x2000,16,0)
        self.assertIsNone(read_pool_dol(data,0x2000,4))

    def test_bad_elf_is_refused(self):
        with tempfile.TemporaryDirectory()as tmp:
            path=Path(tmp)/'bad.o';path.write_bytes(b'not an ELF')
            with self.assertRaises(PoolUnproved):PoolELF(path)

    def test_catalog_ambiguity_is_preserved(self):
        with tempfile.TemporaryDirectory()as tmp:
            path=Path(tmp)/'symbols.txt'
            path.write_text('f = .text:0x80001000;\nf = .text:0x80002000;\nf = .text:0x80002000;\n')
            self.assertIsNone(pool_catalog(path)['f'])

if __name__=='__main__':unittest.main()
