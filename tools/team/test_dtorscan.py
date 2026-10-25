from types import SimpleNamespace
import struct
import unittest

import dtorscan as d
import splitscan as s


def fixture(words, relocations=None):
    function = s.ElfSymbol('example',0,len(words)*4,1,2,1,None)
    elf = SimpleNamespace(data=[None,struct.pack('>'+str(len(words))+'I',*words)],relocations=relocations or {})
    return elf,function


class DestructorCensus(unittest.TestCase):
    def test_branch_payload_is_masked_but_relocation_identity_is_retained(self):
        words=[0x9421fff0,0x7c0802a6,0x2c030000,0x60000000,0x48000001,0x7fe3fb78,0x38210010,0x4e800020]
        a,f=fixture(words,{(1,16):(10,('__dl__FPv',0,0,0),0)})
        other=words[:];other[4]=0x48001235
        b,g=fixture(other,{(1,16):(10,('__dla__FPv',0,0,0),0)})
        self.assertEqual(d.signature(a,f)[0],d.signature(b,g)[0])
        self.assertNotEqual(d.signature(a,f)[1],d.signature(b,g)[1])

    def test_unhandled_relocation_does_not_become_an_identity(self):
        a,f=fixture([0x60000000]*7+[0x4e800020],{(1,4):(255,('unknown',0,0,0),0)})
        self.assertIsNone(d.signature(a,f))

    def test_scaffold_requires_null_receiver_delete_and_receiver_return(self):
        words=[0x9421fff0,0x7c0802a6,0x2c030000,0x60000000,0x48000001,0x7fe3fb78,0x38210010,0x4e800020]
        a,f=fixture(words,{(1,16):(10,('__dl__FPv',0,0,0),0)})
        self.assertTrue(d.destructor_scaffold(a,f))
        a.relocations={}
        self.assertFalse(d.destructor_scaffold(a,f))
        words[2]=0x2c040000
        a,f=fixture(words,{(1,16):(10,('__dl__FPv',0,0,0),0)})
        self.assertFalse(d.destructor_scaffold(a,f))

    def test_original_exactness_does_not_mask_nonrelocated_instruction_changes(self):
        words=[0x9421fff0,0x7c0802a6,0x2c030000,0x60000000,0x48000001,0x7fe3fb78,0x38210010,0x4e800020]
        a,f=fixture(words,{(1,16):(10,('__dl__FPv',0,0,0),0)})
        original=words[:];original[4]=0x48001235
        dol=SimpleNamespace(bytes=lambda address,size:struct.pack('>8I',*original))
        self.assertTrue(d.original_body_equal(a,f,0x80000000,dol))
        original[2]=0x2c040000
        self.assertFalse(d.original_body_equal(a,f,0x80000000,dol))


if __name__=='__main__':
    unittest.main()
