"""SOURCE candidate: four exact direct LLVM/fixture commands; admission disabled.

No import launches a child. No new kill/escalation implementation. Existing L
GroupSupervisor supplies verified-group cleanup only after exact source review.
"""
import hashlib
import json
import os
from pathlib import Path
import shlex
import stat
import sys
import time
import dispatch_platform_exact as exact

AUTHORITY=Path.home()/'MacioGalaxy2-team/scratch-port/solT/gx-init-defaults-candidate-r1/FOCUSED-COMMANDS-v2.json'
AUTHORITY_SHA='9789adaecb5a1572ed0be3da599132a5a2bd98afee6a1f8d67a0f6b18b69003c'
IDS=('compile-old','compile-candidate','old-negative','candidate')
EXPECTED=(0,0,17,0)
SUPERVISOR_SHA='0fcd2b4e74a2423200065aafd4248f6589791bf9b2348726122bf5b2fc4ee5b0'


def need(ok,message):
    if not ok:raise RuntimeError('fixed GXInit: '+message)

def refuse_execution():
    raise RuntimeError('EXECUTION REFUSED: exact helper cleanup ordering and fixed-child ownership controls not qualified')

def bounded(path,cap):
    p=Path(path);s=p.lstat();need(stat.S_ISREG(s.st_mode) and s.st_size<=cap,'bounded regular record')
    with p.open('rb') as f:raw=f.read(cap+1)
    need(len(raw)<=cap,'record grew beyond cap');return raw

def authority():
    raw=bounded(AUTHORITY,131072);need(hashlib.sha256(raw).hexdigest()==AUTHORITY_SHA,'exact9789 authority')
    d=exact.strict_json(raw);need([r['id'] for r in d['steps']]==list(IDS),'exact four IDs')
    need([r['expected_exit'] for r in d['steps']]==list(EXPECTED) and [r['seconds'] for r in d['steps']]==[90,90,5,5],'exact statuses/timeouts')
    return d

def validate_disabled(plan):
    need(set(plan)==PLAN_FIELDS,'exact source plan fields')
    need(type(plan['schema']) is int and plan['schema']==1 and plan['execution_enabled'] is False,'disabled source schema')
    need(plan['status']=='SOURCE_CANDIDATE_CLEANUP_AND_FIXED_CHILD_CONTROLS_OPEN','source status')
    need(plan['authority']=={'path':str(AUTHORITY),'sha256':AUTHORITY_SHA},'authority reference')
    a=authority();need(plan['steps']==a['steps'],'all argv bytes/order/statuses bound')
    need(plan['cwd']==a['cwd'] and plan['output']==a['execution_output'],'exact cwd/output')
    need(plan['supervisor']['sha256']==SUPERVISOR_SHA,'exact existing supervisor pin')
    need(plan['limits']=={'total_seconds':240,'work_seconds':210,'verified_cleanup_seconds':6,'unknown_gate_wait_seconds':12,'terminal_reserve_seconds':18,'maximum_allocated_bytes':536870912,'minimum_free_bytes':53687091200,'log_bytes_each':8388608},'fixed total/resource proposal')
    need(plan['operator'] is None and plan['full_env_sha256'] is None,'no guessed operator/context admission')
    need(set(plan['sources'])=={'gxinit_fixed_four.py','dispatch_platform_exact.py'},'exact caller source inventory')
    for name,pin in plan['sources'].items():need(exact.sha(Path(__file__).with_name(name))==pin,'caller source drift')
    return a


def joined_identity(pid,pgid,sid,reported,parent_uid,nonce):
    need(type(pid) is int and pid>0 and pgid==pid and sid==pid,'actual kernel PID/PGID/SID join failed')
    need(set(reported)=={'pid','pgid','sid','uid','nonce'} and all(type(reported[k]) is int for k in ('pid','pgid','sid','uid')),'exact bounded self report')
    need(reported=={'pid':pid,'pgid':pid,'sid':pid,'uid':parent_uid,'nonce':nonce},'child self identity/UID/nonce mismatch')
    return {'pid':pid,'pgid':pgid,'sid':sid,'uid_self_report':reported['uid'],
            'kernel_uid_observed':False,'ownership':'DIRECT_POPEN_CHILD_AND_KERNEL_PGID_SID_ONLY'}


def accept_status(index,rc,cleanup):
    need(type(index) is int and 0<=index<4 and type(rc) is int,'fixed status shape')
    need(type(cleanup) is dict and set(cleanup)=={'leaders_reaped','groups_absent','error'} and cleanup['leaders_reaped'] is True and cleanup['groups_absent'] is True and cleanup['error'] is None,'strict cleanup required BEFORE status')
    need(rc==EXPECTED[index],'unexpected direct step status; no retry')
    return {'step':IDS[index],'exit':rc,'expected':EXPECTED[index],'cleanup':cleanup}


def dependency_records(arm,text,paths,contract,output,source):
    """Pure intake; paths is exact pre-read resolved regular file metadata list."""
    need(arm in ('old','candidate') and len(text.encode())<=2*1024**2,'bounded arm depfile')
    target,rest=text.replace('\\\n',' ').split(':',1)
    need(shlex.split(target)==[str(Path(output)/arm)],'exact MD target')
    names=shlex.split(rest);need(bool(names) and len(names)==len(set(names))<=contract['max_dependencies_per_arm'],'finite unique MD paths')
    need(set(paths)==set(names),'all dependency metadata supplied, none invented')
    allowed=[Path(p) for p in contract['allowed_resolved_roots']];resolved=[];total=0
    for name in names:
        row=paths[name];need(set(row)=={'resolved','bytes','sha256','regular'},'exact dependency row')
        p=Path(row['resolved']);need(row['regular'] is True and p.is_absolute() and type(row['bytes']) is int and row['bytes']>=0,'regular resolved dependency')
        need(any(p==root or p.is_relative_to(root) for root in allowed),'unknown dependency root')
        need(len(row['sha256'])==64,'dependency content pin');total+=row['bytes'];resolved.append(str(p))
    need(len(resolved)==len(set(resolved)),'duplicate resolved dependency')
    need(total<=contract['max_total_dependency_bytes_per_arm'],'finite MD bytes')
    mandatory=set(contract['mandatory_paths_common'])|{str(source)}
    need(mandatory<=set(resolved),'missing mandatory fixture/core/config dependency')
    return {'arm':arm,'count':len(names),'bytes':total,'resolved':resolved}


def linker_witness(log,plan):
    need(len(log.encode())<=8*1024**2,'finite compiler witness log')
    paths={plan['linker']['path'],plan['linker']['resolved']};matches=[]
    for line in log.splitlines():
        try:tokens=shlex.split(line)
        except ValueError:continue
        if tokens and tokens[0] in paths:matches.append(tokens)
    need(len(matches)==1,'unique actual absolute linker witness')
    libraries=[r['path'] for r in plan['input_pins'] if r['path'].endswith(('/libc++.a','/libc++abi.a','/libunwind.a'))]
    need(len(libraries)==3 and all(p in matches[0] for p in libraries),'actual three static libraries witness')
    return {'linker':matches[0][0],'static_libraries':libraries,'compiler_identity':'pre/post pinned tool; witness not reproducible-build proof'}


def runtime_target(plan,index):
    a=authority();target=Path(a['steps'][index]['argv'][0]);st=target.stat()
    need(stat.S_ISREG(st.st_mode) and not st.st_mode&(stat.S_ISUID|stat.S_ISGID),'regular nonprivileged fixed target')
    if index<2:
        pins=[r['sha256'] for r in a['input_pins'] if r['path']==str(target)]
        need(len(pins)==1 and exact.sha(target)==pins[0],'actual fixed clang target pin')
        return pins[0]
    arm=('old','candidate')[index-2];record=exact.strict_json(bounded(Path(plan['output'])/(arm+'.binary.json'),16384))
    need(set(record)=={'path','sha256','bytes'} and record['path']==str(target) and st.st_size==record['bytes'],'fresh compiled target record')
    need(exact.sha(target)==record['sha256'],'compiled target drift')
    return record['sha256']


def private_record(path,value):
    payload=json.dumps(value,sort_keys=True).encode();need(len(payload)<=16384,'small private record')
    fd=os.open(path,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600)
    try:need(os.write(fd,payload)==len(payload),'short metadata write')
    finally:os.close(fd)


def gated_child(plan_path,pin,identity_path,nonce,index):
    refuse_execution()  # No direct child-mode bypass or bool-only activation.
    # Existing source pattern: identity publication, stdin acknowledgement, same-PID exec.
    import signal
    signal.signal(signal.SIGALRM,signal.SIG_DFL);signal.alarm(10)
    raw=bounded(plan_path,131072);need(hashlib.sha256(raw).hexdigest()==pin,'immutable child request')
    plan=exact.strict_json(raw);a=validate_disabled(plan)
    env=load_environment(plan);exact.exact_env(dict(os.environ),env);verify_inputs(a);runtime_target(plan,index)
    need(str(Path.cwd())==plan['cwd'],'actual child cwd')
    private_record(identity_path,{'pid':os.getpid(),'pgid':os.getpgrp(),'sid':os.getsid(0),'uid':os.getuid(),'nonce':nonce})
    need(os.read(0,1)==b'G','no parent acknowledgement; no payload')
    need(exact.sha(plan_path)==pin,'request unchanged after ack');exact.exact_env(dict(os.environ),env)
    verify_inputs(a);runtime_target(plan,index)
    refuse_execution()
    signal.alarm(0);argv=a['steps'][index]['argv'];os.execve(argv[0],argv,env)


def load_environment(plan):
    need(type(plan['operator']) is dict and set(plan['operator'])=={'path','sha256'},'fresh independently pinned operator missing')
    env,digest=exact.load_operator(plan['operator']['path'],plan['operator']['sha256'],plan['base_env'])
    need(digest==plan['full_env_sha256'],'operator context digest');return env


def run_registered_step(plan,index,sup,helper,check,env,invocation_path,invocation_sha):
    refuse_execution()  # Models do not bypass this to launch a child.
    import signal,threading,subprocess
    step=plan['steps'][index];root=Path(plan['output']);nonce=os.urandom(16).hex();identity_path=root/(step['id']+'.identity.json')
    need(threading.active_count()==1 and not sup.children,'fixed serial single-threaded launch')
    gate=[sys.executable,str(Path(__file__).resolve()),'--gated-child',str(invocation_path),invocation_sha,str(identity_path),nonce,str(index)]
    pending=None;verified=False;cleanup=None
    try:
        with (root/(step['id']+'.log')).open('xb') as log:
            mask=signal.pthread_sigmask(signal.SIG_BLOCK,{signal.SIGTERM,signal.SIGINT,signal.SIGHUP})
            try:
                pending=subprocess.Popen(gate,cwd=plan['cwd'],env=env,stdin=subprocess.PIPE,stdout=log,stderr=subprocess.STDOUT,start_new_session=True,
                   preexec_fn=lambda:signal.pthread_sigmask(signal.SIG_SETMASK,mask))
                # Retain pending handle immediately; not yet signalable by helper.
                until=time.monotonic()+8
                while not identity_path.exists():
                    check();need(pending.poll() is None and time.monotonic()<until,'gate identity unavailable');time.sleep(.02)
                identity=joined_identity(pending.pid,os.getpgid(pending.pid),os.getsid(pending.pid),exact.strict_json(bounded(identity_path,16384)),os.getuid(),nonce)
                target_pin=runtime_target(plan,index)
                private_record(root/(step['id']+'.registered.json'),{'identity':identity,'argv':step['argv'],'step':step['id'],'plan_sha256':invocation_sha,'target_sha256':target_pin})
                verified=True;sup.children.append(pending);sup.events.append({'registered':identity,'step':step['id']});sup.persist()
                pending.stdin.write(b'G');pending.stdin.flush();pending.stdin.close()
            finally:signal.pthread_sigmask(signal.SIG_SETMASK,mask)
            until=time.monotonic()+step['seconds']
            while pending.poll() is None:
                check();need(time.monotonic()<until,'fixed step deadline');time.sleep(.2)
    finally:
        if pending is not None:
            if verified:
                # Existing helper only; exact probe-before-reap disposition OPEN.
                sup.stop_all();need(pending.returncode is not None and not helper.group_exists(pending.pid),'strict existing helper cleanup')
                cleanup={'leaders_reaped':True,'groups_absent':True,'error':None}
                sup.children.remove(pending);sup.events.append({'retired':pending.pid,'status':pending.returncode});sup.persist()
            else:
                # Unknown group: close owned channel and await gate EOF/alarm. NO SIGNAL/PROBE.
                if pending.stdin is not None and not pending.stdin.closed:pending.stdin.close()
                try:pending.wait(timeout=12)
                except subprocess.TimeoutExpired:pass
                private_record(root/(step['id']+'.unknown.json'),{'direct_pid':pending.pid,'reaped_status':pending.returncode,'group_absence':'UNKNOWN','signal_sent':False})
    return accept_status(index,pending.returncode,cleanup)


def verify_inputs(a,check=None):
    need(0<len(a['input_pins'])<=64,'fixed finite pin inventory')
    total=0
    for row in a['input_pins']:
        if check is not None:check()
        p=Path(row['path']);st=p.stat();need(stat.S_ISREG(st.st_mode) and st.st_size<=512*1024**2,'finite tool/source input')
        total+=st.st_size;need(total<=1024**3,'finite total pinned input bytes')
        digest=hashlib.sha256()
        with p.open('rb') as f:
            count=0
            while True:
                if check is not None:check()
                block=f.read(1024**2)
                if not block:break
                count+=len(block);need(count<=st.st_size,'pin input grew');digest.update(block)
        need(count==st.st_size and digest.hexdigest()==row['sha256'],'tool/source input drift')


def load_helper(plan):
    refuse_execution()
    import importlib
    record=plan['supervisor'];need(record['path'] is not None and type(record['module_closure']) is dict,'exact M1 helper closure unavailable')
    names={'recovery_caller.py','recovery_transaction.py','retention_paths.py','symbol_recovery.py'}
    need(set(record['module_closure'])==names,'complete known helper import closure')
    base=Path(record['path']).parent
    for name,pin in record['module_closure'].items():need(exact.sha(base/name)==pin,'helper import source pin')
    need(exact.sha(record['path'])==SUPERVISOR_SHA,'supervisor actual source')
    sys.path.insert(0,str(base));helper=importlib.import_module('recovery_caller')
    for name in names:
        module=sys.modules[name[:-3]];need(Path(module.__file__).resolve()==(base/name).resolve(),'helper import resolution')
    return helper


def execute_four(plan,invocation_path,invocation_sha,helper):
    refuse_execution()
    import signal,shutil
    a=authority();need(exact.sha(sys.executable)==a['python']['sha256'],'actual parent interpreter pin')
    env=load_environment(plan);output=Path(plan['output']);start=time.monotonic();deadline=start+210
    need(not output.exists() and os.getpriority(os.PRIO_PROCESS,0)>=19,'fresh output/nice19')
    def check():
        began=time.monotonic();need(began<deadline,'global210s sampled work deadline');need(not (output/'STOP').exists(),'STOP requested')
        need(shutil.disk_usage(output.parent).free>=50*1024**3+65536,'floor50+metadata reserve')
        total=0;count=0
        if output.exists():
            for base,dirs,files in os.walk(output,followlinks=False):
                count+=len(dirs)+len(files);need(count<=10000,'finite owned file/dir inventory')
                for name in dirs:need(not (Path(base)/name).is_symlink(),'no owned directory symlink traversal')
                for name in files:
                    p=Path(base)/name;st=p.lstat();need(stat.S_ISREG(st.st_mode),'owned regular outputs only')
                    total+=st.st_blocks*512;need(total<=512*1024**2-65536,'owned512MiB allocation cap')
                    if p.suffix=='.log':need(st.st_size<=8*1024**2,'finite log cap')
                need(time.monotonic()-began<=1,'sampled metadata scan bound')
    def interrupted(signum,frame):raise RuntimeError('fixed job interrupt '+str(signum))
    previous={sig:signal.signal(sig,interrupted) for sig in (signal.SIGTERM,signal.SIGINT,signal.SIGHUP)}
    sup=None;results=[];failure=None
    try:
        with helper.exclusive_lock(Path.home()/'MacioGalaxy2-team/slots/native.lock'):
            check();os.umask(0o077);output.mkdir(mode=0o700);(output/'tmp').mkdir();verify_inputs(a,check)
            sup=helper.GroupSupervisor(output/'children.json',check,output/'STOP')
            try:
                for index,step in enumerate(plan['steps']):
                    check();verify_inputs(a,check)
                    result=run_registered_step(plan,index,sup,helper,check,env,invocation_path,invocation_sha)
                    if index<2:
                        arm=('old','candidate')[index];dep=bounded(output/(arm+'.d'),2*1024**2).decode();target,body=dep.replace('\\\n',' ').split(':',1)
                        metadata={};names=shlex.split(body);contract=a['dependency_contract_v2'];total_dep=0
                        need(0<len(names)==len(set(names))<=contract['max_dependencies_per_arm'],'finite unique MD inputs before read')
                        for name in names:
                            check();path=Path(name);path=path if path.is_absolute() else Path(plan['cwd'])/path;path=path.resolve();st=path.stat()
                            need(any(path==Path(root) or path.is_relative_to(Path(root)) for root in contract['allowed_resolved_roots']),'unknown dependency root before read')
                            need(stat.S_ISREG(st.st_mode) and st.st_size<=128*1024**2,'finite dependency read')
                            total_dep+=st.st_size;need(total_dep<=contract['max_total_dependency_bytes_per_arm'],'total finite dependency bytes before read')
                            metadata[name]={'resolved':str(path),'bytes':st.st_size,'sha256':exact.sha(path),'regular':True}
                        sources=[x for x in a['steps'][index]['argv'] if x.endswith('/fixture-source/'+arm+'.cpp')]
                        need(len(sources)==1,'exact arm source token')
                        result['dependencies']=dependency_records(arm,dep,metadata,a['dependency_contract_v2'],output,Path(sources[0]))
                        result['dependency_metadata']=metadata;result['depfile_sha256']=exact.sha(output/(arm+'.d'))
                        result['witness']=linker_witness(bounded(output/(step['id']+'.log'),8*1024**2).decode(errors='strict'),a)
                        binary=output/arm;st=binary.stat();need(stat.S_ISREG(st.st_mode) and st.st_size<=128*1024**2,'bounded compiled output')
                        private_record(output/(arm+'.binary.json'),{'path':str(binary),'sha256':exact.sha(binary),'bytes':st.st_size})
                    check();results.append(result)
                    # Fixed small status file, not a fullproof certificate.
                    payload=json.dumps(results).encode();need(len(payload)<=8*1024**2,'finite steps/dependency metadata')
                    with (output/'steps.json').open('wb') as f:f.write(payload)
                verify_inputs(a,check);check()
            except BaseException as error:
                failure=repr(error);raise
            finally:
                try:
                    if sup is not None:sup.stop_all()
                except BaseException as cleanup_error:
                    failure='CLEANUP_UNCERTAIN '+repr(cleanup_error);raise
                finally:
                    # Never claims success from leader/ps or locked path alone.
                    private_record(output/'completion.json',{'status':'FOUR_COMMAND_STATUSES_ONLY_SEMANTIC_REVIEW_PENDING' if failure is None else 'FAIL_CLOSED',
                      'error':failure,'steps':len(results),'elapsed':time.monotonic()-start,'compiler_escape_qualified':False,'fixture_semantic_acceptance':False,'kernel_uid_observed':False,
                      'cleanup_scope':'verified registry only; pending failures see unknown records',
                      'owned_group_status':'VERIFIED_LIST_EMPTY_ONLY' if failure is None else 'FAIL_CLOSED_SEE_PENDING_AND_GROUP_JOURNALS'})
    finally:
        for sig,handler in previous.items():signal.signal(sig,handler)


PLAN_FIELDS={'schema','status','execution_enabled','authority','steps','cwd','output','supervisor','limits','operator','full_env_sha256','base_env','sources','actual_control_plan'}

def main():
    if len(sys.argv)==7 and sys.argv[1]=='--gated-child':
        gated_child(sys.argv[2],sys.argv[3],sys.argv[4],sys.argv[5],int(sys.argv[6]));return
    import argparse
    p=argparse.ArgumentParser();p.add_argument('--source-plan',type=Path,required=True);p.add_argument('--sha256',required=True);args=p.parse_args()
    raw=bounded(args.source_plan,131072);need(hashlib.sha256(raw).hexdigest()==args.sha256,'source plan byte pin')
    plan=exact.strict_json(raw);validate_disabled(plan)
    refuse_execution()  # BEFORE lock, output, helper import, env mutation or child.
    helper=load_helper(plan);execute_four(plan,args.source_plan,args.sha256,helper)

if __name__=='__main__':main()
