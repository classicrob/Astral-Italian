import copy
import hashlib
import os
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import observer_standalone_caller as caller
from observer_standalone_caller import consumed_md,inspect_otool,execute_after_qualification,bounded_report,OwnedState,require_exact_launch_envelope

class CallerModels(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name).resolve()
        self.header=self.root/'sdk-header.h';self.header.write_text('synthetic SDK header\n')
        self.inventory={'records':{str(self.header):{'sha256':hashlib.sha256(self.header.read_bytes()).hexdigest()}}}
    def dep(self,text):p=self.root/'fixture.d';p.write_text(text);return p
    def test_consumed_system_dependency_must_match_preinventory(self):
        r=consumed_md(self.dep('fixture.o: '+str(self.header)+'\n'),self.root,self.inventory,lambda:None)
        self.assertEqual(len(r['dependencies']),1)
        self.header.write_text('changed')
        with self.assertRaises(RuntimeError):consumed_md(self.root/'fixture.d',self.root,self.inventory,lambda:None)
    def test_unbound_empty_or_malformed_md_rejected(self):
        for text in ('fixture.o: /unbound/sdk.h\n','fixture.o:\n','not a depfile'):
            with self.assertRaises(RuntimeError):consumed_md(self.dep(text),self.root,self.inventory,lambda:None)
    def test_otool_exact_target_and_dynamic_cpp(self):
        p=self.root/'otool.log';exe=self.root/'control';p.write_text(str(exe)+':\n\t/usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1.0.0)\n')
        self.assertEqual(len(inspect_otool(p,exe)['libraries']),1)
        p.write_text(str(exe)+':\n\t/usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 1.0.0)\n')
        with self.assertRaises(RuntimeError):inspect_otool(p,exe)
    def test_no_qualification_pin_means_no_file_lock_or_spawn(self):
        with self.assertRaisesRegex(RuntimeError,'UNQUALIFIED'):execute_after_qualification(None,None,None,None,None,None,'fake')
    def test_exact_external_envelope_no_extra_missing_or_changed_key(self):
        expected=dict(os.environ);require_exact_launch_envelope({'env':expected})
        for altered in (dict(expected,OBSERVER_EXTRA_KEY='extra'),{k:v for k,v in expected.items() if k!=next(iter(expected))},dict(expected,PATH=expected.get('PATH','')+'-changed')):
            with self.assertRaises(RuntimeError):require_exact_launch_envelope({'env':altered})
    def test_state_exclusive_creation_preserves_previous_record(self):
        p=self.root/'state';p.write_text('prior evidence')
        with self.assertRaises(FileExistsError):OwnedState(p)
        self.assertEqual(p.read_text(),'prior evidence')
    def test_owned_state_updates_fd_and_rejects_replaced_path(self):
        p=self.root/'state';state=OwnedState(p)
        try:
            state.update({'phase':'first'});state.update({'phase':'last'})
            self.assertEqual(json.loads(p.read_text()),{'phase':'last'})
            p.rename(self.root/'retained-owned-state');p.write_text('foreign replacement')
            with self.assertRaises(RuntimeError):state.update({'phase':'must not write'})
            self.assertEqual(p.read_text(),'foreign replacement')
        finally:state.close()
    def test_owner_fstat_failure_retains_exact_partial_without_unlink(self):
        p=self.root/'state-owner-unknown'
        with patch.object(caller.os,'fstat',side_effect=OSError('fixture owner observation failure')):
            with self.assertRaisesRegex(RuntimeError,'retained unverified STATE path') as failure:OwnedState(p)
        self.assertIn(str(p),str(failure.exception));self.assertTrue(p.exists());self.assertEqual(p.read_bytes(),b'')
    def test_state_symlink_never_rewrites_target(self):
        target=self.root/'target';target.write_text('retain');p=self.root/'state';p.symlink_to(target)
        with self.assertRaises(FileExistsError):OwnedState(p)
        self.assertEqual(target.read_text(),'retain')
    def test_lock_contention_does_not_create_or_replace_state(self):
        home=self.root/'home';slots=home/'MacioGalaxy2-team/slots';slots.mkdir(parents=True)
        work=home/'MacioGalaxy2-gamedata/fresh';work.parent.mkdir();state=work.parent/(work.name+'.STATE.json')
        for retained in (False,True):
            if retained:state.write_text('prior retained state')
            launch={'env':{'TMPDIR':str(work/'tmp')}}
            with patch.object(caller,'QUALIFIED_CONTEXT_PIN','model'),patch.object(caller,'pinned_json',side_effect=[{'execution_enabled':True,'caller_sources':{}},launch,{}]),patch.object(caller,'checked_plan',return_value={'commands':[]}),patch.object(caller,'require_exact_launch_envelope'),patch.object(caller.os,'getpriority',return_value=19),patch.object(caller.Path,'home',return_value=home),patch.object(caller.fcntl,'lockf',side_effect=BlockingIOError('fixture contention')):
                with self.assertRaises(BlockingIOError):execute_after_qualification('contract','plan','launch','inputs','leaves',work,'model')
            if retained:self.assertEqual(state.read_text(),'prior retained state')
            else:self.assertFalse(state.exists())
    def test_report_never_overwrites_existing_evidence(self):
        p=self.root/'report.json';bounded_report(p,{'status':'synthetic'})
        with self.assertRaises(FileExistsError):bounded_report(p,{'status':'replacement'})

if __name__=='__main__':unittest.main()
