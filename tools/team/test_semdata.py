"""Object-backed regressions; no source/build/metadata mutations in real checkout."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
import semdata

def repo_root():
    if os.environ.get('SEMCLASS_ROOT'):
        return Path(os.environ['SEMCLASS_ROOT']).resolve()
    for root in Path(__file__).resolve().parents:
        if (root/'tools/team/datacontent.py').is_file():
            return root
    if (Path.cwd()/'tools/team/datacontent.py').is_file():
        return Path.cwd()
    raise RuntimeError('Set SEMCLASS_ROOT when running this scratch copy outside a checkout')

REPO = repo_root()
AREA = Path(os.environ.get('SEMCLASS_TEST_TMP',tempfile.gettempdir()))
AREA.mkdir(parents=True,exist_ok=True)
TEXT,DATA,BSS = 0x80004000,0x807D7000,0x807D8000


def elf(path, section, blob, objects, symbol='A', undefined=False, pointers=(),
        instruction=0x80600000, nobits=False, offset=0, instructions=None):
    """Minimal real ELF32-BE, including actual symbols and RELA records."""
    names=['','.text',section,'.rela.text','.rela.data','.symtab','.strtab','.shstrtab']
    strings=bytearray(b'\0')
    def name(s):
        at=len(strings);strings.extend(s.encode()+b'\0');return at
    code = instructions or (instruction,0x4e800020)
    symbols=[(0,0,0,0,0,0),(name('f'),0,len(code)*4,0x12,0,1)]
    indices={'f':1}
    for n,a,z in objects:
        indices[n]=len(symbols);symbols.append((name(n),a,z,1,0,2))
    if undefined:
        indices[symbol]=len(symbols);symbols.append((name(symbol),0,0,0x11,0,0))
    assert symbol in indices
    text=struct.pack('>'+'I'*len(code),*code)
    rt=struct.pack('>IIi',0,indices[symbol]<<8|109,offset)
    rd=b''.join(struct.pack('>IIi',at,indices[n]<<8|kind,add)for at,n,kind,add in pointers)
    sym=b''.join(struct.pack('>IIIBBH',*x)for x in symbols)
    shstr=bytearray(b'\0');name_offsets=[]
    for n in names:
        name_offsets.append(len(shstr));shstr.extend(n.encode()+b'\0')
    chunks=[b'',text,b''if nobits else blob,rt,rd,sym,bytes(strings),bytes(shstr)]
    metadata=[(0,0,0,0,0,0),(1,6,0,0,4,0),(8 if nobits else 1,2 if section in ['.rodata','.sdata2']else 3,0,0,4,0),(4,0,5,1,4,12),(4,0,5,2,4,12),(2,0,6,1,4,16),(3,0,0,0,1,0),(3,0,0,0,1,0)]
    raw=bytearray(bytes(52));headers=[]
    for i,(b,m)in enumerate(zip(chunks,metadata)):
        while len(raw)%4:raw.append(0)
        at=len(raw);raw.extend(b);ty,flags,link,info,align,entry=m
        headers.append((name_offsets[i],ty,flags,0,at,len(blob)if i==2 and nobits else len(b),link,info,align,entry))
    while len(raw)%4:raw.append(0)
    shoff=len(raw)
    for h in headers:raw.extend(struct.pack('>IIIIIIIIII',*h))
    struct.pack_into('>16sHHIIIIIHHHHHH',raw,0,b'\x7fELF\x01\x02\x01'+bytes(9),1,20,1,0,0,shoff,0,52,0,0,40,len(headers),7)
    path.write_bytes(raw)


class DataGateTests(unittest.TestCase):
    def setup_world(self, section='.data', target=b'\0'*12, source=None,
                    target_objects=(('A',0,4),('B',4,4),('T',8,4)),
                    source_objects=None, source_symbol='A', target_symbol='A',
                    source_undefined=False,target_pointers=(),source_pointers=(),
                    source_section=None, target_word=0x80600000, dol_word=None,instructions=None):
        self.temp=tempfile.TemporaryDirectory(dir=AREA);self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        for p in ['tools/team','config/SB4E01','build/SB4E01/obj/Test','build/SB4E01/src/Test','orig/SB4E01/sys']:(self.root/p).mkdir(parents=True,exist_ok=True)
        shutil.copyfile(REPO/'tools/team/datacontent.py',self.root/'tools/team/datacontent.py')
        self.tp=self.root/'build/SB4E01/obj/Test/Unit.o';self.sp=self.root/'build/SB4E01/src/Test/Unit.o'
        source=target if source is None else source;source_objects=target_objects if source_objects is None else source_objects
        elf(self.tp,section,target,target_objects,target_symbol,pointers=target_pointers,instruction=target_word,nobits=section=='.bss',instructions=instructions)
        elf(self.sp,source_section or section,source,source_objects,source_symbol,undefined=source_undefined,pointers=source_pointers,instruction=target_word,nobits=(source_section or section)=='.bss',instructions=instructions)
        base=BSS if section=='.bss'else DATA
        code = instructions or (target_word,0x4e800020)
        text_size=len(code)*4
        metadata='f = .text:0x%08X; // type:function size:0x%X\n'%(TEXT,text_size)
        for n,a,z in target_objects:metadata+='%s = %s:0x%08X; // type:object size:0x%X scope:local\n'%(n,section,base+a,z)
        if source_undefined:metadata=metadata.replace('scope:local','')
        (self.root/'config/SB4E01/symbols.txt').write_text(metadata)
        (self.root/'config/SB4E01/splits.txt').write_text('Test/Unit.c:\n .text start:0x%08X end:0x%08X\n %s start:0x%08X end:0x%08X\n'%(TEXT,TEXT+text_size,section,base,base+len(target)))
        actual=bytearray(target)
        for at,n,kind,add in target_pointers:
            a=next((a for nn,a,z in target_objects if nn==n),0);value=(TEXT if n=='f'else base+a)+add
            if kind==26:value-=base+at
            actual[at:at+4]=(value&0xffffffff).to_bytes(4,'big')
        image=bytearray(0x100+text_size+len(actual));struct.pack_into('>III',image,0,0x100,0x100+text_size,0)
        struct.pack_into('>III',image,0x48,TEXT,DATA,0);struct.pack_into('>III',image,0x90,text_size,len(actual)if section!='.bss'else 0,0)
        struct.pack_into('>II',image,0xd8,BSS,len(actual))
        a=next(a for n,a,z in target_objects if n==target_symbol)
        actual_word=target_word|13<<16|((base+a-semdata.SDA[13])&0xffff)
        struct.pack_into('>'+'I'*len(code),image,0x100,actual_word if dol_word is None else dol_word,*code[1:])
        image[0x100+text_size:]=actual
        (self.root/'orig/SB4E01/sys/main.dol').write_bytes(image);(self.root/'build/SB4E01/main.dol').write_bytes(image)
        config={'units':[{'name':'main/Test/Unit','target_path':'build/SB4E01/obj/Test/Unit.o','base_path':'build/SB4E01/src/Test/Unit.o'}]}
        (self.root/'objdiff.json').write_text(json.dumps(config))

    def audit(self):return semdata.audit_unit(self.root,'Test/Unit',self.tp,self.sp)
    def reasons(self):return {v['reason']for v in self.audit()['functions']['f']['findings']}

    def test_equal_owned_mutable_object_closes(self):
        self.setup_world();self.assertTrue(self.audit()['functions']['f']['complete'])

    def test_equal_zero_distinct_mutable_objects_never_close(self):
        self.setup_world(source_objects=(('B',0,4),),source_symbol='B')
        self.assertIn('different_or_unproven_data_object_identity',self.reasons())

    def test_canonical_name_cannot_hide_changed_section_offset(self):
        self.setup_world(source_objects=(('A',4,4),))
        self.assertIn('different_or_unproven_data_object_identity',self.reasons())

    def test_equal_NOBITS_same_owned_identity_closes_without_synthetic_bytes(self):
        self.setup_world(section='.bss');v=self.audit()['functions']['f'];self.assertTrue(v['complete'],v)

    def test_distinct_zero_NOBITS_identity_is_semantic(self):
        self.setup_world(section='.bss',source_objects=(('B',0,4),),source_symbol='B')
        self.assertIn('mutable_NOBITS_identity_unproven_or_different',self.reasons())

    def test_readonly_scalar_same_value_other_pool_slot_closes(self):
        self.setup_world(section='.rodata',source_objects=(('@literal',4,4),),source_symbol='@literal')
        self.assertTrue(self.audit()['functions']['f']['complete'])

    def test_readonly_scalar_different_value_semantic(self):
        self.setup_world(section='.rodata',source=(1).to_bytes(4,'big')+bytes(8))
        self.assertIn('different_data_bytes',self.reasons())

    def test_different_sections_even_equal_literal_semantic(self):
        self.setup_world(section='.rodata',source_section='.sdata2')
        self.assertIn('different_data_sections',self.reasons())

    def test_pointer_different_pointees_not_masked(self):
        objects=(('T',0,4),('A',4,4),('B',8,4))
        self.setup_world(target_objects=objects,source_symbol='T',target_symbol='T',target_pointers=((0,'A',1,0),),source_pointers=((0,'B',1,0),))
        self.assertIn('different_or_unresolved_pointer_identity',self.reasons())

    def test_same_data_pointer_identity_alone_does_not_close_payload(self):
        objects=(('T',0,4),('A',4,4),('B',8,4))
        self.setup_world(target_objects=objects,source_symbol='T',target_symbol='T',target_pointers=((0,'A',1,0),),source_pointers=((0,'A',1,0),))
        self.assertIn('data_pointee_contents_unverified',self.reasons())

    def test_undefined_source_provider_never_falls_back_to_DOL(self):
        self.setup_world(source_objects=(('Other',4,4),),source_undefined=True)
        self.assertIn('object_resolution_unknown',self.reasons())

    def test_full_object_tail_detected_even_if_loaded_first_word_equal(self):
        self.setup_world(target_objects=(('A',0,12),),source=bytes(8)+b'\0\0\0\x01')
        self.assertIn('different_data_bytes',self.reasons())

    def test_unknown_extent_semantic_not_equal(self):
        self.setup_world(source_objects=(('A',0,0),('Next',4,0)),target_objects=(('A',0,4),))
        self.assertTrue(self.audit()['functions']['f']['findings'])

    def test_actual_original_instruction_binding_required(self):
        self.setup_world(dol_word=0x4e800020)
        self.assertIn('original_DOL_binding_failed',self.reasons())

    def test_no_tokens_escape_incomplete_function(self):
        self.setup_world(source_objects=(('B',0,4),),source_symbol='B')
        result=self.audit();self.assertNotIn('f',result['relocation_tokens']['source'])

    def test_readonly_name_alone_does_not_override_actual_writable_flag(self):
        self.setup_world(section='.rodata',source_objects=(('@literal',4,4),),source_symbol='@literal')
        raw=bytearray(self.tp.read_bytes());header=struct.unpack_from('>16sHHIIIIIHHHHHH',raw)
        struct.pack_into('>I',raw,header[6]+2*header[11]+8,3)
        self.tp.write_bytes(raw)
        self.assertIn('different_or_unproven_data_object_identity',self.reasons())

    def test_advisory_synthesized_sbss2_zero_is_not_DOL_byte_proof(self):
        self.setup_world(section='.sbss2')
        image=bytearray((self.root/'orig/SB4E01/sys/main.dol').read_bytes())
        struct.pack_into('>I',image,0x94,0)
        struct.pack_into('>I',image,0x100,0x80620000|((DATA-semdata.SDA[2])&0xffff))
        (self.root/'orig/SB4E01/sys/main.dol').write_bytes(image)
        self.assertIn('target_initializer_not_original_DOL_file_bytes',self.reasons())

    def test_wide_load_cannot_close_same_owned_narrow_object(self):
        self.setup_world(target_word=0xc8200000)
        self.assertIn('memory_access_exceeds_referent_extent',self.reasons())

    def test_partial_word_access_cannot_escape_narrow_object_tail(self):
        self.setup_world()
        raw=bytearray(self.sp.read_bytes());header=struct.unpack_from('>16sHHIIIIIHHHHHH',raw)
        relocation=struct.unpack_from('>IIIIIIIIII',raw,header[6]+3*header[11])
        struct.pack_into('>i',raw,relocation[4]+8,2)
        self.sp.write_bytes(raw)
        self.assertIn('memory_access_exceeds_referent_extent',self.reasons())

    def test_loaded_pointer_same_identity_but_pointee_payload_differs(self):
        objects=(('T',0,4),('A',4,4),('B',8,4))
        self.setup_world(target_objects=objects,source=bytes(4)+b'\0\0\0\x01'+bytes(4),
            source_symbol='T',target_symbol='T',target_pointers=((0,'A',1,0),),
            source_pointers=((0,'A',1,0),),instructions=(0x80600000,0x80630000,0x4e800020))
        self.assertIn('data_pointee_contents_unverified',self.reasons())

    def test_code_pointer_identity_needs_no_body_recursion(self):
        objects=(('T',0,4),('A',4,4),('B',8,4))
        self.setup_world(target_objects=objects,source_symbol='T',target_symbol='T',
             target_pointers=((0,'f',1,0),),source_pointers=((0,'f',1,0),))
        self.assertTrue(self.audit()['functions']['f']['complete'])

    def test_portable_historical_Jet_four_values_identical_code_bound_DOL(self):
        # Compact reconstructed negative, not claimed to be the original ELF.
        # The four historical bad words are the real indexed JetTurtle values.
        target=bytes.fromhex('41f00000 41f00000 41f00000 '
            '41c00000 41c00000 41c00000 3b03126f 3b03126f 3bb43958 '
            '012c01e0 012c0000 00000000')
        source=bytearray(target)
        for off,value in [(4,'41a00000'),(16,'41a00000'),(24,'3c8b4396'),(28,'3c23d70a')]:
            source[off:off+4]=bytes.fromhex(value)
        # addi r3,r13,table; lfsx f1,r3,r4; blr. Same text, unknown bounded
        # runtime index stays separate from actual four proven byte differences.
        lfsx=31<<26|1<<21|3<<16|4<<11|535<<1
        self.setup_world(section='.rodata',target=target,source=bytes(source),
             target_objects=(('A',0,48),),target_word=0x38600000,
             instructions=(0x38600000,lfsx,0x4e800020))
        session=semdata.Session(self.root)
        te,se=session.dc.Elf(self.tp),session.dc.Elf(self.sp)
        self.assertEqual(te.contents[1],se.contents[1])
        result=self.audit();value=result['functions']['f']
        changed={(v['target_data_offset'],v['target_hex'],v['source_hex'])for v in value['findings']if v['reason']=='different_data_bytes'}
        self.assertEqual(changed,{(4,'41f00000','41a00000'),(16,'41c00000','41a00000'),
                                (24,'3b03126f','3c8b4396'),(28,'3b03126f','3c23d70a')})
        self.assertEqual(result['stats']['definite_byte_difference_functions'],1)
        self.assertEqual(result['stats']['unknown_functions'],1)
        self.assertFalse(value['complete'])

    def test_code_100_real_Jet_has_four_actual_data_negatives(self):
        fixture=Path(os.environ.get('SEMCLASS_FIXTURES','/tmp/m1-2-semclass-fixtures'))/'jet-indexed-constants'
        if not (fixture/'target.o').exists():self.skipTest('frozen real Jet fixture unavailable')
        result=semdata.audit_unit(REPO,'Game/MapObj/JetTurtle',fixture/'target.o',fixture/'source.o')
        value=result['functions']['exeThrowing__9JetTurtleFv'];self.assertFalse(value['complete'])
        changed={(v['target_data_offset'],v['target_hex'],v['source_hex'])for v in value['findings']if v['reason']=='different_data_bytes'}
        self.assertTrue({(4,'41f00000','41a00000'),(16,'41c00000','41a00000'),(24,'3b03126f','3c8b4396'),(28,'3b03126f','3c23d70a')}<=changed)


    def global_binding(self,path,name):
        raw=bytearray(path.read_bytes());e=semdata.Session(self.root).dc.Elf(path)
        symtab=next(v for v in e.sections if v[1]==2)
        for i,v in enumerate(e.symbols):
            if v.name==name:raw[symtab[4]+i*symtab[9]+12]=0x11
        path.write_bytes(raw)

    def padded_world(self,section='.data',target_word=0x80600000,target=None):
        self.setup_world(section=section,target=bytes(8)if target is None else target,
            target_objects=(('A',0,8),),source_objects=(('A',0,4),),target_word=target_word)

    def orphan_world(self,name='instance__Q24Test5State',padding=True,local=False):
        self.setup_world(section='.bss',target=bytes(8 if padding else 4),
            target_objects=((name,0,8 if padding else 4),),source_objects=((name,0,4),),
            source_symbol=name,target_symbol=name)
        path=self.root/'config/SB4E01/splits.txt'
        path.write_text('Test/Unit.c:\n .text start:0x%08X end:0x%08X\n'%(TEXT,TEXT+8))
        if not local:
            meta=self.root/'config/SB4E01/symbols.txt';meta.write_text(meta.read_text().replace(' scope:local',''))
            self.global_binding(self.tp,name);self.global_binding(self.sp,name)
        self.orphan_name=name

    def test_C1_anonymous_mutable_exact_scoped_offset_extent_closes(self):
        self.setup_world(source_objects=(('@anonymous',0,4),),source_symbol='@anonymous')
        value=self.audit()['functions']['f'];self.assertTrue(value['complete'],value)

    def test_C1_target_owner_bound_from_complete_split_range(self):
        self.setup_world(source_objects=(('@anonymous',0,4),),source_symbol='@anonymous')
        ss=semdata.Session(self.root);e=ss.dc.Elf(self.tp)
        obj,error=ss.target_object(e,'Test/Unit',ss.dc.Pointer(next(iter(e.relocations.values()))),65536)
        self.assertIsNone(error);self.assertEqual(obj['owning_unit'],'Test/Unit')

    def test_C1_adjacent_foreign_owner_not_silently_absorbed(self):
        self.padded_world()
        self.global_binding(self.tp,'A')
        p=self.root/'config/SB4E01/symbols.txt';p.write_text(p.read_text().replace(' scope:local',''))
        p=self.root/'config/SB4E01/splits.txt'
        p.write_text('Test/Unit.c:\n .text start:0x%08X end:0x%08X\n .data start:0x%08X end:0x%08X\nTest/Other.c:\n .data start:0x%08X end:0x%08X\n'%(TEXT,TEXT+8,DATA,DATA+4,DATA+4,DATA+8))
        self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_C1_partial_foreign_overlap_rejects_nominal_whole_owner(self):
        self.padded_world()
        p=self.root/'config/SB4E01/splits.txt'
        with p.open('a')as out:out.write('Test/Other.c:\n .data start:0x%08X end:0x%08X\n'%(DATA+4,DATA+8))
        self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_C2_zero_PROGBITS_alignment_tail_direct_reads_close(self):
        self.padded_world();value=self.audit()['functions']['f'];self.assertTrue(value['complete'],value)

    def test_C2_zero_NOBITS_alignment_tail_direct_reads_close(self):
        self.padded_world(section='.bss');value=self.audit()['functions']['f'];self.assertTrue(value['complete'],value)

    def test_C2_nonzero_PROGBITS_tail_remains_unknown(self):
        self.padded_world(target=bytes(7)+b'\x01');value=self.audit()['functions']['f']
        self.assertFalse(value['complete']);self.assertTrue(any(v['reason']=='alignment_padding_unknown'and v['detail']=='alignment_tail_nonzero'for v in value['findings']))

    def test_C2_alias_object_inside_zero_tail_never_waived(self):
        self.padded_world()
        with (self.root/'config/SB4E01/symbols.txt').open('a')as out:
            out.write('Alias = .data:0x%08X; // type:object size:0x4\n'%(DATA+4))
        value=self.audit()['functions']['f'];self.assertFalse(value['complete'])
        self.assertTrue(any(v['reason']=='alignment_padding_unknown'and 'aliases' in v['detail']for v in value['findings']))

    def test_C2_wide_access_cannot_consume_alignment_tail(self):
        self.padded_world(target_word=0xc8200000);value=self.audit()['functions']['f']
        self.assertFalse(value['complete']);self.assertIn('memory_access_exceeds_referent_extent',{v['reason']for v in value['findings']})

    def test_C2_later_out_of_bounds_read_blocks_whole_function_padding(self):
        instructions=(0x38600000,0x80830004,0x4e800020)
        self.setup_world(target=bytes(8),target_objects=(('A',0,8),),source_objects=(('A',0,4),),
            target_word=instructions[0],instructions=instructions)
        value=self.audit()['functions']['f'];self.assertFalse(value['complete'])
        self.assertTrue(any(v['reason']=='alignment_padding_unknown'for v in value['findings']))

    def test_C2_non_alignment_extent_is_not_zero_tail_waiver(self):
        self.setup_world(target=bytes(12),target_objects=(('A',0,12),),source_objects=(('A',0,4),))
        value=self.audit()['functions']['f'];self.assertFalse(value['complete'])
        self.assertTrue(any(v['reason']=='alignment_padding_unknown'and v['detail']=='alignment_extent_incompatible'for v in value['findings']))

    def test_C3_fully_mangled_unique_global_orphan_BSS_with_padding_closes(self):
        self.orphan_world();value=self.audit()['functions']['f'];self.assertTrue(value['complete'],value)

    def test_C3_fully_mangled_unique_global_orphan_BSS_without_padding_closes(self):
        self.orphan_world(padding=False);value=self.audit()['functions']['f'];self.assertTrue(value['complete'],value)

    def test_C3_local_mangled_name_never_uses_global_fallback(self):
        self.orphan_world(local=True);self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_C3_bare_global_name_never_uses_orphan_binding(self):
        self.orphan_world(name='GlobalNerve');self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_C3_duplicate_global_records_even_same_address_rejected(self):
        self.orphan_world();p=self.root/'config/SB4E01/symbols.txt'
        p.write_text(p.read_text()+next(v for v in p.read_text().splitlines()if v.startswith(self.orphan_name+' ='))+'\n')
        self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_C3_adjacent_owner_tail_prevents_orphan_binding(self):
        self.orphan_world()
        with (self.root/'config/SB4E01/splits.txt').open('a')as out:
            out.write('Test/Other.c:\n .bss start:0x%08X end:0x%08X\n'%(BSS+4,BSS+8))
        self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_C3_alias_inside_orphan_alignment_tail_rejected(self):
        self.orphan_world()
        with (self.root/'config/SB4E01/symbols.txt').open('a')as out:
            out.write('Alias = .bss:0x%08X; // type:object size:0x4\n'%(BSS+4))
        self.assertFalse(self.audit()['functions']['f']['complete'])

    def test_three_word_discovery_does_not_make_byte_observation_definite(self):
        code=(0xc0200000,0x7c631b78,0x7c842378,0x4e800020)
        self.setup_world(section='.rodata',target_objects=(('A',0,4),),target=bytes(12),
            source_objects=(('@literal',4,4),),source_symbol='@literal',source=bytes(4)+(1).to_bytes(4,'big')+bytes(4),
            target_word=code[0],instructions=code)
        elf(self.sp,'.rodata',bytes(4)+(1).to_bytes(4,'big')+bytes(4),
            (('@literal',4,4),),symbol='@literal',instructions=(*code[:3],0x60000000,code[3]))
        result=self.audit();value=result['functions']['f'];self.assertFalse(value['complete'])
        self.assertEqual(result['stats']['definite_byte_difference_functions'],0)
        self.assertEqual(result['stats']['observed_byte_difference_functions'],1)
        self.assertIn('data_consumer_correspondence_unknown',{v['reason']for v in value['findings']})



    def test_C1_absent_canonical_named_global_cannot_alias_by_layout_collision(self):
        self.setup_world(source_objects=(('OtherCanonicalObject',0,4),),source_symbol='OtherCanonicalObject')
        self.global_binding(self.sp,'OtherCanonicalObject')
        result=self.audit();self.assertFalse(result['functions']['f']['complete'])
        self.assertIn('different_or_unproven_data_object_identity',{v['reason']for v in result['functions']['f']['findings']})



    def test_C2_returned_address_escape_without_consumer_extent_stays_unknown(self):
        self.padded_world(section='.bss',target_word=0x38600000)
        value=self.audit()['functions']['f'];self.assertFalse(value['complete'])
        self.assertTrue(any(v['reason']=='alignment_padding_unknown'for v in value['findings']))



    def test_identity_cache_binds_actual_ELF_scope_and_contents_each_audit(self):
        self.setup_world();session=semdata.Session(self.root)
        self.assertTrue(session.audit_unit('Test/Unit',self.tp,self.sp)['functions']['f']['complete'])
        # Same path/offset/name/size, but scope changed from local to global;
        # the catalog has only the local identity. A stale path cache must fail.
        self.global_binding(self.sp,'A')
        result=session.audit_unit('Test/Unit',self.tp,self.sp)
        self.assertFalse(result['functions']['f']['complete'])
        self.assertNotIn('f',result['relocation_tokens']['source'])


if __name__ == '__main__':unittest.main()
