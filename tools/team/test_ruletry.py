#!/usr/bin/env python3
"""Rule-directed generation, type boundaries and refusal cases."""
from pathlib import Path
import tempfile
import unittest

import permute
import ruletry as r


class Recipes(unittest.TestCase):
    def generate(self, source, rule, **options):
        return r.generate(source, permute.find_function(source, 'subject'), rule, **options)

    def test_scalar_rule_preserves_grouping_and_requires_f32_operands(self):
        source = 'float subject(float a, float b) { return a * b; }\n'
        candidates = self.generate(source, 'scalar-scope')
        self.assertEqual(len(candidates), 8)
        for _, text in candidates:
            self.assertIn('return left * right;', text)
            self.assertTrue('multiplyScalars(a, b)' in text or 'multiplyScalars(b, a)' in text)
        self.assertFalse(self.generate('double subject(double a, double b) { return a * b; }', 'scalar-scope'))
        self.assertFalse(self.generate('float subject(float a, float b, float c) { return a + b * c; }', 'scalar-scope'))

    def test_unknown_types_do_not_become_float_boundaries(self):
        source = 'float subject(float a, float b) { Widget a; return a * b; }'
        self.assertFalse(self.generate(source, 'scalar-scope'))

    def test_reference_rule_requires_explicit_field_type(self):
        source = 'bool subject() { return mTimer != 0; }'
        self.assertFalse(self.generate(source, 'field-reference-reload'))
        candidates = self.generate(source, 'field-reference-reload', bindings=['u16:mTimer'])
        self.assertEqual(len(candidates), 7)
        self.assertIn('const u16& value', candidates[0][1])
        self.assertIn('return value != 0;', candidates[0][1])
        with self.assertRaises(ValueError):
            self.generate(source, 'field-reference-reload', bindings=['u16:nextTimer()'])

    def test_pointer_binding_preserves_pointer_width_and_constness(self):
        source = 'void subject() { mHost->update(); }'
        candidates = self.generate(source, 'field-reference-reload', bindings=['LiveActor*:mHost'])
        self.assertEqual(len(candidates), 2)
        self.assertIn('LiveActor* const& value', candidates[0][1])
        self.assertIn('LiveActor* const& readMhost', candidates[1][1])

    def test_accessors_cannot_change_reference_return_or_overload_binding(self):
        for source in ('u16& subject() { return mTimer; }', 'void subject() { overloaded(mTimer); }'):
            self.assertFalse(self.generate(source, 'field-reference-reload', bindings=['u16:mTimer']))

    def test_scalar_recipes_do_not_rewrite_macro_arguments(self):
        source = 'float subject(float a, float b) { return STRINGIZE((a * b)); }'
        self.assertFalse(self.generate(source, 'scalar-scope', macro_names={'STRINGIZE'}))

    def test_rule_filter_excludes_unrelated_recipes(self):
        source = 'int subject(int a, int b) {\n int x = a;\n int y = b;\n return x + y;\n}\n'
        self.assertFalse(self.generate(source, 'scalar-scope'))
        candidates = self.generate(source, 'gpr-lifetime')
        self.assertTrue(candidates)
        self.assertFalse(any('terminal_else_to_early_return' in label for label, _ in candidates))

    def test_pool_rule_does_not_generate_declaration_noise(self):
        source = 'int subject(int a) {\n int b = a;\n return b;\n}\n'
        self.assertFalse(self.generate(source, 'pool-layout'))

    def test_temporary_scope_keeps_consumer_and_does_not_escape_via_call(self):
        source = 'float subject(float a, float b) {\n float result;\n float temp = a * b;\n result = temp + b;\n result += a;\n return result;\n}\n'
        candidates = self.generate(source, 'scalar-scope', budget=20)
        scoped = [text for label, text in candidates if label == 'temporary-consumer-scope']
        self.assertTrue(scoped)
        self.assertTrue(any('result = temp + b;\n}' in text for text in scoped))
        escaping = source.replace('result = temp + b;', 'retain(temp);')
        self.assertFalse(any(label == 'temporary-consumer-scope' for label, _ in self.generate(escaping, 'scalar-scope')))

    def test_volatile_and_local_type_context_fail_closed(self):
        with self.assertRaises(ValueError):
            self.generate('float subject(float a, float b) { volatile int flag; return a*b; }', 'scalar-scope')
        with self.assertRaises(ValueError):
            self.generate('float subject(float a, float b) { typedef int local; return a*b; }', 'scalar-scope')

    def test_rule_lookup_is_scoped_by_unit_and_requires_unique_function(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'rules.tsv'
            path.write_text('unit\tfunction\trules\nGame/A\tfoo\tgpr-lifetime\nGame/B\tfoo\tscalar-scope\n')
            self.assertEqual(r.rule_row(path, 'Game/B', 'foo')['rules'], 'scalar-scope')
            with self.assertRaises(ValueError): r.rule_row(path, 'Game/A', 'unknown')


if __name__ == '__main__':
    unittest.main()
