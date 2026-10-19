"""PPC opcode19 control-flow regressions for live post-crclr data consumers."""
import struct
from types import SimpleNamespace
import unittest
from datacontent import Pointer, Relocation, Symbol, compare_bytes, register_flow, trace

class Fixture:
    def __init__(self, middle):
        self.table = Symbol('table', 0, 12, 2, 1, 0)
        self.function = Symbol('fixture', 0, 12 + 4*len(middle), 1, 2, 1)
        words = [0x3BE00000] + middle + [(31<<26)|(1<<21)|(31<<16)|(4<<11)|(535<<1),0x4E800020]
        self.contents = [b'',struct.pack('>'+str(len(words))+'I',*words),bytes(12)]
        self.sections = [(0,)*10,(0,1,6,0,0,len(self.contents[1]),0,0,4,0),(0,1,2,0,0,12,0,0,4,0)]
        self.relocations = {(1,0):Relocation(4,self.table,0)}
    def instruction_relocation(self, section, address):
        return self.relocations.get((section,address))
    def references(self):
        return trace(self,self.function,'fixture',SimpleNamespace(is_data=lambda elf,p:p.relocation.symbol==self.table))

class Opcode19Tests(unittest.TestCase):
    def assert_indexed_reference(self, middle, expected=True):
        f=Fixture(middle);refs,_=f.references();site=4*(len(middle)+1)
        indexed=[r for r in refs if r['kind']=='lfsx' and r['site']==site]
        self.assertEqual(bool(indexed),expected)
        return indexed
    def test_crclr_does_not_hide_changed_indexed_table_tail(self):
        indexed=self.assert_indexed_reference([0x4CC63182]) # crclr cr1eq
        self.assertTrue(indexed[0]['dynamic_index'])
        self.assertEqual(indexed[0]['pointer'].relocation.symbol.name,'table')
        result=compare_bytes(bytes.fromhex('00000000 00000000 00000001'),bytes(12))
        self.assertEqual(result['status'],'scalar_difference')
        self.assertEqual([d['target_offset'] for d in result['scalar_differences']],[8])
    def test_cror_keeps_gpr_pointer(self):
        self.assert_indexed_reference([0x4C000382]) # cror 0,0,0
    def test_isync_keeps_gpr_pointer(self):
        self.assert_indexed_reference([0x4C00012C])
    def test_unconditional_blr_stops_fallthrough(self):
        self.assert_indexed_reference([0x4E800020],False)
    def test_unconditional_bctr_stops_fallthrough(self):
        self.assert_indexed_reference([0x4E800420],False)
    def test_rfi_stops_fallthrough(self):
        self.assert_indexed_reference([0x4C000064],False)
    def test_conditional_blr_preserves_fallthrough(self):
        self.assert_indexed_reference([0x4D820020]) # beqlr
    def test_indirect_call_kills_volatile_pointer_but_keeps_nonvolatile(self):
        f=Fixture([0x38600000,0x4E800421]) # li r3,table; bctrl
        f.relocations[1,4]=Relocation(4,f.table,0)
        states=register_flow(f,f.function)
        self.assertIn(12,states)
        self.assertNotIn(3,states[12])
        self.assertIsInstance(states[12][31],Pointer)
        self.assert_indexed_reference([0x4E800421])

if __name__=='__main__': unittest.main()
