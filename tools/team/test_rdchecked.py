import unittest
from rdchecked import checked_equal, machine_lines


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


if __name__ == '__main__':
    unittest.main()
