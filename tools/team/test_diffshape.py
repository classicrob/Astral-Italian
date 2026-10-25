#!/usr/bin/env python3
"""Instruction-shape fixtures, including non-scoring reference-name noise."""
import unittest
import diffshape as d


def ins(text):
    return {'instruction': {'formatted': text}}


def pool(offset):
    return {'instruction': {'formatted': f'addi r3, r31, {offset:#x}', 'parts': [
        {'opcode': {'mnemonic': 'addi'}}, {'arg': {'opaque': 'r3'}}, {'separator': True},
        {'arg': {'opaque': 'r31'}}, {'separator': True}, {'arg': {'signed': str(offset)}}],
        'relocation': {'type_name': 'R_PPC_NONE', 'target_symbol': 1}}}


def ref(name):
    return {'instruction': {'formatted': f'lis r3, {name}@ha', 'parts': [
        {'opcode': {'mnemonic': 'lis'}}, {'arg': {'opaque': 'r3'}}, {'separator': True},
        {'arg': {'reloc': True}}, {'basic': '@ha'}], 'relocation': {'type': 6, 'target_symbol': 1}}}


def symbol(seq, score=99, start=0):
    return {'name': 'subject', 'size': len(seq)*4, 'address': str(start), 'match_percent': score,
            'instructions': [ins(x) if isinstance(x, str) else x for x in seq]}


class Shapes(unittest.TestCase):
    def classify(self, a, b): return d.shape(symbol(a), symbol(b))[0]

    def test_stack_with_reference_name_noise(self):
        self.assertEqual(self.classify([ref('lbl_80000000'), 'lwz r3, 0x8(r1)'], [ref('@123'), 'lwz r3, 0xc(r1)']), 'same_length:stack_offsets_only')

    def test_register_only(self):
        self.assertEqual(self.classify(['mr r31, r3', 'stw r31, 0x10(r1)'], ['mr r30, r3', 'stw r30, 0x10(r1)']), 'same_length:gprs_only')

    def test_register_and_stack(self):
        self.assertEqual(self.classify(['lwz r3, 0x8(r1)'], ['lwz r4, 0xc(r1)']), 'same_length:registers_and_stack_offsets')

    def test_fp_operand_order_before_generic_registers(self):
        self.assertEqual(self.classify(['fmuls f0, f1, f2'], ['fmuls f0, f2, f1']), 'same_length:fp_operand_order')

    def test_adjacent_loads(self):
        self.assertEqual(self.classify(['lwz r3, 0x8(r1)', 'lfs f0, 0xc(r1)'], ['lfs f0, 0xc(r1)', 'lwz r3, 0x8(r1)']), 'same_length:adjacent_load_swaps=1')

    def test_prologue_load_scheduling(self):
        self.assertEqual(self.classify(['mflr r0', 'lwz r4, 0x10(r3)', 'stw r0, 0x24(r1)'], ['mflr r0', 'stw r0, 0x24(r1)', 'lwz r4, 0x10(r3)']), 'same_length:prologue_load_vs_lr_store')

    def test_extra_move(self):
        self.assertEqual(self.classify(['li r3, 1', 'blr'], ['mr r31, r3', 'li r3, 1', 'blr']), 'extra_missing_mr:delta=+1')

    def test_uniform_immediate_offsets_not_relocation_addends(self):
        self.assertEqual(self.classify([pool(8), pool(16)], [pool(24), pool(32)]), 'same_length:pool_offsets_uniform_shift')
        self.assertEqual(self.classify([pool(8), pool(16)], [pool(24), pool(40)]), 'same_length:relocation_refs_only')

    def test_internal_branches_are_relative_to_function(self):
        a = symbol(['beq 0x78', 'blr'], start=100)
        b = symbol(['beq 0xdc', 'blr'], start=200)
        self.assertEqual(d.normal(a['instructions'][0]['instruction'], 100), d.normal(b['instructions'][0]['instruction'], 200))

    def test_branch_condition_immediates_are_not_addresses(self):
        instruction = {'formatted': 'bc 0xc, 4, 0x78'}
        self.assertEqual(d.normal(instruction, 100), 'bc 0xc, 4, B20')

    def test_exact_and_missing_status(self):
        self.assertEqual(d.shape(symbol(['blr'], score=100), symbol(['blr']))[0], 'matched_since_report')
        self.assertEqual(d.shape(symbol(['blr']), None)[0], 'missing_counterpart')


if __name__ == '__main__': unittest.main()
