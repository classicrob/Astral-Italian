"""Regression checks for triage distinctions that affect data fix decisions."""
from types import SimpleNamespace
import unittest

from datacontent import Relocation, Symbol
from datadiff import classify, positional_candidates, relocations_differ


def elf(section='.data', nobits=False):
    return SimpleNamespace(names=['', section], sections=[(), (0, 8 if nobits else 1)])


def sym(name='table', address=0, size=8):
    return Symbol(name, address, size, 1, 1, 1)


class DataDiffTests(unittest.TestCase):
    def test_relocation_only_mismatch(self):
        e = elf()
        cause, flags = classify(sym(), sym(), e, e, {
            'data_diff': [{'data': 'AAAAAAAAAAA='}],
            'reloc_diff': [{'kind': 'DIFF_REPLACE'}], 'match_percent': 66.66667})
        self.assertEqual(cause, 'relocation_differs')
        self.assertNotIn('bytes_differ', flags)

    def test_bytes_and_relocation_are_both_retained(self):
        e = elf()
        cause, flags = classify(sym(), sym(), e, e, {
            'data_diff': [{'kind': 'DIFF_DELETE'}],
            'reloc_diff': [{'kind': 'DIFF_INSERT'}]})
        self.assertEqual(cause, 'bytes_differ')
        self.assertIn('relocation_differs', flags)

    def test_bss_size_is_not_a_byte_difference(self):
        e = elf('.bss', True)
        self.assertEqual(classify(sym(size=16), sym(size=12), e, e, {'match_percent': 50})[0], 'size_differs')

    def test_bss_interior_label_remains_missing(self):
        e = elf('.bss', True)
        target = sym('lbl_interior', 693, 165931)
        source = sym('gki_cb', 0, 166624)
        self.assertEqual(classify(target, None, e, e, {})[0], 'missing')
        self.assertEqual(positional_candidates(target, e, e, [source]), 'overlap:gki_cb@0x0+0x28ae0')

    def test_exact_symbol_does_not_claim_section_credit(self):
        e = elf()
        self.assertEqual(classify(sym(), sym(), e, e, {'match_percent': 100})[0],
                         'matched_symbol_in_unmatched_section')

    def test_section_and_order_are_separate(self):
        cause, flags = classify(sym(), sym(address=4), elf(), elf('.rodata'), {})
        self.assertEqual(cause, 'section_wrong')
        self.assertIn('order_wrong', flags)

    def test_elf_relocation_identity_when_json_omits_relocations(self):
        left, right = elf(), elf()
        left.local = right.local = lambda s: s.section == 1
        left.relocations = {(1, 0): Relocation(1, Symbol('callee', 0, 0, 0, 0, 1), 0)}
        right.relocations = {(1, 0): Relocation(1, Symbol('other', 0, 0, 0, 0, 1), 0)}
        self.assertTrue(relocations_differ(sym(), sym(), left, right))
        right.relocations = dict(left.relocations)
        self.assertFalse(relocations_differ(sym(), sym(), left, right))

    def test_pool_addend_equivalence(self):
        left, right = elf(), elf()
        left.local = right.local = lambda s: s.section == 1
        left.relocations = {(1, 0): Relocation(1, sym('string', 16, 4), 0)}
        right.relocations = {(1, 0): Relocation(1, sym('pool', 0, 0), 16)}
        self.assertFalse(relocations_differ(sym(), sym(), left, right))


if __name__ == '__main__':
    unittest.main()
