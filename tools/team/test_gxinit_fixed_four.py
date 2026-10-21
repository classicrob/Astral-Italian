"""No subprocess/gate/helper imports: fixed-caller policy models only."""
import ast
import copy
import json
from pathlib import Path
import unittest
import gxinit_fixed_four as f

class FixedModels(unittest.TestCase):
    def plan(self):return json.loads(Path(f.__file__).with_name('GXINIT_FIXED_FOUR_PLAN.json').read_bytes())
    def test_exact_disabled_authority(self):
        p=self.plan();a=f.validate_disabled(p);self.assertEqual(a['steps'],p['steps'])
        q=copy.deepcopy(p);q['execution_enabled']=True
        with self.assertRaises(RuntimeError):f.validate_disabled(q)
        q=copy.deepcopy(p);q['steps'][0]['argv'].append('unapproved')
        with self.assertRaises(RuntimeError):f.validate_disabled(q)
    def test_nonzero17_only_after_strict_cleanup(self):
        clean={'leaders_reaped':True,'groups_absent':True,'error':None}
        self.assertEqual(f.accept_status(2,17,clean)['expected'],17)
        for cleanup in (None,{}, {'leaders_reaped':True,'groups_absent':False,'error':None}, {'leaders_reaped':True,'groups_absent':True,'error':'EPERM'}):
            with self.assertRaises(RuntimeError):f.accept_status(2,17,cleanup)
        for index,rc in ((0,17),(1,1),(2,0),(3,17)):
            with self.assertRaises(RuntimeError):f.accept_status(index,rc,clean)
    def test_join_and_failed_join_no_admission(self):
        report={'pid':123,'pgid':123,'sid':123,'uid':501,'nonce':'abc'}
        result=f.joined_identity(123,123,123,report,501,'abc');self.assertFalse(result['kernel_uid_observed'])
        for pgid,sid in ((1,123),(123,1)):
            with self.assertRaises(RuntimeError):f.joined_identity(123,pgid,sid,report,501,'abc')
        report['uid']=0
        with self.assertRaises(RuntimeError):f.joined_identity(123,123,123,report,501,'abc')
    def test_guards_before_runtime(self):
        for fn,args in [(f.refuse_execution,()),(f.load_helper,({},)),(f.execute_four,({},None,None,None)),(f.run_registered_step,({},0,None,None,None,None,None,None)),(f.gated_child,(None,None,None,None,0))]:
            with self.assertRaisesRegex(RuntimeError,'EXECUTION REFUSED'):fn(*args)
        tree=ast.parse(Path(f.__file__).read_text());run=next(x for x in tree.body if isinstance(x,ast.FunctionDef) and x.name=='run_registered_step')
        rendered=[ast.unparse(x) for x in ast.walk(run)]
        self.assertIn('verified = True',rendered)
        # The only G write is after joined_identity, durable record and append.
        text=ast.unparse(run);self.assertLess(text.index('joined_identity('),text.index("pending.stdin.write(b'G')"))
        self.assertLess(text.index('sup.children.append(pending)'),text.index("pending.stdin.write(b'G')"))
    def test_dependencies_and_missing_unknown_inputs(self):
        root='/fixture';source=root+'/candidate.cpp';names=[source,root+'/required.h']
        contract={'mandatory_paths_common':[names[1]],'allowed_resolved_roots':[root],'max_dependencies_per_arm':5,'max_total_dependency_bytes_per_arm':100}
        records={p:{'resolved':p,'bytes':1,'sha256':'a'*64,'regular':True} for p in names}
        text='/out/candidate: '+names[0]+' \\\n '+names[1]+'\n'
        self.assertEqual(f.dependency_records('candidate',text,records,contract,'/out',source)['count'],2)
        for badtext in ('/out/old: '+names[0],'/out/candidate: '+names[0],'/out/candidate: '+names[0]+' '+names[0]):
            with self.assertRaises(RuntimeError):f.dependency_records('candidate',badtext,records,contract,'/out',source)
        bad=copy.deepcopy(records);bad[names[1]]['resolved']='/unknown/path'
        with self.assertRaises(RuntimeError):f.dependency_records('candidate',text,bad,contract,'/out',source)
    def test_witness_requires_actual_absolute_linker_and_all_libraries(self):
        plan={'linker':{'path':'/tools/ld','resolved':'/tools/lld'},'input_pins':[{'path':'/tools/'+n} for n in ('libc++.a','libc++abi.a','libunwind.a')]}
        log='"/tools/ld" /tools/libc++.a /tools/libc++abi.a /tools/libunwind.a'
        self.assertEqual(f.linker_witness(log,plan)['linker'],'/tools/ld')
        for bad in ('ld /tools/libc++.a',log+'\n'+log,log.replace('/tools/libunwind.a','')):
            with self.assertRaises(RuntimeError):f.linker_witness(bad,plan)

if __name__=='__main__':unittest.main()
