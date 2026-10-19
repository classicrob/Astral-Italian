import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import semdomain as d


class LexicalTests(unittest.TestCase):
    def test_plain_memory(self):
        self.assertFalse(d.volatile_evidence('struct T { int x; };'))

    def test_comments_and_strings(self):
        self.assertFalse(d.volatile_evidence('// volatile int x;\nconst char* s="volatile"; /* volatile */'))

    def test_mutable_member(self):
        self.assertTrue(d.volatile_evidence('struct T { volatile bool done; };'))

    def test_global_extern(self):
        self.assertTrue(d.volatile_evidence('extern volatile int flag;'))

    def test_matching_constant(self):
        self.assertFalse(d.volatile_evidence('static const volatile float speed = 1.0f;'))
        self.assertFalse(d.volatile_evidence('static volatile const float speed = 1.0f;'))

    def test_const_pointer_is_not_proof(self):
        self.assertTrue(d.volatile_evidence('const volatile int* p;'))
        self.assertTrue(d.volatile_evidence('int* const volatile p;'))

    def test_qualified_alias_not_purity(self):
        self.assertFalse(d.volatile_evidence('typedef volatile int vu32;'))
        self.assertTrue(d.volatile_evidence('typedef volatile int Unknown;'))
        self.assertTrue(d.volatile_evidence('using V = volatile int;'))

    def test_macros_and_asm_fail_closed(self):
        self.assertTrue(d.volatile_evidence('#define V const volatile\nV x;'))
        self.assertFalse(d.volatile_evidence('asm volatile("sync");'))

    def test_global_is_symbol_level(self):
        r=d.volatile_evidence('extern volatile int flag;')[0]
        self.assertEqual((r['level'],r['name'],r['pointer']),('symbol','flag',False))

    def test_pointer_global_needs_loaded_base_taint(self):
        r=d.volatile_evidence('extern volatile void* __piReg;')[0]
        self.assertEqual((r['level'],r['name'],r['pointer']),('symbol','__piReg',True))

    def test_member_is_unit_level(self):
        r=d.volatile_evidence('class T { volatile int flag; };')[0]
        self.assertEqual((r['level'],r['name']),('unit','flag'))

    def test_volatile_alias_member_is_not_ignored(self):
        r=d.volatile_evidence('struct NameObj { vu16 mFlags; };')[0]
        self.assertEqual((r['level'],r['name']),('unit','mFlags'))

    def test_fixed_address_is_kernel_device_obligation(self):
        self.assertFalse(d.volatile_evidence('volatile PPCWGPipe GXWGFifo : 0xCC008000;'))

    def test_static_member_is_global(self):
        r=d.volatile_evidence('class T { static volatile int count; };')[0]
        self.assertEqual((r['level'],r['name']),('symbol','count'))

    def test_namespace_global(self):
        r=d.volatile_evidence('namespace T { extern volatile int count; }')[0]
        self.assertEqual((r['level'],r['name']),('symbol','count'))

    def test_function_pointer_unknown_not_symbol_guess(self):
        r=d.volatile_evidence('void (*volatile cb)(void);')[0]
        self.assertEqual(r['level'],'unit')

    def test_local_volatile_unknown_is_strict(self):
        r=d.volatile_evidence('void f() { volatile int count; }')[0]
        self.assertEqual(r['level'],'unit')

    def test_multiple_declarators_unknown(self):
        r=d.volatile_evidence('extern volatile int a, b;')[0]
        self.assertEqual(r['level'],'unit')

    def test_volatile_parameter_not_object_declaration(self):
        self.assertTrue(d.volatile_evidence('static inline void f(float* in, volatile register u16* out) { *out = cvt(*in); }'))

    def test_volatile_return_type_not_object_declaration(self):
        self.assertTrue(d.volatile_evidence('volatile int* f(void);'))

    def test_method_volatile_not_parameter(self):
        self.assertTrue(d.volatile_evidence('struct T { int f() volatile; };'))

    def test_D1_constructor_initialized_volatile_object(self):
        self.assertTrue(d.volatile_evidence('volatile Counter sCounter(1);'))

    def test_D2_initializer_call_does_not_hide_object(self):
        r=d.volatile_evidence('static volatile u32 sTick = OSGetTick();')
        self.assertEqual((r[0]['level'],r[0]['name']),('symbol','sTick'))

    def test_D3_volatile_cast_in_call_is_unknown_strict(self):
        r=d.volatile_evidence('g((volatile u32*)p);')
        self.assertTrue(r)
        self.assertEqual(r[0]['level'],'unit')

    def test_D4_deref_cast_in_argument_is_unknown_strict(self):
        r=d.volatile_evidence('x = h(1, *(volatile s32*)&sY);')
        self.assertTrue(r)
        self.assertEqual(r[0]['level'],'unit')

    def test_pointer_to_volatile_getter_is_unknown_strict(self):
        r=d.volatile_evidence('volatile u32* getReg();')
        self.assertTrue(r)
        self.assertEqual(r[0]['level'],'unit')

    def test_pointer_to_volatile_parameter_is_unknown_strict(self):
        r=d.volatile_evidence('void poll(volatile u32* r) { while (*r) pollAgain(); }')
        self.assertTrue(r)
        self.assertEqual(r[0]['level'],'unit')

    def test_initializer_second_volatile_not_hidden_by_declaration(self):
        r=d.volatile_evidence('static volatile u32 x = *(volatile u32*)p;')
        self.assertTrue(any(v['level']=='unit' for v in r))

    def test_const_decl_does_not_hide_initializer_volatile_cast(self):
        r=d.volatile_evidence('const u32 x = *(volatile u32*)p;')
        self.assertTrue(any(v['level']=='unit' for v in r))

    def test_alternate_keyword(self):
        self.assertTrue(d.volatile_evidence('__volatile__ int x;'))

    def test_no_initializer_const_leak(self):
        self.assertTrue(d.volatile_evidence('volatile int x = const_value;'))

    def test_prior_const_not_current(self):
        self.assertTrue(d.volatile_evidence('const int prior=1; volatile int x;'))

    def test_line_evidence(self):
        self.assertEqual(d.volatile_evidence('/*\n*/\nvolatile int x;')[0]['line'], 3)


class DependencyTests(unittest.TestCase):
    def test_valid(self):
        self.assertEqual(d.parse_deps('obj.o: #deps 2, deps mtime 123 (VALID)\n    a.cpp\n    a.hpp\n\n', 'obj.o'), ['a.cpp', 'a.hpp'])

    def test_stale(self):
        with self.assertRaises(ValueError):
            d.parse_deps('obj.o: #deps 1, deps mtime 123 (STALE)\n    a.cpp\n', 'obj.o')

    def test_truncated(self):
        with self.assertRaises(ValueError):
            d.parse_deps('obj.o: #deps 2, deps mtime 123 (VALID)\n    a.cpp\n', 'obj.o')

    def test_duplicate_extra_report(self):
        with self.assertRaises(ValueError):
            d.parse_deps('obj.o: #deps 1, deps mtime 123 (VALID)\n    a.cpp\nx.o: no deps\n', 'obj.o')

    def fixture(self, root, source='int f() {return 1;}', header='int other;'):
        (root/'src').mkdir()
        (root/'src/x.cpp').write_text(source)
        (root/'x.hpp').write_text(header)
        (root/'.ninja_deps').write_bytes(b'evidence')
        (root/'obj.o').write_bytes(b'object')
        (root/'objdiff.json').write_text(json.dumps(dict(units=[dict(name='main/Game/X', base_path='obj.o', metadata=dict(source_path='src/x.cpp'))])))
        return subprocess.CompletedProcess([],0,'obj.o: #deps 2, deps mtime 1 (VALID)\n    src/x.cpp\n    x.hpp\n\n','')

    def test_full_ordinary_closure(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); run=self.fixture(root)
            with patch.object(d.subprocess,'run',return_value=run):
                result=d.audit_unit(root,'Game/X')
            self.assertTrue(result['complete'])
            self.assertEqual(result['domain'],'ordinary')
            self.assertIn(str((root/'x.hpp').resolve()),result['pins'])

    def test_header_trigger(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); run=self.fixture(root,header='struct T { volatile int done; };')
            with patch.object(d.subprocess,'run',return_value=run):
                result=d.audit_unit(root,'Game/X')
            self.assertTrue(result['complete'])
            self.assertEqual(result['domain'],'strict')

    def test_known_member_range_not_whole_unit(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);run=self.fixture(root,header='struct T { volatile int done; };')
            layout=dict(path='x.hpp',name='done',qualified_name='T::done',offset=8,width=4)
            with patch.object(d.subprocess,'run',return_value=run),patch.object(d,'load_layouts',return_value=[layout]):
                result=d.audit_unit(root,'Game/X')
            self.assertEqual(result['domain'],'ordinary')
            self.assertEqual(result['volatile_members'][0]['offset'],8)

    def test_global_symbol_not_whole_unit(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);run=self.fixture(root,header='extern volatile void* __piReg;')
            with patch.object(d.subprocess,'run',return_value=run):
                result=d.audit_unit(root,'Game/X')
            self.assertEqual(result['domain'],'ordinary')
            self.assertEqual(result['strict_symbols'][0]['name'],'__piReg')

    def test_stale_layout_hash_no_promotion(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);self.fixture(root)
            manifest={'layouts':[dict(path='x.hpp',header_sha256='WRONG',name='done',offset=8,width=4)]}
            real=Path.is_file
            def exists(p):
                return True if p.name=='semdomain_layouts.json' else real(p)
            original=Path.read_bytes
            def read(p):
                return json.dumps(manifest).encode() if p.name=='semdomain_layouts.json' else original(p)
            with patch.object(Path,'is_file',exists),patch.object(Path,'read_bytes',read):
                self.assertFalse(d.load_layouts(root,{}))

    def test_missing_deps_defaults_strict(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); self.fixture(root)
            with patch.object(d.subprocess,'run',return_value=subprocess.CompletedProcess([],0,'obj.o: deps not found\n','')):
                result=d.audit_unit(root,'Game/X')
            self.assertFalse(result['complete'])
            self.assertEqual(result['domain'],'strict')

    def test_missing_header_defaults_strict(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); run=self.fixture(root);(root/'x.hpp').unlink()
            with patch.object(d.subprocess,'run',return_value=run):
                result=d.audit_unit(root,'Game/X')
            self.assertFalse(result['complete'])
            self.assertEqual(result['domain'],'strict')

    def test_dep_newer_than_object_defaults_strict(self):
        import os
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); run=self.fixture(root)
            future=(root/'obj.o').stat().st_mtime_ns+1000000000
            os.utime(root/'x.hpp',ns=(future,future))
            with patch.object(d.subprocess,'run',return_value=run):
                result=d.audit_unit(root,'Game/X')
            self.assertFalse(result['complete'])
            self.assertEqual(result['domain'],'strict')

    def test_changed_input_never_ordinary(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp); run=self.fixture(root)
            original=d.sha
            # The two calls must see different hashes, not just the same mock.
            calls=0
            def unstable(p):
                nonlocal calls
                if Path(p).name=='.ninja_deps':
                    calls+=1
                    return 'first' if calls==1 else 'second'
                return original(p)
            with patch.object(d.subprocess,'run',return_value=run),patch.object(d,'sha',side_effect=unstable):
                result=d.audit_unit(root,'Game/X')
            self.assertFalse(result['complete'])
            self.assertEqual(result['domain'],'strict')


if __name__ == '__main__':
    unittest.main()
