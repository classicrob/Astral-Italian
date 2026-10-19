"""Adversarial safety/scoring tests; no game build required."""
import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from flagsearch import (parse_edge, rewrite_edge, unsupported_diagnostics, validate_range,
                        place_pragma, score_rows, merge_full_report, compare_scores,
                        materialize_snapshot, parse_dependencies, sha)


class FlagSearchTests(unittest.TestCase):
    def test_argv_rewrite_spaces_and_literals(self):
        with tempfile.TemporaryDirectory(prefix='flag search ') as td:
            root=Path(td).resolve(); (root/'src').mkdir(); (root/'include').mkdir()
            command='build/tools/wibo build/tools/sjiswrap.exe build/compilers/Wii/1.0/mwcceppc.exe -pragma "cats off" -i include -MMD -c "src/file name.cpp" -o "build/a b" && python3 tools/transform_dep.py "build/a b/file name.d" "build/a b/file name.d"'
            e=parse_edge(command); snap=root/'scratch snapshot'; source=root/'variant source.cpp'; dest=root/'variant output'
            a=rewrite_edge(e,root,snap,source,dest,['-pragma','opt_propagation off'])
            self.assertIn('cats off',a);self.assertIn('opt_propagation off',a)
            self.assertEqual(a[a.index('-c')+1],str(source));self.assertEqual(a[a.index('-o')+1],str(dest))
            self.assertNotIn('build/a b',a); self.assertIn(str(snap/'include'),a)
            self.assertNotIn('&&',a)
    def test_shell_commands_fail_closed(self):
        base='build/tools/wibo build/compilers/Wii/1.0/mwcceppc.exe -MMD -c src/a.cpp -o out'
        for suffix in ('; touch X',' | cat',' && rm -rf X',' $(touch X)',' `touch X`'):
            with self.subTest(suffix=suffix),self.assertRaises(ValueError):parse_edge(base+suffix)
    def test_unsupported_and_ignored_diagnostics(self):
        for text in ('Warning : illegal #pragma','Warning: pragma option ignored','Error: Unknown option -abc','unsupported pragma x','Command line warning: foo'):
            with self.subTest(text=text):self.assertTrue(unsupported_diagnostics(text))
        self.assertFalse(unsupported_diagnostics('Warning : variable x is unused'))
    def test_ranges_complete_functions_inside_namespace(self):
        t='#include "a.hpp"\nnamespace MR {\nint f() { return 1; }\n\nint g() { const char* s="}"; return f(); }\n}\n'
        self.assertEqual(validate_range(t,3,5)[2],2)
        z=place_pragma(t,'opt_propagation off','range',(3,5))
        self.assertIn('namespace MR {\n#pragma push\n',z);self.assertIn('#pragma pop\n#line 6\n}',z)
        for start,end in ((2,5),(3,6),(3,4)):
            # 3:4 IS a complete single-function span including blank line.
            if (start,end)==(3,4):continue
            with self.assertRaises(ValueError):validate_range(t,start,end)
    def test_ranges_reject_partial_function_expression_and_conditional(self):
        t='int f() {\n return 1;\n}\n'
        for bounds in ((1,2),(2,2),(2,3)):
            with self.assertRaises(ValueError):validate_range(t,*bounds)
        with self.assertRaises(ValueError):validate_range('#if A\nint f() {return 1;}\n#endif\n',2,2)
        with self.assertRaises(ValueError):validate_range('int x;\nint f() {return x;}\n',1,2)
    def test_actual_mwcc_mmd_source_only_and_wrapped_headers(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td).resolve();(root/'src').mkdir();source=root/'src/a.cpp';source.write_text('source')
            header=root/'header with space.hpp';header.write_text('header')
            d=root/'a.d';d.write_text('object\\a.o: src\\a.cpp\n')
            self.assertEqual(parse_dependencies(d,root),{str(source):sha(source)})
            d.write_text('object\\a.o: src\\a.cpp \\\n\tZ:'+str(header).replace('/','\\')+'\n')
            self.assertEqual(parse_dependencies(d,root),{str(source):sha(source),str(header):sha(header)})
    def test_snapshot_never_writes_source_and_detects_tamper(self):
        with tempfile.TemporaryDirectory() as td:
            root=Path(td).resolve()/'repo';root.mkdir();(root/'a.cpp').write_text('unchanged')
            original=(root/'a.cpp').read_bytes();pins={'a.cpp':sha(root/'a.cpp')}
            snap=materialize_snapshot(root,Path(td)/'out',pins,'abc')
            self.assertEqual((root/'a.cpp').read_bytes(),original);self.assertNotEqual((root/'a.cpp').stat().st_ino,(snap/'a.cpp').stat().st_ino)
            self.assertEqual((snap/'a.cpp').stat().st_mode&0o222,0)
            (snap/'a.cpp').chmod(0o644);(snap/'a.cpp').write_text('tampered')
            with self.assertRaises(ValueError):materialize_snapshot(root,Path(td)/'out',pins,'abc')
            self.assertEqual((root/'a.cpp').read_bytes(),original)
    def test_duplicate_local_names_keep_target_address_and_occurrence(self):
        symbols=[{'name':'local','offset':0,'size':4,'section':'.text'}, {'name':'local','offset':4,'size':4,'section':'.text'}, {'name':'local','offset':4,'size':4,'section':'.text'}]
        left=[{'name':'local','size':'4','address':str(i),'match_percent':100,'target_symbol':j+1,'instructions':[{}]}for j,i in enumerate((0,4,4))]
        rows=score_rows({'left':{'symbols':left},'right':{'symbols':[{'name':'local','size':'4'}]*3}},symbols,{'.text':0x80001000})
        self.assertEqual(list(rows),['80001000:0:local','80001004:0:local','80001004:1:local'])
    def row(self,p,exact=None):
        return {'target_bytes':100,'fuzzy_percent':p,'exact':p==100 if exact is None else exact,'diff_percent':p,'diff_exact':p==100}
    def test_full_report_anchor_cannot_be_lost_when_sg_anchor_stays(self):
        diag={'k':{'key':'k','name':'f','target_offset':0,'target_bytes':100,'target_address':'0x80000000','source_bytes':100,'fuzzy_percent':99.7,'exact':False}}
        def full(p):return {'units':[{'name':'main/Game/Test','measures':{'matched_code':'100'if p==100 else'0'},'functions':[{'name':'f','size':'100','fuzzy_match_percent':p}]}]}
        base=merge_full_report(full(100),diag,'Game/Test'); candidate=merge_full_report(full(99),diag,'Game/Test')
        c=compare_scores(base,candidate);self.assertEqual(c['exact_anchor_losses'],['k']);self.assertFalse(c['qualifies_exact_anchor_guard'])
        self.assertFalse(c['diffJSON_exact_losses'])
    def test_fuzzy_drop_visible_but_default_anchor_view_qualifies(self):
        c=compare_scores({'a':self.row(100),'b':self.row(90)},{'a':self.row(100),'b':self.row(80)})
        self.assertTrue(c['qualifies_exact_anchor_guard']);self.assertFalse(c['qualifies_all_fuzzy_guard']);self.assertEqual(c['fuzzy_regressions'],['b'])
        c=compare_scores({'a':self.row(100)},{'a':self.row(99)})
        self.assertFalse(c['qualifies_exact_anchor_guard']);self.assertEqual(c['exact_target_bytes_gained'],-100)
    def test_inventory_and_missing_function_reported(self):
        c=compare_scores({'a':self.row(100)},{},{'helper:0':16},{})
        self.assertEqual(c['source_inventory_removed'],['helper:0']);self.assertEqual(c['target_inventory_missing'],['a']);self.assertFalse(c['qualifies_exact_anchor_guard'])

if __name__=='__main__':unittest.main()
