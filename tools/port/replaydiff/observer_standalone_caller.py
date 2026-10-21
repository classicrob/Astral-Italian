#!/usr/bin/env python3
"""Exact observer14 caller source. Native execution is BLOCKED, no qualified pin."""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import stat
import time
from observer_standalone_supervisor import checked_plan,load_leaves,make_job_class,MECHANISMS

PLAN='ddac833b086fe0cee342610f4b4a48c640b45e1a7345a399ba1458f778ae0414'
LAUNCH='f9518da3f8df36b1c5a0abe007a7635cf1bfe3cfee9df30e681c1af7ac3d1b2f'
INPUTS='397305e1ed271187cec5080b02edf2164cd697e827a4d01b00cf421f9e10bc63'
QUALIFIED_CONTEXT_PIN=None
MAX_REPORT=2*1024**2


def need(value,message):
    if not value:raise RuntimeError(message)


def digest(path,check=lambda:None):
    h=hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda:f.read(1048576),b''):check();h.update(block)
    return h.hexdigest()


def pinned_json(path,pin):
    need(digest(path)==pin,'operator-pinned metadata drift: '+str(path))
    return json.loads(Path(path).read_text())


def verify_record(path,expected,check):
    check();need(digest(path,check)==expected,'actual input/tool/source drift: '+str(path));check()


def revalidate(plan,launch,metadata,check):
    # Explicitly closes run_next's bypass of base.launch_context().
    for path,pin in metadata:verify_record(path,pin,check)
    for path,pin in plan['source_files'].items():verify_record(path,pin,check)
    for record in plan['tool_inputs']:
        need(str(Path(record['path']).resolve())==record['resolved'],'actual tool resolution drift')
        verify_record(record['path'],record['sha256'],check)
    verify_record(launch['python'],launch['python_sha256'],check)
    verify_record(launch['gate'],launch['gate_sha256'],check)
    for path,pin in launch['mechanism_files'].items():verify_record(path,pin,check)
    for argv0,row in launch['targets'].items():
        need(str(Path(argv0).resolve())==row['path'],'chosen target resolution drift')
        verify_record(argv0,row['sha256'],check)
    require_exact_launch_envelope(launch)


def require_exact_launch_envelope(launch):
    expected=launch['env']
    need(type(expected) is dict and all(type(k) is str and type(v) is str for k,v in expected.items()),'invalid exact launch envelope')
    need(dict(os.environ)==expected,'byte-exact validated launch envelope required; no environment keys ignored or installed')


class OwnedState:
    def __init__(self,path):
        self.path=Path(path);self.fd=None
        fd=os.open(self.path,os.O_RDWR|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600)
        try:
            observed=os.fstat(fd)
            need(stat.S_ISREG(observed.st_mode) and observed.st_uid==os.getuid() and observed.st_nlink==1,'fresh STATE ownership unavailable')
            self.identity=(observed.st_dev,observed.st_ino,observed.st_uid)
            self.fd=fd
        except BaseException as error:
            try:os.close(fd)
            except OSError:pass
            raise RuntimeError('retained unverified STATE path; no unlink or rewrite: '+str(self.path)) from error
    def verify(self):
        need(self.fd is not None,'STATE descriptor closed')
        for observed in (os.fstat(self.fd),self.path.lstat()):
            need(stat.S_ISREG(observed.st_mode) and observed.st_nlink==1 and
                 (observed.st_dev,observed.st_ino,observed.st_uid)==self.identity,'owned STATE identity changed; refusing rewrite')
    def update(self,value):
        raw=(json.dumps(value)+'\n').encode();need(len(raw)<=65536,'STATE exceeds bounded metadata')
        self.verify();os.lseek(self.fd,0,os.SEEK_SET);offset=0
        while offset<len(raw):
            try:written=os.write(self.fd,raw[offset:])
            except InterruptedError:continue
            need(written>0,'short STATE write');offset+=written
        os.ftruncate(self.fd,len(raw));os.fsync(self.fd);self.verify()
    def close(self):
        if self.fd is not None:
            fd=self.fd;self.fd=None;os.close(fd)


def consumed_md(path,cwd,inventory,check):
    path=Path(path);need(path.stat().st_size<=4*1024**2,'depfile exceeds bounded metadata')
    raw=path.read_text();target,separator,deps=raw.replace('\\\n',' ').partition(':')
    need(bool(target.strip()) and separator==':','malformed raw -MD output')
    tokens=shlex.split(deps);need(bool(tokens),'empty raw -MD dependency list')
    rows=[]
    for token in tokens:
        check();p=Path(token);p=(p if p.is_absolute() else Path(cwd)/p).resolve()
        need(str(p) in inventory['records'],'unbound consumed dependency: '+str(p))
        record=inventory['records'][str(p)];verify_record(p,record['sha256'],check)
        rows.append(dict(path=str(p),sha256=record['sha256']))
    return dict(depfile_sha256=digest(path,check),dependencies=rows)


def inspect_otool(path,executable):
    path=Path(path);need(path.stat().st_size<=1024**2,'otool log exceeds bounded metadata')
    lines=path.read_text().splitlines();need(len(lines)>=2 and lines[0]==str(executable)+':','otool target/header mismatch')
    for line in lines[1:]:
        need(line.startswith('\t') and '(compatibility version ' in line,'unknown otool output')
        need('libc++' not in line and 'libunwind' not in line,'unexpected dynamic C++/unwind runtime')
    return dict(stdout_sha256=digest(path),libraries=[line.strip() for line in lines[1:]])


def bounded_report(path,result):
    raw=(json.dumps(result,indent=2)+'\n').encode();need(len(raw)<=MAX_REPORT,'final report size bound')
    with Path(path).open('xb') as f:
        need(f.write(raw)==len(raw),'short final report write');f.flush();os.fsync(f.fileno())


def seal(plan_path,launch_path,inputs_path,leaves,caller_sources):
    plan=checked_plan(plan_path,PLAN);launch=pinned_json(launch_path,LAUNCH);inventory=pinned_json(inputs_path,INPUTS)
    need(inventory['status']=='CANDIDATE_INPUT_PINS_ONLY','candidate input inventory incomplete')
    for name,pin in MECHANISMS.items():need(digest(Path(leaves)/(name+'.py'))==pin,'supervision leaf drift')
    return dict(schema_version=1,status='SEALED_SOURCE_ONLY_UNQUALIFIED',execution_enabled=False,
        plan_sha256=PLAN,launch_sha256=LAUNCH,input_inventory_sha256=INPUTS,
        caller_sources={str(Path(p).resolve()):digest(p) for p in caller_sources},
        helper_qualification='71679 actual cwd/env guard FAILED beforeidentity; cause/absence UNKNOWN; newer gate NOT substituted',
        limits=dict(seconds=120,jobs=1,nice=19,output_bytes=268435456),
        actual_native_commands='UNRUN',qualification='BLOCKED',qualified_frames=None)


def execute_after_qualification(contract,plan_path,launch_path,inputs_path,leaves,work,contract_pin):
    started=time.monotonic()
    # No approved actual-context receipt currently exists; caller remains disabled.
    need(QUALIFIED_CONTEXT_PIN is not None and contract_pin==QUALIFIED_CONTEXT_PIN,'native execution blocked:71679 actual context remains UNQUALIFIED')
    contract=pinned_json(contract,contract_pin);need(contract['execution_enabled'] is True,'source-only contract cannot execute')
    for path,pin in contract['caller_sources'].items():verify_record(path,pin,lambda:None)
    plan=checked_plan(plan_path,PLAN);launch=pinned_json(launch_path,LAUNCH);inventory=pinned_json(inputs_path,INPUTS)
    require_exact_launch_envelope(launch)
    need(os.getpriority(os.PRIO_PROCESS,0)>=19,'nice19 required')
    work=Path(work).resolve();need(not work.exists(),'fresh output root required')
    need((Path.home()/'MacioGalaxy2-gamedata') in work.parents,'private output required')
    need(Path(launch['env']['TMPDIR']).parent==work,'work root differs from exact launch context')
    for command in plan['commands']:
        argv=command['argv'];label=command['label']
        if label.startswith('compile-'):
            need(all(Path(argv[argv.index(flag)+1]).parent==work for flag in ('-o','-MF')),'compile output escapes fixed work root')
        elif label.startswith('link-'):need(Path(argv[argv.index('-o')+1]).parent==work,'link output escapes fixed work root')
        elif label.startswith('control-'):need(Path(argv[0]).parent==work,'control outside fixed work root')
        elif label.startswith('dependencies-'):need(Path(argv[-1]).parent==work,'metadata target outside fixed work root')
    metadata=[(plan_path,PLAN),(launch_path,LAUNCH),(inputs_path,INPUTS)]
    # Persistent lock is never removed. Do not queue behind another job.
    lockpath=Path.home()/'MacioGalaxy2-team/slots/native.lock'
    lockfd=os.open(lockpath,os.O_RDWR|os.O_CREAT|os.O_NOFOLLOW,0o600)
    held=False;cleanup_confirmed=False;job=None;result={};state_owner=None
    state=work.parent/(work.name+'.STATE.json')
    try:
        fcntl.lockf(lockfd,fcntl.LOCK_EX|fcntl.LOCK_NB);held=True
        need(shutil.disk_usage(work.parent).free>=50*1024**3,'free-space floor50GiB')
        state_owner=OwnedState(state)
        state_owner.update(dict(host='M1',source=contract['caller_sources'],binary='UNRUN',phase='PREPARE_OWNED_JOB',output=str(work),pid=os.getpid(),resume='retain failure; no retry/partial acceptance'))
        modules=load_leaves(leaves);Job=make_job_class(modules)
        work.mkdir(mode=0o700);(work/'tmp').mkdir(mode=0o700)
        job=Job(work,plan,launch);job.total_started=started;job.absolute_work_end=min(job.absolute_work_end,started+110);result=dict(status='INCOMPLETE',plan_sha256=PLAN,commands=[],deps={},otool={},qualification='BLOCKED',qualified_frames=None)
        with job:
            # All actual input/tool hashing is inside the same110s work budget.
            for real,record in inventory['records'].items():verify_record(real,record['sha256'],job.remaining)
            for command in plan['commands']:
                revalidate(plan,launch,metadata,job.remaining)
                for path,pin in contract['caller_sources'].items():verify_record(path,pin,job.remaining)
                state_owner.update(dict(host='M1',phase=command['label'],output=str(work),pid=os.getpid(),resume='retain failure; owned supervisor cleanup required'))
                job.run_next();label=command['label'];argv=command['argv']
                if label.startswith('compile-'):
                    result['deps'][label]=consumed_md(argv[argv.index('-MF')+1],command['cwd'],inventory,job.remaining)
                if label.startswith('dependencies-'):
                    result['otool'][label]=inspect_otool(work/(label+'.log'),argv[-1])
                revalidate(plan,launch,metadata,job.remaining)
                result['commands'].append(dict(label=label,status='exit0+strict_group_absence'))
            for real,record in inventory['records'].items():verify_record(real,record['sha256'],job.remaining)
        cleanup=json.loads((work/'disk-budget.json').read_text())['cleanup']
        cleanup_confirmed=cleanup['all_owned_groups_absence_confirmed'] is True and not job.groups
        need(cleanup_confirmed,'owned group absence not confirmed')
        result.update(status='STANDALONE_NATIVE_CONTROLS_ONLY',cleanup=cleanup,seconds=time.monotonic()-started)
        need(result['seconds']<=120,'actual total deadline exceeded')
        bounded_report(work/'RESULT.json',result)
        need(time.monotonic()-started<=120,'report exceeded actual total deadline')
    except BaseException as error:
        result.update(status='FAILED_OR_INCOMPLETE',error=repr(error),seconds=time.monotonic()-started)
        if work.exists():
            try:bounded_report(work/'FAILURE.json',result)
            except BaseException:pass
        raise
    finally:
        # Do not describe a failed/unknown group as released or successful.
        try:
            if state_owner is not None:
                try:state_owner.update(dict(host='M1',phase='DONE' if cleanup_confirmed else 'FAILED_OR_UNCERTAIN',output=str(work),pid=os.getpid(),lock_release_claim=cleanup_confirmed,resume='root review; no retry'))
                finally:state_owner.close()
        finally:
            if not held:os.close(lockfd)
            elif job is None or cleanup_confirmed:fcntl.lockf(lockfd,fcntl.LOCK_UN);os.close(lockfd)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('plan','launch','inputs','leaves','contract'):p.add_argument('--'+name,required=True)
    p.add_argument('--work');p.add_argument('--execute-contract-sha256')
    a=p.parse_args()
    if a.execute_contract_sha256:execute_after_qualification(a.contract,a.plan,a.launch,a.inputs,a.leaves,a.work,a.execute_contract_sha256)
    else:
        root=Path(__file__).parent
        r=seal(a.plan,a.launch,a.inputs,a.leaves,[__file__,root/'observer_standalone_supervisor.py',root/'prepare_observer_launch_context.py'])
        bounded_report(a.contract,r);print(json.dumps(dict(status=r['status'],execution_enabled=False,sha256=digest(a.contract))))
