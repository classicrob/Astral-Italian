#!/usr/bin/env python3
"""Pure Python fixture/capture guards; no compiler, game, host input or GUI."""
import subprocess
from pathlib import Path
import tempfile
import unittest
from capture_schema2_tape import inventory
from replay_tape import mods_env


class CaptureGuards(unittest.TestCase):
    def test_fixture_inventory_detects_bytes_and_xattrs(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='capture-fixture-unit-',dir=root) as temporary:
            directory=Path(temporary);file=directory/'save';file.write_bytes(b'original')
            original=inventory(directory)
            file.write_bytes(b'changed');self.assertNotEqual(original,inventory(directory))
            file.write_bytes(b'original');self.assertEqual(original,inventory(directory))
            subprocess.run(['/usr/bin/xattr','-w','com.petari.test.fixture','proof',str(file)],check=True)
            changed=inventory(directory)
            self.assertNotEqual(original,changed)
            self.assertEqual(changed['xattrs']['save']['com.petari.test.fixture'],b'proof'.hex())

    def test_fixture_refuses_symlink(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='capture-fixture-unit-',dir=root) as temporary:
            directory=Path(temporary);(directory/'save').write_bytes(b'original')
            (directory/'link').symlink_to(directory/'save')
            with self.assertRaises(ValueError):inventory(directory)

    def test_vanilla_capture_does_not_enable_default_mod(self):
        self.assertEqual(mods_env([]),'-OdysseyJumpDive')


if __name__=='__main__':unittest.main()
