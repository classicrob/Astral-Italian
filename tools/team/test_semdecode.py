import random
import unittest
import rdchecked as rd
import semdecode as d


class DecodeTests(unittest.TestCase):
    def test_existing_instruction_exact_fallback(self):
        for line in ['lwz r3, 8(r4)','bl CALLEE_0x80004000','addi r3,r0,8',
                     'fadds f1,f2,f3','cmpwi r3,4','blr']:
            self.assertEqual(d.decode(line),rd.decode(line))

    def test_unknown_remains_unknown(self):
        for line in ['unknown r3,r4','subi. r3,r4,1','bctr','lwzx. r3,r4,r5']:
            with self.subTest(line=line),self.assertRaises(rd.Unproved):d.decode(line)

    def test_extrwi_canonical_rotate_mask(self):
        self.assertEqual(d.decode('extrwi r3,r4,7,2'),rd.decode('rlwinm r3,r4,9,25,31'))

    def test_extrwi_record_keeps_xer_and_cr0(self):
        value=d.decode('extrwi. r3,r4,1,22')
        self.assertEqual(value,rd.decode('rlwinm. r3,r4,23,31,31'))
        self.assertIn('xer',value['use'])
        self.assertIn('cr0',value['def'])

    def test_extrwi_32_bits(self):
        self.assertEqual(d.decode('extrwi r3,r4,32,0'),rd.decode('rlwinm r3,r4,0,0,31'))

    def test_extrwi_all_admitted_bit_ranges_independent_integer_oracle(self):
        rng=random.Random(0x53423445)
        inputs=[0,0xffffffff,0x80000000,1,0x12345678]+[rng.getrandbits(32)for _ in range(20)]
        count=0
        for width in range(1,33):
            for start in range(33-width):
                fields=d.decode(f'extrwi r3,r4,{width},{start}')['const']
                shift,mb,me=map(int,fields)
                for x in inputs:
                    rotated=((x<<shift)|(x>>((32-shift)&31)))&0xffffffff
                    mask=((1<<(32-mb))-1) ^ ((1<<(31-me))-1)
                    actual=rotated&mask
                    expected=(x>>(32-start-width))&((1<<width)-1)
                    self.assertEqual(actual,expected,(width,start,x));count+=1
        self.assertEqual(count,528*len(inputs))

    def test_extrwi_bad_ranges_and_arity_unknown(self):
        for line in ['extrwi r3,r4,0,0','extrwi r3,r4,33,0',
                     'extrwi r3,r4,2,31','extrwi r3,r4,1,-1',
                     'extrwi r3,r4,1','extrwi f1,r4,1,2','extrwi r3,r4,X,2']:
            with self.subTest(line=line),self.assertRaises(rd.Unproved):d.decode(line)

    def test_subi_signed_immediates(self):
        for amount in [-32767,-1,0,1,32767,32768]:
            self.assertEqual(d.decode(f'subi r3,r4,{amount}'),rd.decode(f'addi r3,r4,{-amount}'))

    def test_subi_ra_zero_semantics(self):
        self.assertEqual(d.decode('subi r3,r0,7')['use'],[])

    def test_subi_bad_immediate_unknown(self):
        for value in [-32768,32769,65535]:
            with self.subTest(value=value),self.assertRaises(rd.Unproved):d.decode(f'subi r3,r4,{value}')

    def test_lwzx_dynamic_base_index_roles(self):
        x=d.decode('lwzx r3,r4,r5')
        self.assertEqual((x['base'],x['index'],x['width'],x['use'],x['def']),('r4','r5',4,['r4','r5'],['r3']))

    def test_lwzx_ra_zero_not_r0_value(self):
        x=d.decode('lwzx r3,r0,r5')
        self.assertIsNone(x['base']);self.assertEqual(x['use'],['r5'])

    def test_lwzx_index_r0_is_real_register(self):
        x=d.decode('lwzx r3,r4,r0')
        self.assertEqual(x['use'],['r4','r0'])

    def test_lwzx_destination_aliases_index(self):
        x=d.decode('lwzx r5,r4,r5')
        self.assertEqual(x['use'],['r4','r5']);self.assertEqual(x['def'],['r5'])

    def test_lwzx_invalid_register_and_arity_unknown(self):
        for line in ['lwzx r3,r32,r4','lwzx r3,r4','lwzx f1,r4,r5']:
            with self.subTest(line=line),self.assertRaises(rd.Unproved):d.decode(line)

    def test_bctrl_all_abi_inputs_plus_ctr(self):
        x=d.decode('bctrl');call=rd.decode('bl CALLEE_0x80004000')
        self.assertEqual(x['kind'],'call')
        self.assertEqual(x['use'],['ctr']+call['use'])
        self.assertEqual(x['def'],call['def'])

    def test_bctrl_havoc_includes_link_and_counter(self):
        self.assertIn('lr',d.decode('bctrl')['def'])
        self.assertIn('ctr',d.decode('bctrl')['def'])

    def test_bctrl_extra_operands_unknown(self):
        with self.assertRaises(rd.Unproved):d.decode('bctrl r3')


if __name__=='__main__':unittest.main()
