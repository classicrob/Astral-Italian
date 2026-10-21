"""Source/tmpfile/synthetic models only; no compiler/tool/gate.main/process/signal."""
import ast
import copy
import hashlib
import json
import os
from pathlib import Path
import struct
import sys
import tempfile
import types
import unittest
from unittest.mock import patch
import dispatch_platform_exact as exact
import dispatch_tools_profile as profile
import dispatch_tools_gate as gate
import dispatch_tools_chosen as chosen

class ToolsProfileModels(unittest.TestCase):
    def plan(self):
        return exact.strict_json(Path(profile.__file__).with_name('DISPATCH_TOOLS_PLAN.json').read_bytes())
    def test_official_rule_only_three_declared_token_changes(self):
        p=self.plan();spec=exact.strict_json(Path(p['canonical_spec']['path']).read_bytes())
        before,after,sub=profile.rule_tokens(spec,p['tools'])
        self.assertEqual(before,p['official_tokens']);self.assertEqual(after,p['compile_tokens'])
        self.assertEqual([i for i,(a,b) in enumerate(zip(before,after)) if a!=b],[66,72,73])
        self.assertEqual(sub,p['substitutions']);self.assertEqual(before[68],after[68])
        wrong=copy.deepcopy(p);wrong['compile_tokens'][10]='UNREVIEWED_FLAG'
        with self.assertRaises(RuntimeError):profile.validate_plan(wrong)
        wrong=copy.deepcopy(p);wrong['commands'][3].append('EXTRA')
        with self.assertRaises(RuntimeError):profile.validate_plan(wrong)
    def test_fixture_graph_source_transform_pins_and_drift(self):
        p=self.plan()
        with tempfile.TemporaryDirectory() as d,patch.object(profile,'ROOT',Path(d)):
            profile.prepare_fixture(p);profile.check_fixture(p)
            fixture=Path(d)/'fixture';self.assertEqual((fixture/profile.SRC).read_bytes(),profile.FIXTURE)
            self.assertEqual((fixture/'tools/transform_dep.py').read_bytes(),Path(p['tools']['transform_dep']['path']).read_bytes())
            (fixture/'compiler.ninja').write_bytes(b'UNREVIEWED_COMMAND')
            with self.assertRaises(RuntimeError):profile.check_fixture(p)
    def synthetic_elf(self):
        name=b'\0solP_chosen_compiler_context\0';body=bytes.fromhex('386000114e800020')
        textoff=52;stroff=textoff+len(body);symoff=(stroff+len(name)+3)&~3
        syms=b'\0'*16+struct.pack('>IIIBBH',1,0,len(body),0x12,0,1);shoff=symoff+len(syms)
        raw=bytearray(shoff+4*40);raw[:16]=b'\x7fELF\x01\x02\x01'+b'\0'*9
        struct.pack_into('>HHIIIIIHHHHHH',raw,16,1,20,1,0,0,shoff,0,52,0,0,40,4,0)
        raw[textoff:textoff+len(body)]=body;raw[stroff:stroff+len(name)]=name;raw[symoff:symoff+len(syms)]=syms
        for i,row in enumerate(((0,0,0,0,0,0,0,0,0,0),(0,1,6,0,textoff,len(body),0,0,4,0),
                                (0,3,0,0,stroff,len(name),0,0,1,0),(0,2,0,0,symoff,len(syms),2,1,4,16))):
            struct.pack_into('>10I',raw,shoff+i*40,*row)
        return bytes(raw)
    def test_synthetic_valid_elf_named_function_and_real_rejects(self):
        raw=self.synthetic_elf();self.assertEqual(profile.elf_function(raw)['size'],8)
        for offset in (0,19,47):
            wrong=bytearray(raw);wrong[offset]^=1
            with self.assertRaises(RuntimeError):profile.elf_function(bytes(wrong))
        wrong=raw.replace(b'solP_chosen_compiler_context',b'XXXX_chosen_compiler_context')
        with self.assertRaises(RuntimeError):profile.elf_function(wrong)
        wrong=bytearray(raw);shoff=struct.unpack_from('>I',raw,32)[0];struct.pack_into('>I',wrong,shoff+3*40+24,99)
        with self.assertRaises(RuntimeError):profile.elf_function(bytes(wrong))
    def test_consumed_dependency_join_rejects_foreign_extra_or_stale(self):
        text=profile.OBJ+': \\\n\t'+profile.SRC+'\n'
        consumed=profile.OBJ+': #deps 1, deps mtime 1 (VALID)\n    '+profile.SRC+'\n'
        self.assertEqual(profile.dependency_join(text,consumed)['consumed_paths'],[profile.SRC])
        for wrong_text,wrong_log in ((text+'EXTRA.hpp\n',consumed),(text,consumed.replace('(VALID)','(STALE)')),
                                    (text.replace(profile.SRC,'/foreign/'+profile.SRC),consumed)):
            with self.assertRaises(RuntimeError):profile.dependency_join(wrong_text,wrong_log)
    def test_fixed_native_profile_gate_and_unknown_extra_reject(self):
        p=self.plan();p['execution_enabled']=True
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);env={**p['base_env'],exact.KEY:'SYNTHETIC_NOT_OBSERVED'};data=env[exact.KEY].encode()
            record={'schema':1,'env':env,'env_sha256':hashlib.sha256(exact.canonical(env)).hexdigest(),
                    'named_value':{'key':exact.KEY,'present':True,'representation':'strict UTF-8 hex',
                                   'value_hex':data.hex(),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()}}
            operator=root/'operator.json';operator.write_bytes(exact.canonical(record));operator.chmod(0o600)
            p['operator']={'path':str(operator),'sha256':exact.sha(operator)};p['full_env_sha256']=record['env_sha256'];p['commands']=profile.commands(p)
            plan=root/'plan.json';plan.write_bytes(exact.canonical(p));index=profile.LABELS.index('ninja_version');argv=p['commands'][index]
            request={'nonce':'a'*32,'argv':argv,'cwd':str(profile.ROOT/'fixture'),'env':env,
                'target':str(Path(argv[0]).resolve()),'target_sha256':exact.sha(argv[0]),'gate_sha256':exact.sha(gate.__file__),
                'python_sha256':exact.sha(sys.executable),'operator':p['operator'],'base_env':p['base_env'],
                'full_env_sha256':p['full_env_sha256'],'source_files':p['source_files'],'profile':'ninja_version',
                'plan_reference':{'path':str(plan),'sha256':exact.sha(plan)}}
            with patch.object(gate,'os',types.SimpleNamespace(environ=env)),patch.object(gate.Path,'cwd',return_value=profile.ROOT/'fixture'):
                self.assertEqual(gate.check(request),request['target'])
            with patch.object(gate,'os',types.SimpleNamespace(environ={**env,'TENTH':'SECRET'})),patch.object(gate.Path,'cwd',return_value=profile.ROOT/'fixture'):
                with self.assertRaises(RuntimeError):gate.check(request)
    def test_source_disabled_single_build_explicitenv_profile_count(self):
        p=self.plan();self.assertIs(p['execution_enabled'],False);self.assertEqual(len(p['commands']),5)
        self.assertEqual(p['commands'][3][4:8],['1','-k','1','-d'])
        self.assertIn('keepdepfile',p['commands'][3]);self.assertEqual(p['commands'][4][-3:],['-t','deps',profile.OBJ])
        tree=ast.parse(Path(chosen.__file__).read_text());calls=[n for n in ast.walk(tree) if isinstance(n,ast.Call) and ast.unparse(n.func).endswith('subprocess.Popen')]
        self.assertEqual(len(calls),1);self.assertEqual(ast.unparse(next(k.value for k in calls[0].keywords if k.arg=='env')),"dict(self.launch['env'])")
        self.assertNotIn('dispatch_tools_chosen',Path(chosen.__file__).with_name('dispatch_fullproof.py').read_text())
        self.assertIn('tools bundle blocked: Ninja nonconsole child-group cleanup unqualified',Path(chosen.__file__).read_text())
        self.assertEqual(p['execution_lifecycle']['status'],'BLOCKED')

if __name__=='__main__':unittest.main()
