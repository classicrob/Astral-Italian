"""Skipped split padding must not replace the live section's address or bounds."""
import struct
import tempfile
from pathlib import Path
from types import SimpleNamespace
import unittest

from datacontent import Pointer, Relocation, Symbol, World


class SkippedSplitRangeTests(unittest.TestCase):
    def make_world(self, section, ranges):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        tree = Path(temp.name)
        (tree / 'build/SB4E01').mkdir(parents=True)
        (tree / 'config/SB4E01').mkdir(parents=True)
        image = bytearray(0x108)
        struct.pack_into('>I', image, 0, 0x100)
        struct.pack_into('>I', image, 0x48, 0x80700000)
        struct.pack_into('>I', image, 0x90, 8)
        image[0x100:0x104] = bytes.fromhex('3f800000')
        (tree / 'build/SB4E01/main.dol').write_bytes(image)
        (tree / 'config/SB4E01/symbols.txt').write_text(
            f'value = {section}:0x80700000; // type:object size:0x4 scope:local\n')
        (tree / 'config/SB4E01/splits.txt').write_text('Game/Fixture.cpp:\n' + ranges)
        elf = SimpleNamespace(names=['', section], sections=[(), ()], relocations={},
                              local=lambda symbol: symbol.section == 1)
        pointer = Pointer(Relocation(109, Symbol('value', 0, 4, 1, 1, 0), 0))
        return World(tree), elf, pointer

    def test_trailing_skip_preserves_live_scalar_address_and_extent(self):
        for section in ['.sdata', '.sdata2']:
            with self.subTest(section=section):
                world, elf, pointer = self.make_world(section,
                    f'\t{section} start:0x80700000 end:0x80700004\n'
                    f'\t{section} start:0x80700004 end:0x80700008 skip\n')
                self.assertEqual(world.resolve_address(elf, 'Game/Fixture', pointer), (0x80700000, None))
                obj, error = world.target_bound(elf, 'Game/Fixture', pointer, 64)
                self.assertIsNone(error)
                self.assertEqual(obj['hex'], '3f800000')
                self.assertFalse(world.owns('Game/Fixture', section, 0x80700004))

    def test_skip_only_range_does_not_create_owning_section(self):
        world, elf, pointer = self.make_world('.sdata2',
            '\t.sdata2 start:0x80700000 end:0x80700008 skip\n')
        self.assertEqual(world.resolve_address(elf, 'Game/Fixture', pointer),
                         (None, 'target_owning_section_unknown'))

    def test_live_range_after_skipped_padding_still_resolves(self):
        world, elf, pointer = self.make_world('.sdata2',
            '\t.sdata2 start:0x806FFFFC end:0x80700000 skip\n'
            '\t.sdata2 start:0x80700000 end:0x80700004\n')
        self.assertEqual(world.resolve_address(elf, 'Game/Fixture', pointer), (0x80700000, None))

    def test_skip_in_comment_is_not_a_split_flag(self):
        world, elf, pointer = self.make_world('.sdata2',
            '\t.sdata2 start:0x80700000 end:0x80700004 // skip padding is a separate range\n')
        self.assertEqual(world.resolve_address(elf, 'Game/Fixture', pointer), (0x80700000, None))


if __name__ == '__main__':
    unittest.main()
