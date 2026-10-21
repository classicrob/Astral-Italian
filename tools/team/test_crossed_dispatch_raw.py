"""Tiny synthetic ELF bytes only; no compiler, game object or original image."""
import copy
import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
import crossed_dispatch_raw as rawparser
import crossed_dispatch_component as component


def elf():
    names=['','.text','.strtab.a','.symtab.a','.strtab.b','.symtab.b','.rela.text','.shstrtab']
    strings=b'\0';offsets=[]
    for name in names:
        offsets.append(len(strings));strings+=name.encode()+b'\0'
    # Deliberately different ordinal1 in TWO tables; RELA must use tableB.
    payloads=[b'',bytes(16),b'\0@1\0unused\0',bytes(16)+struct.pack('>IIIBBH',1,0,4,2,0,1),
              b'\0Beta\0',bytes(16)+struct.pack('>IIIBBH',1,8,4,0x12,0,1),
              struct.pack('>IIi',4,(1<<8)|1,0),strings]
    types=[0,1,3,2,3,2,4,3];links=[0,0,0,2,0,4,5,0];infos=[0,0,0,2,0,1,1,0];aligns=[0,4,1,4,1,4,4,1];ents=[0,0,0,16,0,16,12,0]
    data=bytearray(52);headers=[]
    for i,payload in enumerate(payloads):
        if i==0:headers.append([0]*10);continue
        while len(data)%aligns[i]:data.append(0)
        headers.append([offsets[i],types[i],6 if i==1 else 0,0,len(data),len(payload),links[i],infos[i],aligns[i],ents[i]])
        data.extend(payload)
    while len(data)%4:data.append(0)
    shoff=len(data)
    for h in headers:data.extend(struct.pack('>10I',*h))
    data[:16]=b'\x7fELF\x01\x02\x01'+bytes(9)
    struct.pack_into('>HHIIIIIHHHHHH',data,16,1,20,1,0,0,shoff,0,52,0,0,40,len(headers),7)
    return bytes(data),headers,shoff

class RawModels(unittest.TestCase):
    def test_own_sh_link_and_all_local_metadata(self):
        data,_,_=elf();snapshot=rawparser.read_snapshot(data)
        self.assertEqual(snapshot['semantic']['relocations for .text#0'][0][3],('sym','Beta'))
        self.assertEqual(len(snapshot['raw_metadata']['symbols']),4)
        self.assertEqual(snapshot['sha256'],hashlib.sha256(data).hexdigest())
        self.assertFalse(snapshot['raw_qualification'])
    def test_wrong_link_and_full_relocation_extent(self):
        data,headers,shoff=elf()
        bad=bytearray(data);struct.pack_into('>I',bad,shoff+6*40+24,2)
        with self.assertRaises(ValueError):rawparser.read_snapshot(bytes(bad))
        bad=bytearray(data);struct.pack_into('>I',bad,headers[6][4],13)
        with self.assertRaises(ValueError):rawparser.read_snapshot(bytes(bad))
        bad=bytearray(data);struct.pack_into('>I',bad,headers[6][4]+4,(9<<8)|1)
        with self.assertRaises(ValueError):rawparser.read_snapshot(bytes(bad))
    def test_local_info_visibility_size_and_raw_rela_identity(self):
        data,headers,shoff=elf();a=rawparser.read_snapshot(data)
        for at,value in [(headers[3][4]+16+13,2),(headers[3][4]+16+7,1)]:
            b=bytearray(data);b[at]=value
            after=rawparser.read_snapshot(bytes(b))
            with self.assertRaises(ValueError):rawparser.strict_raw_metadata(a,after)
        b=bytearray(data);struct.pack_into('>I',b,shoff+6*40+24,3)
        after=rawparser.read_snapshot(bytes(b))
        self.assertEqual(after['semantic']['relocations for .text#0'][0][3],('local','.text#0',0))
        with self.assertRaises(ValueError):rawparser.strict_raw_metadata(a,after)
    def test_symbol_and_payload_header_bounds(self):
        data,headers,shoff=elf()
        for at,value in [(headers[3][4]+16+8,17),(shoff+3*40+36,8),(shoff+1*40+16,len(data)+100)]:
            b=bytearray(data);struct.pack_into('>I',b,at,value)
            with self.assertRaises(ValueError):rawparser.read_snapshot(bytes(b))
    def test_unused_string_payload_not_silently_dropped(self):
        data,headers,_=elf();a=rawparser.read_snapshot(data)
        bad=bytearray(data);bad[headers[2][4]+4]=ord('X');b=rawparser.read_snapshot(bytes(bad))
        self.assertEqual(a['semantic'],b['semantic'])
        with self.assertRaises(ValueError):rawparser.strict_raw_metadata(a,b)
    def test_same_read_is_hashed_and_parsed_no_reopen(self):
        data,_,_=elf()
        with tempfile.TemporaryDirectory() as directory:
            p=Path(directory)/'synthetic.elf';p.write_bytes(data)
            with patch.object(rawparser,'read_snapshot',wraps=rawparser.read_snapshot) as parser,patch('builtins.open',side_effect=AssertionError('no parser reopen')):
                observed=component.snapshot_component(p,hashlib.sha256(data).hexdigest())
                self.assertEqual(parser.call_args.args,(data,));self.assertEqual(observed['sha256'],hashlib.sha256(data).hexdigest())
            with self.assertRaises(ValueError):component.snapshot_component(p,'0'*64)
            link=Path(directory)/'link';link.symlink_to(p)
            with self.assertRaises(OSError):component.snapshot_component(link,hashlib.sha256(data).hexdigest())

if __name__=='__main__':unittest.main()
