#!/usr/bin/env python3
"""Library configuration isolation, quiet gating and private report pipeline."""
import functools
import json
from pathlib import Path
import tempfile
import time
import unittest
from unittest.mock import patch

import libtrial2 as lib


class ConfigurationIsolation(unittest.TestCase):
    def test_game_default_exception_changes_do_not_change_library_key(self):
        original='cflags_game=["same"]\ndef GameLib(x):\n return "Wii/1.0"\ndef RVLLib(x):\n return "Wii/1.0"\n'
        changed='GAME_MW10_EXCEPTIONS={"Game/A.cpp"}\n'+original.replace('return "Wii/1.0"','return "Wii/1.3"',1)
        self.assertEqual(lib.library_config_key(original),lib.library_config_key(changed))
        self.assertNotEqual(lib.library_config_key(original),lib.library_config_key(changed.replace('def RVLLib(x):\n return "Wii/1.0"','def RVLLib(x):\n return "Wii/1.3"')))

    def test_group_scope_includes_msl_c_but_not_automatic_units(self):
        self.assertEqual(lib.group_of('main/MSL_C/PPC_EABI/SRC/math_ppc'),'MSL')
        self.assertNotIn(lib.group_of('main/auto_03_80000000_text'),lib.GROUPS)
        manifest={'units':[{'unit':{'name':'main/JSystem/A'},'versions':lib.WII+['GC/2.7']},
                           {'unit':{'name':'main/RVL_SDK/A'},'versions':lib.WII+['GC/3.0a3']}]}
        self.assertEqual(len(lib.expected(manifest,'baseline')),2)
        self.assertEqual([x['unit']['name'] for x in lib.expected(manifest,'GC/2.7')],['main/JSystem/A'])
        self.assertEqual(len(lib.expected(manifest,'Wii/1.3')),2)


class PrivatePipeline(unittest.TestCase):
    def fixture(self, root):
        (root/'src').mkdir();source=root/'src/Test.c';source.write_text('int f(void) {return 1;}\n')
        edge={'argv':['build/tools/wibo','build/compilers/GC/3.0a3/mwcceppc.exe','-opt','noloop','-c','src/Test.c','-o','build/shared'],
              'compiler_index':1,'source':'src/Test.c','tails':[]}
        item={'unit':{'name':'main/RVL_SDK/Test'},'edge':edge,'group':'RVL_SDK','configured_version':'GC/3.0a3','source_sha256':lib.g.sha(source),'target':str(root/'target.o')}
        return {'root':str(root),'out':str(root)},item

    def test_serial_compiler_takes_quiet_lock_and_scores_without_shared_writes(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);manifest,item=self.fixture(root);private=root/'private';private.mkdir()
            calls=[]
            def execute(argv,cwd,timeout):
                calls.append(argv)
                if 'report' in argv:
                    self.assertIn('functionRelocDiffs=none',argv)
                    project=Path(argv[argv.index('-p')+1]);cfg=json.loads((project/'objdiff.json').read_text())
                    self.assertEqual(Path(cfg['units'][0]['base_path']).read_text(),'private object')
                    report={'units':[{'functions':[{'name':'f','size':'4','fuzzy_match_percent':100}],
                        'sections':[{'name':'.data','size':'4','fuzzy_match_percent':100}]}]}
                    return dict(returncode=0,stdout=json.dumps(report),stderr='',seconds=.01)
                dest=Path(argv[argv.index('-o')+1]);(dest/'Test.o').write_text('private object')
                return dict(returncode=0,stdout='',stderr='',seconds=.02)
            temporary=tempfile.TemporaryDirectory
            with patch.object(lib.tempfile,'TemporaryDirectory',functools.partial(temporary,dir=private)),patch.object(lib.fnsearch,'run',execute):
                row=lib.compile_unit(manifest,item,'Wii/1.3',time.monotonic()+10,True)
            self.assertEqual(calls[0][:3],['lockf','-k',str(lib.QUIET_LOCK)])
            self.assertIn('build/compilers/Wii/1.3/mwcceppc.exe',calls[0]);self.assertIn('noloop',calls[0])
            self.assertEqual(row['status'],'scored');self.assertEqual(row['configured_version'],'GC/3.0a3')
            self.assertEqual(list(private.iterdir()),[]);self.assertFalse((root/'build/shared').exists())
            self.assertEqual(lib.g.sha(root/'src/Test.c'),item['source_sha256'])

    def test_quiet_marker_prevents_further_parallel_compiles(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);manifest,item=self.fixture(root);(root/'quiet').touch()
            with patch.object(lib.fnsearch,'run') as execute:
                self.assertIsNone(lib.compile_unit(manifest,item,'Wii/1.3',time.monotonic()+10,False))
                execute.assert_not_called()

    def test_source_change_refuses_compile(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);manifest,item=self.fixture(root);(root/'src/Test.c').write_text('changed')
            with patch.object(lib.fnsearch,'run') as execute,self.assertRaises(ValueError):
                lib.compile_unit(manifest,item,'Wii/1.3',time.monotonic()+10,True)
            execute.assert_not_called()


if __name__=='__main__':unittest.main()
