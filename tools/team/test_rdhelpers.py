"""Fail-closed standard integer Runtime helper summaries."""
import struct
import tempfile
import unittest
from pathlib import Path
from rdchecked import (checked_equal, checked_equal_mem, blocks, lower_gpr_helpers,
                       gpr_helper, validate_gpr_runtime, Unproved)


def asm(text):
    return [x.strip() for x in text.split(';') if x.strip()]


def function(body='', first=29, restore=None, frame=96):
    restore = first if restore is None else restore
    return asm(f'stwu r1, -{frame}(r1); mflr r0; stw r0, {frame+4}(r1); '
               f'addi r11, r1, {frame}; bl _savegpr_{first}; {body}; '
               f'addi r11, r1, {frame}; bl _restgpr_{restore}; '
               f'lwz r0, {frame+4}(r1); mtlr r0; addi r1, r1, {frame}; blr')


class GprHelperTests(unittest.TestCase):
    def yes(self, left, right):
        for checker in (checked_equal, checked_equal_mem):
            ok, why = checker(left, right)
            self.assertTrue(ok, (checker.__name__, why))

    def no(self, left, right=None):
        for checker in (checked_equal, checked_equal_mem):
            ok, why = checker(left, left if right is None else right)
            self.assertFalse(ok, (checker.__name__, why))

    def test_actual_runtime_and_all_36_entry_effects(self):
        validate_gpr_runtime()
        for n in range(14, 32):
            graph = lower_gpr_helpers(blocks(function(first=n)))
            body = graph[0][0]
            stores = [d for d in body if d['kind'] == 'store' and d['args'][0] != 'r0'
                      and d['op'] != 'stwu']
            loads = [d for d in body if d['kind'] == 'load' and d['args'][0] != 'r0']
            self.assertEqual([(d['args'][0], d['offset']) for d in stores],
                             [(f'r{i}', 96 - 4 * (32-i)) for i in range(n, 32)])
            self.assertEqual([(d['args'][0], d['offset']) for d in loads],
                             [(f'r{i}', 96 - 4 * (32-i)) for i in range(n, 32)])
            self.yes(function(first=n), function(first=n))

    def test_independent_volatile_return_and_cr_preserved(self):
        left = function('li r3, 7; cmpwi r4, 0')
        right = left.copy()
        a, b = right.index('bl _savegpr_29'), right.index('cmpwi r4, 0')
        compare, result = right.pop(b), right.pop(b-1)
        right[a:a] = [compare, result]
        self.yes(left, right)

    def test_independent_frame_store_scheduling(self):
        left = function('stw r3, 8(r1)')
        right = left.copy()
        right.remove('stw r3, 8(r1)')
        right.insert(right.index('bl _savegpr_29'), 'stw r3, 8(r1)')
        self.yes(left, right)

    def test_actual_helper_preserves_r4_and_fp_result(self):
        self.yes(function('fmr f1, f2; li r4, 9'),
                 function('li r4, 9; fmr f1, f2'))

    def test_wrong_return_value_and_nonvolatile_value(self):
        self.no(function('li r3, 1'), function('li r3, 2'))
        self.no(function('li r4, 1'), function('li r4, 2'))
        bad = function()
        bad.insert(-1, 'li r31, 8')
        self.no(bad)

    def test_changed_save_value_rejected(self):
        bad = function()
        bad.insert(bad.index('bl _savegpr_29'), 'mr r29, r3')
        self.no(bad)

    def test_wrong_slot_contents_width_or_offset(self):
        for store in ('stw r3, 84(r1)', 'stb r3, 85(r1)', 'stfd f0, 80(r1)'):
            self.no(function(store))
        self.no(function(restore=30))
        self.no(function(restore=28))
        bad = function()
        bad[bad.index('addi r11, r1, 96')] = 'addi r11, r1, 92'
        self.no(bad)

    def test_unknown_alias_store_invalidates_restoration(self):
        self.no(function('stw r3, 0(r4)'))
        left = function('lwz r3, 0(r4)')
        right = left.copy()
        right.remove('lwz r3, 0(r4)')
        right.insert(right.index('bl _savegpr_29'), 'lwz r3, 0(r4)')
        self.no(left, right)

    def test_unknown_missing_and_out_of_frame_r11(self):
        self.no(asm('bl _savegpr_29; blr'))
        for replacement in ('mr r11, r3', 'li r11, 96', 'addi r11, r1, 8',
                            'addi r11, r1, 112', 'add r11, r1, r3'):
            bad = function()
            bad[bad.index('addi r11, r1, 96')] = replacement
            self.no(bad)

    def test_frame_escape_by_store_argument_return_or_backchain(self):
        for body in ('stw r1, 8(r1)', 'addi r3, r1, 8; bl ordinary',
                     'mr r3, r1', 'lwz r3, 0(r1); bl ordinary'):
            self.no(function(body))

    def test_malformed_names_aliases_and_other_helper_families(self):
        for name in ('_savegpr_13', '_savegpr_32', '_savegpr_029', '_savegpr_31_alias',
                     '_savegpr_31+4', '__save_gpr', '__restore_gpr', '_restgpr_31@plt',
                     '_savefpr_31', '_savef31', '_restgpr_14x'):
            with self.assertRaises(Unproved):
                gpr_helper(name)
            bad = function()
            bad[bad.index('bl _savegpr_29')] = 'bl '+name
            self.no(bad)

    def test_missing_pair_lr_restore_or_stack_release(self):
        for remove in ('bl _savegpr_29', 'bl _restgpr_29', 'mtlr r0', 'addi r1, r1, 96'):
            bad = function()
            bad.remove(remove)
            self.no(bad)

    def test_lr_return_address_is_not_equal_across_binaries(self):
        self.no(function('mflr r3'))

    def test_wrong_lr_stack_value(self):
        self.no(function('stw r3, 100(r1)'))

    def test_cr_is_not_clobbered_or_ignored(self):
        self.no(function('cmpwi cr2, r3, 0'), function('cmpwi cr2, r3, 1'))
        left = function('cmpwi r3, 0')
        right = function('cmpwi r3, 1')
        self.no(left, right)

    def test_no_blr_helper_tail_or_unallocated_frame(self):
        self.no(asm('addi r11, r1, 96; bl _savegpr_29; bl _restgpr_29; blr'))
        self.no(function(frame=24))
        self.no(function()+asm('bl _savegpr_29; blr'))

    def test_forward_branch_pairing(self):
        left = function('cmpwi r3, 0; beq 0x20; li r3, 1')
        self.yes(left, left)
        bad = function('cmpwi r3, 0; beq 0x24; stw r3, 84(r1)')
        self.no(bad)


    def test_ordinary_call_is_still_a_memory_barrier(self):
        left = function('bl ordinary; mr r3, r31')
        right = function('bl ordinary')
        right.insert(right.index('bl _restgpr_29')+1, 'mr r3, r31')
        self.no(left, right)

    def test_extra_helper_store_and_changed_memory_observer_are_not_skipped(self):
        self.no(function('lwz r3, 84(r1)'), function('lwz r3, 88(r1)'))
        self.no(function(first=29), function(first=30))

    def test_cr0_branch_across_helper_preserves_predicate(self):
        left = function()
        i = left.index('bl _savegpr_29')
        left.insert(i, 'cmpwi r3, 0')
        # Forward conditional skips only a pure temporary definition.
        j = left.index('addi r11, r1, 96', i+2)
        left[j:j] = [f'beq {4*(j+2)}', 'li r12, 9']
        self.yes(left, left)
        right = left.copy()
        right[right.index('cmpwi r3, 0')] = 'cmpwi r4, 0'
        self.no(left, right)

    def test_runtime_validation_rejects_wrong_body_or_mapping(self):
        root = Path(__file__).resolve().parents[2]
        original = root/'orig/SB4E01/sys/main.dol'
        symbols = root/'config/SB4E01/symbols.txt'
        with tempfile.TemporaryDirectory() as temp:
            tmp = Path(temp)
            dp = tmp/'orig/SB4E01/sys/main.dol'
            sp = tmp/'config/SB4E01/symbols.txt'
            dp.parent.mkdir(parents=True)
            sp.parent.mkdir(parents=True)
            sp.write_text(symbols.read_text().replace('_savegpr_14 = .text:0x8062E130',
                                                     '_savegpr_14 = .text:0x8062E134'))
            dp.write_bytes(original.read_bytes())
            with self.assertRaises(Unproved):
                validate_gpr_runtime(tmp)
            sp.write_text(symbols.read_text())
            data = bytearray(original.read_bytes())
            for i in range(18):
                off = struct.unpack_from('>I', data, i*4)[0]
                address = struct.unpack_from('>I', data, 0x48+i*4)[0]
                size = struct.unpack_from('>I', data, 0x90+i*4)[0]
                if address <= 0x8062e174 < address+size:
                    data[off+0x8062e174-address+3] ^= 4
                    break
            dp.write_bytes(data)
            with self.assertRaises(Unproved):
                validate_gpr_runtime(tmp)
            dp.write_bytes(original.read_bytes())
            validate_gpr_runtime(tmp)
            import os
            stamp = dp.stat()
            data = bytearray(original.read_bytes())
            for i in range(18):
                off = struct.unpack_from('>I', data, i*4)[0]
                address = struct.unpack_from('>I', data, 0x48+i*4)[0]
                size = struct.unpack_from('>I', data, 0x90+i*4)[0]
                if address <= 0x8062e178 < address+size:
                    data[off+0x8062e178-address:off+0x8062e178-address+4] = bytes.fromhex('60000000')
                    break
            dp.write_bytes(data)
            os.utime(dp, ns=(stamp.st_atime_ns, stamp.st_mtime_ns))
            with self.assertRaises(Unproved):
                validate_gpr_runtime(tmp)


if __name__ == '__main__':
    unittest.main()
