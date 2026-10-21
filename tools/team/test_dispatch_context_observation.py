"""No-process pre-guard observation controls; no gate.main/spawn/signal/exec."""
import ast
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import dispatch_exec_gate as gate
import test_dispatch_exec_gate as existing

class ContextObservationModels(unittest.TestCase):
    def observe(self, env, requested=None, cwd='/actual', wanted='/actual'):
        return gate.context_observation({'env':env if requested is None else requested,'cwd':wanted},cwd,env)

    def test_exact_allowlisted_key_and_presence(self):
        value=self.observe({'LANG':'C','PATH':'/bin'},{'LANG':'other','TMPDIR':'/private'})
        self.assertFalse(value['env_equal']);self.assertTrue(value['cwd_equal'])
        self.assertEqual([key for key,row in value['environment'].items() if not row['equal']],['PATH','LANG','TMPDIR'])
        self.assertNotIn('other',json.dumps(value))
        self.assertNotIn('/private',json.dumps(value))

    def test_unrelated_values_not_exported_names_only(self):
        value=self.observe({'PRIVATE_TOKEN_NAME':'actual-secret'},{'DIFFERENT_SECRET_NAME':'requested-secret'})
        raw=json.dumps(value)
        for secret in ('actual-secret','requested-secret'):
            self.assertNotIn(secret,raw)
        self.assertEqual(value['outside_allowlist']['different_names'],[{'name':'DIFFERENT_SECRET_NAME','actual_present':False,'requested_present':True},{'name':'PRIVATE_TOKEN_NAME','actual_present':True,'requested_present':False}])
        self.assertFalse(value['env_equal'])

    def test_cwd_values_and_bound(self):
        value=self.observe({},cwd='/actual',wanted='/requested')
        self.assertFalse(value['cwd_equal']);self.assertTrue(value['env_equal'])
        self.assertEqual(value['cwd']['actual']['text'],'/actual')
        self.assertEqual(value['cwd']['requested']['text'],'/requested')
        huge='\x01'*10000
        value=self.observe({'LANG':'x'*100000},cwd=huge,wanted=huge)
        self.assertTrue(value['cwd']['actual']['text_truncated'])
        self.assertLessEqual(len(json.dumps(value,sort_keys=True).encode()),gate.CONTEXT_RECORD_CAP)

    def test_record_precedes_reject_without_identity(self):
        request=existing.GateSourceModels().request();request['cwd']='/different-requested-cwd'
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);path=root/'context-before-identity.json'
            with patch.object(gate.os,'execv') as execute:
                with self.assertRaisesRegex(RuntimeError,'launch cwd drift'):
                    gate.check_request(request,path)
                execute.assert_not_called()
            value=json.loads(path.read_text())
            self.assertFalse(value['cwd_equal']);self.assertTrue(value['env_equal'])
            self.assertFalse((root/'identity.json').exists())
            old=path.read_bytes()
            with self.assertRaises(RuntimeError):gate.check_request(request,path)
            self.assertEqual(path.read_bytes(),old)

    def test_main_observation_is_before_identity_and_not_after_ack(self):
        tree=ast.parse(Path(gate.__file__).read_text())
        main=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='main')
        calls=[n for n in ast.walk(main) if isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id=='check_request']
        self.assertEqual([len(c.args) for c in calls],[2,1])
        self.assertLess(calls[0].lineno,next(n.lineno for n in ast.walk(main) if isinstance(n,ast.Assign) and any(isinstance(t,ast.Name) and t.id=='identity' for t in n.targets)))

if __name__=='__main__':unittest.main()
