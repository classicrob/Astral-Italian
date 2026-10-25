#!/usr/bin/env python3
"""Read-only boundary signals and conservative negative controls."""
from pathlib import Path
from types import SimpleNamespace
import tempfile
import unittest

import splitscan as ss
import tuscan as t


class InlineWindows(unittest.TestCase):
    def test_two_load_leaf_matches_with_different_registers(self):
        leaf = [0x80630000, 0x80630008, 0x4e800020]
        caller = [0x9421fff0, 0x80a40000, 0x80a50008, 0x4e800020]
        self.assertEqual(t.inline_window(leaf, caller)['offset'], 4)

    def test_load_offsets_and_widths_are_not_discarded(self):
        leaf = [0x80630000, 0x80630008, 0x4e800020]
        self.assertIsNone(t.inline_window(leaf, [0x80a40000, 0x80a5000c]))
        self.assertIsNone(t.inline_window(leaf, [0x88a40000, 0x80a50008]))

    def test_single_operation_calls_and_stack_frames_do_not_prove_inlining(self):
        self.assertIsNone(t.inline_window([0x80630000, 0x4e800020], [0x80630000]))
        self.assertIsNone(t.inline_window([0x48000001, 0x80630000, 0x4e800020], [0x48000001, 0x80630000]))
        self.assertIsNone(t.inline_window([0x9421fff0, 0x80630000, 0x4e800020], [0x9421fff0, 0x80630000]))


class StringsAndRanking(unittest.TestCase):
    def test_raw_strings_require_boundaries_and_terminated_printable_bytes(self):
        data = b'Idle\0\x80\x01\x02\x03\0Run\0Jump\0unterminated'
        self.assertEqual(t.raw_strings(data), {0: b'Idle', 14: b'Jump'})

    def test_duplicate_function_report_occurrences_survive_ranking(self):
        report = {'functions': [
            {'name': 'same', 'size': '20', 'fuzzy_match_percent': 99, 'metadata': {'virtual_address': '4096'}},
            {'name': 'same', 'size': '24', 'fuzzy_match_percent': 100, 'metadata': {'virtual_address': '8192'}}]}
        observation = dict(kind='inline_pin', confidence='weak', functions=['same@0x00001000', 'same@0x00002000'], addresses=[], pins=[{}])
        row = t.summarize_unit('main/Game/A', [observation], report)
        self.assertEqual(row['affected_residue_bytes'], 20)
        self.assertEqual(row['affected_total_functions'], 2)

    def test_uncorroborated_padding_or_repeated_constants_are_not_candidates(self):
        for kind in ('mid_text_padding', 'constant_pool_restart'):
            self.assertIsNone(t.summarize_unit('main/Game/A', [dict(kind=kind, confidence='weak', functions=[], addresses=[])], {}))

    def test_pin_detection_ignores_comments_and_captures_line_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'a.cpp'
            path.write_text('// #pragma auto_inline off\n#pragma auto_inline off\n#define VEC_OUTLINE\n')
            rows = t.inline_pins(path, {})
            self.assertEqual([p['line'] for p in rows[0]['pins']], [2, 3])


class MergeSignature(unittest.TestCase):
    def setUp(self):
        self.catalog = ss.Catalog('caller = .text:0x80100020; // type:function size:0x20\n')
        self.splits = ss.Splits('Game/Before.cpp:\n .text start:0x80100000 end:0x80100020\n .data start:0x80600000 end:0x80600080\nGame/After.cpp:\n .text start:0x80100020 end:0x80100040\n .data start:0x80600080 end:0x80600100\n')
        self.function = ss.ElfSymbol('caller', 0, 32, 1, 2, 1, 0x80100020)
        self.elf = SimpleNamespace(functions=lambda: [self.function], origins={1: 0x80100020})

    def test_preceding_base_exact_size_shift_and_text_adjacency_are_recorded(self):
        left = ss.Ref('caller', 4, '.data', 0x80600098, 'pool', 0x80600000, 0, 'derived_addi', 0x98)
        right = ss.Ref('caller', 4, '.data', 0x18, 'pool', 0, 0, 'derived_addi', 0x18)
        rows = t.merge_signals({'name': 'main/Game/After'}, self.elf, self.elf, self.catalog, self.splits, [left], [right])
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['proposed_partner'], 'main/Game/Before')
        self.assertEqual(rows[0]['confidence'], 'high')
        self.assertEqual(rows[0]['source_minus_target_immediate'], -0x80)

    def test_direct_shared_global_or_current_unit_anchor_is_not_a_merge_signal(self):
        for anchor, address in [(0x80600000, 0x80600018), (0x80600080, 0x80600098)]:
            ref = ss.Ref('caller', 4, '.data', address, 'pool', anchor, 0, 'derived_addi', 0x18)
            self.assertFalse(t.merge_signals({'name': 'main/Game/After'}, self.elf, self.elf, self.catalog, self.splits, [ref], []))


class IndependentPoolGroups(unittest.TestCase):
    def test_distinct_class_groups_do_not_dilute_each_other(self):
        functions = [ss.ElfSymbol(name, offset, 16, 1, 2, 1, 0x80100000+offset)
                     for name, offset in [('draw', 0), ('controller', 16)]]
        elf = SimpleNamespace(functions=lambda: functions, origins={1: 0x80100000})
        left, right = [], []
        for function, delta in zip(functions, [0x48, 0xd0]):
            for offset, immediate in [(4, 8), (8, 16)]:
                left.append(ss.Ref(function.name, function.value+offset, '.data', 0x80600000+immediate,
                                   'base', 0x80600000, 0, 'derived_addi', immediate, 0x38630000+immediate))
                right.append(ss.Ref(function.name, function.value+offset, '.data', immediate+delta,
                                    'base', 0, 0, 'derived_addi', immediate+delta, 0x38630000+immediate+delta))
        rows = t.shift_signals(elf, elf, ss.Catalog(''), left, right)
        self.assertEqual([r['delta'] for r in rows], [0x48, 0xd0])
        self.assertEqual(rows[0]['functions'], ['draw@0x80100000'])
        self.assertEqual(rows[1]['addresses'], ['0x80100014', '0x80100018'])


if __name__ == '__main__':
    unittest.main()
