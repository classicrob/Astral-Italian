"""DISABLED isolated chosen Python full-mapping check. No fullproof integration."""
import argparse
import fcntl
import json
import os
from pathlib import Path
import sys
import time
import traceback

import dispatch_full_budget as original
import dispatch_exec_gate as publication
import dispatch_platform_exact as exact

ROOT=Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-platform-chosen-v1'


def context(plan):
    exact.source_check(plan['source_files'])
    base=original.launch_context(ROOT)
    if base['env']!=plan['base_env']:raise RuntimeError('operator independent base eight differs')
    op=plan['operator'];env,digest=exact.load_operator(op['path'],op['sha256'],base['env'])
    if digest!=plan['full_env_sha256']:raise RuntimeError('independent full mapping pin differs')
    if base['python']!=plan['python']['resolved'] or base['python_sha256']!=plan['python']['sha256']:
        raise RuntimeError('chosen Python pin differs')
    return {'python':base['python'],'python_sha256':base['python_sha256'],
            'gate':str(Path(__file__).with_name('dispatch_platform_gate.py').resolve()),
            'gate_sha256':plan['source_files']['dispatch_platform_gate.py'],
            'env':env,'targets':{base['python']:{'path':base['python'],'sha256':base['python_sha256']}}}


class ChosenJob(original.FullBudgetJob):
    def observe_fixture(self):
        if self.active_launch is not None and self.groups and not self.in_cleanup:
            if context(self.plan)!=self.launch:raise RuntimeError('platform context changed before ack')
            exact.exact_env(dict(os.environ),self.launch['env'])
        return super().observe_fixture()  # Accepted identity+request+target+ack join.

    def run_one(self,argv):
        if self.groups or self.active_launch is not None or self.launch_number:
            raise RuntimeError('exactly one chosen command; no retries')
        if context(self.plan)!=self.launch:raise RuntimeError('pinned platform launch context drift')
        exact.exact_env(dict(os.environ),self.launch['env'])
        if argv!=exact.payload(self.launch['python'],self.plan['operator']):raise RuntimeError('only exact Python full-mapping check')
        self.launch_number=1;directory=self.work/'launch-0001';directory.mkdir(mode=0o700)
        request={'nonce':os.urandom(16).hex(),'argv':list(argv),'cwd':str(Path.cwd().resolve()),
                 'env':self.launch['env'],'target':self.launch['python'],
                 'target_sha256':self.launch['python_sha256'],'gate_sha256':self.launch['gate_sha256'],
                 'python_sha256':self.launch['python_sha256'],'operator':self.plan['operator'],
                 'base_env':self.plan['base_env'],'full_env_sha256':self.plan['full_env_sha256'],
                 'source_files':self.plan['source_files']}
        publication.publish(directory/'request.json',request);pin=exact.sha(directory/'request.json')
        self.identities.clear();self.ownership.clear();self.fixture_identity_path=directory/'identity.json'
        row={'directory':directory,'request':request,'pin':pin,'acknowledged':False};self.active_launch=row
        try:
            self.check();self.remaining();p=original.accepted.p
            with (self.work/'python.log').open('xb') as stream:
                if p.threading.active_count()!=1:raise RuntimeError('single-thread mask-restoration requirement')
                mask=p.signal.pthread_sigmask(p.signal.SIG_BLOCK,set(self.previous))
                try:
                    proc=p.subprocess.Popen([self.launch['python'],self.launch['gate'],str(directory/'request.json'),pin],
                         stdout=stream,stderr=p.subprocess.STDOUT,start_new_session=True,
                         env=dict(self.launch['env']),
                         preexec_fn=lambda:p.signal.pthread_sigmask(p.signal.SIG_SETMASK,mask))
                    self.groups[proc.pid]=proc;row['registered_pid']=proc.pid
                finally:p.signal.pthread_sigmask(p.signal.SIG_SETMASK,mask)
                code=self.operation('wait-reap-normal',proc.pid,lambda:proc.wait(timeout=self.remaining()))
                row['tool_exit_status']=code
                if code:raise p.subprocess.CalledProcessError(code,argv)
                if self.alive(proc.pid):raise RuntimeError('leader exited with surviving chosen group')
                del self.groups[proc.pid];self.events.append({'pgid':proc.pid,'leader_reaped_and_group_absent':True})
            self.remaining()
        finally:
            pid=row.get('registered_pid')
            self.launches.append({'request_sha256':pin,'request':request,'registered_pid':pid,
                 'acknowledged':row['acknowledged'],'identity':self.identities.get(pid),
                 'ownership':self.ownership.get(pid),'tool_exit_status':row.get('tool_exit_status'),
                 'retired':pid is not None and pid not in self.groups})
            if not self.groups:self.active_launch=None;self.fixture_identity_path=None


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--plan',type=Path,required=True)
    ap.add_argument('--execute-plan-sha256',required=True);args=ap.parse_args()
    raw=args.plan.read_bytes();plan=exact.strict_json(raw);pin=exact.sha(args.plan)
    if pin!=args.execute_plan_sha256 or plan['execution_enabled'] is not True:
        raise RuntimeError('isolated chosen platform check disabled/unallocated')
    if (type(plan['schema']) is not int or plan['schema']!=1 or plan['root']!=str(ROOT)
            or [plan[k] for k in ('total_seconds','work_seconds','cleanup_seconds','maximum_allocated_bytes','minimum_free_bytes')]
                !=[60,20,4,64*1024**2,50*1024**3]):raise RuntimeError('chosen exact scope/limits')
    launch=context(plan)
    if plan['commands']!=[exact.payload(launch['python'],plan['operator'])]:raise RuntimeError('one fixed Python payload only')
    if ROOT.exists() or ROOT.is_symlink() or os.getpriority(os.PRIO_PROCESS,0)<19:
        raise RuntimeError('fresh root/nice19 required')
    if original.shutil.disk_usage(ROOT.parent).free<50*1024**3+65536:raise RuntimeError('free floor')
    lock=(Path.home()/'MacioGalaxy2-team/slots/native.lock').open('a')
    try:
        fcntl.lockf(lock,fcntl.LOCK_EX|fcntl.LOCK_NB);start=time.monotonic();job=None;error=None
        os.umask(0o077);ROOT.mkdir(mode=0o700);work=ROOT/'work';work.mkdir();(ROOT/'tmp').mkdir()
        (ROOT/'invocation.json').write_bytes(raw)
        os.environ.clear();os.environ.update(launch['env'])
        try:
            with ChosenJob(work,20,ROOT,50*1024**3+65536,64*1024**2-65536,.05,launch=launch) as job:
                job.plan=plan;job.run_one(plan['commands'][0])
                identity=exact.strict_json((work/'launch-0001/identity.json').read_bytes())
                lines=(work/'python.log').read_text().splitlines()
                if lines!=['SOLP_EXEC_IDENTITY '+' '.join(str(identity[k]) for k in ('pid','pgid','sid','uid')),
                           'SOLP_POSTEXEC_ENV_SHA256 '+plan['full_env_sha256']]:
                    raise RuntimeError('postexec identity or exact mapping evidence differs')
                for name in ('before-identity-context.json','after-ack-context.json'):
                    report=exact.strict_json((work/'launch-0001'/name).read_bytes())
                    if report!={'operator_sha256':plan['operator']['sha256'],'full_env_sha256':plan['full_env_sha256'],
                                'exact_nine_key_equal':True,'cwd_equal':True}:raise RuntimeError('gate context join differs')
                if context(plan)!=launch:raise RuntimeError('closing exact platform context drift')
                exact.exact_env(dict(os.environ),launch['env'])
        except BaseException as exc:
            error={'error':repr(exc),'traceback':traceback.format_exc()}
            until=min(start+59,time.monotonic()+10.1)
            while time.monotonic()<until:time.sleep(min(.05,max(0,until-time.monotonic())))
        finally:
            elapsed=time.monotonic()-start;allocated=original.bounded_allocated(ROOT)
            success=(error is None and elapsed<=60 and allocated+65536<=64*1024**2
                     and original.shutil.disk_usage(ROOT).free>=50*1024**3+65536 and not job.groups)
            publication.publish(ROOT/'completion.json',{'status':'ISOLATED_PYTHON_CONTEXT_COMPLETE' if success else 'FAIL_CLOSED',
                 'seconds':elapsed,'allocated_bytes':allocated,'error':error,'plan_sha256':pin,
                 'operator_sha256':plan['operator']['sha256'],'full_env_sha256':plan['full_env_sha256'],
                 'remaining_registered_groups':list(job.groups) if job else [],'fullproof_authorized':False,
                 'scope':'one Python actual mapping only; no Ninja/Git/compiler/other target stability','retry':False})
    finally:lock.close()
    return 0 if success else 1


if __name__=='__main__':raise SystemExit(main())
