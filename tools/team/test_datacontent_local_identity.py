"""Local-name collision regression: real OSAlarm/OSMemory record addresses/bytes."""
import struct
import tempfile
from pathlib import Path
from types import SimpleNamespace
import unittest
from datacontent import World, Symbol, Relocation, Pointer, compare_bytes

class LocalIdentityTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.tree = Path(self.temp.name)
        (self.tree/'build/SB4E01').mkdir(parents=True)
        (self.tree/'config/SB4E01').mkdir(parents=True)
        first, second = 0x8070CBB0, 0x8070E0F0
        self.alarm = bytes.fromhex('805b49a0 ffffffff 00000000 00000000')
        self.other = bytes.fromhex('805ba4d0 0000007f 00000000 00000000')
        image = bytearray(0x100 + second - first + 16)
        struct.pack_into('>I',image,0,0x100)
        struct.pack_into('>I',image,0x48,first)
        struct.pack_into('>I',image,0x90,len(image)-0x100)
        image[0x100:0x110] = self.alarm
        image[-16:] = self.other
        (self.tree/'build/SB4E01/main.dol').write_bytes(image)
        (self.tree/'config/SB4E01/symbols.txt').write_text(
            'ShutdownFunctionInfo = .data:0x8070CBB0; // type:object size:0x10 scope:local\n'
            'ShutdownFunctionInfo = .data:0x8070E0F0; // type:object size:0x10 scope:local\n')
        (self.tree/'config/SB4E01/splits.txt').write_text(
            'RVL_SDK/os/OSAlarm.c:\n\t.data start:0x8070CBB0 end:0x8070CBC0\n'
            'RVL_SDK/os/OSMemory.c:\n\t.data start:0x8070E0F0 end:0x8070E100\n')
        self.world = World(self.tree)
        self.elf = SimpleNamespace(names=['','.data'],sections=[(),()],relocations={},
                                   local=lambda s: s.section == 1)
        self.symbol = Symbol('ShutdownFunctionInfo',0,16,1,1,0)
        self.pointer = Pointer(Relocation(4,self.symbol,0))

    def test_osalarm_uintmax_is_not_falsely_compared_to_other_local_127(self):
        self.assertEqual(len(self.world.symbols['ShutdownFunctionInfo']),2)
        obj,error = self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',self.pointer,64)
        self.assertIsNone(error)
        self.assertEqual(obj['start'],0x8070CBB0)
        self.assertEqual(obj['owning_unit'],'RVL_SDK/os/OSAlarm')
        self.assertEqual(bytes.fromhex(obj['hex']),self.alarm)
        result=compare_bytes(bytes.fromhex(obj['hex']),self.alarm)
        self.assertEqual(result['status'],'equal')
        self.assertEqual(result['scalar_differences'],[])

    def test_osalarm_wrong_127_still_reports_real_priority_difference(self):
        obj,error = self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',self.pointer,64)
        source=bytearray(self.alarm);source[4:8]=bytes.fromhex('0000007f')
        result=compare_bytes(bytes.fromhex(obj['hex']),bytes(source))
        self.assertEqual(result['status'],'scalar_difference')
        self.assertEqual([r['target_offset'] for r in result['scalar_differences']],[4])

    def test_same_name_in_other_owning_unit_reads_other_address(self):
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSMemory',self.pointer,64)
        self.assertIsNone(error)
        self.assertEqual(obj['start'],0x8070E0F0)
        self.assertEqual(bytes.fromhex(obj['hex']),self.other)

    def test_undefined_local_name_reports_unresolved_instead_of_last_wins(self):
        ptr=Pointer(Relocation(4,Symbol('ShutdownFunctionInfo',0,0,0,1,1),0))
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_local_symbol_requires_owning_section')

    def test_missing_owning_split_never_falls_back_to_same_name(self):
        obj,error=self.world.target_bound(self.elf,'missing',self.pointer,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_owning_section_unknown')

    def test_duplicate_global_identities_are_ambiguous(self):
        records=self.world.symbols['ShutdownFunctionInfo']
        for record in records:record['local']=False
        ptr=Pointer(Relocation(4,Symbol('ShutdownFunctionInfo',0,0,0,1,1),0))
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'ambiguous_target_symbol_identity')

    def test_local_symbol_outside_owning_split_is_unresolved(self):
        ptr=Pointer(Relocation(4,Symbol('ShutdownFunctionInfo',0x1540,16,1,1,0),0))
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_symbol_outside_owning_section')

    def test_undefined_elf_local_never_uses_matching_global_name(self):
        for record in self.world.symbols['ShutdownFunctionInfo']:record['local']=False
        self.world.symbols['ShutdownFunctionInfo']=self.world.symbols['ShutdownFunctionInfo'][:1]
        ptr=Pointer(Relocation(4,Symbol('ShutdownFunctionInfo',0,0,0,1,0),0))
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_local_symbol_requires_owning_section')

    def test_unique_global_external_retains_address_and_addend(self):
        self.world.symbols['Exported']=[{'name':'Exported','section':'.data',
            'address':0x8070CBB0,'size':16,'function':False,'local':False}]
        ptr=Pointer(Relocation(4,Symbol('Exported',0,0,0,1,1),4),delta=4)
        self.assertEqual(self.world.resolve_address(self.elf,'different',ptr),(0x8070CBB8,None))

    def test_ambiguous_global_is_not_silently_filtered_as_function(self):
        for record in self.world.symbols['ShutdownFunctionInfo']:
            record['local']=False;record['function']=True
        ptr=Pointer(Relocation(4,Symbol('ShutdownFunctionInfo',0,0,0,1,1),0))
        self.assertTrue(self.world.is_data(self.elf,ptr))

    def test_local_addend_cannot_escape_to_foreign_global_record(self):
        self.world.symbols['ShutdownFunctionInfo'][1]['local']=False
        ptr=Pointer(Relocation(4,self.symbol,0x1540))
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_effective_address_outside_owning_section')

    def test_local_delta_cannot_escape_to_foreign_global_record(self):
        self.world.symbols['ShutdownFunctionInfo'][1]['local']=False
        ptr=Pointer(Relocation(4,self.symbol,0),delta=0x1540)
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_effective_address_outside_owning_section')

    def test_negative_addend_and_delta_inside_owning_split_resolve(self):
        sym=Symbol('ShutdownFunctionInfo',8,8,1,1,0)
        ptr=Pointer(Relocation(4,sym,-4),delta=-4)
        self.assertEqual(self.world.resolve_address(self.elf,'RVL_SDK/os/OSAlarm',ptr),(0x8070CBB0,None))

    def test_negative_effective_address_escape_is_advisory(self):
        ptr=Pointer(Relocation(4,self.symbol,-1))
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',ptr,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_effective_address_outside_owning_section')

    def test_local_object_extent_cannot_cross_owning_split(self):
        self.world.symbols['ShutdownFunctionInfo'][0]['size']=32
        self.world.data_symbols[0]['size']=32
        obj,error=self.world.target_bound(self.elf,'RVL_SDK/os/OSAlarm',self.pointer,64)
        self.assertIsNone(obj)
        self.assertEqual(error,'target_local_object_extent_outside_owning_section')

if __name__ == '__main__':unittest.main()
