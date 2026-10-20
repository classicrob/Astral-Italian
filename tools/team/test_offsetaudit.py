#!/usr/bin/env python3
"""Source inventory, pin/guard fail-closed and measured-layout regressions."""
import contextlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
import offsetaudit as audit


class OffsetAuditTests(unittest.TestCase):
    def rows(self, source):
        return audit.scan_text('test.cpp', source.encode())[0]

    def test_comments_strings_raw_strings_do_not_create_candidates(self):
        source = '''// (u8*)this + 0x10\nconst char *s="_10 (char*)p";
/* p->_20 */ auto x=R"tag((u8*)this + _30)tag";
auto y = *(bool*)((char*)p + 0x10);'''
        rows = self.rows(source)
        self.assertEqual(len(rows), 3)
        self.assertTrue(all(r['line'] == 4 for r in rows))

    def test_suffixed_field_unary_postfix_and_bare_advances(self):
        rows = self.rows('*(u8*)&pActor->mBinder->_1EC |= 0x20; _2C_new=waitKey; src += 0x10; pEnd += sizeof(CMemBlock);')
        cast = next(r for r in rows if r['kind'] == 'c_pointer_cast')
        self.assertEqual(cast['expression'], '(u8*)&pActor->mBinder->_1EC')
        self.assertTrue(any(r['expression'] == '_2C_new' for r in rows))
        advances = [r for r in rows if r['kind'] == 'address_arithmetic_candidate']
        self.assertEqual(len(advances), 2)
        self.assertTrue(any('sizeof(CMemBlock)' in r['expression'] for r in advances))
        rows = self.rows('void validateCollisionParts(CollisionParts* pParts) {}')
        self.assertEqual(rows[0]['kind'], 'possible_pointer_declarator')
        self.assertNotIn('{', rows[0]['expression'])

    def test_utf8_c1_bytes_are_not_preprocessor_line_boundaries(self):
        source = 'auto name="ファイアラン前兆";\n#ifdef GALAXY_NATIVE\na->_10;\n#else\nb->_934;\n#endif'
        rows, _ = audit.scan_text('encoded.cpp', source.encode('utf8'))
        members = {r['expression']: r['native_guard'] for r in rows if r['kind'] == 'unnamed_member'}
        self.assertEqual(members, {'_10': 'included', '_934': 'excluded'})

    def test_shift_jis_offsets_are_byte_offsets(self):
        rows, _ = audit.scan_text('x.cpp', b'//\x82\xa0\n(u8*)this + 12;')
        self.assertEqual(rows[0]['byte_offset'], 5)

    def test_named_nested_cast_and_arithmetic(self):
        rows = self.rows('*reinterpret_cast<bool*>(reinterpret_cast<u8*>(p) + 0x10);')
        self.assertEqual(len([r for r in rows if r['kind'] == 'named_pointer_cast']), 2)
        self.assertTrue(all(r['arithmetic_in_statement'] for r in rows))
        self.assertTrue(all(16 in r['nearby_integer_constants'] for r in rows))

    def test_alias_integer_and_functional_address_casts_are_not_silently_lost(self):
        for source in ['using BytePtr=u8*; auto q=reinterpret_cast<BytePtr>(p)+12;',
                       'auto q=reinterpret_cast<unsigned long>(p)+12;',
                       'auto q=(BytePtr)p+12;', 'auto q=std::uintptr_t(p)+12;']:
            self.assertTrue(self.rows(source), source)

    def test_octal_constant(self):
        self.assertEqual(audit.integer('012u'), 10)
        self.assertEqual(audit.integer('0x12UL'), 18)

    def test_this_arithmetic_and_unnamed_declaration_access(self):
        rows = self.rows('u32 _10; p->_10 = this + 4; x._2C = _10;')
        self.assertEqual([r['kind'] for r in rows].count('unnamed_identifier'), 2)
        self.assertEqual([r['kind'] for r in rows].count('unnamed_member'), 2)
        self.assertEqual([r['kind'] for r in rows].count('this_arithmetic'), 1)

    def test_guards_else_elif_and_unknown(self):
        states = audit.guard_lines('#ifdef GALAXY_NATIVE\na\n#else\nb\n#endif\n#if OTHER\nc\n#elif GALAXY_NATIVE\nd\n#endif')
        self.assertEqual(states[2], 'included')
        self.assertEqual(states[4], 'excluded')
        self.assertEqual(states[7], 'unknown')
        self.assertEqual(states[9], 'unknown')

    def test_directives_inside_comments_strings_and_spliced_comments_are_ignored(self):
        samples = [
            '/*\n#ifndef GALAXY_NATIVE\n*/\nauto x=(u8*)p+12;\n/*\n#endif\n*/',
            'auto s=R"tag(\n#ifndef GALAXY_NATIVE\n)tag";\nauto x=(u8*)p+12;',
            '// continued ' + chr(92) + '\n#ifndef GALAXY_NATIVE\nauto x=(u8*)p+12;',
        ]
        for source in samples:
            rows = [r for r in self.rows(source) if r['kind'] == 'c_pointer_cast']
            self.assertEqual(len(rows), 1)
            self.assertEqual(rows[0]['native_guard'], 'included')

    def test_native_macro_redefinition_invalidates_assumed_guard(self):
        rows = self.rows('#undef GALAXY_NATIVE\n#ifndef GALAXY_NATIVE\n(u8*)p+12;\n#endif')
        self.assertEqual(rows[0]['native_guard'], 'unknown')

    def test_unknown_guard_and_macro_offsetof_stay_unknown(self):
        for source in ['#if FEATURE\noffsetof(A,b);\n#endif', '#define offsetof(T,F)12\noffsetof(A,b);']:
            for row in self.rows(source):
                audit.classify(row, None, {}, {})
                self.assertEqual(row['classification'], 'UNKNOWN')

    def test_nested_false_guard_stays_excluded(self):
        states = audit.guard_lines('#ifndef GALAXY_NATIVE\n#if UNKNOWN\nx\n#endif\n#endif')
        self.assertEqual(states[3], 'excluded')

    def test_dump_direct_fields_not_nested_fields(self):
        dump = ''' 0 | class Outer
 0 |   struct Nested child
 0 |     int inner
 8 |   int * ptr
16:0-2 |   unsigned int bits
 | [sizeof=24, dsize=24, align=8,
 | nvsize=24, nvalign=8]
'''
        record = audit.parse_layout_dump(dump)['Outer']
        self.assertEqual(record['size'], 24)
        self.assertNotIn('inner', record['fields'])
        self.assertEqual(record['fields']['ptr']['offset_bits'], 64)
        self.assertEqual(record['fields']['bits']['size_bits'], 3)

    def bound(self, source='(u8*)this + 12;', kind=None, domain='host_object'):
        row = self.rows(source)[0]
        if kind:
            row['kind'] = kind
        binding = {'source_sha256': row['source_sha256'], 'expression_sha256': row['expression_sha256'],
                   'record': 'MemoryChunk', 'field': 'mBuffer', 'domain': domain,
                   'raw_offset_bytes': 12, 'evidence': ['fixture: exact access base and member']}
        layouts = {abi: {'producer': {'accuracy': 'measured'}, 'records': {'MemoryChunk':
                   {'fields': {'mBuffer': {'offset_bits': offset * 8}}}}}
                   for abi, offset in [('wii', 12), ('native', 16)]}
        return row, binding, layouts, {'wii': [], 'native': []}

    def test_actual_pinned_raw_host_offset_delta(self):
        args = self.bound()
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'BOUND_LAYOUT_MISMATCH')
        self.assertEqual(args[0]['native_offset_bits'], 128)

    def test_approximate_ppc_not_promoted_to_measured(self):
        args = self.bound()
        args[2]['wii']['producer']['accuracy'] = 'approximation'
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'LAYOUT_MISMATCH_CANDIDATE')

    def test_typed_unnamed_offset_moves_without_raw_bug(self):
        args = self.bound('p->_0C;', kind='unnamed_member')
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'TYPED_MEMBER_ADJUSTS')

    def test_serialized_this_size_not_host_bug(self):
        args = self.bound('(u8*)this + mSize;', domain='serialized')
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'NON_HOST_DOMAIN')

    def test_stale_access_binding_cannot_exempt(self):
        args = self.bound(domain='serialized')
        args[1]['source_sha256'] = 'stale'
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'UNKNOWN')

    def test_missing_layout_cannot_approve(self):
        args = self.bound()
        args[3]['native'] = ['source pin drift']
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'UNKNOWN')

    def test_wrong_wii_field_cannot_be_called_native_mismatch(self):
        args = self.bound()
        args[1]['raw_offset_bytes'] = 8
        audit.classify(*args)
        self.assertEqual(args[0]['classification'], 'UNKNOWN')

    def test_guarded_native_offsetof_inventory(self):
        rows = self.rows('#ifdef GALAXY_NATIVE\nchar*p=(char*)this + offsetof(C,b)-offsetof(C,a);\n#else\nchar*p=(char*)this+0x3c;\n#endif')
        offsets = [r for r in rows if r['kind'] == 'offsetof']
        self.assertEqual(len(offsets), 2)
        self.assertTrue(all(r['native_guard'] == 'included' for r in offsets))
        old = rows[-1]
        audit.classify(old, None, {}, {})
        self.assertEqual(old['classification'], 'NATIVE_EXCLUDED')

    def test_pinned_ast_mechanically_binds_byte_cast_base_and_offset(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source = '(u8*)p + 12;'
            (root/'test.cpp').write_text(source)
            rows = self.rows(source)
            # Faithful reduced Clang JSON shape; this is labelled synthetic,
            # not passed off as measured ABI evidence.
            ast = {'kind': 'BinaryOperator', 'opcode': '+',
                   'range': {'begin': {'offset': 0}, 'end': {'offset': 9, 'tokLen': 2}},
                   'inner': [
                       {'kind': 'CStyleCastExpr', 'type': {'qualType': 'u8 *'},
                        'range': {'begin': {'offset': 0}, 'end': {'offset': 5, 'tokLen': 1}},
                        'inner': [{'kind': 'DeclRefExpr', 'type': {'qualType': 'MemoryChunk *'}}]},
                       {'kind': 'IntegerLiteral', 'value': '12'}]}
            payload = json.dumps(ast).encode()
            (root/'ast.json').write_bytes(payload)
            manifest = {'schema': 1, 'primary_source': 'test.cpp', 'ast': 'ast.json',
                        'ast_sha256': audit.sha(payload), 'files': {'test.cpp': audit.sha(source.encode())},
                        'producer': {'kind': 'synthetic-test', 'command': ['fixture']}}
            (root/'manifest.json').write_text(json.dumps(manifest))
            layout = {'records': {'MemoryChunk': {'fields': {'mBuffer': {'offset_bits': 96}}}}}
            domains = {'records': {'MemoryChunk': {'domain': 'host_object', 'scope': 'all_instances', 'evidence': ['fixture']}}}
            derived, errors = audit.ast_accesses(root, root/'manifest.json', rows, layout, domains)
            self.assertFalse(errors)
            self.assertEqual(derived[rows[0]['id']]['record'], 'MemoryChunk')
            self.assertEqual(derived[rows[0]['id']]['raw_offset_bytes'], 12)
            self.assertEqual(derived[rows[0]['id']]['field'], 'mBuffer')
            self.assertEqual(derived[rows[0]['id']]['domain'], 'host_object')
            # Pointee-size dependence cannot be interpreted as byte offset.
            ast['inner'][0]['type']['qualType'] = 'int *'
            payload = json.dumps(ast).encode()
            (root/'ast.json').write_bytes(payload)
            manifest['ast_sha256'] = audit.sha(payload)
            (root/'manifest.json').write_text(json.dumps(manifest))
            derived, errors = audit.ast_accesses(root, root/'manifest.json', rows, layout, domains)
            self.assertFalse(derived)
            self.assertFalse(errors)
            # A pointer-producing call can take an unrelated record argument;
            # the argument does not establish the returned buffer's base.
            ast['inner'][0]['type']['qualType'] = 'u8 *'
            ast['inner'][0]['inner'] = [{'kind': 'CallExpr', 'type': {'qualType': 'u8 *'},
                                       'inner': [{'kind': 'DeclRefExpr', 'type': {'qualType': 'MemoryChunk *'}}]}]
            payload = json.dumps(ast).encode()
            (root/'ast.json').write_bytes(payload)
            manifest['ast_sha256'] = audit.sha(payload)
            (root/'manifest.json').write_text(json.dumps(manifest))
            derived, errors = audit.ast_accesses(root, root/'manifest.json', rows, layout, domains)
            self.assertFalse(derived)
            self.assertFalse(errors)
            # A stale AST must not bind anything, even if source stayed same.
            manifest['ast_sha256'] = 'stale'
            (root/'manifest.json').write_text(json.dumps(manifest))
            derived, errors = audit.ast_accesses(root, root/'manifest.json', rows, layout, domains)
            self.assertFalse(derived)
            self.assertTrue(errors)

    def test_default_scan_cannot_launch_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'x.cpp').write_text('(u8*)this + 12;')
            with patch.object(audit.subprocess, 'run', side_effect=AssertionError('implicit subprocess')):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(audit.main(['--root', tmp, '--scope', 'x.cpp', '--out', str(root/'out.json')]), 0)
            report = json.loads((root/'out.json').read_text())
            self.assertFalse(report['coverage']['compiler_executed'])
            self.assertFalse(report['coverage']['semantic_completeness'])
            self.assertEqual(report['counts'], {'UNKNOWN': 3})

    def test_stale_layout_file_pin(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'header.hpp').write_text('changed')
            payload = {'schema': 1, 'abi': 'wii', 'files': {'header.hpp': 'old'},
                       'producer': {'command': ['clang++'], 'kind': 'test', 'accuracy': 'measured'}}
            path = root/'layout.json'
            path.write_text(json.dumps(payload))
            _, errors = audit.artifact(root, path, 'wii')
            self.assertTrue(any('stale' in e for e in errors))

    def test_collection_rejects_stale_pins_without_compiler(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            path = root/'plan.json'
            path.write_text(json.dumps({'schema': 1, 'files': {'missing': 'old'}}))
            with patch.object(audit.subprocess, 'run', side_effect=AssertionError('must not launch')):
                with contextlib.redirect_stderr(io.StringIO()):
                    self.assertEqual(audit.main(['--root', tmp, '--collect-plan', str(path), '--out', str(root/'out')]), 2)


if __name__ == '__main__':
    unittest.main()
