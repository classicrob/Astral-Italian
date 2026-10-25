#!/usr/bin/env python3
"""Compiler-only command changes and whole-unit report guards."""
import ast
import json
from pathlib import Path
import tempfile
import unittest

import gametrial as g
import fnsearch


class MatrixGuards(unittest.TestCase):
    def scores(self):
        return dict(functions={'near':dict(score=99,size=100),'exact':dict(score=100,size=40)},sections={'.data':dict(score=99,size=20),'.bss':dict(score=100,size=16)},matched_code=40,matched_data=16)

    def test_any_fuzzy_function_drop_rejects_a_code_gain(self):
        before=self.scores();after=json.loads(json.dumps(before))
        after['functions']['near']['score']=100;after['functions']['exact']['score']=99.99;after['matched_code']=100
        c=g.compare(before,after)
        self.assertFalse(c['strict_gain']);self.assertEqual(c['function_drops'],{'exact':[100,99.99]})

    def test_data_section_drop_rejects_an_exact_function_win(self):
        before=self.scores();after=json.loads(json.dumps(before))
        after['functions']['near']['score']=100;after['matched_code']=140;after['sections']['.bss']['score']=99.999
        c=g.compare(before,after)
        self.assertFalse(c['strict_gain']);self.assertIn('.bss',c['data_section_drops'])

    def test_missing_function_or_data_section_is_a_drop(self):
        before=self.scores();after=json.loads(json.dumps(before))
        del after['functions']['exact'];del after['sections']['.data']
        c=g.compare(before,after)
        self.assertIn('exact',c['function_drops']);self.assertIn('.data',c['data_section_drops'])

    def test_fuzzy_only_and_data_only_survivors_are_distinguished(self):
        before=self.scores();after=json.loads(json.dumps(before));after['functions']['near']['score']=99.1
        c=g.compare(before,after)
        self.assertTrue(c['strict_gain']);self.assertEqual(c['matched_code_gain'],0);self.assertEqual(c['new_exact_bytes'],0)
        after=self.scores();after['sections']['.data']['score']=100;after['matched_data']=36
        c=g.compare(before,after)
        self.assertTrue(c['strict_gain']);self.assertEqual(c['matched_data_gain'],20)

    def test_equal_scores_are_not_a_gain(self):
        self.assertFalse(g.compare(self.scores(),self.scores())['strict_gain'])

    def test_report_extracts_all_target_functions_and_all_noncode_sections(self):
        report={'units':[{'functions':[{'name':'f','size':'12','fuzzy_match_percent':100}], 'sections':[{'name':'.text','size':'16','fuzzy_match_percent':90},{'name':'extab','size':'8','fuzzy_match_percent':50},{'name':'.bss','size':'4','fuzzy_match_percent':100}]}]}
        scores=g.report_scores(report)
        self.assertEqual(set(scores['sections']),{'extab','.bss'})
        self.assertEqual((scores['matched_code'],scores['matched_data']),(12,4))


class CompilerAndPatch(unittest.TestCase):
    def test_only_compiler_and_output_operands_change(self):
        edge={'argv':['build/tools/wibo','build/tools/sjiswrap.exe','build/compilers/Wii/1.0/mwcceppc.exe','-pragma','cats off','-ipa','file','-opt','noloop','-i','include','-MMD','-c','src/Game/Test.cpp','-o','build/shared'], 'compiler_index':2}
        got=g.compiler_argv(edge,'Wii/1.3',Path('/private/result'))
        changed=[i for i,(a,b) in enumerate(zip(edge['argv'],got)) if a!=b]
        self.assertEqual(changed,[2,15])
        self.assertEqual(got[2],'build/compilers/Wii/1.3/mwcceppc.exe')
        self.assertEqual(edge['argv'][-1],'build/shared')

    def test_config_patch_preserves_other_flags_and_does_not_write_source(self):
        source='objects = [Object(NonMatching, "Game/A.cpp", extra_cflags=["-opt noloop"]),\nObject(NonMatching, "Game/B.cpp", mw_version="Wii/1.0",),\nObject(NonMatching, "Game/C.cpp",\n extra_cflags=["-rostr"],\n)]\n'
        patch=g.config_patch(source,{'main/Game/A':'Wii/1.1','main/Game/B':'Wii/1.3','main/Game/C':'Wii/1.1'})
        self.assertIn('extra_cflags=["-opt noloop"], mw_version="Wii/1.1"',patch)
        self.assertIn('mw_version="Wii/1.3",',patch)
        self.assertIn('extra_cflags=["-rostr"], mw_version="Wii/1.1",',patch)
        ast.parse(source)

    def test_missing_or_duplicate_object_call_refuses_patch(self):
        with self.assertRaises(ValueError):g.config_patch('x=[]\n',{'main/Game/A':'Wii/1.1'})
        source='x=[Object(N,"Game/A.cpp"),Object(N,"Game/A.cpp")]\n'
        with self.assertRaises(ValueError):g.config_patch(source,{'main/Game/A':'Wii/1.1'})

    def test_cached_edges_preserve_unit_scope_and_invalidate_after_graph_changes(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);path=root/'build.ninja'
            graph='rule mwcc\n  command = build/tools/wibo build/compilers/$mw_version/mwcceppc.exe $cflags -c $in -o build\nmw_version = Wii/1.0\nbuild build/A.o: mwcc src/A.cpp\n  cflags = -opt noloop\nbuild build/B.o: mwcc src/B.cpp\n  cflags = -opt nocse\n'
            path.write_text(graph);cache={}
            a=fnsearch.compiler_edge(root,'build/A.o',cache);b=fnsearch.compiler_edge(root,'build/B.o',cache)
            self.assertIn('noloop',a['argv']);self.assertNotIn('noloop',b['argv']);self.assertIn('nocse',b['argv'])
            path.write_text(graph.replace('nocse','dead'))
            changed=fnsearch.compiler_edge(root,'build/B.o',cache)
            self.assertIn('dead',changed['argv']);self.assertNotIn('nocse',changed['argv'])



class PrivateCompilePipeline(unittest.TestCase):
    def test_each_scored_unit_cleans_its_outputs_and_keeps_source_unchanged(self):
        import functools,sys,time
        from unittest.mock import patch
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'src/Game').mkdir(parents=True);(root/'private').mkdir()
            source=root/'src/Game/Test.cpp';source.write_text('int fixture() { return 1; }\n')
            tools=root/'build/tools';tools.mkdir(parents=True)
            wrapper=tools/'wibo';wrapper.write_text('#!/bin/sh\nexec "$@"\n');wrapper.chmod(0o755)
            for version in ('1.0','1.1'):
                compiler=root/f'build/compilers/Wii/{version}/mwcceppc.exe';compiler.parent.mkdir(parents=True)
                compiler.write_text('#!'+sys.executable+'\n'+'''import sys
from pathlib import Path
out=Path(sys.argv[sys.argv.index('-o')+1])
(out/'Test.o').write_text(Path(__file__).parent.name)
''');compiler.chmod(0o755)
            objdiff=tools/'objdiff-cli'
            objdiff.write_text('#!'+sys.executable+'\n'+'''import json,sys
from pathlib import Path
assert 'functionRelocDiffs=none' in sys.argv
u=json.loads((Path(sys.argv[sys.argv.index('-p')+1])/'objdiff.json').read_text())['units'][0]
new=Path(u['base_path']).read_text()=='1.1'
print(json.dumps({'units':[{'functions':[{'name':'f','size':'4','fuzzy_match_percent':100 if new else 99}],'sections':[{'name':'.data','size':'4','fuzzy_match_percent':100}]}]}))
''');objdiff.chmod(0o755)
            target=root/'target.o';target.write_text('target')
            edge={'argv':['build/tools/wibo','build/compilers/Wii/1.0/mwcceppc.exe','-MMD','-c','src/Game/Test.cpp','-o','build/shared'],'compiler_index':1,'source':'src/Game/Test.cpp','tails':[]}
            item={'edge':edge,'unit':{'name':'main/Game/Test'},'source_sha256':g.sha(source),'target':str(target)}
            manifest={'root':str(root),'out':str(root)}
            temporary=tempfile.TemporaryDirectory
            with patch.object(g.tempfile,'TemporaryDirectory',functools.partial(temporary,dir=root/'private')):
                base=g.compile_unit(manifest,item,'baseline',time.monotonic()+30)
                newer=g.compile_unit(manifest,item,'Wii/1.1',time.monotonic()+30)
            self.assertEqual((base['status'],newer['status']),('scored','scored'))
            self.assertTrue(g.compare(base['scores'],newer['scores'])['strict_gain'])
            self.assertEqual(list((root/'private').iterdir()),[])
            self.assertFalse((root/'build/shared').exists())
            self.assertEqual(g.sha(source),item['source_sha256'])

if __name__=='__main__':unittest.main()
