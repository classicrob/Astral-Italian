"""DISABLED fixed two-case gate observation. No payload path or tool argv."""
import time
CLI_STARTED=time.monotonic()  # Includes later source/import/context work.
import hashlib
import json
import os
from pathlib import Path
import sys
import gxinit_fixed_four as fixed

ROOT=Path('/Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solP/gxinit-before-g-controls-v1')
CASES=('identity-eof','registered-parent-unwind')
OPERATOR={'path':'/Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solL/gxinit-before-g-operator-r1/operator.json','sha256':'c219d2e4827e61690b2f0fe99ddcd3af8c4a05cec3fe70ba4875254dc7e251c7'}
FULL_ENV_SHA='0c37269f97066b52893be54e2c689e6101863159f4ba04dd6580c516b7a6780c'
PYTHON={'path':'/opt/homebrew/opt/python@3.15/bin/python3.15','resolved':'/opt/homebrew/Cellar/python@3.15/3.15.0/Frameworks/Python.framework/Versions/3.15/bin/python3.15','sha256':'c33fec4bd58962690baa18120ba966eecb7ca98bd329d8d94c342e861351fdab'}
LIMITS={'total_seconds':45,'work_seconds':23,'cleanup_reserve_seconds':18,'terminal_reserve_seconds':4,'maximum_allocated_bytes':67108864,'minimum_free_bytes':53687091200,'metadata_reserve_bytes':65536,'gate_alarm_seconds':10,'identity_seconds':8,'direct_wait_seconds':12,'helper_cleanup_interval_seconds':6}


def refuse_controls():
    raise RuntimeError('CONTROLS REFUSED: exact operator/helper/order review and separate allocation required')


def validate_contract(plan,enabled,phase):
    fixed.need(set(plan)=={'schema','execution_enabled','root','cwd','limits','sources','helper','operator','full_env_sha256','base_env','cases','scope','python'},'exact control fields')
    fixed.need(type(plan['schema']) is int and plan['schema']==1 and plan['execution_enabled'] is enabled,'exact control admission bit')
    fixed.need(plan['root']==plan['cwd']==str(ROOT),'fixed control root')
    validate_root(phase)
    fixed.need(plan['limits']==LIMITS,'23work+18cleanup+4terminal inside45')
    fixed.need(plan['python']==PYTHON,'exact M1 interpreter image pin')
    fixed.need(plan['cases']==list(CASES) and plan['scope']=='BEFORE_G_ONLY_NO_PAYLOAD_NO_COMPILER','no tool/payload scope')
    fixed.need(plan['operator']==OPERATOR and plan['full_env_sha256']==FULL_ENV_SHA,'independent exact control-only operator; no ambient fallback')
    fixed.need(set(plan['sources'])=={'gxinit_before_g_controls.py','gxinit_fixed_four.py','dispatch_platform_exact.py'},'exact three sources')
    for name,pin in plan['sources'].items():fixed.need(fixed.exact.sha(Path(__file__).with_name(name))==pin,'control source drift')
    fixed.need(set(plan['helper'])=={'path','sha256','module_closure'} and plan['helper']['sha256']==fixed.SUPERVISOR_SHA,'known helper')
    fixed.need(plan['helper']['path']=='/Users/roberthaisfield/MacioGalaxy2-team/scratch-port/solP/gxinit-helper-0fcd2b4e-source/recovery_caller.py','fixed staged helper entry')
    base=Path(plan['helper']['path']).parent
    fixed.need(set(plan['helper']['module_closure'])=={'recovery_caller.py','recovery_transaction.py','retention_paths.py','symbol_recovery.py'},'four helper files')
    for name,pin in plan['helper']['module_closure'].items():fixed.need(fixed.exact.sha(base/name)==pin,'staged helper drift')
    fixed.need(plan['helper']['module_closure']['recovery_caller.py']==fixed.SUPERVISOR_SHA,'helper entry pin')
    fixed.need(plan['base_env']=={'HOME':'/Users/roberthaisfield','PATH':'/usr/bin:/bin:/usr/sbin:/sbin','LANG':'C','LC_ALL':'C','TMPDIR':str(ROOT/'tmp'),'PYTHONCOERCECLOCALE':'0','PYTHONUTF8':'1','PYTHONNOUSERSITE':'1'},'exact independent base eight')


def validate_root(phase):
    import stat
    fixed.need(phase in ('parent','child'),'known validation phase')
    if phase=='parent':fixed.need(not ROOT.exists() and not ROOT.is_symlink(),'fresh absent control root')
    else:
        for path in (ROOT,ROOT/'tmp'):
            st=path.lstat()
            fixed.need(stat.S_ISDIR(st.st_mode) and not path.is_symlink() and st.st_uid==os.getuid() and stat.S_IMODE(st.st_mode)==0o700,'owned private existing child root/tmp')
        fixed.need(str(Path.cwd())==str(ROOT),'child fixed cwd')


def validate_disabled(plan):validate_contract(plan,False,'parent')


def validate_runtime(plan,phase):
    validate_contract(plan,True,phase)
    verify_python(plan['python'])


def verify_python(record):
    fixed.need(record==PYTHON and str(Path(sys.executable))==record['path'] and str(Path(sys.executable).resolve())==record['resolved'],'actual fixed interpreter path/resolution')
    raw=fixed.bounded(record['resolved'],16*1024**2)
    fixed.need(hashlib.sha256(raw).hexdigest()==record['sha256'],'actual M1 interpreter image drift')


def work_clock(start,now=None):
    import math
    current=time.monotonic() if now is None else now
    fixed.need(all(math.isfinite(x) for x in (start,current)) and 0<=current-start<LIMITS['work_seconds'],'23s sampled work cutoff;18cleanup+4terminal reserved')


def helper_hashes(plan,check):
    record=plan['helper'];base=Path(record['path']).parent;result={}
    for name,pin in record['module_closure'].items():
        check();path=base/name
        fixed.need(not path.is_symlink() and str(path.resolve())==str(path),'fixed helper regular path resolution')
        raw=fixed.bounded(path,1024**2);digest=hashlib.sha256(raw).hexdigest()
        fixed.need(digest==pin,'control helper bytes drift');result[name]=digest;check()
    return result


def load_control_helper(plan,check):
    # Control-only import. Production fixed.load_helper and payload guards stay.
    import importlib
    check();before=helper_hashes(plan,check);base=Path(plan['helper']['path']).parent
    names={name[:-3] for name in before}
    fixed.need(not names.intersection(sys.modules),'no ambient helper module reuse')
    old_path=list(sys.path);old_bytecode=sys.dont_write_bytecode
    try:
        sys.path.insert(0,str(base));sys.dont_write_bytecode=True
        helper=importlib.import_module('recovery_caller');check()
        resolutions={}
        for name in names:
            module=sys.modules.get(name);expected=base/(name+'.py')
            fixed.need(module is not None and Path(module.__file__).resolve()==expected,'actual helper module resolution')
            resolutions[name+'.py']=str(Path(module.__file__).resolve())
        after=helper_hashes(plan,check);fixed.need(after==before,'helper pre/post bytes join')
    finally:sys.path[:]=old_path;sys.dont_write_bytecode=old_bytecode
    return helper,{'pre':before,'post':after,'actual_resolution':resolutions}



def gate(plan_path,plan_sha,identity_path,nonce):
    refuse_controls()  # Independently reviewed control admission not implemented.
    import signal
    signal.signal(signal.SIGALRM,signal.SIG_DFL);signal.alarm(10)
    raw=fixed.bounded(plan_path,131072);fixed.need(hashlib.sha256(raw).hexdigest()==plan_sha,'exact child plan')
    plan=fixed.exact.strict_json(raw)
    validate_runtime(plan,'child')
    env=fixed.load_environment(plan);fixed.exact.exact_env(dict(os.environ),env)
    fixed.need(str(Path.cwd())==plan['cwd'],'child cwd')
    fixed.private_record(identity_path,{'pid':os.getpid(),'pgid':os.getpgrp(),'sid':os.getsid(0),'uid':os.getuid(),'nonce':nonce})
    fixed.need(os.read(0,1)==b'','control requires EOF; any byte including G rejects')
    fixed.exact.exact_env(dict(os.environ),env);validate_runtime(plan,'child')
    signal.alarm(0)
    # No exec, payload dispatch, target argument or source/compiler operation.


class BeforeGUnwind(RuntimeError):pass


def run_case(case,plan,plan_path,plan_sha,sup,helper,env,check):
    refuse_controls()
    import signal,subprocess,threading
    fixed.need(case in CASES and not sup.children and threading.active_count()==1,'fixed serial case')
    root=Path(plan['root']);nonce=os.urandom(16).hex();identity_path=root/(case+'.identity.json')
    argv=[plan['python']['path'],str(Path(__file__).resolve()),'--before-g-child',str(plan_path),plan_sha,str(identity_path),nonce]
    pending=None;verified=False;primary=None;cleanup_error=None;injected=False
    state={'attempted':False,'result':None,'error':None}
    try:
        with (root/(case+'.log')).open('xb') as log:
            mask=signal.pthread_sigmask(signal.SIG_BLOCK,{signal.SIGTERM,signal.SIGINT,signal.SIGHUP})
            try:
                pending=subprocess.Popen(argv,cwd=plan['cwd'],env=env,stdin=subprocess.PIPE,stdout=log,stderr=subprocess.STDOUT,start_new_session=True,
                    preexec_fn=lambda:signal.pthread_sigmask(signal.SIG_SETMASK,mask))
                until=time.monotonic()+8
                while not identity_path.exists():
                    check();fixed.need(pending.poll() is None and time.monotonic()<until,'control identity unavailable');time.sleep(.02)
                identity=fixed.joined_identity(pending.pid,os.getpgid(pending.pid),os.getsid(pending.pid),fixed.exact.strict_json(fixed.bounded(identity_path,16384)),os.getuid(),nonce)
                fixed.private_record(root/(case+'.registered.json'),{'identity':identity,'plan_sha256':plan_sha,'python':plan['python'],'payload_permitted':False})
                sup.children.append(pending);verified=True;sup.events.append({'registered_before_g':identity});sup.persist()
            finally:signal.pthread_sigmask(signal.SIG_SETMASK,mask)
            if case=='registered-parent-unwind':
                injected=True;raise BeforeGUnwind('controlled parent exception BEFORE G; not an OS signal test')
    except BaseException as error:primary=error
    finally:
        if pending is not None:
            try:fixed.cleanup_step_once(pending,verified,False,sup,helper,state)
            except BaseException as error:cleanup_error=error
            # No fallback cleanup. Unknown identity retains group UNKNOWN, no signal.
            try:
                fixed.private_record(root/(case+'.outcome.json'),{'case':case,'identity_verified':verified,'injected_parent_unwind':injected,
                    'os_signal_test':False,'ack_attempted':False,'payload_permitted':False,'direct_status':pending.returncode,
                    'primary_error':repr(primary) if primary is not None else None,'cleanup_error':repr(cleanup_error) if cleanup_error is not None else None,'cleanup':state})
            except BaseException as record_error:
                if cleanup_error is None:cleanup_error=record_error
    expected_unwind=case=='registered-parent-unwind' and type(primary) is BeforeGUnwind and injected
    if cleanup_error is not None or (primary is not None and not expected_unwind):
        raise fixed.StepFailure(repr(primary) if primary is not None else None,repr(cleanup_error) if cleanup_error is not None else None) from primary
    fixed.need(verified and pending.returncode==0 and state['result']=={'leaders_reaped':True,'groups_absent':True,'error':None} and not sup.children,'strict before-G outcome')
    return {'case':case,'status':'BEFORE_G_ONLY','direct_status':pending.returncode,'os_signal_test':False,'kernel_uid_observed':False,'compiler_cleanup_qualified':False}


def run_controls(plan,plan_path,plan_sha,started):
    refuse_controls()
    import shutil,signal
    # Clock comes from CLI entry, not after loader/context. No new work
    # after23; the ONE active handle may use12+6cleanup, then4terminal.
    start=started;work_clock(start);validate_runtime(plan,'parent');work_clock(start)
    env=fixed.load_environment(plan);work_clock(start)
    # Explicit operator mapping, never ambient inheritance or ignored extras.
    # Only this standalone control process is changed; values stay private.
    os.environ.clear();os.environ.update(env)
    fixed.exact.exact_env(dict(os.environ),env);work_clock(start)
    helper,resolution=load_control_helper(plan,lambda:work_clock(start))
    fixed.need(os.getpriority(os.PRIO_PROCESS,0)>=19 and not ROOT.exists(),'nice19/fresh root')
    results=[];failure=None;sup=None
    def check():
        began=time.monotonic();work_clock(start,began)
        fixed.need(shutil.disk_usage(ROOT.parent).free>=50*1024**3+65536,'floor50+reserve')
        count=0;allocated=0
        if ROOT.exists():
            for base,dirs,files in os.walk(ROOT,followlinks=False):
                count+=len(dirs)+len(files);fixed.need(count<=128,'finite control outputs')
                for name in dirs:fixed.need(not (Path(base)/name).is_symlink(),'no directory symlink')
                for name in files:
                    st=(Path(base)/name).lstat();fixed.need(not (Path(base)/name).is_symlink(),'no output symlink')
                    allocated+=st.st_blocks*512
                fixed.need(allocated<=64*1024**2-65536 and time.monotonic()-began<=1,'sampled control size/scan')
    def interrupted(sig,frame):raise RuntimeError('control parent interrupt '+str(sig))
    with helper.exclusive_lock(Path.home()/'MacioGalaxy2-team/slots/native.lock'):
        check();os.umask(0o077);ROOT.mkdir(mode=0o700);(ROOT/'tmp').mkdir()
        fixed.private_record(ROOT/'helper-resolution.json',resolution)
        fixed.private_record(ROOT/'invocation.json',{'plan_sha256':plan_sha,'python':plan['python'],'operator':plan['operator'],'full_env_sha256':plan['full_env_sha256'],'started_monotonic':start,'limits':plan['limits']})
        previous={sig:signal.signal(sig,interrupted) for sig in (signal.SIGTERM,signal.SIGINT,signal.SIGHUP)}
        try:
            sup=helper.GroupSupervisor(ROOT/'children.json',check,ROOT/'STOP')
            for case in CASES:
                check();results.append(run_case(case,plan,plan_path,plan_sha,sup,helper,env,check));check()
        except BaseException as error:failure=repr(error);raise
        finally:
            # No second cleanup attempt. A deadline error is separate from the
            # original case/cleanup error. No terminal work can launch a child.
            elapsed=time.monotonic()-start;deadline_error=None
            if elapsed>41:deadline_error='18s cleanup/report reservation exceeded before terminal report'
            try:
                report={'results':results,'error':failure,'deadline_error':deadline_error,'elapsed_before_report':elapsed,
                    'remaining_registered_pids':[p.pid for p in sup.children] if sup is not None else [],'actual_signals_tested':False,
                    'compiler_qualified':False,'context_values':'PRIVATE_M1_ONLY','limits':plan['limits'],
                    'status':'FAIL_CLOSED' if failure is not None or deadline_error is not None else 'CASES_COMPLETE_POSTWRITE_TIME_CHECK_REQUIRED'}
                try:
                    fixed.private_record(ROOT/'completion.json',report)
                    final_elapsed=time.monotonic()-start
                    # Report status is conditional on this post-write check,
                    # process exit and any terminal-failure record. No claimed
                    # bound on a stalled syscall or universal self-monitoring.
                    fixed.need(final_elapsed<=45 and deadline_error is None,'45s total including terminal report')
                    fixed.private_record(ROOT/'terminal-report.json',{'status':'COMPLETE' if failure is None else 'FAIL_CLOSED',
                        'elapsed_seconds':final_elapsed,'primary_error':failure,'compiler_qualified':False,
                        'acceptance_requires':'exit0, final stdout elapsed<=45, no terminal-failure.json and strict per-case cleanup'})
                    final_elapsed=time.monotonic()-start
                    fixed.need(final_elapsed<=45,'post terminal-write45s limit')
                    print(json.dumps({'status':'COMPLETE' if failure is None else 'FAIL_CLOSED','elapsed_seconds':final_elapsed,'compiler_qualified':False}),flush=True)
                except BaseException as terminal_error:
                    try:fixed.private_record(ROOT/'terminal-failure.json',{'terminal_error':repr(terminal_error),'elapsed_seconds':time.monotonic()-start,'primary_error':failure,'status':'FAIL_CLOSED'})
                    except BaseException:pass
                    if failure is None:raise
            finally:
                for sig,handler in previous.items():signal.signal(sig,handler)



def main():
    if len(sys.argv)==6 and sys.argv[1]=='--before-g-child':
        gate(sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5]);return
    import argparse
    parser=argparse.ArgumentParser();parser.add_argument('--plan',type=Path,required=True);parser.add_argument('--sha256',required=True);args=parser.parse_args()
    raw=fixed.bounded(args.plan,131072);fixed.need(hashlib.sha256(raw).hexdigest()==args.sha256,'control plan raw pin')
    plan=fixed.exact.strict_json(raw);validate_disabled(plan);work_clock(CLI_STARTED)
    refuse_controls()  # BEFORE helper import, lock, environment change or output.
    run_controls(plan,args.plan,args.sha256,CLI_STARTED)

if __name__=='__main__':main()
