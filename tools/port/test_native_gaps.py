"""Synthetic source snippets only; no compiler or game artifacts."""
import json
from pathlib import Path
import tempfile
import unittest

import native_gaps as ng


class NativeGapTests(unittest.TestCase):
    def scan(self, body, path='src/Game/Util/MathUtil.cpp'):
        findings, errors = ng.scan_text(path, body)
        self.assertFalse(errors)
        return findings

    def categories(self, body):
        return {row['category'] for row in self.scan(body)}

    def test_a_nonasm_executable_mwerks_guard(self):
        text='void blend(Vec* out) {\n#ifdef __MWERKS__\n out->x = 1;\n#endif\n}'
        self.assertEqual(self.categories(text), {'a','d'})
        row=self.scan(text)[0]
        self.assertEqual((row['line'],row['function'],row['lane'],row['risk']),(2,'blend','core','high'))

    def test_a_if_spelling_and_return(self):
        for guard in ('#if __MWERKS__','#if defined(__MWERKS__)','#if defined __MWERKS__'):
            with self.subTest(guard=guard):
                self.assertIn('a',self.categories('int f() {\n'+guard+'\n return work();\n#endif\n}'))

    def test_a_declarations_are_inventory_candidates(self):
        self.assertIn('a',self.categories('void f() {\n#ifdef __MWERKS__\n int scratch;\n#endif\n}'))

    def test_b_native_exclusions(self):
        for guard in ('#ifndef GALAXY_NATIVE','#if !defined(GALAXY_NATIVE)','#if ! defined GALAXY_NATIVE'):
            with self.subTest(guard=guard):
                text='void f() { before();\n'+guard+'\n out->velocity += delta;\n#endif\n after(); }'
                self.assertEqual(self.categories(text),{'b'})

    def test_b_ignores_pure_declarations_and_directives(self):
        text='void f() {\n#ifndef GALAXY_NATIVE\n int scratch;\n const Vec* ptr;\n void helper();\n typedef int Number;\n using Value = int;\n#pragma optimize off\n#include <header.h>\n#endif\n after(); }'
        self.assertNotIn('b',self.categories(text))

    def test_b_call_and_early_return_are_not_declarations(self):
        for statement in ('return;','return 0;','doWork();','*out = value;','counter++;','if (ready) work();'):
            with self.subTest(statement=statement):
                self.assertIn('b',self.categories('void f() {\n#ifndef GALAXY_NATIVE\n'+statement+'\n#endif\n}'))

    def test_alternative_elif_and_else_are_not_a_or_b(self):
        for start in ('#ifdef __MWERKS__','#ifndef GALAXY_NATIVE'):
            for alternate in ('#else','#elif GALAXY_NATIVE'):
                self.assertFalse({'a','b','e'} & self.categories('void f() {\n'+start+'\n asm { ps_add f0,f1,f2 }\n'+alternate+'\n host();\n#endif\n}'))

    def test_c_empty_comment_and_zero_branches(self):
        for content in ('','// TODO: missing host implementation','return;','return 0;'):
            with self.subTest(content=content):
                findings=self.scan('int f() {\n#ifdef GALAXY_NATIVE\n'+content+'\n#else\n return calculate();\n#endif\n}')
                self.assertTrue(any(row['category']=='c' for row in findings))

    def test_c_nontrivial_native_branch_not_stub(self):
        self.assertNotIn('c',self.categories('int f() {\n#ifdef GALAXY_NATIVE\n return calculateNative();\n#else\n return calculateWii();\n#endif\n}'))

    def test_c_empty_wii_side_not_logic(self):
        self.assertNotIn('c',self.categories('void f() {\n#ifdef GALAXY_NATIVE\n return;\n#else\n // nothing\n#endif\n}'))

    def test_c_filescope_separate_definitions(self):
        text='#ifdef GALAXY_NATIVE\nint compute(int x) { return 0; }\n#else\nint compute(int x) { return x * x; }\n#endif'
        findings=self.scan(text)
        self.assertTrue(any(row['category']=='c' and row['function']=='compute' for row in findings))

    def test_d_known_arbitrary_guard(self):
        for guard in ('#if 0','#ifdef __PPC__','#if !GALAXY_NATIVE'):
            with self.subTest(guard=guard):
                self.assertIn('d',self.categories('void f() {\n'+guard+'\n calculate();\n#endif\n}'))

    def test_d_unknown_is_labeled_conditional(self):
        rows=self.scan('void f() {\n#ifdef FEATURE\n calculate();\n#endif\n}')
        d=[row for row in rows if row['category']=='d']
        self.assertEqual(len(d),1)
        self.assertEqual(d[0]['confidence'],'configuration-dependent')

    def test_d_unknown_complete_nonempty_branches_not_empty(self):
        self.assertNotIn('d',self.categories('void f() {\n#ifdef FEATURE\n first();\n#else\n second();\n#endif\n}'))

    def test_d_not_whole_function_when_native_tail_remains(self):
        self.assertNotIn('d',self.categories('void f() {\n#ifdef __MWERKS__\n calculate();\n#endif\n finish();\n}'))

    def test_e_asm_spellings_mid_function(self):
        for statement in ('asm volatile("nop");','__asm__("nop");','__asm volatile("nop");','asm { psq_l f0,0(r3),0,0 }'):
            with self.subTest(statement=statement):
                self.assertIn('e',self.categories('void f() { before(); '+statement+' after(); }'))

    def test_e_empty_alternative_does_not_protect_asm(self):
        self.assertIn('e',self.categories('void f() {\n#ifdef GALAXY_NATIVE\n // omitted\n#else\n asm { ps_add f0,f1,f2 }\n#endif\n}'))

    def test_e_real_alternative_protects_asm(self):
        self.assertNotIn('e',self.categories('void f() {\n#ifdef __MWERKS__\n asm { ps_add f0,f1,f2 }\n#else\n host();\n#endif\n}'))

    def test_e_asm_functions_are_not_inline_asm(self):
        self.assertNotIn('e',self.categories('asm void f() { ps_add f0, f1, f2 }'))

    def test_e_dead_on_both_profiles_not_flagged(self):
        self.assertNotIn('e',self.categories('void f() {\n#if 0\n asm volatile("nop");\n#endif\n}'))

    def test_f_markers_count_occurrences_not_lines(self):
        text='#ifdef GALAXY_NATIVE\n// TODO TODO FIXME PLACEHOLDER\nvoid f() { /* FIXME */ }\n#else\n// TODO not native\n#endif'
        rows=[r for r in self.scan(text) if r['category']=='f']
        self.assertEqual(len(rows),5)
        self.assertEqual(sum(r['detail']=='TODO' for r in rows),2)
        self.assertEqual(rows[-1]['function'],'f')

    def test_f_negative_guard_else_is_native(self):
        text='#ifndef GALAXY_NATIVE\n// TODO Wii\n#else\n// FIXME host\n#endif'
        self.assertEqual([r['detail'] for r in self.scan(text) if r['category']=='f'],['FIXME'])

    def test_comments_and_literals_do_not_create_code_or_markers(self):
        text='''// void fake() { asm("nop"); }
void real() {
 const char* text = R"tag(#ifdef GALAXY_NATIVE
 TODO asm { return 0; }
 #endif)tag";
 const char* other = "FIXME asm";
 // #ifdef __MWERKS__
 // call();
 // #endif
}
'''
        self.assertEqual(self.categories(text),set())

    def test_nested_guards_and_namespace_function_names(self):
        text='namespace MR { struct Matrix {\n void normalize() const {\n#ifdef __MWERKS__\n#if 1\n call();\n#endif\n#endif\n }\n}; }'
        rows=self.scan(text)
        self.assertTrue(rows)
        self.assertTrue(all(r['function']=='MR::Matrix::normalize' for r in rows))

    def test_multiline_signature_and_local_define(self):
        text='namespace MR {\n#define LOCAL 0\ninline void\nnormalize(Vec* out) {\n#if LOCAL\n out->x = 1;\n#endif\n}\n}'
        rows=self.scan(text)
        self.assertTrue(any(r['category']=='d' and r['function']=='MR::normalize' for r in rows))

    def test_continued_if(self):
        text='void f() {\n#if defined(__MWERKS__) && \\\n 1\n call();\n#endif\n}'
        self.assertIn('d',self.categories(text))

    def test_unmatched_conditionals_are_reported(self):
        _,errors=ng.scan_text('src/file.cpp','void f() {\n#ifdef GALAXY_NATIVE\n return;\n}')
        self.assertTrue(errors)

    def test_scope_and_branch_function_names(self):
        text='''namespace MR {
int sqrt(float x) {
#ifdef GALAXY_NATIVE
 return 0;
#else
 return calculate(x);
#endif
}
}
'''
        rows=self.scan(text)
        self.assertTrue(any(r['category']=='c' and r['function']=='MR::sqrt' for r in rows))

    def test_statement_classifier(self):
        self.assertFalse(ng.statements('int scratch; const Vec* ptr; void helper();'))
        self.assertTrue(ng.statements('return calculate();'))
        self.assertTrue(ng.statements('out->x = 0;'))

    def test_expression_unknown_and_short_circuit(self):
        env=ng.profile(True)
        self.assertEqual(ng.expression('GALAXY_NATIVE && !defined(__MWERKS__)',env),True)
        self.assertIsNone(ng.expression('FEATURE',env))
        self.assertEqual(ng.expression('0 && FEATURE',env),False)
        self.assertEqual(ng.expression('1 || FEATURE',env),True)

    def test_whole_wii_definition_with_host_alternative_is_not_empty_native(self):
        text='#ifdef __MWERKS__\nasm void normalize() {\n#ifdef __MWERKS__\n ps_add f0,f1,f2\n#endif\n}\n#else\nvoid normalize() { host(); }\n#endif'
        self.assertFalse({'a','c','d'} & self.categories(text))

    def test_nested_inactive_native_split_not_false_stub(self):
        text='void setTrans() {\n#ifdef __MWERKS__\n#ifdef GALAXY_NATIVE\n native();\n#else\n wii();\n#endif\n#else\n native();\n#endif\n}'
        self.assertNotIn('c',self.categories(text))

    def test_nested_wii_only_block_under_real_host_alternative(self):
        text='int cast(float x) {\n#ifdef GALAXY_NATIVE\n return host(x);\n#else\n#ifdef __MWERKS__\n asm { psq_st f0,0(r3),1,2 }\n#endif\n return out;\n#endif\n}'
        self.assertFalse({'a','c','d','e'} & self.categories(text))

    def test_template_class_scope(self):
        text='namespace JGeometry { template<class T> struct Vec { void normalize() {\n#ifdef __MWERKS__\n call();\n#endif\n} }; }'
        rows=self.scan(text)
        self.assertTrue(rows)
        self.assertTrue(all(row['function']=='JGeometry::Vec::normalize' for row in rows))

    def test_native_feature_markers_count_even_when_unknown(self):
        rows=self.scan('#if GALAXY_NATIVE && FEATURE\n// TODO\n#endif')
        self.assertEqual([r['detail'] for r in rows],['TODO'])

    def test_combined_native_exclusion(self):
        rows=self.scan('void f() {\n#if !defined(GALAXY_NATIVE) && FEATURE\n call();\n#endif\n}')
        self.assertIn('b',{r['category'] for r in rows})

    def test_scan_and_markdown_end_to_end(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory)
            path=root/'src/Game/Player/sample.cpp'
            path.parent.mkdir(parents=True)
            path.write_text('void physics() {\n#ifdef __MWERKS__\n integrate();\n#endif\n}')
            (root/'include').mkdir()
            (root/'libs').mkdir()
            output=root/'report.md'
            detail=root/'report.json'
            self.assertEqual(ng.main(['--root',str(root),'--output',str(output),'--json',str(detail)]),0)
            report=json.loads(detail.read_text())
            self.assertEqual(report['files_scanned'],1)
            self.assertEqual(report['counts']['a'],1)
            text=output.read_text()
            self.assertIn('src/Game/Player/sample.cpp:2',text)
            self.assertIn('astra',text)
            self.assertIn('physics',text)

    def test_extensionless_standard_headers_are_scanned(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            header = root / 'libs/MSL_C/include/cmath'
            header.parent.mkdir(parents=True)
            header.write_text('inline void f() {\n#ifdef __MWERKS__\n work();\n#endif\n}')
            report = ng.scan(root)
            self.assertEqual(report['files_scanned'], 1)
            self.assertEqual(report['counts']['a'], 1)


if __name__=='__main__':
    unittest.main()
