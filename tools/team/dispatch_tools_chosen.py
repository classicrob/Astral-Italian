"""DISABLED fixed tools-v1 component. No fullproof integration."""
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
import dispatch_tools_profile as profile

ROOT=profile.ROOT


def context(plan):return profile.context(plan)

class ToolsJob(original.FullBudgetJob):
    def observe_fixture(self):
        if self.active_launch is not None and self.groups and not self.in_cleanup:
            if context(self.plan)!=self.launch:raise RuntimeError('platform context changed before ack')
            exact.exact_env(dict(os.environ),self.launch['env'])
        return super().observe_fixture()  # Accepted identity+request+target+ack join.

    def run_profile(self,label,argv):
        if (self.groups or self.active_launch is not None or self.launch_number>=len(profile.LABELS)
                or label!=profile.LABELS[self.launch_number]):raise RuntimeError('fixed five-profile order, no retries')
        if context(self.plan)!=self.launch:raise RuntimeError('pinned platform launch context drift')
        exact.exact_env(dict(os.environ),self.launch['env'])
        if argv!=profile.commands(self.plan)[self.launch_number]:raise RuntimeError('only exact fixed-profile argv')
        self.launch_number+=1;directory=self.work/('launch-%04d'%self.launch_number);directory.mkdir(mode=0o700)
        target=self.launch['targets'][argv[0]]
        request={'nonce':os.urandom(16).hex(),'argv':list(argv),'cwd':str(ROOT/'fixture'),
                 'env':self.launch['env'],'target':target['path'],
                 'target_sha256':target['sha256'],'gate_sha256':self.launch['gate_sha256'],
                 'python_sha256':self.launch['python_sha256'],'operator':self.plan['operator'],
                 'base_env':self.plan['base_env'],'full_env_sha256':self.plan['full_env_sha256'],
                 'source_files':self.plan['source_files'],'profile':label,
                 'plan_reference':{'path':str(ROOT/'invocation.json'),'sha256':self.plan_pin}}
        publication.publish(directory/'request.json',request);pin=exact.sha(directory/'request.json')
        self.identities.clear();self.ownership.clear();self.fixture_identity_path=directory/'identity.json'
        row={'directory':directory,'request':request,'pin':pin,'acknowledged':False};self.active_launch=row
        try:
            self.check();self.remaining();p=original.accepted.p
            with (self.work/(label+'.log')).open('xb') as stream:
                if p.threading.active_count()!=1:raise RuntimeError('single-thread mask-restoration requirement')
                mask=p.signal.pthread_sigmask(p.signal.SIG_BLOCK,set(self.previous))
                try:
                    proc=p.subprocess.Popen([self.launch['python'],self.launch['gate'],str(directory/'request.json'),pin],
                         cwd=ROOT/'fixture',stdout=stream,stderr=p.subprocess.STDOUT,start_new_session=True,
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
    # Source-only draft: no enabled-plan bool can qualify the separate child PGID.
    raise RuntimeError('tools bundle blocked: Ninja nonconsole child-group cleanup unqualified')
    if (type(plan['schema']) is not int or plan['schema']!=1 or plan['root']!=str(ROOT)
            or [plan[k] for k in ('total_seconds','work_seconds','cleanup_seconds','maximum_allocated_bytes','minimum_free_bytes')]
                !=[60,20,4,64*1024**2,50*1024**3]):raise RuntimeError('chosen exact scope/limits')
    launch=context(plan)
    if plan['commands']!=profile.commands(plan):raise RuntimeError('only fixed five-profile commands')
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
            reports={}
            with ToolsJob(work,20,ROOT,50*1024**3+65536,64*1024**2-65536,.05,launch=launch) as job:
                job.plan=plan;job.plan_pin=pin;profile.prepare_fixture(plan)
                for index,(label,argv) in enumerate(zip(profile.LABELS,plan['commands']),1):
                    profile.check_fixture(plan)
                    if label=='compiler_rule':
                        for rel in (profile.OBJ,profile.DEP):
                            path=ROOT/'fixture'/rel
                            if path.exists() or path.is_symlink():raise RuntimeError('compiler output/depfile must be fresh')
                    job.run_profile(label,argv)
                    directory=work/('launch-%04d'%index)
                    for name in ('before-identity-context.json','after-ack-context.json'):
                        observed=exact.strict_json((directory/name).read_bytes())
                        expected={'operator_sha256':plan['operator']['sha256'],'full_env_sha256':plan['full_env_sha256'],
                                  'exact_nine_key_equal':True,'cwd_equal':True,'profile':label}
                        if observed!=expected:raise RuntimeError('profile gate-context join differs')
                    text=profile.read_bounded(work/(label+'.log'),65536).decode()
                    if label=='python_context':
                        identity=exact.strict_json((directory/'identity.json').read_bytes())
                        if text.splitlines()!=['SOLP_EXEC_IDENTITY '+' '.join(str(identity[k]) for k in ('pid','pgid','sid','uid')),
                                               'SOLP_POSTEXEC_ENV_SHA256 '+plan['full_env_sha256']]:raise RuntimeError('Python actual mapping/identity differs')
                    elif label=='ninja_version':
                        if text.strip()!='1.13.2':raise RuntimeError('pinned Ninja version')
                    elif label=='git_version':
                        if len(text)>4096 or not text.strip().startswith('git version '):raise RuntimeError('pinned Git version output')
                    elif label=='compiler_rule':
                        obj=ROOT/'fixture'/profile.OBJ
                        reports['ELF']={'sha256':exact.sha(obj),'function':profile.elf_function(profile.read_bounded(obj,1024**2))}
                    elif label=='consumed_deps':
                        dep=ROOT/'fixture'/profile.DEP
                        reports['dependencies']=profile.dependency_join(profile.read_bounded(dep,65536).decode(),text)
                        reports['dependencies']['depfile_sha256']=exact.sha(dep)
                        reports['dependencies']['consumed_log_sha256']=exact.sha(work/(label+'.log'))
                        reports['dependencies']['ninja_deps_sha256']=exact.sha(ROOT/'fixture/.ninja_deps')
                        reports['dependencies']['ninja_log_sha256']=exact.sha(ROOT/'fixture/.ninja_log')
                    profile.check_fixture(plan)
                if context(plan)!=launch:raise RuntimeError('closing tools/operator/source/tool drift')
                exact.exact_env(dict(os.environ),launch['env'])
                publication.publish(work/'artifact-joins.json',reports)
        except BaseException as exc:
            error={'error':repr(exc),'traceback':traceback.format_exc()}
            until=min(start+59,time.monotonic()+10.1)
            while time.monotonic()<until:time.sleep(min(.05,max(0,until-time.monotonic())))
        finally:
            elapsed=time.monotonic()-start;allocated=original.bounded_allocated(ROOT)
            success=(error is None and elapsed<=60 and allocated+65536<=64*1024**2
                     and original.shutil.disk_usage(ROOT).free>=50*1024**3+65536 and not job.groups)
            publication.publish(ROOT/'completion.json',{'status':'CHOSEN_TOOLS_COMPONENT_COMPLETE' if success else 'FAIL_CLOSED',
                 'seconds':elapsed,'allocated_bytes':allocated,'error':error,'plan_sha256':pin,
                 'operator_sha256':plan['operator']['sha256'],'full_env_sha256':plan['full_env_sha256'],
                 'remaining_registered_groups':list(job.groups) if job else [],'fullproof_authorized':False,
                 'scope':'five fixed profiles; one synthetic Wii1.0 chain; native/guestpostexecenv, othercompiler variants and escapeddescendants UNQUALIFIED','retry':False})
    finally:lock.close()
    return 0 if success else 1


if __name__=='__main__':raise SystemExit(main())
