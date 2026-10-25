#!/usr/bin/env python3
"""Safety and extraction checks for the source-variant searcher."""
import tempfile
from pathlib import Path
import unittest

import fnsearch as f
import permute


class VariantSafety(unittest.TestCase):
    def candidates(self, source, name='subject', vector=False):
        span = permute.find_function(source, name)
        return f.variants(source, span, 250, vector_ok=vector)

    def test_no_float_commutation(self):
        source = 'float subject(float a, float b) { return a * b; }\n'
        self.assertFalse(self.candidates(source))

    def test_integer_precedence_is_preserved(self):
        source = 'int subject(int a, int b, int c) { return a + b * c; }\n'
        self.assertFalse(self.candidates(source))
        source = 'int subject(int a, int b, int c) { return (a + b) * c; }\n'
        candidates = self.candidates(source)
        self.assertTrue(any('(b + a) * c' in text for _, text in candidates))

    def test_integer_constant_first_does_not_include_float_comparisons(self):
        source = 'bool subject(int a) { return (a == 0); }\n'
        self.assertTrue(any('(0 == a)' in text for _, text in self.candidates(source)))
        source = 'bool subject(float a) { return (a == 0); }\n'
        self.assertFalse(self.candidates(source))

    def test_large_declaration_groups_vary_every_position(self):
        orders = f.declaration_orders(7)
        self.assertEqual(len(orders), 120)
        self.assertEqual({order[0] for order in orders}, set(range(7)))
        self.assertTrue(all(sorted(order) == list(range(7)) for order in orders))

    def test_comma_declarations_can_reorder_without_initializers(self):
        source = 'float subject(float a) {\n float x, y;\n x = a; y = a;\n return x + y;\n}\n'
        self.assertTrue(any('float y, x;' in text for _, text in self.candidates(source)))

    def test_source_extraction_allows_macro_continuations_outside_body(self):
        source = '#define DECLARE_THING(x) \\\n int x;\nint subject(int a) { return a; }\n'
        span = f.source_span(source, dict(name='subject', demangled_name='subject(int)'))
        self.assertEqual(source[span.start:span.end], 'int subject(int a) { return a; }')

    def test_macro_arguments_cannot_be_commuted(self):
        for expression in ('STRINGIZE(a + b)', 'STRINGIZE((a + b))'):
            source = 'const char* subject(int a, int b) { return '+expression+'; }\n'
            self.assertFalse(any('commutative_integer' in label for label, _ in self.candidates(source)))

    def test_statement_looking_macro_arguments_are_not_edited(self):
        source = 'void subject(bool ready) { STRINGIZE(noop(); if (ready) { noop(); }); }\n'
        self.assertFalse(self.candidates(source))

    def test_unknown_type_shadow_cannot_be_treated_as_integer(self):
        for declaration in ('Widget a;', 'Widget *a;', 'Widget x, *a;'):
            source = 'int subject(int a, int b) {\n { '+declaration+' return a + b; }\n}\n'
            self.assertFalse(any('commutative_integer' in label for label, _ in self.candidates(source)))

    def test_comments_cannot_supply_fake_integer_types(self):
        source = 'float global;\nfloat subject() {\n // int global;\n return global + 1;\n}\n'
        self.assertFalse(any('commutative_integer' in label for label, _ in self.candidates(source)))

    def test_local_typedef_cannot_reclassify_a_builtin_alias(self):
        source = 'int subject() { typedef Widget s32; s32 a, b; return a + b; }\n'
        with self.assertRaisesRegex(ValueError, 'local type declarations'):
            self.candidates(source)

    def test_qualified_alias_is_not_assumed_to_be_the_global_integer_type(self):
        source = 'int subject(Fake::s32 a, Fake::s32 b) { return a + b; }\n'
        self.assertFalse(any('commutative_integer' in label for label, _ in self.candidates(source)))

    def test_identifier_prefix_is_not_a_type_declaration(self):
        source = 'float eger; void integer();\nfloat subject() { integer(); return eger + 1; }\n'
        self.assertFalse(any('commutative_integer' in label for label, _ in self.candidates(source)))

    def test_string_literals_cannot_supply_fake_integer_types(self):
        source = 'float global;\nfloat subject() {\n const char* text = "int global";\n consume(text);\n return global + 1;\n}\n'
        self.assertFalse(any('commutative_integer' in label for label, _ in self.candidates(source)))

    def test_hoisting_cannot_change_implicit_macro_name_lookup(self):
        source = '#define USE_GLOBAL() consume(x)\nint x;\nint subject() {\n USE_GLOBAL();\n int x = 3;\n return x;\n}\n'
        self.assertFalse(any('hoist_local_declaration' in label for label, _ in self.candidates(source)))

    def test_macro_initializer_cannot_see_a_hoisted_later_name(self):
        source = '#define READ_B() get(b)\nint b;\nint subject() {\n int a = READ_B();\n int b = 2;\n return a + b;\n}\n'
        self.assertFalse(any('declaration_order_preserve_initializers' in label for label, _ in self.candidates(source)))

    def test_staged_bool_avoids_names_from_included_headers(self):
        source = 'void subject(bool ready) { if (ready) { action(); } }\n'
        span = permute.find_function(source, 'subject')
        candidates = f.variants(source, span, 20, reserved_names={'fnsearchCondition'})
        self.assertTrue(any('bool fnsearchCondition_' in text for _, text in candidates))

    def test_calls_keep_evaluation_order_when_declarations_move(self):
        source = 'int first(); int second();\nint subject() {\n int a = first();\n int b = second();\n return a - b;\n}\n'
        candidates = self.candidates(source)
        self.assertTrue(any('declaration_order_preserve_initializers' in label for label, _ in candidates))
        for _, text in candidates:
            body = text[text.index('int subject'):]
            self.assertEqual(body.count('first()'), 1)
            self.assertEqual(body.count('second()'), 1)
            self.assertLess(body.index('first()'), body.index('second()'))

    def test_later_name_cannot_shadow_an_initializer(self):
        source = 'int b;\nint subject() {\n int a = b;\n int b = 2;\n return a - b;\n}\n'
        for label, _ in self.candidates(source):
            self.assertNotIn('declaration_order_preserve_initializers', label)
            self.assertNotIn('local_declaration_order', label)

    def test_hoist_does_not_shadow_prior_global_use(self):
        source = 'int x; void consume(int);\nint subject() {\n consume(x);\n int x = 3;\n return x;\n}\n'
        self.assertFalse(any('hoist_local_declaration' in label for label, _ in self.candidates(source)))

    def test_const_pointer_requires_arrow_only_uses(self):
        safe = 'int subject(Node* ptr) { return ptr->value; }\n'
        self.assertTrue(any('Node* const ptr' in text for _, text in self.candidates(safe)))
        unsafe = 'int use(Node*&); int subject(Node* ptr) { return use(ptr); }\n'
        self.assertFalse(any('const_pointer' in label for label, _ in self.candidates(unsafe)))

    def test_unknown_and_float_temporary_are_not_inlined(self):
        sources = [
            'float subject(double a) {\n float x = a;\n return x;\n}\n',
            'Widget subject(Widget a) {\n Widget x = a;\n return x;\n}\n',
        ]
        for source in sources:
            self.assertFalse(any('inline_integral' in label for label, _ in self.candidates(source)))

    def test_integral_copy_temporary_can_be_inlined(self):
        source = 'int subject(int a) {\n int x = a;\n return x;\n}\n'
        self.assertTrue(any('inline_integral_copy_temporary' in label for label, _ in self.candidates(source)))

    def test_unknown_class_constructors_cannot_move(self):
        source = 'void subject() {\n Widget a;\n Widget b;\n consume(a, b);\n}\n'
        self.assertFalse(self.candidates(source))

    def test_vector_transforms_need_repository_type_check(self):
        source = 'void subject(const TVec3f& a) {\n TVec3f b(a);\n consume(b);\n}\n'
        self.assertFalse(self.candidates(source, vector=False))
        self.assertTrue(any('vector_copy_initialization' in label for label, _ in self.candidates(source, vector=True)))

    def test_only_selected_definition_and_line_positions_change(self):
        source = 'int before() { return 1; }\nint subject(bool ready, int a, int b) {\n int x = a;\n\n int y = b;\n if (ready) { return x + y; }\n return x - y;\n}\nint after() { return 2; }\n'
        span = permute.find_function(source, 'subject')
        variants = self.candidates(source)
        self.assertGreater(len(variants), 3)
        for _, text in variants:
            self.assertTrue(text.startswith(source[:span.start]))
            self.assertTrue(text.endswith(source[span.end:]))
            self.assertEqual(source.count('\n'), text.count('\n'))
            self.assertNotIn('#pragma', text)
            self.assertNotIn('volatile', text)

    def test_initializer_macro_keeps_its_line(self):
        source = '#define HERE() get(__LINE__)\nint subject() {\n int a = HERE();\n\n int b = 2;\n return a + b;\n}\n'
        # Location-dependent sources fail closed, even outside the function.
        with self.assertRaisesRegex(ValueError, 'location-dependent'):
            self.candidates(source)

    def test_unknown_macro_calls_keep_their_original_line(self):
        source = 'int subject() {\n int a = TRACE();\n\n int b = 2;\n return a + b;\n}\n'
        original_line = next(i for i, line in enumerate(source.splitlines()) if 'TRACE()' in line)
        for _, text in self.candidates(source):
            self.assertEqual(original_line, next(i for i, line in enumerate(text.splitlines()) if 'TRACE()' in line))

    def test_preprocessor_and_volatile_rejected(self):
        for source in ['int subject() {\n#ifdef FLAG\n return 1;\n#endif\n return 0;\n}\n',
                       'int subject() { volatile int x = 1; return x; }\n']:
            with self.assertRaises(ValueError): self.candidates(source)

    def test_every_other_function_must_not_drop(self):
        old = {'selected': 99, 'exact': 100, 'fuzzy': 63.5, 'removed': 50}
        new = {'selected': 100, 'exact': 100, 'fuzzy': 63.49}
        self.assertEqual(f.drops(old, new, 'selected'), {'fuzzy': [63.5, 63.49], 'removed': [50, 0]})


class CompileEdge(unittest.TestCase):
    def test_scoped_flags_and_quoted_pragma_are_preserved(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root/'build.ninja').write_text('mw_version = GC/3.0\npython = python3\nrule mwcc\n  command = build/tools/wibo build/compilers/$mw_version/mwcceppc.exe $cflags -MMD -c $in -o $basedir && $python tools/transform_dep.py $basefile.d $basefile.d\nbuild build/Test.o: mwcc src/Test.cpp | dependency\n  mw_version = Wii/1.0\n  cflags = -O4,s -pragma "cats off" $\n    -i include -DVERSION=0\n  basedir = build\n  basefile = build/Test\n')
            edge = f.compiler_edge(root, 'build/Test.o')
            self.assertEqual(edge['argv'], ['build/tools/wibo', 'build/compilers/Wii/1.0/mwcceppc.exe', '-O4,s', '-pragma', 'cats off', '-i', 'include', '-DVERSION=0', '-MMD', '-c', 'src/Test.cpp', '-o', 'build'])

    def test_unknown_postprocessor_is_refused(self):
        with self.assertRaises(ValueError):
            f.flagsearch.parse_edge('build/tools/wibo build/compilers/Wii/1.0/mwcceppc.exe -c src/Test.cpp -o build && touch shared-target')

    def test_overlay_preserves_relative_includes_without_source_writes(self):
        with tempfile.TemporaryDirectory() as temp:
            top = Path(temp); root = top/'repo'; root.mkdir()
            (root/'src/A').mkdir(parents=True); (root/'src/B').mkdir()
            (root/'src/A/Test.cpp').write_text('original')
            (root/'src/A/local.inc').write_text('local')
            (root/'src/B/sibling.inc').write_text('sibling')
            output = f.overlay(root, top/'overlay', Path('src/A/Test.cpp'), 'candidate')
            self.assertEqual(output.read_text(), 'candidate')
            self.assertEqual((root/'src/A/Test.cpp').read_text(), 'original')
            self.assertEqual((output.parent/'local.inc').read_text(), 'local')
            self.assertEqual((output.parent/'../B/sibling.inc').read_text(), 'sibling')


class ScratchPipeline(unittest.TestCase):
    def fake_checkout(self, root, guard=''):
        import subprocess, sys
        (root/'src').mkdir()
        (root/'build/tools').mkdir(parents=True)
        (root/'build/compilers/Wii/1.0').mkdir(parents=True)
        subprocess.run(['git', 'init', '-q', str(root)], check=True)
        subprocess.run(['git', '-C', str(root), '-c', 'core.hooksPath=/dev/null', '-c', 'user.name=Fixture', '-c', 'user.email=fixture@example.test', 'commit', '-q', '--allow-empty', '-m', 'fixture'], check=True)
        source = 'int subject(int a, int b) { return a + b; }\nint guard() { return 1; }\n'+guard
        (root/'src/Test.cpp').write_text(source)
        (root/'target.o').write_text('fake target')
        (root/'objdiff.json').write_text(__import__('json').dumps({'units':[{'name':'main/Test','base_path':'build/Test.o','target_path':'target.o'}]}))
        (root/'build.ninja').write_text('rule mwcc\n  command = build/tools/wibo build/compilers/Wii/1.0/mwcceppc.exe -MMD -c $in -o $basedir\nbuild build/Test.o: mwcc src/Test.cpp\n  basedir = build\n')
        wrapper = root/'build/tools/wibo'
        wrapper.write_text('#!/bin/sh\nexec "$@"\n'); wrapper.chmod(0o755)
        compiler = root/'build/compilers/Wii/1.0/mwcceppc.exe'
        compiler.write_text('#!'+sys.executable+'\n'+'''import sys
from pathlib import Path
args=sys.argv
source=Path(args[args.index('-c')+1]); out=Path(args[args.index('-o')+1])
(out/(source.stem+'.o')).write_text(source.read_text())
(out/(source.stem+'.d')).write_text('object: '+str(source)+'\\n')
'''); compiler.chmod(0o755)
        objdiff = root/'build/tools/objdiff-cli'
        objdiff.write_text('#!'+sys.executable+'\n'+'''import sys,json
from pathlib import Path
args=sys.argv
assert 'functionRelocDiffs=none' in args
cfg=json.loads((Path(args[args.index('-p')+1])/'objdiff.json').read_text())
source=Path(cfg['units'][0]['base_path']).read_text()
improved='return b + a;' in source
score=100 if improved else 99
regressed=improved and '// diff-regression' in source
report_regressed=improved and '// report-regression' in source
if 'diff' in args:
 symbols=[dict(name=n, demangled_name=n+'()',size='4',match_percent=s,instructions=[{'instruction':{'formatted':'blr'}}]) for n,s in [('subject',score),('guard',99 if regressed else 100)]]
 print(json.dumps({'left':{'symbols':symbols},'right':{'symbols':symbols}}))
else:
 print(json.dumps({'units':[{'name':'main/Test','functions':[{'name':'subject','fuzzy_match_percent':score},{'name':'guard','fuzzy_match_percent':99 if report_regressed else 100}]}]}))
'''); objdiff.chmod(0o755)
        return source

    def launch(self, root, out, write=None):
        import os, subprocess, sys
        if f.shutil.disk_usage(tempfile.gettempdir()).free < 50_000_000_000:
            self.skipTest('search requires 50 GB free')
        args=[sys.executable,str(Path(f.__file__).resolve()),'Test','subject','--root',str(root),'--out',str(out),'--budget','3','--workers','1']
        if write: args += ['--write', str(write)]
        env={**os.environ,'FNSEARCH_HEAVY_LOCKED':str(Path.home()/'MacioGalaxy2-team/slots/server-heavy.lock')}
        return subprocess.run(args,capture_output=True,text=True,env=env)

    def test_default_emits_patch_without_writing_checkout(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)/'repo'; root.mkdir(); original=self.fake_checkout(root)
            out=Path(temp)/'result'; result=self.launch(root,out)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual((root/'src/Test.cpp').read_text(),original)
            self.assertIn('+int subject(int a, int b) { return b + a; }',(out/'best.patch').read_text())
            self.assertFalse((root/'build/Test.o').exists())

    def test_diff_regression_rejects_better_selected_score(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)/'repo'; root.mkdir(); original=self.fake_checkout(root,'// diff-regression\n')
            out=Path(temp)/'result'; result=self.launch(root,out)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual((out/'best.patch').read_text(),'')
            self.assertIn('other_function_regressed',(out/'scores.tsv').read_text())
            self.assertEqual((root/'src/Test.cpp').read_text(),original)

    def test_full_report_regression_also_rejects_survivor(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)/'repo'; root.mkdir(); self.fake_checkout(root,'// report-regression\n')
            out=Path(temp)/'result'; result=self.launch(root,out)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertEqual((out/'best.patch').read_text(),'')
            self.assertIn('report_regressed',(out/'scores.tsv').read_text())

    def test_write_requires_explicit_tree(self):
        with tempfile.TemporaryDirectory() as temp:
            root=Path(temp)/'repo'; root.mkdir(); self.fake_checkout(root)
            out=Path(temp)/'result'; result=self.launch(root,out,write=root)
            self.assertEqual(result.returncode,0,result.stderr)
            self.assertIn('return b + a;',(root/'src/Test.cpp').read_text())


if __name__ == '__main__': unittest.main()
