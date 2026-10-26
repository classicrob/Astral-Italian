#!/usr/bin/env python3
"""Pure Python fixture/capture guards; no compiler, game, host input or GUI."""
import copy
import json
import subprocess
from pathlib import Path
import tempfile
import unittest
from capture_schema2_tape import inventory, warm_source, PRODUCTION_PIPELINE_ENV
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

    def test_warm_source_requires_complete_fresh_same_exe_and_production_env(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='capture-warm-unit-',dir=root) as temporary:
            source=Path(temporary);(source/'cache').mkdir();(source/'replay').mkdir()
            exe=Path('/owned/Petari.app/Contents/MacOS/Petari')
            files={'launch.json':dict(exe_sha256='exact',command=[str(exe)],warm_from=None,environment=PRODUCTION_PIPELINE_ENV),
                   'RESULT.json':dict(passed=True), 'process.json':dict(exit=0,timed_out=False),
                   'replay/divergence.json':dict(finished=True,stopped='')}
            def write(values):
                for name,value in values.items():(source/name).write_text(json.dumps(value))
            write(files)
            self.assertEqual(warm_source(source,exe,'exact')['cache_hashes'],{})
            negatives=[('RESULT.json','passed',False),('launch.json','exe_sha256','other'),
                       ('launch.json','command',['/other/app']),('launch.json','warm_from','/earlier/warm'),
                       ('process.json','exit',1),('process.json','timed_out',True),
                       ('replay/divergence.json','finished',False),('replay/divergence.json','stopped','position bits'),
                       ('launch.json','environment',dict(PETARI_PIPELINE_DIAG='0')),
                       ('launch.json','environment',dict(PETARI_PIPELINE_DIAG='0',PETARI_TRACE_BOOT='1'))]
            for name,key,value in negatives:
                with self.subTest(name=name,key=key,value=value):
                    bad=copy.deepcopy(files);bad[name][key]=value;write(bad)
                    with self.assertRaises(ValueError):warm_source(source,exe,'exact')
            write(files)
            (source/'cache').rmdir()
            with self.assertRaises(ValueError):warm_source(source,exe,'exact')

    def test_vanilla_capture_does_not_enable_default_mod(self):
        self.assertEqual(mods_env([]),'-OdysseyJumpDive')


if __name__=='__main__':unittest.main()
