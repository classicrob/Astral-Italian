"""Adversarial byte comparisons for datacontent (stdlib, no game build needed).

Run with: python3 tools/team/test_datacontent.py

The JetTurtle table contains three float[3] arrays, a u16[3] array and padding.
These real target/old bits expose four stale entries despite an equal first slot.
Direct-load cases pass only bytes actually read; indexed-array cases pass the
whole array. Object discovery and load-width inference require separate CLI tests.
"""

import struct
from types import SimpleNamespace
import unittest

from datacontent import Relocation, Symbol, compare_bytes, instruction_alignment, trace


JET_TARGET = bytes.fromhex(
    "41f00000 41f00000 41f00000"  # Straight: 30, 30, 30
    "41c00000 41c00000 41c00000"  # Homing: 24, 24, 24
    "3b03126f 3b03126f 3bb43958"  # Gravity: .002, .002, .0055
    "012c 01e0 012c 000000000000"  # Reset steps: 300, 480, 300; padding
)
JET_OLD = bytes.fromhex(
    "41f00000 41a00000 41f00000"  # Straight[1]: 20
    "41c00000 41a00000 41c00000"  # Homing[1]: 20
    "3c8b4396 3c23d70a 3bb43958"  # Gravity[0:2]: .017, .01
    "012c 01e0 012c 000000000000"
)
POINTER_TARGET = bytes.fromhex("803046c0 41f00000")
POINTER_SOURCE = bytes.fromhex("00000000 41f00000")
TARGET_POINTERS = {0: {"symbol": "fn_803046C0", "status": "unresolved"}}
SOURCE_POINTERS = {0: {"symbol": "unlinked_external", "status": "unresolved"}}


class InstructionFixture:
    """Only the ELF fields needed to test a small, explicit PPC control flow."""

    def __init__(self, redefine_base):
        self.table = Symbol("table", 0, 12, 2, 1, 0)
        callee = Symbol("callee", 0, 0, 0, 2, 1)
        words = [
            0x3BE00000,  # addi r31,r0,table@l
            0x2C030000,  # cmpwi r3,0
            0x4182000C,  # beq +12 (alternate path)
            0x3BE00000 if redefine_base else 0x48000001,  # li r31,0 / bl callee
            0x48000008,  # b +8 (join)
            0x60000000,  # nop (alternate path preserves r31)
            (31 << 26) | (1 << 21) | (31 << 16) | (4 << 11) | (535 << 1),  # lfsx f1,r31,r4
            0x4E800020,  # blr
        ]
        self.contents = [b"", struct.pack(">8I", *words), bytes(12)]
        self.sections = [(0,) * 10, (0, 1, 6, 0, 0, 32, 0, 0, 4, 0),
                         (0, 1, 2, 0, 0, 12, 0, 0, 4, 0)]
        self.function = Symbol("fixture", 0, 32, 1, 2, 1)
        self.relocations = {(1, 0): Relocation(4, self.table, 0)}
        if not redefine_base:
            self.relocations[1, 12] = Relocation(10, callee, 0)

    def instruction_relocation(self, section, address):
        return self.relocations.get((section, address))

    def references(self):
        world = SimpleNamespace(is_data=lambda elf, pointer: pointer.relocation.symbol == self.table)
        return trace(self, self.function, "fixture", world)


def offset(value):
    return int(value, 0) if isinstance(value, str) else value


class DataContentTests(unittest.TestCase):
    def assert_equal(self, target, source):
        result = compare_bytes(target, source)
        self.assertEqual(result["status"], "equal")
        self.assertEqual(result["scalar_differences"], [])
        return result

    def assert_different(self, target, source, changed_words=(), **kwargs):
        result = compare_bytes(target, source, **kwargs)
        self.assertNotEqual(result["status"], "equal")
        self.assertTrue(result["scalar_differences"], "changed scalar/string bytes were missed")
        covered = set()
        for difference in result["scalar_differences"]:
            at = offset(difference["target_offset"])
            source_at = offset(difference["source_offset"])
            target_bytes = bytes.fromhex(difference["target_hex"])
            source_bytes = bytes.fromhex(difference["source_hex"])
            self.assertEqual(target_bytes, target[at:at + len(target_bytes)])
            self.assertEqual(source_bytes, source[source_at:source_at + len(source_bytes)])
            self.assertNotEqual(target_bytes, source_bytes)
            covered.update(range(at, at + len(target_bytes)))
        changed_bytes = {i for i, (a, b) in enumerate(zip(target, source)) if a != b}
        for start in changed_words:
            self.assertTrue(covered & changed_bytes & set(range(start, start + 4)),
                            f"changed word at {start:#x} was missed")
        return result, covered

    def test_real_jet_table_flags_all_four_stale_values(self):
        self.assertEqual(len(JET_TARGET), 48)
        self.assertEqual(JET_TARGET[:4], JET_OLD[:4])
        self.assert_different(JET_TARGET, JET_OLD, changed_words=(4, 16, 24, 28))

    def test_correct_jet_table_is_equal(self):
        self.assert_equal(JET_TARGET, bytes.fromhex(JET_TARGET.hex()))

    def test_indexed_array_flags_later_slot_with_equal_first_slot(self):
        self.assert_different(JET_TARGET[:12], JET_OLD[:12], changed_words=(4,))

    def test_string_suffix_matters_after_equal_first_four_bytes(self):
        target, source = b"SpinBlurRed\0", b"SpinBlurGold\0"
        self.assertEqual(target[:4], source[:4])
        _, covered = self.assert_different(target, source)
        self.assertTrue(covered & set(range(8, len(target))))

    def test_direct_load_ignores_unread_array_tail(self):
        # Caller extracted one lfs operand; an indexed address needs the full array.
        self.assert_equal(JET_TARGET[:4], JET_OLD[:4])

    def test_direct_load_flags_changed_read_slot(self):
        self.assert_different(JET_TARGET[4:8], JET_OLD[4:8], changed_words=(0,))

    def test_unresolved_pointer_bytes_are_annotated_not_scalar_differences(self):
        result = compare_bytes(POINTER_TARGET, POINTER_SOURCE,
                               target_pointers=TARGET_POINTERS, source_pointers=SOURCE_POINTERS)
        self.assertEqual(result["status"], "pointer_unresolved")
        self.assertEqual(result["scalar_differences"], [])
        self.assertTrue(result["pointer_annotations"])

    def test_pointer_mask_does_not_hide_adjacent_changed_scalar(self):
        source = bytes.fromhex("00000000 41a00000")
        result, covered = self.assert_different(
            POINTER_TARGET, source, changed_words=(4,),
            target_pointers=TARGET_POINTERS, source_pointers=SOURCE_POINTERS)
        self.assertTrue(result["pointer_annotations"])
        self.assertFalse(covered & set(range(4)), "pointer bytes reported as scalar differences")

    def test_halfword_relocation_does_not_mask_following_scalar(self):
        target = bytes.fromhex("46c0 41f00000")
        source = bytes.fromhex("0000 41a00000")
        pointers = {0: {"relocation_kind": 4, "symbol": "halfword"}}
        result, covered = self.assert_different(target, source, changed_words=(2,),
                                               target_pointers=pointers, source_pointers=pointers)
        self.assertIn(3, covered, "a halfword relocation hid the neighboring scalar byte")
        self.assertTrue(result["advisories"])

    def test_branch_redefinition_invalidates_indexed_base_at_join(self):
        references, issues = InstructionFixture(redefine_base=True).references()
        self.assertFalse(any(r["kind"] == "lfsx" and r["site"] == 24 for r in references))
        self.assertTrue(any(i["site"] == 24 and i["reason"] == "indexed_base_unknown_or_ambiguous"
                            for i in issues))

    def test_nonvolatile_indexed_base_survives_branch_and_call(self):
        fixture = InstructionFixture(redefine_base=False)
        references, issues = fixture.references()
        indexed = [r for r in references if r["kind"] == "lfsx" and r["site"] == 24]
        self.assertEqual(len(indexed), 1)
        self.assertEqual(indexed[0]["pointer"].relocation.symbol, fixture.table)
        self.assertEqual(indexed[0]["width"], 4)
        self.assertTrue(indexed[0]["dynamic_index"])
        self.assertFalse(any(i["site"] == 24 for i in issues))

    def test_inserted_instruction_shifts_relocated_load_pairing(self):
        target = [0x7C0802A6, 0x3C600000, 0xC0230000, 0xEC210072, 0xD0230000, 0x4E800020]
        source = target[:1] + [0x60000000] + target[1:]
        alignment = instruction_alignment(target, source)
        self.assertEqual(alignment[8], 12, "load paired by stale byte offset")
        self.assertNotEqual(alignment[8], 8)

    def test_repeated_instruction_context_stays_unpaired(self):
        repeated = [0x3C600000, 0xC0230000, 0xEC210072]
        target = repeated * 3
        source = [0x60000000] + repeated * 4
        self.assertEqual(instruction_alignment(target, source), {})


if __name__ == "__main__":
    unittest.main()
