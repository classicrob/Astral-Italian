import struct
import tempfile
import unittest
from pathlib import Path
import operand_contract as contract


class OperandContractTests(unittest.TestCase):
    def test_relative_bl_forward_and_backward(self):
        for pc,target in [(0x803C6788,0x8003F840),(0x80000000,0x80001000)]:
            word=0x48000001|((target-pc)&0x03fffffc)
            self.assertEqual(contract.branch_link_target(pc,word),target)
        for word in [0x48000000,0x48000003,0x4e800020]:
            with self.assertRaises(ValueError):contract.branch_link_target(0x80000000,word)

    def test_section_range_and_truncation(self):
        data=bytearray(0x110)
        for offset,value in [(0,0x100),(0x48,0x80000000),(0x90,0x10)]:
            struct.pack_into('>I',data,offset,value)
        self.assertEqual(contract.section_bytes(data,0x80000000,4),bytes(4))
        for addr,size in [(0x80000001,4),(0x8000000c,8),(0x80000020,4)]:
            with self.assertRaises(ValueError):contract.section_bytes(data,addr,size)
        with self.assertRaises(ValueError):contract.section_bytes(data[:-8],0x8000000c,4)

    def test_macho_file_addresses_and_truncated_commands(self):
        data=bytearray(0x100);data[:4]=b'\xcf\xfa\xed\xfe'
        struct.pack_into('<I',data,16,1)
        struct.pack_into('<II',data,32,0x19,72)
        struct.pack_into('<QQQQ',data,56,0x100000000,16,0xf0,16)
        data[0xf0:]=bytes(range(16))
        self.assertEqual(contract.macho_bytes(data,0x100000004,4),bytes([4,5,6,7]))
        with self.assertRaises(ValueError):contract.macho_bytes(data,0x10000000f,4)
        with self.assertRaises(ValueError):contract.macho_bytes(data[:40],0x100000000,4)

    def test_committed_site_inventory_matches_reviewed_constants(self):
        import json
        inventory=json.loads(Path(contract.__file__).with_name('operand_sites_v1.json').read_text())
        self.assertEqual(inventory['dol_sha256'],contract.DOL_SHA256)
        self.assertEqual(set(inventory['calls']),set(contract.CALLS))
        for key,(_,pc,target,_) in contract.CALLS.items():
            self.assertEqual(int(inventory['calls'][key]['pre_pc'],16),pc)
            self.assertEqual(int(inventory['calls'][key]['post_pc'],16),pc+4)
            self.assertEqual(int(inventory['calls'][key]['target'],16),target)
        self.assertEqual(inventory['native_bridge']['sha256'],contract.NATIVE_SHA256)

    def test_unpinned_dol_and_native_fail_closed(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'binary';p.write_bytes(bytes(256))
            with self.assertRaisesRegex(ValueError,'identity'):contract.inspect_dol(p)
            with self.assertRaisesRegex(ValueError,'identity'):contract.inspect_native(p)

    def test_read_port_rejects_all_control_and_write_packets(self):
        calls=[];port=contract.ReadPort(lambda command:calls.append(command))
        for command in ['P46=0','G00','M80000000,4:00000000','Z0,80000000,4',
                        'z0,80000000,4','c','s','g','qRcmd,00','p46;P46=0','p47','mcc000000,4','m80000000,10000']:
            with self.assertRaises(ValueError):port.query(command)
        self.assertEqual(calls,[])

    def test_read_port_rejects_bad_lengths_and_non_ram(self):
        port=contract.ReadPort(lambda _: '00')
        with self.assertRaises(ValueError):port.register(70)
        with self.assertRaises(ValueError):port.register(71)
        for address,size in [(0xCC000000,4),(0x817ffffc,8),(0x80000000,257)]:
            with self.assertRaises(ValueError):port.memory(address,size)

    def test_snapshot_preserves_raw_fpr_and_fpscr_without_float_conversion(self):
        calls=[]
        def request(command):
            calls.append(command)
            if command.startswith('m'):return '8000000000000001'
            n=int(command[1:],16)
            if n==64:return '803c6788'
            if n==70:return '00000004'
            return '7ff0000000000001' if 32<=n<64 else '00000000'
        r=contract.passive_snapshot(contract.ReadPort(request),0x803c6788,{'operand':(0x80000000,8)})
        self.assertEqual(r['registers']['33'],'7ff0000000000001')
        self.assertEqual(r['memory_hex']['operand'],'8000000000000001')
        self.assertIsNone(r['ps1'])
        self.assertEqual(r['mode']['fpscr_bits'],'00000004')
        self.assertTrue(all(c.startswith(('p','m')) for c in calls))

    def test_snapshot_rejects_changed_fpscr(self):
        reads=0
        def request(command):
            nonlocal reads
            if command=='p46':
                reads+=1;return '00000004' if reads==1 else '00000000'
            if command=='p40':return '803c6788'
            n=int(command[1:],16);return '0'*(16 if 32<=n<64 else 8)
        with self.assertRaisesRegex(ValueError,'changed'):
            contract.passive_snapshot(contract.ReadPort(request),0x803c6788,{})

    def test_snapshot_rejects_wrong_pc_before_reading_memory(self):
        def request(command):
            self.assertTrue(command.startswith('p'))
            n=int(command[1:],16);return '0'*(16 if 32<=n<64 else 8)
        with self.assertRaisesRegex(ValueError,'PC'):
            contract.passive_snapshot(contract.ReadPort(request),0x803c6788,{'v':(0x80000000,12)})


if __name__=='__main__':unittest.main()
