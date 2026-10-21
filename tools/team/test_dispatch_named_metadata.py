"""Source/pure/tmpfile models only; no main/child/Popen/signals or env reads."""
import ast
import hashlib
import json
import os
from pathlib import Path
import tempfile
import types
import unittest
from unittest.mock import patch
import dispatch_named_metadata as metadata
import dispatch_named_metadata_run as runner

class MetadataSourceModels(unittest.TestCase):
    def plan(self):
        return {'source_files':{'synthetic.py':'a'*64},'python':{'resolved':'/SYNTHETIC/python','sha256':'b'*64}}
    def record(self,stage='childentry'):
        p=self.plan()
        r=metadata.stage_record(stage,{metadata.KEY:'SYNTHETIC_VALUE_NOT_OBSERVED'},p,'c'*64,os.getpid())
        r['identity']={'pid':707,'pgid':707,'sid':707,'uid':os.getuid()}
        return p,r
    def test_selected_only_and_exact_value_format(self):
        selected=metadata.selected_only({metadata.KEY:'SYNTHETIC','UNRELATED_SECRET':'NEVER_RETAIN'})
        self.assertEqual(selected,{metadata.KEY:'SYNTHETIC'})
        r=metadata.stage_record('launcherentry',selected,self.plan(),'c'*64,1)
        raw=json.dumps(r)
        self.assertNotIn('UNRELATED_SECRET',raw);self.assertNotIn('NEVER_RETAIN',raw)
        self.assertEqual(r['observation']['value_hex'],b'SYNTHETIC'.hex())
        with self.assertRaises(ValueError):metadata.stage_record('payload',selected,self.plan(),'c'*64,1)
    def test_record_roundtrip_private_and_stale_preserved(self):
        with tempfile.TemporaryDirectory() as d:
            p,r=self.record();path=Path(d)/'childentry.json';metadata.publish(path,r)
            self.assertEqual(path.stat().st_mode & 0o777,0o600)
            self.assertEqual(metadata.read_stage(path,'childentry',p,'c'*64,os.getpid()),r)
            old=path.read_bytes()
            with self.assertRaises(RuntimeError):metadata.publish(path,r)
            self.assertEqual(path.read_bytes(),old)
    def test_size_duplicate_bool_schema_wrong_stage_parent_and_pins_reject(self):
        with tempfile.TemporaryDirectory() as d:
            p,r=self.record();path=Path(d)/'childentry.json'
            for wrong in ({**r,'schema':True},{**r,'stage':'launcherentry'},{**r,'parent_pid':0},
                          {**r,'plan_sha256':'d'*64},{**r,'source_files':{}},
                          {**r,'observation':{**r['observation'],'value_hex':'00'}},
                          {**r,'identity':{**r['identity'],'uid':True}}):
                path.unlink(missing_ok=True);metadata.publish(path,wrong)
                with self.assertRaises((RuntimeError,ValueError)):metadata.read_stage(path,'childentry',p,'c'*64,os.getpid())
            with self.assertRaises(ValueError):metadata.strict_json(b'{"schema":1,"schema":1}')
            path.unlink()
            with self.assertRaises(RuntimeError):metadata.publish(path,{'oversize':'x'*metadata.CAP})
            self.assertFalse(path.exists())
    def test_missing_identity_no_signal_and_contradicted_kernel_blocks(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);work=root/'work';work.mkdir();p,r=self.record()
            job=runner.MetadataJob(work,10,root,0,64*1024**2,.05)
            job.plan=p;job.pin='c'*64;job.fixture_identity_path=root/'childentry.json'
            job.groups[707]=types.SimpleNamespace(pid=707)
            with patch.object(runner.accepted.os,'killpg') as sig:
                job.observe_fixture()
                with self.assertRaises(RuntimeError):job.require_signal_ownership(707)
                sig.assert_not_called()
            metadata.publish(job.fixture_identity_path,r)
            with patch.object(runner.accepted.os,'getpgid',return_value=808),patch.object(runner.accepted.os,'getsid',return_value=808),patch.object(runner.accepted.os,'killpg') as sig:
                with self.assertRaises(RuntimeError):job.observe_fixture()
                with self.assertRaises(RuntimeError):job.require_signal_ownership(707)
                sig.assert_not_called()
    def test_verified_kernel_identity_and_mutation_reject(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);work=root/'work';work.mkdir();p,r=self.record()
            job=runner.MetadataJob(work,10,root,0,64*1024**2,.05);job.plan=p;job.pin='c'*64
            job.fixture_identity_path=root/'childentry.json';job.groups[707]=types.SimpleNamespace(pid=707)
            metadata.publish(job.fixture_identity_path,r)
            with patch.object(runner.accepted.os,'getpgid',return_value=707),patch.object(runner.accepted.os,'getsid',return_value=707),patch.object(runner.accepted.os,'killpg') as sig:
                job.observe_fixture();self.assertEqual(job.ownership[707]['status'],'VERIFIED')
                r['observation']=metadata.observe_named_only({})
                job.fixture_identity_path.write_text(json.dumps(r))
                with self.assertRaises(RuntimeError):job.observe_fixture()
                sig.assert_not_called()
    def test_disabled_guard_and_child_no_exec_or_ack(self):
        tree=ast.parse(Path(runner.__file__).read_text());main=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='main')
        guard=next(n for n in main.body if isinstance(n,ast.If) and 'execution_enabled' in ast.unparse(n.test))
        lock=next(n for n in ast.walk(main) if isinstance(n,ast.Call) and ast.unparse(n.func)=='fcntl.lockf')
        self.assertLess(guard.lineno,lock.lineno)
        source=Path(metadata.__file__).read_text()
        self.assertNotIn('os.exec',source);self.assertNotIn('Popen',source)
        self.assertNotIn("'ack.json'",source)
        self.assertEqual(metadata.STAGES,('launcherentry','parentaftereightkeysetup','childentry'))

if __name__=='__main__':unittest.main()
