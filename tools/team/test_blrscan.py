from types import SimpleNamespace
import unittest

import blrscan as b
import splitscan as s


class StaticReferences(unittest.TestCase):
    def test_section_symbol_and_addend_resolve_without_a_named_symbol(self):
        elf=SimpleNamespace(tuples={},origins={3:0x8033a300})
        self.assertEqual(b.resolve_target(elf,('.text',0,0,3),0xdc,s.Catalog('')),0x8033a3dc)

    def test_local_absolute_identity_precedes_misleading_encoded_name(self):
        symbol=('fn_80381750',0,4,1)
        elf=SimpleNamespace(tuples={symbol:[SimpleNamespace(absolute=0x80383780)]},origins={1:0x80380000})
        self.assertEqual(b.resolve_target(elf,symbol,0,s.Catalog('')),0x80383780)

    def test_direct_and_conditional_branch_destinations(self):
        self.assertEqual(b.branch_destination(0x48000011,0x8033a3cc),0x8033a3dc)
        self.assertEqual(b.branch_destination(0x4182fff0,0x8033a3ec),0x8033a3dc)
        self.assertIsNone(b.branch_destination(0x4e800020,0x8033a3dc))

    def test_original_operand_bounds_cover_ha_lo_and_sda(self):
        self.assertTrue(b.encoded_reference(6,0x3c608034,0,0x8033a3dc))
        self.assertTrue(b.encoded_reference(4,0x3863a3dc,0,0x8033a3dc))
        self.assertFalse(b.encoded_reference(6,0x3c60807f,0,0x8033a3dc))
        self.assertFalse(b.encoded_reference(109,0x886d0000,0,0x8033a3dc))
        self.assertIsNone(b.encoded_reference(109,0x88650000,0,0x8033a3dc))


class MetadataPatch(unittest.TestCase):
    def test_extends_only_predecessor_and_removes_only_orphan(self):
        text='named = .text:0x8033A380; // type:function size:0x5C\nfn_8033A3DC = .text:0x8033A3DC; // type:function size:0x4\nreal = .text:0x8033A3E0; // type:function size:0x20\n'
        row={'previous':{'name':'named','size':92},'orphan':{'name':'fn_8033A3DC'}}
        result=b.proposal_symbols(text,[row])
        self.assertIn('size:0x60',result)
        self.assertNotIn('fn_8033A3DC',result)
        self.assertIn('real = .text:0x8033A3E0; // type:function size:0x20',result)
        row['previous']['size']=88
        with self.assertRaises(ValueError):b.proposal_symbols(text,[row])


if __name__=='__main__':unittest.main()
