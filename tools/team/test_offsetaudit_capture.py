#!/usr/bin/env python3
"""Exact production-token mapping and opt-in compiler guard regressions."""
from pathlib import Path
import contextlib, hashlib, importlib.util, io, json, sys, tempfile, unittest
from types import SimpleNamespace
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parent))
import offsetaudit as audit
import offsetaudit_capture as capture


class CaptureTests(unittest.TestCase):
    def setUp(self):
        capture.reset_context_pins()

    def setup_tree(self, directory, original, encoded):
        capture.ROOT=Path(directory)/'root';capture.LEGACY=Path(directory)/'legacy'
        for root in [capture.ROOT,capture.LEGACY]:root.mkdir(exist_ok=True)
        (capture.ROOT/'x.cpp').write_text(original)
        prefix='#include <petari/utf16.h>\n#include <petari/msl_stdio.h>\n#line 1 "x.cpp"\n'
        (capture.LEGACY/'x.cpp').write_text(prefix+encoded)
        capture.AUD=audit;capture.MAPS={}
        return len(prefix.encode())

    def test_utf16_and_literal_expansion_exact_token_mapping(self):
        with tempfile.TemporaryDirectory() as tmp:
            raw='wchar_t names[4]; auto s="日本語"; p->_2C=4;'
            enc='PetariChar16 names[4]; auto s="\\223\\372\\226\\173\\214\\352"; p->_2C=4;'
            prefix=self.setup_tree(tmp,raw,enc)
            self.assertIsNotNone(capture.tokenmap('x.cpp'))
            rawpos=raw.encode().index(b'_2C');encpos=prefix+enc.encode().index(b'_2C')
            self.assertEqual(capture.original_offset('x.cpp',encpos),rawpos)
            self.assertEqual(capture.original_offset('x.cpp',encpos+3,True),rawpos+3)

    def test_unapproved_token_change_cannot_map(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.setup_tree(tmp,'p->_2C=4;','p->_2D=4;')
            self.assertIsNone(capture.tokenmap('x.cpp'))

    def test_macro_expansion_retains_provenance_and_does_not_validate_as_field_token(self):
        with tempfile.TemporaryDirectory() as tmp:
            prefix=self.setup_tree(tmp,'GET;','GET;')
            capture.BYFILE={'x.cpp':[{'id':'row','byte_offset':0,'end_byte_offset':3}]}
            loc={'file':str(capture.LEGACY/'x.cpp'),'offset':prefix,'tokLen':3}
            node={'id':'node','kind':'MemberExpr','name':'flag','type':{'qualType':'bool'},
                  'range':{'begin':{'spellingLoc':loc,'expansionLoc':loc},
                           'end':{'spellingLoc':loc,'expansionLoc':loc}}}
            facts,decls,errors=capture.reduce_ast(node)
            self.assertEqual(len(facts),1)
            self.assertIn('macro_provenance',facts[0])
            self.assertFalse(facts[0]['token_validation'])

    def test_cached_dependency_drift_rejects(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'type.hpp';p.write_text('changed')
            with self.assertRaisesRegex(ValueError,'dependency drift'):
                capture.verify_dependencies({str(p):{'sha256':'old'}})

    def test_constructor_retains_field_identity_without_inventing_initializer_range(self):
        with tempfile.TemporaryDirectory() as tmp:
            source='C::C() : _10(0) {}'
            prefix=self.setup_tree(tmp,source,source)
            capture.BYFILE={'x.cpp':[{'id':'row','byte_offset':9,'end_byte_offset':12}]}
            node={'id':'ctor','kind':'CXXConstructorDecl','name':'C',
                  'type':{'qualType':'void ()'},
                  'range':{'begin':{'file':str(capture.LEGACY/'x.cpp'),'offset':prefix,'tokLen':1},
                           'end':{'offset':prefix+len(source)-1,'tokLen':1}},
                  'inner':[{'kind':'CXXCtorInitializer',
                            'anyInit':{'id':'field','kind':'FieldDecl','name':'_10','type':{'qualType':'int'}},
                            'inner':[{'kind':'IntegerLiteral','value':'0','type':{'qualType':'int'}}]}]}
            facts,_,errors=capture.reduce_ast(node)
            self.assertFalse(errors)
            ctor=next(f for f in facts if f['kind']=='CXXConstructorDecl')
            initializer=ctor['children'][0]
            self.assertEqual(initializer['anyInit']['name'],'_10')
            self.assertNotIn('range',initializer)
            self.assertFalse(any(f['kind']=='CXXCtorInitializer' for f in facts))

    def test_jasbank_anyinit_preview_preserves_header_fields(self):
        # Reduced from JASBank.cpp / JASBasicInst.hpp. The real header fields
        # precede the ctor's location-less anyInit previews in Clang's AST.
        with tempfile.TemporaryDirectory() as tmp:
            source='JASInstParam::JASInstParam() : _18(0), _24(false) {}'
            prefix=self.setup_tree(tmp,source,source)
            capture.BYFILE={'x.cpp':[{'id':'row','byte_offset':30,'end_byte_offset':33}]}
            header=str(capture.LEGACY/'libs/JSystem/include/JSystem/JAudio2/JASBasicInst.hpp')
            fields=[('0x130280370','_18','u32',100),('0x130280578','_24','bool',140)]
            full=[];initializers=[]
            for ident,name,typ,offset in fields:
                preview={'id':ident,'kind':'FieldDecl','name':name,'type':{'qualType':typ}}
                full.append(dict(preview,loc={'file':header,'offset':offset,'tokLen':3},
                                 range={'begin':{'offset':offset-4,'tokLen':3},
                                        'end':{'offset':offset,'tokLen':3}}))
                initializers.append({'kind':'CXXCtorInitializer','anyInit':preview})
            record={'id':'record','kind':'CXXRecordDecl','name':'JASInstParam','inner':full}
            ctor={'id':'0x110392e20','kind':'CXXConstructorDecl','name':'JASInstParam',
                  'type':{'qualType':'void ()'},
                  'range':{'begin':{'file':str(capture.LEGACY/'x.cpp'),'offset':prefix,'tokLen':12},
                           'end':{'offset':prefix+len(source)-1,'tokLen':1}},'inner':initializers}
            for children in [[record,ctor],[ctor,record]]:
                _,decls,errors=capture.reduce_ast({'kind':'TranslationUnitDecl','inner':children})
                self.assertFalse(errors)
                for ident,name,typ,offset in fields:
                    field=decls[ident]
                    self.assertEqual(field['location']['file'],'libs/JSystem/include/JSystem/JAudio2/JASBasicInst.hpp')
                    self.assertEqual(field['location']['offset'],offset)
                    self.assertEqual(field['range']['end']['offset'],offset)
                    self.assertEqual(field['parent']['id'],'record')
                    self.assertEqual(field['constructor_references'][0]['id'],'0x110392e20')

    def test_case_differing_include_dependency_without_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.setup_tree(tmp,'int x;','int x;')
            relative='include/Game/Animation/XanimeCore.hpp'
            header=capture.LEGACY/relative;header.parent.mkdir(parents=True)
            header.write_text('struct XanimeCore {};')
            original=capture.ROOT/relative;original.parent.mkdir(parents=True)
            original.write_bytes(header.read_bytes())
            alias=header.with_name('Xanimecore.hpp')
            # M1: this already exists and is the same inode. On case-sensitive
            # CI, a symlink supplies a real filesystem alias with the same spelling.
            if not alias.exists():alias.symlink_to(header.name)
            self.assertTrue(alias.samefile(header))
            capture.pin_context_file(header)
            canonical,binding=capture.bind_context_dependency(alias)
            self.assertEqual(canonical,str(header))
            self.assertEqual(binding['reported_path'],str(alias))
            self.assertEqual(binding['sha256'],hashlib.sha256(header.read_bytes()).hexdigest())
            capture.OUT=Path(tmp)/'out';capture.OUT.mkdir()
            capture.INPUT={'files':{'x.cpp':hashlib.sha256((capture.ROOT/'x.cpp').read_bytes()).hexdigest()}}
            capture.CTX={'command':['never-a-compiler','-fsyntax-only'],'source_base':'fixture'}
            capture.PRODUCER_SHA=capture.AUDIT_SHA=capture.ENCODER_SHA='fixture'
            capture.COMPILER={};capture.BYFILE={}
            def fake_compiler(command,**kwargs):
                # Produce deterministic files only: no compiler process is spawned.
                kwargs['stdout'].write('{"kind":"TranslationUnitDecl"}')
                Path(command[command.index('-MF')+1]).write_text('x.o: '+str(alias)+'\n')
                return SimpleNamespace(returncode=0)
            with patch.object(capture.subprocess,'run',side_effect=fake_compiler) as run:
                summary=capture.collect('x.cpp')
            self.assertEqual(run.call_count,1)
            report=json.loads((capture.OUT/summary['evidence_file']).read_text())
            self.assertEqual(set(report['dependencies']),{str(header)})
            self.assertEqual(report['dependencies'][str(header)]['original']['path'],relative)

    def test_case_only_names_distinct_real_files_never_alias_even_same_bytes(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);left=root/'left'/'XanimeCore.hpp';right=root/'right'/'Xanimecore.hpp'
            left.parent.mkdir();right.parent.mkdir()
            left.write_text('same bytes');right.write_bytes(left.read_bytes())
            self.assertFalse(left.samefile(right))
            # Two actual files/inodes with case-only leaf names. Project their
            # resolved spelling into one case-sensitive namespace, so the test
            # remains meaningful on M1's case-insensitive filesystem. Only the
            # spelling is projected: stat, size and content are the actual files'.
            views={left:root/'view'/'XanimeCore.hpp',right:root/'view'/'Xanimecore.hpp'}
            with patch.object(Path,'resolve',autospec=True,side_effect=lambda p:views[p]):
                capture.pin_context_file(left)
                with self.assertRaisesRegex(ValueError,'dependency absent/changed'):
                    capture.bind_context_dependency(right)
                # Negative control: a blanket casefold would select the pinned
                # file despite the actual different inode and identical bytes.
                folded={str(p).casefold() for p in capture.CONTEXT_REALPATHS}
                self.assertIn(str(views[right]).casefold(),folded)

    def test_same_inode_changed_bytes_reject(self):
        with tempfile.TemporaryDirectory() as tmp:
            path=Path(tmp)/'Header.hpp';path.write_text('before')
            capture.pin_context_file(path);path.write_text('after!')
            with self.assertRaisesRegex(ValueError,'dependency absent/changed'):
                capture.bind_context_dependency(path)

    def test_no_compiler_in_dry_run(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'inventory.json').write_text(json.dumps({'files':{'x.cpp':'fixture'},'rows':[{'id':'x','file':'x.cpp','byte_offset':0}]}))
            plan={'schema':1,'root':tmp,'inventory':'inventory.json','command':['clang++','-fsyntax-only']}
            (root/'plan.json').write_text(json.dumps(plan))
            with patch.object(capture.subprocess,'run',side_effect=AssertionError('no implicit compiler')):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(capture.main(['--plan',str(root/'plan.json'),'--out',str(root/'out')]),0)

    def test_stale_source_pin_rejects_before_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            capture.ROOT=Path(tmp);(capture.ROOT/'x.cpp').write_text('changed')
            capture.INPUT={'files':{'x.cpp':'old'}}
            with self.assertRaisesRegex(ValueError,'source pin drift'):
                capture.verify_source_pins()


if __name__=='__main__':unittest.main()
