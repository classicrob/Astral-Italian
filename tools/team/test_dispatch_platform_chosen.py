"""Synthetic/tmpfile/AST only; no main/spawn/signal/exec or actual platform read."""
import ast
import hashlib
import json
import os
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch
import dispatch_platform_exact as exact
import dispatch_platform_gate as gate
import dispatch_platform_chosen as chosen

class PlatformChosenModels(unittest.TestCase):
    def setup_record(self,root):
        base={k:'SYNTHETIC_'+k for k in exact.BASE_KEYS};value='SYNTHETIC_PLATFORM_NOT_OBSERVED'
        env={**base,exact.KEY:value};rawvalue=value.encode()
        record={'schema':1,'env':env,'env_sha256':hashlib.sha256(exact.canonical(env)).hexdigest(),
                'named_value':{'key':exact.KEY,'present':True,'representation':'strict UTF-8 hex',
                               'value_hex':rawvalue.hex(),'bytes':len(rawvalue),'sha256':hashlib.sha256(rawvalue).hexdigest()}}
        path=root/'operator.json';path.write_bytes(exact.canonical(record));path.chmod(0o600)
        return base,env,record,{'path':str(path),'sha256':exact.sha(path)}
    def test_exact_record_pin_schema_duplicate_and_named_hash_reject(self):
        with tempfile.TemporaryDirectory() as d:
            base,env,record,op=self.setup_record(Path(d))
            self.assertEqual(exact.load_operator(op['path'],op['sha256'],base)[0],env)
            with self.assertRaises(RuntimeError):exact.load_operator(op['path'],'0'*64,base)
            for wrong in ({**record,'schema':True},{**record,'env_sha256':'0'*64},
                          {**record,'env':{**env,'TENTH':'no'}},
                          {**record,'named_value':{**record['named_value'],'bytes':True}}):
                p=Path(op['path']);p.write_bytes(exact.canonical(wrong))
                with self.assertRaises(RuntimeError):exact.load_operator(p,exact.sha(p),base)
            with self.assertRaises(ValueError):exact.strict_json(b'{"schema":1,"schema":1}')
    def request(self,root):
        base,env,record,op=self.setup_record(root)
        python=str(Path(sys.executable).resolve())
        request={'nonce':'a'*32,'argv':exact.payload(python,op),'cwd':str(Path.cwd().resolve()),'env':env,
                 'target':python,'target_sha256':exact.sha(python),'gate_sha256':exact.sha(gate.__file__),
                 'python_sha256':exact.sha(sys.executable),'operator':op,'base_env':base,
                 'full_env_sha256':record['env_sha256'],'source_files':{n:exact.sha(Path(exact.__file__).with_name(n)) for n in exact.SOURCE_NAMES}}
        return request,env
    def test_gate_before_identity_and_postack_same_exact_mapping(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);request,env=self.request(root)
            for checkpoint in ('before-identity','after-ack'):
                with patch.object(gate,'os',types.SimpleNamespace(environ=dict(env))):
                    self.assertEqual(gate.check(request,root/(checkpoint+'.json')),request['target'])
                self.assertTrue(json.loads((root/(checkpoint+'.json')).read_text())['exact_nine_key_equal'])
            self.assertFalse((root/'identity.json').exists())
    def test_gate_missing_tenth_changed_and_operator_drift_reject(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);request,env=self.request(root)
            for wrong in ({k:v for k,v in env.items() if k!=exact.KEY},{**env,'EXTRA':'SECRET'},
                          {**env,exact.KEY:'DIFFERENT'}, {**env,exact.KEY.lower():'x'}):
                with patch.object(gate,'os',types.SimpleNamespace(environ=wrong)):
                    with self.assertRaises(RuntimeError):gate.check(request)
            request['operator']['sha256']='0'*64
            with patch.object(gate,'os',types.SimpleNamespace(environ=env)):
                with self.assertRaises(RuntimeError):gate.check(request)
    def test_non_identity_payload_and_source_drift_reject(self):
        with tempfile.TemporaryDirectory() as d:
            request,env=self.request(Path(d));request['argv']=[request['target'],'-c','UNREVIEWED']
            with patch.object(gate,'os',types.SimpleNamespace(environ=env)):
                with self.assertRaises(RuntimeError):gate.check(request)
            request['source_files']['dispatch_platform_gate.py']='0'*64
            with self.assertRaises(RuntimeError):exact.source_check(request['source_files'])
    def test_parent_context_drift_prevents_ack(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);work=root/'work';work.mkdir()
            job=chosen.ChosenJob(work,20,root,0,64*1024**2,.05,launch={'env':{}})
            job.plan={};job.active_launch={'pending':True};job.groups[707]=types.SimpleNamespace(pid=707)
            with patch.object(chosen,'context',return_value={'DIFFERENT':'context'}),patch.object(chosen.publication,'publish') as publish:
                with self.assertRaises(RuntimeError):job.observe_fixture()
                publish.assert_not_called()

    def test_source_explicit_env_single_payload_and_no_full_wiring(self):
        tree=ast.parse(Path(chosen.__file__).read_text())
        calls=[n for n in ast.walk(tree) if isinstance(n,ast.Call) and ast.unparse(n.func).endswith('subprocess.Popen')]
        self.assertEqual(len(calls),1)
        self.assertEqual(ast.unparse(next(k.value for k in calls[0].keywords if k.arg=='env')),"dict(self.launch['env'])")
        ast.parse(exact.POSTEXEC_CODE)
        self.assertIn('actual==expected',exact.POSTEXEC_CODE)
        self.assertIn('sys.argv[2]',exact.POSTEXEC_CODE)
        self.assertIn('record["env_sha256"]',exact.POSTEXEC_CODE)
        self.assertNotIn('--version',Path(chosen.__file__).read_text())
        self.assertNotIn('dispatch_platform_chosen',Path(chosen.__file__).with_name('dispatch_fullproof.py').read_text())

if __name__=='__main__':unittest.main()
