#!/usr/bin/env python3
"""No-build controls for dispatch landing's guards. Real dry-run is separate."""
import os
from pathlib import Path
import subprocess
import tempfile
import json
import unittest
from unittest.mock import patch
import dispatchlanding as dl


class Guards(unittest.TestCase):
    def test_allowed_paths(self):
        for path in ('src/Game/Test.cpp','src/Game/Boss/A.hpp','src/Game/Boss/A.h','src/Game/A.inc',
                     'include/Game/A','include/Game/Boss/A.hpp','config/SB4E01/symbols.txt',
                     'tools/team/dispatchcheck-allow.txt'):
            with self.subTest(path=path):self.assertTrue(dl.allowed_path(path))

    def test_refused_paths(self):
        for path in ('libs/A.cpp','tools/team/dispatchproof.py','native/A.cpp','config/SB4E01/splits.txt',
                     'config/SB4E01/build.sha1','src/JSystem/A.cpp','include/JSystem/A.hpp','src/GameX/A.cpp',
                     'src/Game/A.py','src/Game/../Other.cpp','src//Game/A.cpp','/src/Game/A.cpp'):
            with self.subTest(path=path):self.assertFalse(dl.allowed_path(path))

    def test_conflicting_modes(self):
        for key in ('REMOTE_PROOF_RECEIPT','REMOTE_PROOF_ROOT','REMOTE_PROOF_MARIO_52','MARIO_52'):
            with self.subTest(key=key),self.assertRaisesRegex(dl.Refused,key):
                dl.check_environment({'DISPATCH_PROOF':'1',key:'1'})

    def test_explicit_opt_in(self):
        with self.assertRaises(dl.Refused):dl.check_environment({})

    def test_jobs(self):
        self.assertEqual(dl.check_environment({'DISPATCH_PROOF':'1'}),4)
        self.assertEqual(dl.check_environment({'DISPATCH_PROOF':'1','HOOKPROOF_JOBS':'2'}),2)
        for value in ('0','-1','oops','2;echo nope'):
            with self.subTest(value=value),self.assertRaises(dl.Refused):
                dl.check_environment({'DISPATCH_PROOF':'1','HOOKPROOF_JOBS':value})

    def test_dryrun_typo_refused(self):
        with self.assertRaises(dl.Refused):dl.check_environment({'DISPATCH_PROOF':'1','DISPATCH_PROOF_DRYRUN':'yes'})

    def test_dirty_worktree_refused(self):
        with patch.object(dl,'git',return_value=b' M tools/bad.py\n'),self.assertRaisesRegex(dl.Refused,'tools/bad.py'):
            dl.clean()

    def test_disallowed_diff_names_offender(self):
        with patch.object(dl,'git',return_value=b'src/Game/Good.cpp\0libs/bad.cpp\0'),self.assertRaisesRegex(dl.Refused,'libs/bad.cpp'):
            dl.changed_paths('a','b')

    def test_low_space_before_build(self):
        with patch.object(dl.shutil,'disk_usage',return_value=type('Usage',(),{'free':49*1024**3})()),self.assertRaises(dl.Refused):
            dl.ensure_space()

    def test_snapshot_space_margin(self):
        with patch.object(dl.shutil,'disk_usage',return_value=type('Usage',(),{'free':50*1024**3})()),self.assertRaises(dl.Refused):
            dl.ensure_space(1)

    def test_symlink_blob_refused(self):
        with patch.object(dl,'command',return_value=(0,b'120000 blob abc\tsrc/Game/A.cpp\0')),self.assertRaises(dl.Refused):
            dl.blob('HEAD','src/Game/A.cpp')

    def test_default_shell_body_preserved(self):
        root=Path(__file__).resolve().parents[2]
        guard=['# Dispatch mode has its own fail-closed contract; the ordinary path is unchanged.\n',
               'if [ "${DISPATCH_PROOF:-}" = 1 ]; then\n']
        for name,mode in (('hookproof.sh','hook'),('proofland.sh','land')):
            old=subprocess.check_output(['git','show',f'be7b0ec0:tools/team/{name}'],cwd=root).decode()
            new=(root/'tools/team'/name).read_text()
            addition=''.join(guard)+f'  exec python3 tools/team/dispatchlanding.py {mode} "$@"\nfi\n'
            self.assertEqual(new.count(addition),1)
            new=new.replace(addition,'',1).replace('-j "${HOOKPROOF_JOBS:-4}"','-j 4')
            self.assertEqual(new,old)


class FinalGate(unittest.TestCase):
    def test_dryrun_never_calls_land(self):
        with tempfile.TemporaryDirectory(prefix='R-landing-') as temp, patch.dict(os.environ,{'DISPATCH_PROOF':'1','DISPATCH_PROOF_DRYRUN':'1'},clear=True), patch.object(dl,'revision',side_effect=['b','h']), patch.object(dl,'clean'), patch.object(dl,'changed_paths'), patch.object(dl,'command') as command:
            dl.finish_landing(Path(temp),'b','h','123',{'status':'DISPATCH PROOF OK'})
            command.assert_not_called()
            receipt=json.loads((Path(temp)/'receipt.json').read_text())
            self.assertEqual(receipt['landing'],'SKIPPED: DISPATCH_PROOF_DRYRUN=1')

    def test_changed_revision_never_lands(self):
        with patch.object(dl,'revision',return_value='changed'), patch.object(dl,'command') as command, self.assertRaises(dl.Refused):
            try:dl.finish_landing(Path('unused'),'b','h','123',{'status':'DISPATCH PROOF OK'})
            finally:command.assert_not_called()

    def test_failed_proof_never_lands(self):
        with patch.object(dl,'command') as command,self.assertRaises(dl.Refused):
            try:dl.finish_landing(Path('unused'),'b','h','123',{'status':'FAILED'})
            finally:command.assert_not_called()

    def test_production_preserves_lock_and_jobs(self):
        with tempfile.TemporaryDirectory(prefix='R-landing-') as temp, patch.dict(os.environ,{'DISPATCH_PROOF':'1','HOOKPROOF_JOBS':'2'},clear=True), patch.object(dl,'revision',side_effect=['b','h']), patch.object(dl,'clean'), patch.object(dl,'changed_paths'), patch.object(dl,'command',return_value=(0,b'')) as command:
            dl.finish_landing(Path(temp),'b','h','123',{'status':'DISPATCH PROOF OK'})
            self.assertEqual(command.call_args.args[0],['sh','tools/team/land.sh'])
            self.assertEqual(command.call_args.kwargs['env']['LAND_LOCK_HELD'],'123')
            self.assertEqual(command.call_args.kwargs['env']['TEAM_BUILD_JOBS'],'2')


if __name__=='__main__':unittest.main(verbosity=2)
