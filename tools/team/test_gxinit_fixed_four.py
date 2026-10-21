"""No subprocess/gate/helper imports: fixed-caller policy models only."""
import ast
import copy
import json
from pathlib import Path
import unittest
import gxinit_fixed_four as f
import gxinit_before_g_controls as controls

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
    def cleanup_doubles(self,events,stop_error=None,wait_error=None,exists=False):
        class Pipe:
            closed=False
            def close(self):events.append('close');self.closed=True
        class Handle:
            pid=123;returncode=None;stdin=Pipe()
            def wait(self,timeout):
                events.append(('wait',timeout))
                if wait_error:raise wait_error
                self.returncode=0;return 0
        handle=Handle()
        class Supervisor:
            children=[handle]
            def __init__(self):self.events=[]
            def stop_all(self):
                events.append('stop_all')
                if stop_error:raise stop_error
                handle.returncode=0
            def persist(self):events.append('persist')
        class Helper:
            @staticmethod
            def group_exists(pid):events.append('probe');return exists
        return handle,Supervisor(),Helper
    def test_before_g_reaps_before_existing_helper_and_unknown_never_probes(self):
        for verified in (True,False):
            events=[];h,s,helper=self.cleanup_doubles(events)
            state={'attempted':False,'result':None,'error':None}
            result=f.cleanup_step_once(h,verified,False,s,helper,state)
            self.assertEqual(events[:2],['close',('wait',12)])
            if verified:
                self.assertLess(events.index(('wait',12)),events.index('stop_all'))
                self.assertTrue(result['groups_absent'])
            else:
                self.assertEqual(events,['close',('wait',12)])
                self.assertFalse(result['groups_absent'])
    def test_cleanup_uncertainty_and_wait_failure_never_retry(self):
        for stop_error,wait_error in ((PermissionError('EPERM'),None),(None,TimeoutError('wait'))):
            events=[];h,s,helper=self.cleanup_doubles(events,stop_error,wait_error)
            state={'attempted':False,'result':None,'error':None}
            with self.assertRaises((PermissionError,TimeoutError)):f.cleanup_step_once(h,True,False,s,helper,state)
            snapshot=list(events)
            with self.assertRaisesRegex(RuntimeError,'no retry'):f.cleanup_step_once(h,True,False,s,helper,state)
            self.assertEqual(snapshot,events);self.assertTrue(state['attempted']);self.assertIsNotNone(state['error'])
            if wait_error:self.assertNotIn('stop_all',events)
            self.assertEqual(s.children,[h])
    def test_ack_delivery_uncertainty_is_not_before_g(self):
        events=[];h,s,helper=self.cleanup_doubles(events)
        f.cleanup_step_once(h,True,True,s,helper,{'attempted':False,'result':None,'error':None})
        self.assertEqual(events[0],'stop_all');self.assertNotIn('close',events)
    def test_primary_cleanup_errors_and_no_outer_retry(self):
        error=f.StepFailure('primary','cleanup')
        self.assertEqual((error.primary_error,error.cleanup_error),('primary','cleanup'))
        tree=ast.parse(Path(f.__file__).read_text());execute=next(x for x in tree.body if isinstance(x,ast.FunctionDef) and x.name=='execute_four')
        self.assertNotIn('sup.stop_all()',ast.unparse(execute))
        cleanup=next(x for x in tree.body if isinstance(x,ast.FunctionDef) and x.name=='cleanup_step_once')
        text=ast.unparse(cleanup);self.assertLess(text.index("state['attempted'] = True"),text.index('pending.wait('))
    def test_disabled_controls_pins_and_no_payload_path(self):
        plan=json.loads(Path(f.__file__).with_name('GXINIT_BEFORE_G_CONTROL_PLAN.json').read_bytes())
        controls.validate_disabled(plan)
        changed=copy.deepcopy(plan);changed['execution_enabled']=True
        with self.assertRaises(RuntimeError):controls.validate_disabled(changed)
        for fn,args in [(controls.run_controls,({},None,None,0.0)),(controls.run_case,('identity-eof',{},None,None,None,None,None,None)),(controls.gate,(None,None,None,None))]:
            with self.assertRaises(RuntimeError):fn(*args)
        tree=ast.parse(Path(controls.__file__).read_text())
        for node in ast.walk(tree):
            if isinstance(node,ast.Call) and isinstance(node.func,ast.Attribute):
                self.assertNotIn(node.func.attr,('execve','execvpe','execv','killpg','kill'))
        rendered=ast.unparse(tree)
        self.assertNotIn("write(b'G')",rendered)
        self.assertNotIn('sup.stop_all()',rendered)
        self.assertIn('os_signal_test',rendered)
    def test_runtime_contract_and_budget_models(self):
        from unittest.mock import patch
        plan=json.loads(Path(f.__file__).with_name('GXINIT_BEFORE_G_CONTROL_PLAN.json').read_bytes())
        runtime=copy.deepcopy(plan);runtime['execution_enabled']=True
        with patch.object(controls,'validate_root') as root,patch.object(controls,'verify_python') as python,patch.object(controls,'CALLER_ROOT',Path(controls.__file__).resolve().parent),patch.object(controls,'exact_source_entries'),patch.object(controls.os,'getpriority',return_value=19):
            runtime['caller_source_root']=str(Path(controls.__file__).resolve().parent)
            controls.validate_runtime(runtime,'child');root.assert_called_with('child');python.assert_called_with(plan['python'])
        for field,value in [('python',{}),('operator',{}),('cases',[]),('limits',dict(plan['limits'],work_seconds=45)),('base_env',dict(plan['base_env'],LANG='en_US'))]:
            bad=copy.deepcopy(runtime);bad[field]=value
            with self.assertRaises(RuntimeError):controls.validate_runtime(bad,'fresh-parent')
        controls.work_clock(100.0,122.999)
        for now in (99,123,145,float('nan'),float('inf')):
            with self.assertRaises(RuntimeError):controls.work_clock(100.0,now)
        self.assertEqual(sum(plan['limits'][x] for x in ('work_seconds','cleanup_reserve_seconds','terminal_reserve_seconds')),45)
    def test_control_only_loader_boundaries_without_import(self):
        tree=ast.parse(Path(controls.__file__).read_text())
        loader=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='load_control_helper')
        text=ast.unparse(loader)
        self.assertLess(text.index('before = helper_hashes'),text.index('importlib.import_module'))
        self.assertLess(text.index('importlib.import_module'),text.index('after = helper_hashes'))
        self.assertIn('module.__file__',text);self.assertIn('sys.dont_write_bytecode = True',text)
        self.assertNotIn('fixed.load_helper(',ast.unparse(tree))
        with self.assertRaisesRegex(RuntimeError,'EXECUTION REFUSED'):f.load_helper({})
    def test_exact_source_entries_and_post_stdout_guard(self):
        from unittest.mock import patch
        from types import SimpleNamespace
        import stat
        class Scan:
            def __init__(self,names):self.entries=[SimpleNamespace(name=n,stat=lambda **k:SimpleNamespace(st_mode=stat.S_IFREG)) for n in names]
            def __enter__(self):return iter(self.entries)
            def __exit__(self,*args):pass
        expected={'a.py','MANIFEST.json'}
        for names,valid in [(['a.py','MANIFEST.json'],True),(['a.py'],False),(['a.py','MANIFEST.json','__pycache__'],False),(['a.py','MANIFEST.json','a.pyc'],False)]:
            with patch.object(controls.os,'scandir',return_value=Scan(names)):
                if valid:controls.exact_source_entries('/not-read',expected)
                else:
                    with self.assertRaises(RuntimeError):controls.exact_source_entries('/not-read',expected)
        tree=ast.parse(Path(controls.__file__).read_text());text=ast.unparse(tree)
        self.assertLess(text.index('sys.dont_write_bytecode = True'),text.index('import gxinit_fixed_four'))
        main_guard=text.index("if __name__ == '__main__':")
        self.assertLess(main_guard,text.index('import gxinit_fixed_four'))
        self.assertGreater(text.index("'post stdout flush45s limit'"),text.index('flush=True'))
    def test_control_phase_entry_admission_without_process(self):
        tree=ast.parse(Path(controls.__file__).read_text())
        methods={n.name:n for n in tree.body if isinstance(n,ast.FunctionDef)}
        for name,phase in [('gate','child'),('run_case','registered-parent'),('run_controls','fresh-parent')]:
            text=ast.unparse(methods[name]);self.assertIn(repr(phase),text)
            self.assertNotIn('refuse_controls()',text)
            self.assertIn('read_runtime_request(',text)
        main=ast.unparse(methods['main'])
        self.assertIn('validate_disabled(plan)',main);self.assertIn('refuse_controls()',main)
        self.assertIn("validate_runtime(plan, 'fresh-parent')",main)
        self.assertLess(main.index('validate_runtime('),main.index('run_controls('))
        from unittest.mock import patch
        with patch.object(controls,'validate_root') as root:
            with self.assertRaises(RuntimeError):controls.validate_runtime({},'source-parent')
            root.assert_not_called()
        for args in [(None,None),('/not-read','a'*63),('/not-read','Z'*64)]:
            with self.assertRaises(RuntimeError):controls.request_arguments(*args)
    def test_root_phase_policy_without_filesystem(self):
        from unittest.mock import patch
        from types import SimpleNamespace
        import stat
        staged=controls.CALLER_ROOT;execution=controls.ROOT
        fake=SimpleNamespace(st_mode=stat.S_IFDIR|0o700,st_uid=501)
        with patch.object(Path,'exists',return_value=False),patch.object(Path,'is_symlink',return_value=False),patch.object(Path,'cwd',return_value=staged):
            controls.validate_root('fresh-parent')
        with patch.object(Path,'lstat',return_value=fake),patch.object(Path,'is_symlink',return_value=False),patch.object(controls.os,'getuid',return_value=501),patch.object(Path,'cwd',return_value=staged):
            controls.validate_root('registered-parent')
            with self.assertRaises(RuntimeError):controls.validate_root('child')
        with patch.object(Path,'lstat',return_value=fake),patch.object(Path,'is_symlink',return_value=False),patch.object(controls.os,'getuid',return_value=501),patch.object(Path,'cwd',return_value=execution):
            controls.validate_root('child')
            with self.assertRaises(RuntimeError):controls.validate_root('registered-parent')
    def test_budget_check_immediately_precedes_spawn(self):
        tree=ast.parse(Path(controls.__file__).read_text())
        case=next(n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='run_case')
        matches=[]
        for node in ast.walk(case):
            if isinstance(node,ast.Try):
                for index,statement in enumerate(node.body):
                    if isinstance(statement,ast.Assign) and isinstance(statement.value,ast.Call) and ast.unparse(statement.value.func)=='subprocess.Popen':
                        self.assertGreater(index,0)
                        self.assertEqual(ast.unparse(node.body[index-1]),'check()');matches.append(statement)
        self.assertEqual(len(matches),1)
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
