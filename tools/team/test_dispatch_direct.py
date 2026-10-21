"""No process/signal/tool execution: source, authority and negative models only."""
import ast
import copy
import hashlib
import json
import os
from pathlib import Path
import sys
import unittest
from unittest.mock import patch
import dispatch_direct_profile as profile
import dispatch_direct_gate as gate
import dispatch_direct_chosen as chosen

class DirectModels(unittest.TestCase):
    def plan(self):return json.loads(Path(profile.__file__).with_name('DISPATCH_DIRECT_PLAN.json').read_bytes())
    def test_independent_raw_command_and_three_spans(self):
        data,command=profile.authority()
        self.assertEqual(len(command.encode()),975)
        self.assertEqual(hashlib.sha256(command.encode()).hexdigest(),profile.SYNTHETIC_SHA)
        self.assertEqual([r['offset'] for r in data['substitution_spans_in_original']],[729,873,923])
        self.assertEqual(profile.commands(self.plan()),[['/bin/sh','-c',command]])
    def test_disabled_source_plan_matches_private_operator_without_disclosure(self):
        p=self.plan();env=profile.validate_plan(p)
        self.assertEqual(set(env),set(profile.exact.BASE_KEYS)|{profile.exact.KEY})
        self.assertEqual(profile.context(p)['targets'],{'/bin/sh':p['tools']['sh']})
    def test_changed_commands_reject(self):
        p=self.plan()
        for command in (['/bin/sh','-c','true'],p['commands'][0]+['extra'],['/bin/sh','-c',p['commands'][0][2]+' '],['/bin/sh','-c',p['commands'][0][2].replace(' && ',' ; ')]):
            q=copy.deepcopy(p);q['commands']=[command]
            with self.assertRaises(RuntimeError):profile.validate_plan(q)
    def test_context_bindings_reject(self):
        p=self.plan()
        for key,value in [('cwd','/tmp'),('root','/tmp'),('execution_enabled',True),('full_env_sha256','0'*64),('completion_reserved_bytes',0),('work_seconds',21),('schema',True)]:
            q=copy.deepcopy(p);q[key]=value
            with self.assertRaises(RuntimeError):profile.validate_plan(q)
        for key in ('substitution_spans_in_original','output_binding','depfile_binding','operator','tools','source_files'):
            q=copy.deepcopy(p);q[key]={}
            with self.assertRaises(RuntimeError):profile.validate_plan(q)
    def test_authority_missing_or_changed_reject(self):
        with patch.object(profile,'bounded',return_value=b'{}'):
            with self.assertRaises(RuntimeError):profile.authority()
    def test_no_bool_enable_or_direct_job_entry(self):
        with self.assertRaisesRegex(RuntimeError,'UNQUALIFIED'):profile.require_admission()
        # run_profile refuses before accessing any process/lifecycle object state.
        with self.assertRaisesRegex(RuntimeError,'UNQUALIFIED'):chosen.DirectJob.run_profile(None,None,None)
        tree=ast.parse(Path(chosen.__file__).read_text())
        main=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='main')
        self.assertFalse(any(isinstance(n,ast.Attribute) and n.attr in ('Popen','execv','mkdir','lockf') for n in ast.walk(main)))
        gtree=ast.parse(Path(gate.__file__).read_text());gmain=next(n for n in gtree.body if isinstance(n,ast.FunctionDef) and n.name=='main')
        calls=[ast.unparse(n) for n in gmain.body]
        refusal=next(i for i,s in enumerate(calls) if 'profile.require_admission()' in s)
        identity=next(i for i,s in enumerate(calls) if "'identity.json'" in s)
        self.assertLess(refusal,identity)
    def test_gate_exact_argv_and_environment(self):
        p=self.plan();env=profile.validate_plan(p)
        request={'nonce':'0'*32,'argv':p['commands'][0],'cwd':p['cwd'],'env':env,'target':'/bin/sh',
         'target_sha256':p['tools']['sh']['sha256'],'gate_sha256':p['source_files']['dispatch_direct_gate.py'],
         'python_sha256':profile.exact.sha(sys.executable),'operator':p['operator'],'base_env':p['base_env'],
         'full_env_sha256':p['full_env_sha256'],'source_files':p['source_files'],'profile':profile.LABELS[0],
         'plan_reference':{'path':str(Path(profile.__file__).with_name('DISPATCH_DIRECT_PLAN.json')),
                           'sha256':profile.exact.sha(Path(profile.__file__).with_name('DISPATCH_DIRECT_PLAN.json'))}}
        with patch.dict(os.environ,env,clear=True),patch.object(Path,'cwd',return_value=Path(p['cwd'])):
            self.assertEqual(gate.check(request),'/bin/sh')
            q=copy.deepcopy(request);q['argv']=['/bin/sh','-c','true']
            with self.assertRaises(RuntimeError):gate.check(q)
            with patch.dict(os.environ,{'UNEXPECTED':'x'}):
                with self.assertRaises(RuntimeError):gate.check(request)
    def test_old_sources_unchanged_and_no_ninja_target(self):
        p=self.plan();self.assertEqual(p['commands'][0][0],'/bin/sh')
        self.assertNotIn('ninja',p['tools'])
        self.assertFalse(profile.ROOT.exists());self.assertFalse(profile.ROOT.is_symlink())

if __name__=='__main__':unittest.main()
