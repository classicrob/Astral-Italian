"""DISABLED exact one-child named-key metadata proposal, not a launch admission."""
import argparse
import fcntl
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import threading
import time
import traceback

import dispatch_budget as accepted
import dispatch_full_budget as finite
import dispatch_named_metadata as metadata

ROOT=Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-platform-named-metadata-v1'
SOURCE_NAMES=('dispatch_named_metadata_run.py','dispatch_named_metadata.py',
              'dispatch_platform_context_proposal.py','dispatch_full_budget.py',
              'dispatch_exec_gate.py','dispatch_budget.py','dispatch_preflight.py')


class MetadataJob(accepted.LiveBudgetJob):
    # Same finite sampled scan, no full caller/gate/admission behavior.
    check=finite.FullBudgetJob.check

    def observe_fixture(self):
        if self.fixture_identity_path is None:return
        if not self.fixture_identity_path.exists():
            return super().observe_fixture()  # Existing missing/uncertain policy.
        try:
            record=metadata.read_stage(self.fixture_identity_path,'childentry',self.plan,self.pin,os.getpid())
            record_sha=metadata.sha(self.fixture_identity_path)
            if getattr(self,'observed_record_sha',record_sha)!=record_sha:raise RuntimeError('immutable child record changed')
            self.observed_record_sha=record_sha
            data=record['identity'];pid=data['pid']
            if pid not in self.groups:raise RuntimeError('identity is not registered child')
            if self.ownership.get(pid,{}).get('status') in ('CONTRADICTED','OBSERVATION_FAILED'):
                raise RuntimeError('prior ownership failure is not recoverable')
            if data['uid']!=os.getuid():raise RuntimeError('child UID differs from controller')
            old=self.identities.get(pid)
            if old is not None and old!=data:raise RuntimeError('immutable child identity changed')
            if old is None:
                observed={'pgid':self.operation('getpgid-observe',pid,lambda:os.getpgid(pid)),
                          'sid':self.operation('getsid-observe',pid,lambda:os.getsid(pid))}
                if observed['pgid']!=pid or observed['sid']!=pid or data['pgid']!=pid or data['sid']!=pid:
                    self.ownership_state(pid,'CONTRADICTED',observed=observed,reported_identity=data)
                    raise RuntimeError('child is not registered new-session leader')
                self.identities[pid]=dict(data)
                self.ownership_state(pid,'VERIFIED',observed=observed,identity=dict(data))
                self.journal({'event':'actual_child_identity','identity':dict(data)})
        except BaseException as error:
            for pid in self.groups:
                if self.ownership.get(pid,{}).get('status')!='CONTRADICTED':
                    self.ownership_state(pid,'OBSERVATION_FAILED',reason=repr(error))
            raise

    def run_observer(self,argv):
        # One fixed metadata child; original atomic spawn/register/mask and
        # journaled wait/reap, followed by strict verified group absence.
        if self.groups or getattr(self,'observer_started',False):raise RuntimeError('only one metadata child permitted')
        expected=[self.plan['python']['resolved'],str(Path(metadata.__file__).resolve()),'--child',
                  '--plan',str(self.root/'invocation.json'),'--plan-sha256',self.pin,'--parent-pid',str(os.getpid())]
        if argv!=expected:raise RuntimeError('only fixed metadata child argv permitted')
        self.observer_started=True
        metadata.source_check(self.plan)
        self.check();self.remaining()
        with (self.work/'child.log').open('xb') as stream:
            if threading.active_count()!=1:raise RuntimeError('single-thread preexec requirement')
            mask=signal.pthread_sigmask(signal.SIG_BLOCK,set(self.previous))
            try:
                proc=subprocess.Popen(argv,stdout=stream,stderr=subprocess.STDOUT,start_new_session=True,
                                      preexec_fn=lambda:signal.pthread_sigmask(signal.SIG_SETMASK,mask))
                self.groups[proc.pid]=proc
            finally:signal.pthread_sigmask(signal.SIG_SETMASK,mask)
            status=self.operation('wait-reap-normal',proc.pid,lambda:proc.wait(timeout=self.remaining()))
            if status:raise subprocess.CalledProcessError(status,argv)
            self.observe_fixture()
            self.require_signal_ownership(proc.pid)
            if self.alive(proc.pid):raise RuntimeError('surviving observer group')
            del self.groups[proc.pid]
            self.fixture_identity_path=None  # Retired only after wait/reap + strict ESRCH.
            self.events.append({'pgid':proc.pid,'leader_reaped_and_group_absent':True})
        self.remaining()


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--plan',type=Path,required=True)
    parser.add_argument('--execute-plan-sha256',required=True);args=parser.parse_args()
    raw=args.plan.read_bytes();plan=metadata.strict_json(raw);pin=metadata.sha(args.plan)
    if pin!=args.execute_plan_sha256 or plan['execution_enabled'] is not True:
        raise RuntimeError('named metadata observation remains disabled/unreviewed')
    if (type(plan['schema']) is not int or plan['schema']!=1 or plan['maximum_records']!=3
            or plan['root']!=str(ROOT) or plan['key']!=metadata.KEY or plan['stages']!=list(metadata.STAGES)
            or plan['total_seconds']!=30 or plan['work_seconds']!=10 or plan['cleanup_seconds']!=4
            or plan['maximum_allocated_bytes']!=64*1024**2 or plan['minimum_free_bytes']!=50*1024**3
            or plan['maximum_record_bytes']!=16384 or plan['maximum_value_utf8_bytes']!=512
            or set(plan['source_files'])!=set(SOURCE_NAMES)):
        raise RuntimeError('exact metadata scope/limits required')
    metadata.source_check(plan)
    context=finite.launch_context(ROOT)
    if context['env']!=plan['eight_key_setup'] or plan['python']['resolved']!=context['python'] or plan['python']['sha256']!=context['python_sha256']:
        raise RuntimeError('exact base context mismatch')
    if ROOT.exists() or ROOT.is_symlink() or os.getpriority(os.PRIO_PROCESS,0)<19:
        raise RuntimeError('fresh private root/nice19 required')
    if finite.shutil.disk_usage(ROOT.parent).free<50*1024**3+65536:raise RuntimeError('free floor')
    lock=(Path.home()/'MacioGalaxy2-team/slots/native.lock').open('a')
    try:
        fcntl.lockf(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        start=time.monotonic();job=None;error=None
        os.umask(0o077);ROOT.mkdir(mode=0o700);work=ROOT/'work';work.mkdir();(ROOT/'tmp').mkdir()
        (ROOT/'invocation.json').write_bytes(raw)
        try:
            metadata.publish(ROOT/'launcherentry.json',metadata.stage_record('launcherentry',metadata.selected_only(os.environ),plan,pin,os.getppid()))
            os.environ.clear();os.environ.update(plan['eight_key_setup'])
            metadata.publish(ROOT/'parentaftereightkeysetup.json',metadata.stage_record('parentaftereightkeysetup',metadata.selected_only(os.environ),plan,pin,os.getppid()))
            argv=[plan['python']['resolved'],str(Path(metadata.__file__).resolve()),'--child',
                  '--plan',str(ROOT/'invocation.json'),'--plan-sha256',pin,'--parent-pid',str(os.getpid())]
            job=MetadataJob(work,10,ROOT,50*1024**3+65536,64*1024**2-65536,0.05,baseline=finite.bounded_allocated(ROOT))
            job.plan=plan;job.pin=pin;job.fixture_identity_path=ROOT/'childentry.json'
            with job:job.run_observer(argv)
            for stage in metadata.STAGES:
                metadata.read_stage(ROOT/(stage+'.json'),stage,plan,pin,os.getpid() if stage=='childentry' else os.getppid())
            metadata.source_check(plan)
        except BaseException as exc:
            error={'error':repr(exc),'traceback':traceback.format_exc()}
            # No added signals/probes: unknown children independently expire10s.
            until=min(start+29,time.monotonic()+10.1)
            while time.monotonic()<until:time.sleep(min(.05,max(0,until-time.monotonic())))
        finally:
            allocated=finite.bounded_allocated(ROOT);elapsed=time.monotonic()-start
            success=(error is None and elapsed<=30 and allocated+65536<=64*1024**2
                     and finite.shutil.disk_usage(ROOT).free>=50*1024**3+65536 and not job.groups)
            report={'status':'NAMED_METADATA_ONLY_COMPLETE' if success else 'FAIL_CLOSED',
                    'seconds':elapsed,'allocated_bytes':allocated,'error':error,'plan_sha256':pin,
                    'remaining_registered_groups':list(job.groups) if job else [],
                    'admission_or_payload_authorized':False,'retry':False,
                    'completion_reserved_bytes':65536,'limits':'sampled syscall/scan bounds, not universal realtime quota'}
            metadata.publish(ROOT/'completion.json',report)
    finally:lock.close()  # Includes every failure after acquisition.
    return 0 if report['status']=='NAMED_METADATA_ONLY_COMPLETE' else 1


if __name__=='__main__':raise SystemExit(main())
