"""Synthetic PPC semantic programs, not game instruction payload fixtures."""
import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parent))
import dispatchownership as own


def form(op, rt, ra, imm):
    return op << 26 | rt << 21 | ra << 16 | (imm & 65535)


def mr(destination, source):
    return 31 << 26 | source << 21 | destination << 16 | source << 11 | 444 << 1


def jump(pc, destination, link=False):
    return 18 << 26 | ((destination - pc) & 0x03fffffc) | int(link)


def synthetic_program(scope):
    """Emit a small ISA program from semantic edge declarations, with no game reads."""
    words = {}
    def put(pc, op, rt, ra, imm):
        words[pc] = form(op, rt, ra, imm)
    put(0x80004294, 15, 13, 0, own.R13 >> 16)
    put(0x80004298, 24, 13, 13, own.R13 & 65535)
    contract = own.CONTRACTS[scope]
    by_name = {r['identity']: r for r in contract['chains']}
    for row in contract['chains']:
        arg, call = contract['sinit'] + row['arg'], contract['sinit'] + row['call']
        put(arg, 14, 3, 13, row['singleton'] - own.R13)
        for pc in range(arg + 4, call, 4):
            put(pc, 36, 0, 1, 20)
        words[call] = jump(call, row['ctor'], True)
        put(row['ctor'], 15, 4, 0, (row['vtable'] + 32768) >> 16)
        put(row['ctor'] + 4, 14, 4, 4, row['vtable'] & 65535)
        put(row['ctor'] + 8, 36, 4, 3, 0)
        words[row['ctor'] + 12] = 0x4e800020
        words[row['vtable'] + 8] = row['wrapper']
        put(row['wrapper'], 32, 3, 4, 0)
        if row['size'] == 12:
            put(row['wrapper'] + 4, 14, 4, 13, by_name[row['select']]['singleton'] - own.R13)
        pc = row['wrapper'] + row['size'] - 4
        words[pc] = jump(pc, row['target'])
    for method, start, arg, call, identity in contract['selectors']:
        put(start + arg, 14, 4, 13, by_name[identity]['singleton'] - own.R13)
        for pc in range(start + arg + 4, start + call, 4):
            put(pc, 36, 0, 1, 20)
        pc = start + call
        words[pc] = jump(pc, own.INIT_NERVE if method == 'init' else own.SET_NERVE, True)
    for pc, op, rt, ra, immediate in [
        (0x804c4cc0, 32, 3, 3, 4), (0x8024327c, 36, 31, 30, 8),
        (0x802432fc, 32, 3, 31, 8), (0x80243300, 14, 0, 0, 0),
        (0x80243304, 36, 3, 31, 4), (0x80243308, 36, 0, 31, 8),
        (0x8024330c, 36, 0, 31, 12), (0x802431e8, 32, 3, 31, 4),
        (0x802431f0, 32, 12, 3, 0), (0x802431f4, 32, 12, 12, 8),
        (0x804c4c80, 36, 3, 29, 4), (0x80243160, 14, 0, 0, 0),
        (0x80243174, 36, 4, 3, 0), (0x80243178, 36, 0, 3, 4),
        (0x8024317c, 36, 5, 3, 8), (0x80243180, 36, 0, 3, 12)]:
        put(pc, op, rt, ra, immediate)
    for pc, destination, source in [
        (0x80243240, 31, 4), (0x80243248, 30, 3), (0x802432d0, 31, 3),
        (0x802431e0, 31, 3), (0x802431ec, 4, 31), (0x804c4c54, 29, 3),
        (0x804c4c58, 30, 4), (0x804c4c5c, 31, 5), (0x804c4c70, 4, 29),
        (0x804c4c74, 5, 30), (0x804c4c78, 6, 31)]:
        words[pc] = mr(destination, source)
    for pc, target, link in [(0x804c4cc4, 0x80243230, False),
                              (0x802431e4, 0x802432c0, True),
                              (0x804c4c7c, 0x80243150, True)]:
        words[pc] = jump(pc, target, link)
    words[0x802431f8] = 0x7d8903a6
    words[0x802431fc] = 0x4e800421
    if scope == 'TU14':
        words[0x804f97c8] = mr(31, 3)
        put(0x804f97d0, 14, 0, 0, 1)
        put(0x804f97d4, 38, 0, 31, 8)
    return words


class OwnershipTests(unittest.TestCase):
    def decode(self, words, scope='TU13'):
        return own.decode_original_words(words.__getitem__, scope)

    def test_complete_tu13(self):
        result = self.decode(synthetic_program('TU13'))
        self.assertEqual(len(result['chains']), 3)
        self.assertEqual(result['Spine']['Spine_argument_register'], 4)
        self.assertFalse(result['production_acceptance'])

    def test_complete_tu14(self):
        result = self.decode(synthetic_program('TU14'), 'TU14')
        self.assertEqual([r['size'] for r in result['chains']], [12, 8])
        self.assertFalse(result['production_acceptance'])

    def test_wrong_executor_base_register(self):
        words = synthetic_program('TU13')
        words[0x804f9730] = form(32, 3, 3, 0)
        with self.assertRaisesRegex(ValueError, 'register roles'):
            self.decode(words)

    def test_every_chain_selector_constructor_slot_and_wrapper_rejects(self):
        for scope in ('TU13', 'TU14'):
            contract = own.CONTRACTS[scope]
            for row in contract['chains']:
                mutations = {
                    'sinit_arg': (contract['sinit'] + row['arg'], form(14, 3, 13, 0)),
                    'ctor_call': (contract['sinit'] + row['call'], jump(contract['sinit'] + row['call'], row['ctor'] + 16, True)),
                    'ctor_store': (row['ctor'] + 8, form(36, 4, 3, 4)),
                    'ctor_address': (row['ctor'] + 4, form(14, 4, 4, 0)),
                    'vtable': (row['vtable'] + 8, row['wrapper'] + 16),
                    'wrapper': (row['wrapper'] + row['size'] - 4, jump(row['wrapper'] + row['size'] - 4, row['target'] + 4)),
                }
                for label, (pc, instruction) in mutations.items():
                    with self.subTest(scope=scope, owner=row['identity'], mutation=label):
                        words = synthetic_program(scope)
                        words[pc] = instruction
                        with self.assertRaises(ValueError):
                            self.decode(words, scope)

    def test_old_tu14_packet_mislabels_addi_as_bl(self):
        words = synthetic_program('TU14')
        for pc, ctor in ((0x804f9978, 0x804f99a0), (0x804f9984, 0x804f99b0)):
            with self.subTest(pc=pc):
                bad = words.copy(); bad[pc] = jump(pc, ctor, True)
                with self.assertRaisesRegex(ValueError, 'opcode/register'):
                    self.decode(bad, 'TU14')

    def test_dead_flag_is_required(self):
        words = synthetic_program('TU14')
        words[0x804f97d0] = form(14, 0, 0, 0)
        with self.assertRaisesRegex(ValueError, 'immediate'):
            self.decode(words, 'TU14')

    def test_zoomout_must_select_distinct_wait(self):
        words = synthetic_program('TU14')
        words[0x804f99d4] = form(14, 4, 13, 0x807d5fe0 - own.R13)
        with self.assertRaisesRegex(ValueError, 'selected singleton'):
            self.decode(words, 'TU14')

    def test_all_producer_selectors(self):
        for scope in ('TU13', 'TU14'):
            for method, start, arg, call, _ in own.CONTRACTS[scope]['selectors']:
                with self.subTest(scope=scope, method=method):
                    words = synthetic_program(scope)
                    words[start + arg] = form(14, 4, 13, 0)
                    with self.assertRaisesRegex(ValueError, 'selected singleton'):
                        self.decode(words, scope)

    def test_argument_cannot_be_overwritten_in_prologue(self):
        words = synthetic_program('TU13')
        words[0x804f93ac] = form(14, 4, 0, 0)
        with self.assertRaisesRegex(ValueError, 'argument overwritten'):
            self.decode(words)

    def test_unknown_instruction_in_argument_gap(self):
        words = synthetic_program('TU13')
        words[0x804f93ac] = 0
        with self.assertRaisesRegex(ValueError, 'unqualified'):
            self.decode(words)

    def test_spine_field_and_virtual_slot_controls(self):
        for pc, replacement in [(0x802431f4, form(32, 12, 12, 12)),
                                (0x80243304, form(36, 3, 31, 8)),
                                (0x8024327c, form(36, 31, 30, 4)),
                                (0x804c4c74, mr(5, 29)),
                                (0x8024317c, form(36, 4, 3, 8)),
                                (0x802431fc, 0x4e800020)]:
            with self.subTest(pc=pc):
                words = synthetic_program('TU13'); words[pc] = replacement
                with self.assertRaises(ValueError):
                    self.decode(words)

    def test_bootstrap_requires_ori_and_correct_base(self):
        for word in (form(14, 13, 13, own.R13 & 65535), form(24, 13, 13, 0)):
            words = synthetic_program('TU13'); words[0x80004298] = word
            with self.assertRaises(ValueError):
                self.decode(words)

    def test_branch_link_roles(self):
        pc = 0x804f96b0
        words = synthetic_program('TU13'); words[pc] = jump(pc, 0x804f96e0)
        with self.assertRaisesRegex(ValueError, 'link'):
            self.decode(words)

    def test_effective_local_alias(self):
        view = own.ObjectView.__new__(own.ObjectView)
        view.keys = ['#0', '.text#0']
        view.sections = [{}, {}]
        view.symbols = [{'name': 'target', 'other': 0, 'shndx': 1, 'value': 32}]
        self.assertTrue(view.effective_target(('local', '.text#0', 32), 'target'))
        self.assertFalse(view.effective_target(('local', '.text#0', 36), 'target'))
        self.assertFalse(view.effective_target(('local', '.text#1', 32), 'target'))


if __name__ == '__main__':
    unittest.main()
