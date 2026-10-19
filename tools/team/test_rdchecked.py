import unittest
from rdchecked import checked_equal, checked_equal_mem, machine_lines


def asm(text):
    return [s.strip() for s in text.split(';') if s.strip()]


class CheckedProofTests(unittest.TestCase):
    def yes(self, a, b):
        ok, why = checked_equal(asm(a), asm(b))
        self.assertTrue(ok, why)

    def no(self, a, b):
        ok, why = checked_equal(asm(a), asm(b))
        self.assertFalse(ok, why)

    def test_copy_elimination(self):
        self.yes('mr r11, r3; addi r3, r11, 4; blr', 'addi r3, r3, 4; blr')
        self.yes('fmr f0, f1; fadds f1, f0, f2; blr', 'fadds f1, f1, f2; blr')

    def test_independent_load_reorder(self):
        self.yes('lwz r11, 0(r3); lwz r12, 4(r3); add r3, r11, r12; blr',
                 'lwz r12, 4(r3); lwz r11, 0(r3); add r3, r11, r12; blr')

    def test_commutative_and_register_allocation(self):
        self.yes('lwz r11, 0(r3); lwz r12, 4(r3); add r3, r11, r12; blr',
                 'lwz r0, 4(r3); lwz r12, 0(r3); add r3, r0, r12; blr')

    def test_independent_store_load(self):
        self.yes('stw r4, 0(r3); lwz r3, 4(r3); blr',
                 'lwz r11, 4(r3); stw r4, 0(r3); mr r3, r11; blr')

    def test_stack_independent(self):
        self.yes('stw r4, 8(r1); lwz r3, 12(r1); blr',
                 'lwz r3, 12(r1); stw r4, 8(r1); blr')

    def test_aliasing_store(self):
        self.no('stw r4, 0(r3); lwz r3, 0(r3); blr',
                'lwz r11, 0(r3); stw r4, 0(r3); mr r3, r11; blr')

    def test_overlap(self):
        self.no('stw r4, 0(r3); lhz r3, 2(r3); blr',
                'lhz r11, 2(r3); stw r4, 0(r3); mr r3, r11; blr')

    def test_different_bases_may_alias(self):
        self.no('stw r4, 0(r5); lwz r3, 8(r6); blr',
                'lwz r3, 8(r6); stw r4, 0(r5); blr')

    def test_different_producer(self):
        self.no('addi r11, r3, 4; addi r12, r3, 8; mr r3, r11; blr',
                'addi r11, r3, 4; addi r12, r3, 8; mr r3, r12; blr')

    def test_swapped_call_order(self):
        self.no('bl foo; bl bar; blr', 'bl bar; bl foo; blr')

    def test_constant(self):
        self.no('li r3, 1; blr', 'li r3, 2; blr')

    def test_branch_target(self):
        self.no('cmpwi r3, 0; beq 0x10; li r3, 1; b 0x14; li r3, 2; blr',
                'cmpwi r3, 0; beq 0x14; li r3, 1; b 0x14; li r3, 2; blr')

    def test_call_arg_swap(self):
        self.no('mr r11, r3; mr r3, r4; mr r4, r11; bl foo; blr', 'bl foo; blr')

    def test_call_fp_arg_swap(self):
        self.no('fmr f0, f1; fmr f1, f2; fmr f2, f0; bl foo; blr', 'bl foo; blr')

    def test_call_clobber(self):
        self.no('mr r11, r3; bl foo; mr r3, r11; blr', 'bl foo; blr')

    def test_load_call_order(self):
        self.no('lwz r11, 0(r3); bl foo; blr', 'bl foo; lwz r11, 0(r3); blr')

    def test_64_bit_return(self):
        self.no('li r4, 1; blr', 'li r4, 2; blr')

    def test_copy_overwrite(self):
        self.no('mr r11, r3; li r3, 7; mr r3, r11; blr', 'li r3, 7; blr')

    def test_join_producers(self):
        self.no('cmpwi r3, 0; beq 0x10; li r3, 1; b 0x14; li r3, 2; blr',
                'cmpwi r3, 0; beq 0x10; li r3, 2; b 0x14; li r3, 1; blr')

    def test_copy_cfg_relative_targets(self):
        self.yes('cmpwi r3, 0; beq 0x14; mr r11, r3; addi r3, r11, 1; b 0x18; li r3, 2; blr',
                 'cmpwi r3, 0; beq 0x10; addi r3, r3, 1; b 0x14; li r3, 2; blr')

    def test_fp_multiply_is_ordered(self):
        for op in ('fmul', 'fmuls'):
            self.no(f'{op} f1, f2, f3; blr', f'{op} f1, f3, f2; blr')
        for op in ('fmadd', 'fmadds', 'fmsub', 'fnmadd', 'fnmsub'):
            self.no(f'{op} f1, f2, f3, f4; blr', f'{op} f1, f3, f2, f4; blr')

    def test_unknown_and_loop_fail_closed(self):
        self.no('mystery r3, r4; blr', 'mystery r3, r4; blr')
        self.no('addi r3, r3, 1; b 0x0', 'addi r3, r3, 1; b 0x0')

    def test_record_condition(self):
        self.no('add. r11, r3, r4; beq 0x10; li r3, 1; blr; li r3, 2; blr',
                'add r11, r3, r4; beq 0x10; li r3, 1; blr; li r3, 2; blr')

    def test_stack_pointer_call_input(self):
        self.no('addi r1, r1, -16; bl foo; addi r1, r1, 16; blr',
                'bl foo; addi r1, r1, -16; addi r1, r1, 16; blr')

    def test_preserved_cr_across_call(self):
        self.no('cmpwi cr2, r3, 1; bl foo; blr', 'cmpwi cr2, r3, 2; bl foo; blr')

    def test_varargs_cr_call_input(self):
        self.no('cmpwi cr1, r3, 1; bl foo; blr', 'bl foo; cmpwi cr1, r3, 1; blr')

    def test_stack_update_store(self):
        self.yes('stwu r1, -16(r1); mr r11, r3; addi r3, r11, 4; addi r1, r1, 16; blr',
                 'stwu r1, -16(r1); addi r3, r3, 4; addi r1, r1, 16; blr')

    def test_unknown_call_outputs(self):
        self.no('bl foo; mflr r3; blr', 'bl foo; mflr r3; blr')
        self.no('bl foo; mr r3, r11; blr', 'bl foo; mr r3, r11; blr')

    def test_valid_call_copy(self):
        self.yes('mflr r12; stw r12, 4(r1); mr r11, r3; mr r3, r11; bl foo; lwz r12, 4(r1); mtlr r12; blr',
                 'mflr r12; stw r12, 4(r1); bl foo; lwz r12, 4(r1); mtlr r12; blr')

    def test_paired_load_reorder(self):
        self.yes('psq_l f0, 0(r3), 0, qr0; psq_l f9, 8(r3), 1, qr0; psq_st f0, 0(r4), 0, qr0; psq_st f9, 8(r4), 1, qr0; blr',
                 'psq_l f9, 8(r3), 1, qr0; psq_l f0, 0(r3), 0, qr0; psq_st f0, 0(r4), 0, qr0; psq_st f9, 8(r4), 1, qr0; blr')

    def test_paired_width_alias(self):
        self.no('psq_st f0, 0(r3), 0, qr0; lwz r3, 4(r3); blr',
                'lwz r11, 4(r3); psq_st f0, 0(r3), 0, qr0; mr r3, r11; blr')
        self.yes('psq_st f0, 0(r3), 1, qr0; lwz r3, 4(r3); blr',
                 'lwz r11, 4(r3); psq_st f0, 0(r3), 1, qr0; mr r3, r11; blr')

    def test_paired_gqr_and_mode(self):
        self.no('psq_l f0, 0(r3), 0, qr0; blr', 'psq_l f0, 0(r3), 0, qr1; blr')
        self.no('psq_st f0, 0(r3), 0, qr0; blr', 'psq_st f0, 0(r3), 1, qr0; blr')

    def test_paired_update(self):
        self.yes('psq_lu f0, 8(r3), 0, qr0; mr r11, r3; addi r3, r11, 4; blr',
                 'psq_lu f0, 8(r3), 0, qr0; addi r3, r3, 4; blr')
        self.yes('psq_stu f0, 8(r3), 0, qr0; mr r11, r3; addi r3, r11, 4; blr',
                 'psq_stu f0, 8(r3), 0, qr0; addi r3, r3, 4; blr')

    def test_alignment_is_not_unknown_instruction(self):
        self.assertEqual(machine_lines([{'instruction': None}, {'instruction': {}}], ['', '']), [''])
        self.no(';'.join(machine_lines([{'instruction': {}}], [''])), 'blr')

    def test_effect_multiplicity(self):
        self.no('stw r4, 0(r3); stw r4, 0(r3); blr', 'stw r4, 0(r3); blr')


class MemoryCheckedProofTests(unittest.TestCase):
    def yes(self, a, b):
        ok, why = checked_equal_mem(asm(a), asm(b))
        self.assertTrue(ok, why)

    def no(self, a, b):
        ok, why = checked_equal_mem(asm(a), asm(b))
        self.assertFalse(ok, why)

    def test_copy_elimination(self):
        self.yes('mr r11, r3; addi r3, r11, 4; blr', 'addi r3, r3, 4; blr')
        self.yes('fmr f0, f1; fadds f1, f0, f2; blr', 'fadds f1, f1, f2; blr')

    def test_independent_load_reorder(self):
        self.yes('lwz r11, 0(r3); lwz r12, 4(r3); add r3, r11, r12; blr',
                 'lwz r12, 4(r3); lwz r11, 0(r3); add r3, r11, r12; blr')

    def test_commutative_and_register_allocation(self):
        self.yes('lwz r11, 0(r3); lwz r12, 4(r3); add r3, r11, r12; blr',
                 'lwz r0, 4(r3); lwz r12, 0(r3); add r3, r0, r12; blr')

    def test_independent_store_load(self):
        self.yes('stw r4, 0(r3); lwz r3, 4(r3); blr',
                 'lwz r11, 4(r3); stw r4, 0(r3); mr r3, r11; blr')

    def test_stack_independent(self):
        self.yes('stw r4, 8(r1); lwz r3, 12(r1); blr',
                 'lwz r3, 12(r1); stw r4, 8(r1); blr')

    def test_aliasing_store(self):
        self.no('stw r4, 0(r3); lwz r3, 0(r3); blr',
                'lwz r11, 0(r3); stw r4, 0(r3); mr r3, r11; blr')

    def test_overlap(self):
        self.no('stw r4, 0(r3); lhz r3, 2(r3); blr',
                'lhz r11, 2(r3); stw r4, 0(r3); mr r3, r11; blr')

    def test_different_bases_may_alias(self):
        self.no('stw r4, 0(r5); lwz r3, 8(r6); blr',
                'lwz r3, 8(r6); stw r4, 0(r5); blr')

    def test_different_producer(self):
        self.no('addi r11, r3, 4; addi r12, r3, 8; mr r3, r11; blr',
                'addi r11, r3, 4; addi r12, r3, 8; mr r3, r12; blr')

    def test_swapped_call_order(self):
        self.no('bl foo; bl bar; blr', 'bl bar; bl foo; blr')

    def test_constant(self):
        self.no('li r3, 1; blr', 'li r3, 2; blr')

    def test_branch_target(self):
        self.no('cmpwi r3, 0; beq 0x10; li r3, 1; b 0x14; li r3, 2; blr',
                'cmpwi r3, 0; beq 0x14; li r3, 1; b 0x14; li r3, 2; blr')

    def test_call_arg_swap(self):
        self.no('mr r11, r3; mr r3, r4; mr r4, r11; bl foo; blr', 'bl foo; blr')

    def test_call_fp_arg_swap(self):
        self.no('fmr f0, f1; fmr f1, f2; fmr f2, f0; bl foo; blr', 'bl foo; blr')

    def test_call_clobber(self):
        self.no('mr r11, r3; bl foo; mr r3, r11; blr', 'bl foo; blr')

    def test_load_call_order(self):
        self.no('lwz r11, 0(r3); bl foo; blr', 'bl foo; lwz r11, 0(r3); blr')

    def test_64_bit_return(self):
        self.no('li r4, 1; blr', 'li r4, 2; blr')

    def test_copy_overwrite(self):
        self.no('mr r11, r3; li r3, 7; mr r3, r11; blr', 'li r3, 7; blr')

    def test_join_producers(self):
        self.no('cmpwi r3, 0; beq 0x10; li r3, 1; b 0x14; li r3, 2; blr',
                'cmpwi r3, 0; beq 0x10; li r3, 2; b 0x14; li r3, 1; blr')

    def test_copy_cfg_relative_targets(self):
        self.yes('cmpwi r3, 0; beq 0x14; mr r11, r3; addi r3, r11, 1; b 0x18; li r3, 2; blr',
                 'cmpwi r3, 0; beq 0x10; addi r3, r3, 1; b 0x14; li r3, 2; blr')

    def test_fp_multiply_is_ordered(self):
        for op in ('fmul', 'fmuls'):
            self.no(f'{op} f1, f2, f3; blr', f'{op} f1, f3, f2; blr')
        for op in ('fmadd', 'fmadds', 'fmsub', 'fnmadd', 'fnmsub'):
            self.no(f'{op} f1, f2, f3, f4; blr', f'{op} f1, f3, f2, f4; blr')

    def test_unknown_and_loop_fail_closed(self):
        self.no('mystery r3, r4; blr', 'mystery r3, r4; blr')
        self.no('addi r3, r3, 1; b 0x0', 'addi r3, r3, 1; b 0x0')

    def test_record_condition(self):
        self.no('add. r11, r3, r4; beq 0x10; li r3, 1; blr; li r3, 2; blr',
                'add r11, r3, r4; beq 0x10; li r3, 1; blr; li r3, 2; blr')

    def test_stack_pointer_call_input(self):
        self.no('addi r1, r1, -16; bl foo; addi r1, r1, 16; blr',
                'bl foo; addi r1, r1, -16; addi r1, r1, 16; blr')

    def test_preserved_cr_across_call(self):
        self.no('cmpwi cr2, r3, 1; bl foo; blr', 'cmpwi cr2, r3, 2; bl foo; blr')

    def test_varargs_cr_call_input(self):
        self.no('cmpwi cr1, r3, 1; bl foo; blr', 'bl foo; cmpwi cr1, r3, 1; blr')

    def test_stack_update_store(self):
        self.yes('stwu r1, -16(r1); mr r11, r3; addi r3, r11, 4; addi r1, r1, 16; blr',
                 'stwu r1, -16(r1); addi r3, r3, 4; addi r1, r1, 16; blr')

    def test_unknown_call_outputs(self):
        self.no('bl foo; mflr r3; blr', 'bl foo; mflr r3; blr')
        self.no('bl foo; mr r3, r11; blr', 'bl foo; mr r3, r11; blr')

    def test_valid_call_copy(self):
        self.yes('mflr r12; stw r12, 4(r1); mr r11, r3; mr r3, r11; bl foo; lwz r12, 4(r1); mtlr r12; blr',
                 'mflr r12; stw r12, 4(r1); bl foo; lwz r12, 4(r1); mtlr r12; blr')

    def test_paired_load_reorder(self):
        self.yes('psq_l f0, 0(r3), 0, qr0; psq_l f9, 8(r3), 1, qr0; psq_st f0, 0(r4), 0, qr0; psq_st f9, 8(r4), 1, qr0; blr',
                 'psq_l f9, 8(r3), 1, qr0; psq_l f0, 0(r3), 0, qr0; psq_st f0, 0(r4), 0, qr0; psq_st f9, 8(r4), 1, qr0; blr')

    def test_paired_width_alias(self):
        self.no('psq_st f0, 0(r3), 0, qr0; lwz r3, 4(r3); blr',
                'lwz r11, 4(r3); psq_st f0, 0(r3), 0, qr0; mr r3, r11; blr')
        self.yes('psq_st f0, 0(r3), 1, qr0; lwz r3, 4(r3); blr',
                 'lwz r11, 4(r3); psq_st f0, 0(r3), 1, qr0; mr r3, r11; blr')

    def test_paired_gqr_and_mode(self):
        self.no('psq_l f0, 0(r3), 0, qr0; blr', 'psq_l f0, 0(r3), 0, qr1; blr')
        self.no('psq_st f0, 0(r3), 0, qr0; blr', 'psq_st f0, 0(r3), 1, qr0; blr')

    def test_paired_update(self):
        self.yes('psq_lu f0, 8(r3), 0, qr0; mr r11, r3; addi r3, r11, 4; blr',
                 'psq_lu f0, 8(r3), 0, qr0; addi r3, r3, 4; blr')
        self.yes('psq_stu f0, 8(r3), 0, qr0; mr r11, r3; addi r3, r11, 4; blr',
                 'psq_stu f0, 8(r3), 0, qr0; addi r3, r3, 4; blr')

    def test_alignment_is_not_unknown_instruction(self):
        self.assertEqual(machine_lines([{'instruction': None}, {'instruction': {}}], ['', '']), [''])
        self.no(';'.join(machine_lines([{'instruction': {}}], [''])), 'blr')

    def test_redundant_field_load(self):
        self.yes('lwz r11, 0(r3); lwz r12, 0(r3); add r3, r11, r12; blr',
                 'lwz r11, 0(r3); add r3, r11, r11; blr')
        self.yes('lfs f0, 0(r3); lfs f9, 0(r3); fadds f1, f0, f9; blr',
                 'lfs f0, 0(r3); fadds f1, f0, f0; blr')
        self.yes('psq_l f0, 0(r3), 0, qr0; psq_l f9, 0(r3), 0, qr0; psq_st f9, 0(r4), 0, qr0; blr',
                 'psq_l f0, 0(r3), 0, qr0; psq_st f0, 0(r4), 0, qr0; blr')

    def test_redundant_load_disjoint_store(self):
        self.yes('lwz r11, 0(r3); stw r4, 4(r3); lwz r12, 0(r3); add r3, r11, r12; blr',
                 'lwz r11, 0(r3); stw r4, 4(r3); add r3, r11, r11; blr')

    def test_redundant_load_unknown_store(self):
        self.no('lwz r11, 0(r3); stw r4, 8(r5); lwz r12, 0(r3); add r3, r11, r12; blr',
                'lwz r11, 0(r3); stw r4, 8(r5); add r3, r11, r11; blr')

    def test_redundant_load_call(self):
        before = 'mflr r0; stw r0, 4(r1); mr r12, r3; lwz r11, 0(r3); stw r11, 8(r1); stw r12, 12(r1); bl foo; lwz r12, 12(r1); lwz r11, 8(r1); '
        after = '; lwz r0, 4(r1); mtlr r0; blr'
        self.no(before + 'lwz r3, 0(r12)' + after,
                before + 'mr r3, r11' + after)

    def test_store_word_forward(self):
        self.yes('stw r4, 0(r3); lwz r3, 0(r3); blr',
                 'stw r4, 0(r3); mr r3, r4; blr')

    def test_forward_unknown_store(self):
        self.no('stw r4, 0(r3); stw r6, 0(r5); lwz r3, 0(r3); blr',
                'stw r4, 0(r3); stw r6, 0(r5); mr r3, r4; blr')

    def test_forward_call(self):
        before = 'mflr r0; stw r0, 4(r1); stw r3, 8(r1); stw r4, 12(r1); bl foo; '
        after = '; lwz r0, 4(r1); mtlr r0; blr'
        self.no(before + 'lwz r3, 12(r1)' + after,
                before + 'lwz r3, 8(r1)' + after)

    def test_no_width_sign_extension_forward(self):
        for store, load in [('stw', 'lbz'), ('stw', 'lhz'), ('stw', 'lha'),
                            ('sth', 'lhz'), ('sth', 'lha'), ('stb', 'lbz')]:
            self.no(f'{store} r4, 0(r3); {load} r3, 0(r3); blr',
                    f'{store} r4, 0(r3); mr r3, r4; blr')
        self.no('lwz r11, 0(r3); lha r3, 0(r3); blr',
                'lwz r11, 0(r3); mr r3, r11; blr')
        self.no('lha r11, 0(r3); lhz r3, 0(r3); blr',
                'lha r11, 0(r3); mr r3, r11; blr')

    def test_no_float_narrowing_forward(self):
        self.no('stfs f1, 0(r3); lfs f1, 0(r3); blr',
                'stfs f1, 0(r3); blr')
        self.no('stfd f1, 0(r3); lfs f1, 0(r3); blr',
                'stfd f1, 0(r3); blr')

    def test_private_stack_spill(self):
        self.yes('stwu r1, -32(r1); addi r11, r3, 4; stw r11, 8(r1); lwz r3, 8(r1); addi r1, r1, 32; blr',
                 'stwu r1, -32(r1); addi r3, r3, 4; addi r1, r1, 32; blr')

    def test_private_spill_unknown_store(self):
        self.no('stwu r1, -32(r1); stw r4, 8(r1); stw r4, 0(r5); lwz r3, 8(r1); addi r1, r1, 32; blr',
                'stwu r1, -32(r1); stw r4, 0(r5); mr r3, r4; addi r1, r1, 32; blr')

    def test_private_spill_unknown_observer(self):
        self.no('stwu r1, -32(r1); stw r4, 8(r1); lwz r11, 0(r5); lwz r3, 8(r1); addi r1, r1, 32; blr',
                'stwu r1, -32(r1); lwz r11, 0(r5); mr r3, r4; addi r1, r1, 32; blr')

    def test_private_spill_outside_frame(self):
        self.no('stwu r1, -32(r1); stw r4, 36(r1); lwz r3, 36(r1); addi r1, r1, 32; blr',
                'stwu r1, -32(r1); mr r3, r4; addi r1, r1, 32; blr')

    def test_private_spill_address_escape(self):
        self.no('stwu r1, -32(r1); addi r5, r1, 8; stw r4, 8(r1); lwz r3, 8(r1); addi r1, r1, 32; blr',
                'stwu r1, -32(r1); addi r5, r1, 8; mr r3, r4; addi r1, r1, 32; blr')

    def test_private_spill_wrong_type(self):
        self.no('stwu r1, -32(r1); stw r4, 8(r1); lha r3, 8(r1); addi r1, r1, 32; blr',
                'stwu r1, -32(r1); mr r3, r4; addi r1, r1, 32; blr')

    def test_runtime_helper_abi_is_unknown(self):
        self.no('bl _savegpr_29; blr', 'bl _savegpr_29; blr')
        self.no('bl _savefpr_29; blr', 'bl _savefpr_29; blr')

    def test_private_spill_call_blocks_deletion(self):
        self.no('stwu r1, -32(r1); mflr r0; stw r0, 36(r1); stw r4, 8(r1); bl foo; lwz r3, 8(r1); lwz r0, 36(r1); mtlr r0; addi r1, r1, 32; blr',
                'stwu r1, -32(r1); mflr r0; stw r0, 36(r1); bl foo; lwz r3, 8(r1); lwz r0, 36(r1); mtlr r0; addi r1, r1, 32; blr')

    def test_effect_multiplicity(self):
        self.no('stw r4, 0(r3); stw r4, 0(r3); blr', 'stw r4, 0(r3); blr')

class PairedLaneGuardTests(unittest.TestCase):
    def rejected(self, text):
        for checker in (checked_equal, checked_equal_mem):
            ok, why = checker(asm(text), asm(text))
            self.assertFalse(ok, (checker.__name__, why))
            self.assertIn('PS0-only', why)

    def accepted(self, text):
        for checker in (checked_equal, checked_equal_mem):
            ok, why = checker(asm(text), asm(text))
            self.assertTrue(ok, (checker.__name__, why))

    def test_fmr_pair_read(self):
        self.rejected('fmr f0, f1; psq_st f0, 0(r3), 0, qr0; blr')

    def test_lfd_pair_read(self):
        self.rejected('lfd f0, 0(r3); psq_st f0, 0(r4), 0, qr0; blr')

    def test_lfdu_pair_read(self):
        self.rejected('lfdu f0, 8(r3); psq_st f0, 0(r4), 0, qr0; blr')

    def test_eliminated_copy_chain(self):
        self.rejected('fmr f0, f1; fmr f9, f0; psq_st f9, 0(r3), 0, qr0; blr')

    def test_partial_write_on_one_predecessor(self):
        self.rejected('cmpwi r3, 0; beq 0x10; fmr f0, f1; b 0x14; lfs f0, 0(r3); psq_st f0, 0(r4), 0, qr0; blr')

    def test_complete_redefinition_on_all_paths(self):
        self.accepted('cmpwi r3, 0; beq 0x10; fmr f0, f1; b 0x14; lfs f0, 0(r3); psq_l f0, 0(r3), 0, qr0; psq_st f0, 0(r4), 0, qr0; blr')

    def test_lfs_full_redefinition(self):
        self.accepted('fmr f0, f1; lfs f0, 0(r3); psq_st f0, 0(r4), 0, qr0; blr')

    def test_psq_full_redefinition(self):
        self.accepted('lfd f0, 0(r3); psq_l f0, 0(r3), 0, qr0; psq_st f0, 0(r4), 0, qr0; blr')


class LiteralAndCommutativitySafetyTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        import ast
        import collections
        import re
        import struct
        from pathlib import Path
        path = Path(__file__).with_name('rdclass.py')
        tree = ast.parse(path.read_text())
        functions = {'rd', 'dol_bss_ranges', 'litonly', 'rebind_equal', 'fixed'}
        constants = {'LIT', 'TOK', 'REG', 'COMM', 'NODEF', 'VOL'}
        selected = [n for n in tree.body if
                    isinstance(n, ast.FunctionDef) and n.name in functions or
                    isinstance(n, ast.Assign) and any(isinstance(t, ast.Name) and t.id in constants for t in n.targets)]
        cls.api = {'re': re, 'struct': struct, 'collections': collections}
        exec(compile(ast.Module(body=selected, type_ignores=[]), str(path), 'exec'), cls.api)

    def test_fp_add_sources_are_ordered_in_rebind(self):
        for op in ('fadd', 'fadds'):
            ok, _ = self.api['rebind_equal']([f'{op} f1, f1, f9', 'blr'],
                                            [f'{op} f1, f9, f1', 'blr'])
            self.assertFalse(ok)
            for checker in (checked_equal, checked_equal_mem):
                self.assertFalse(checker([f'{op} f1, f1, f9', 'blr'],
                                         [f'{op} f1, f9, f1', 'blr'])[0])

    def test_integer_commutative_sources_remain_allowed(self):
        self.assertTrue(self.api['rebind_equal'](['add r3, r3, r4', 'blr'],
                                                ['add r3, r4, r3', 'blr'])[0])

    def reader(self, ranges=()):
        self.api.update(dol=b'abcdefghijklmnop', offs=[4], addrs=[0x1000], sizes=[8],
                        bss_ranges=list(ranges))
        return self.api['rd']

    def test_unmapped_dol_address_is_unresolved(self):
        rd = self.reader()
        self.assertIsNone(rd(0x2000, 4))
        self.assertEqual(self.api['litonly']('lfs f0, lbl_00002000@sda21', lambda _: rd(0x2000, 4)),
                         'lfs f0, lbl_00002000@sda21')

    def test_file_section_bytes_and_bounds(self):
        rd = self.reader()
        self.assertEqual(rd(0x1000, 4), b'efgh')
        self.assertIsNone(rd(0x1006, 4))
        self.api['dol'] = b'abcde'
        self.assertIsNone(rd(0x1000, 4))

    def test_known_bss_only_and_span_bounds(self):
        rd = self.reader([(0x2000, 0x2010)])
        self.assertEqual(rd(0x2000, 4), bytes(4))
        self.assertIsNone(rd(0x1ffc, 4))
        self.assertIsNone(rd(0x200e, 4))
        self.assertIsNone(rd(0x2010, 4))

    def test_dol_header_bss_range(self):
        import struct
        header = bytearray(0x100)
        struct.pack_into('>II', header, 0xd8, 0x2000, 0x20)
        self.assertEqual(self.api['dol_bss_ranges'](header), [(0x2000, 0x2020)])


if __name__ == '__main__':
    unittest.main()
