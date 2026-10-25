#!/usr/bin/env python3
"""Scope controls for named local copies, without compiling game code."""
import collections
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest

import dispatchcheck as dc


NAME = '__as__8_GXColorFRC8_GXColor'
OWNER = 'Game/Map/ShadowColorSetter'
OTHER = 'Game/Player/MarioActor'
GLOBAL = 0x8014CB40
LOCAL = 0x80268A70


class LocalSymbols(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / 'symbols.txt'
        self.bases = {(OWNER, '.text'): 0x80268900, (OTHER, '.text'): 0x80140000}
        self.ends = {(OWNER, '.text'): 0x80268B00, (OTHER, '.text'): 0x80150000}

    def symbols(self, local=True, extra=''):
        self.path.write_text(
            f'{NAME} = .text:0x{GLOBAL:X}; // type:function size:0x24\n'
            f'{NAME} = .text:0x{LOCAL:X}; // type:function size:0x24'
            + (' scope:local' if local else '') + '\n' + extra)
        return dc.Symbols(self.path, self.bases, self.ends)

    def program(self, symbols):
        p = dc.Prog.__new__(dc.Prog)
        p.sy = symbols
        p.bases, p.ends = self.bases, self.ends
        p.inferred = collections.defaultdict(lambda: collections.defaultdict(list))
        p.local_inferred = collections.defaultdict(lambda: collections.defaultdict(list))
        p.vtable_anchors = {}
        return p

    @staticmethod
    def elf():
        return SimpleNamespace(syms=[dict(name=NAME, bind=2, type=2, sec=1, off=0, size=36)],
                               names=['', '.text'], s=[None, None])

    def exact_calls(self, e, local=LOCAL):
        rows = []
        for unit, at, target in ((OWNER, 0x80268AB0, local), (OTHER, 0x80300000, GLOBAL)):
            instruction = 0x48000001 | ((target - at) & 0x03FFFFFC)
            rows.append((unit, e, {'name': 'caller'}, at, struct.pack('>I', instruction),
                         {0: (dc.R_REL24, 0, 0)}))
        return rows

    def test_local_copy_shadows_global_only_in_owner(self):
        sy = self.symbols()
        self.assertEqual(sy.addr(NAME, OWNER), LOCAL)
        self.assertEqual(sy.addr(NAME, OTHER), GLOBAL)
        self.assertEqual(sy.addr(NAME), GLOBAL)
        self.assertNotIn(NAME, sy.dups)
        self.assertEqual(sy.name_at(LOCAL + 4), NAME + '+0x4')
        p, e = self.program(sy), self.elf()
        p.infer(self.exact_calls(e))
        self.assertEqual(set(p.inferred[NAME]), {GLOBAL})
        self.assertEqual(set(p.local_inferred[OWNER, NAME]), {LOCAL})
        self.assertEqual(p.target(e, OWNER, 0, 0), (LOCAL, 'sym'))
        self.assertEqual(p.target(e, OTHER, 0, 4), (GLOBAL + 4, 'sym'))
        self.assertEqual(self.conflicts(p, e), [])

    def conflicts(self, p, e):
        p.elfs = {OWNER: e}
        p.func_info = {}
        p.exact_functions = lambda: iter(self.exact_calls(e))
        p.dol_chains = lambda: {}
        p.dol_ctor_vtables = lambda: {}
        p.anchor_vtables = lambda: None
        return p.run(layers=())[0]

    def test_nonlocal_duplicate_still_conflicts(self):
        sy = self.symbols(local=False)
        p, e = self.program(sy), self.elf()
        p.infer(self.exact_calls(e))
        self.assertIn(NAME, sy.dups)
        self.assertIsNone(sy.addr(NAME, OWNER))
        self.assertEqual(set(p.inferred[NAME]), {GLOBAL, LOCAL})
        self.assertEqual(p.target(e, OWNER, 0, 0), (None, 'inferred_conflict'))
        self.assertEqual(p.target(e, OTHER, 0, 0), (None, 'inferred_conflict'))
        rows = self.conflicts(p, e)
        self.assertEqual([(r['function'], r['verdict']) for r in rows], [(NAME, 'CONFLICT')])

    def test_local_duplicate_does_not_hide_two_global_definitions(self):
        sy = self.symbols(extra=f'{NAME} = .text:0x80300000; // type:function size:0x24\n')
        self.assertEqual(sy.addr(NAME, OWNER), LOCAL)
        self.assertIsNone(sy.addr(NAME, OTHER))
        self.assertIn(NAME, sy.dups)

    def test_two_local_definitions_in_one_owner_are_ambiguous(self):
        sy = self.symbols(extra=f'{NAME} = .text:0x80268AA0; // type:function size:0x24 scope:local\n')
        self.assertIsNone(sy.addr(NAME, OWNER))
        self.assertEqual(sy.addr(NAME, OTHER), GLOBAL)

    def test_unique_local_anchor_retains_existing_behavior(self):
        self.path.write_text('helper = .text:0x80268A70; // type:function size:0x24 scope:local\n')
        sy = dc.Symbols(self.path, self.bases, self.ends)
        self.assertEqual(sy.addr('helper'), LOCAL)
        self.assertEqual(sy.addr('helper', OWNER), LOCAL)

    def test_sibling_splits_are_loaded_by_standalone_symbols_users(self):
        self.symbols()
        (self.path.parent / 'splits.txt').write_text(
            f'{OWNER}.cpp:\n\t.text start:0x80268900 end:0x80268B00\n')
        sy = dc.Symbols(self.path)
        self.assertEqual(sy.addr(NAME, OWNER), LOCAL)
        self.assertEqual(sy.addr(NAME, OTHER), GLOBAL)

    def test_unowned_local_copy_is_never_promoted_to_global(self):
        sy = self.symbols(extra=f'{NAME} = .text:0x80900000; // type:function size:0x24 scope:local\n')
        self.assertEqual(sy.addr(NAME), GLOBAL)
        self.assertEqual(sy.addr(NAME, 'unowned'), GLOBAL)

    def test_global_wrong_target_remains_wrong(self):
        p, e = self.program(self.symbols()), self.elf()
        p.infer(self.exact_calls(e, local=LOCAL + 4))
        expected, _ = p.target(e, OWNER, 0, 0)
        self.assertEqual(expected, LOCAL)
        self.assertNotEqual(p.verdict(LOCAL + 4, expected, NAME), 'MATCH')


if __name__ == '__main__':
    unittest.main()
