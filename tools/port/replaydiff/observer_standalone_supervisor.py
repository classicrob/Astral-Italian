"""SOURCE ONLY, UNQUALIFIED. No executable caller or allocation switch.

Narrow fourteen-command adaptation of the pinned v7/v9 supervision leaves.
Do not call job entry/run APIs without separate root/L review and allocation.
No fullproof monitor, package, publisher or certificate module is imported.
"""
import hashlib
import importlib
import json
import os
from pathlib import Path
import sys
import time

MECHANISMS={
    'dispatch_preflight':'adfaae0ee51350fedb883937ae887f1f1d443f986b0c4b98f334894f266c1d72',
    'dispatch_budget':'166df6ad94ae91090702aa931c35cedd766dca5d505ce2c028c0e6219f43f1a0',
    'dispatch_exec_gate':'4f510bfe6a3e17efbbdcd2395c1712a53d08e89f5b0e8b5b66c5c8bb43cd2c7c',
    'dispatch_full_budget':'b392e6d9723ad64f0eae915c131274b6fac157089a974e94d9b0adb5b44ab6bc',
}
COMPILER='/Users/roberthaisfield/toolchains/LLVM-20.1.8-macOS-ARM64/bin/clang++'
STATIC_TARGETS={COMPILER:dict(path='/Users/roberthaisfield/toolchains/LLVM-20.1.8-macOS-ARM64/bin/clang-20',sha256='44698f93798de0177959f17941dbd889ecd802fabd88124dbe86f5cfd93bc1ba'),
    '/usr/bin/otool':dict(path='/usr/bin/otool',sha256='3573dc149c40b6dbba10a2465334e080da2e5f598b2f14178074ab8ddd013307')}
LABELS=['compile-'+name for name in ('draw_observer','draw_observer_json','draw_observer_tests','draw_pilot_bridge','draw_pilot_bridge_tests','draw_snapshot','draw_snapshot_no_xfb','draw_pilot_host')]+[
    action+'-'+name for name in ('observer-core-controls','pilot-bridge-controls') for action in ('link','control','dependencies')]


def digest(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def checked_plan(path,pin):
    if digest(path)!=pin:raise RuntimeError('operator-pinned command arrays changed')
    plan=json.loads(Path(path).read_text())
    if [c['label'] for c in plan['commands']]!=LABELS:raise RuntimeError('not the exact14-command scope')
    if plan['limits']!=dict(wall_seconds=120,parallel_jobs=1,nice=19,private_output_bytes=268435456):raise RuntimeError('standalone limits changed')
    for c in plan['commands']:
        if c['label'].startswith('compile-') and ('-MD' not in c['argv'] or '-MMD' in c['argv']):raise RuntimeError('system dependency closure omitted')
    return plan


def load_leaves(root):
    root=Path(root).resolve()
    for name,pin in MECHANISMS.items():
        if name in sys.modules:raise RuntimeError('supervision namespace already loaded; isolated controller required')
        if digest(root/(name+'.py'))!=pin:raise RuntimeError('supervision leaf drift: '+name)
    sys.path.insert(0,str(root))
    try:
        modules={name:importlib.import_module(name) for name in MECHANISMS}
        for name,module in modules.items():
            if Path(module.__file__).resolve()!=root/(name+'.py'):raise RuntimeError('supervision import path mismatch')
    finally:sys.path.remove(str(root))
    return modules


def make_job_class(modules):
    accepted=modules['dispatch_budget'];base=modules['dispatch_full_budget'];gate=modules['dispatch_exec_gate']
    class StandaloneObserverJob(base.FullBudgetJob):
        """Serial job only; actual-context lifecycle controls still UNRUN."""
        def __init__(self,work,plan,launch):
            if [c['label'] for c in plan['commands']]!=LABELS:raise RuntimeError('not the exact14-command inventory')
            if launch['targets']!=STATIC_TARGETS:raise RuntimeError('chosen compiler/otool context differs')
            self.total_started=time.monotonic();self.absolute_work_end=self.total_started+110.0
            self.plan=json.loads(json.dumps(plan));self.command_index=0;self.linked_outputs={};self.total_seconds=None
            super().__init__(Path(work),110,root=Path(work),min_free=50*1024**3,
                             max_growth=256*1024**2,interval=0.25,baseline=0,launch=launch)
        def remaining(self):
            remaining=min(super().remaining(),self.absolute_work_end-time.monotonic())
            if remaining<=0:raise accepted.DiskBudgetExceeded('standalone work deadline;10s cleanup/report reserve')
            return remaining
        def run(self,*args,**kwargs):
            raise RuntimeError('generic run disabled; only exact run_next inventory is supported')
        def run_next(self):
            if self.command_index>=14:raise RuntimeError('fourteen-command inventory exhausted')
            command=self.plan['commands'][self.command_index];argv=command['argv'];label=command['label'];cwd=command['cwd']
            if label.startswith(('compile-','link-')) and argv[0]!=COMPILER:raise RuntimeError('foreign compiler target')
            if label.startswith('dependencies-') and argv[0]!='/usr/bin/otool':raise RuntimeError('foreign metadata target')
            if self.groups or self.active_launch is not None:raise RuntimeError('overlapping command forbidden')
            if dict(os.environ)!=self.launch['env']:raise RuntimeError('exact controlled execution environment differs')
            if gate.digest(self.launch['python'])!=self.launch['python_sha256'] or gate.digest(self.launch['gate'])!=self.launch['gate_sha256']:raise RuntimeError('gate/interpreter drift')
            target=self.launch['targets'].get(argv[0])
            if label.startswith('control-'):
                target=self.linked_outputs.get(argv[0])
                if target is None:raise RuntimeError('control binary lacks a successful owned link producer')
            if not target or str(Path(argv[0]).resolve())!=target['path'] or digest(target['path'])!=target['sha256']:raise RuntimeError('chosen tool/output target is not pinned')
            self.launch_number+=1
            directory=self.work/('launch-%04d'%self.launch_number);directory.mkdir(mode=0o700)
            request=dict(nonce=os.urandom(16).hex(),argv=list(argv),cwd=str(Path(cwd).resolve()),env=self.launch['env'],
                         target=target['path'],target_sha256=target['sha256'],gate_sha256=self.launch['gate_sha256'],python_sha256=self.launch['python_sha256'])
            gate.publish(directory/'request.json',request);pin=gate.digest(directory/'request.json')
            self.identities.clear();self.ownership.clear();self.fixture_identity_path=directory/'identity.json'
            self.active_launch=dict(directory=directory,request=request,pin=pin,acknowledged=False);row=self.active_launch
            try:
                self.check();self.remaining();p=accepted.p
                self.journal({'event':'standalone_spawn_intent','request_sha256':pin,'label':label})
                with (self.work/(label+'.log')).open('ab') as stream:
                    if p.threading.active_count()!=1:raise RuntimeError('single controller thread required')
                    mask=p.signal.pthread_sigmask(p.signal.SIG_BLOCK,set(self.previous))
                    try:
                        proc=p.subprocess.Popen([self.launch['python'],self.launch['gate'],str(directory/'request.json'),pin],cwd=cwd,
                            stdout=stream,stderr=p.subprocess.STDOUT,start_new_session=True,
                            preexec_fn=lambda:p.signal.pthread_sigmask(p.signal.SIG_SETMASK,mask))
                        self.groups[proc.pid]=proc;row['registered_pid']=proc.pid
                    finally:p.signal.pthread_sigmask(p.signal.SIG_SETMASK,mask)
                    self.journal({'event':'standalone_spawn_registered','pid':proc.pid,'request_sha256':pin,'label':label})
                    code=self.operation('wait-reap-normal',proc.pid,lambda:proc.wait(timeout=self.remaining()))
                    row['tool_exit_status']=code
                    if code:raise p.subprocess.CalledProcessError(code,argv)
                    if self.alive(proc.pid):raise RuntimeError('leader exited with surviving owned group')
                    del self.groups[proc.pid];self.events.append({'pgid':proc.pid,'leader_reaped_and_group_absent':True})
                self.remaining()
                if label.startswith('link-'):
                    output=Path(argv[argv.index('-o')+1])
                    if output.is_symlink() or output.resolve().parent!=self.work.resolve():raise RuntimeError('linked control output escaped private work root')
                    self.linked_outputs[str(output)]=dict(path=str(output.resolve()),sha256=digest(output),producer=label)
                self.command_index+=1
            finally:
                pid=row.get('registered_pid')
                self.launches.append(dict(label=label,request_sha256=pin,request=request,registered_pid=pid,acknowledged=row['acknowledged'],
                    identity=self.identities.get(pid),ownership=self.ownership.get(pid),tool_exit_status=row.get('tool_exit_status'),retired=pid is not None and pid not in self.groups))
                if not self.groups:self.active_launch=None;self.fixture_identity_path=None
        def __exit__(self,typ,value,tb):
            try:result=super().__exit__(typ,value,tb)
            finally:self.total_seconds=time.monotonic()-self.total_started
            if self.total_seconds>120:raise RuntimeError('standalone total deadline exceeded; no bounded acceptance')
            if typ is None and self.command_index!=14:raise RuntimeError('incomplete fourteen-command scope')
            return result
    return StandaloneObserverJob


if __name__=='__main__':
    raise SystemExit('SOURCE_ONLY_UNQUALIFIED: no executable caller/allocation; do not launch native controls')
