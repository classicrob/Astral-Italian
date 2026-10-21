"""DISABLED single direct-shell source adapter. No fullproof integration."""
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
import dispatch_direct_profile as profile

ROOT=profile.ROOT


def context(plan):return profile.context(plan)

class DirectJob(original.FullBudgetJob):
    def observe_fixture(self):
        if self.active_launch is not None and self.groups and not self.in_cleanup:
            if context(self.plan)!=self.launch:raise RuntimeError('platform context changed before ack')
            exact.exact_env(dict(os.environ),self.launch['env'])
        return super().observe_fixture()  # Accepted identity+request+target+ack join.

    def run_profile(self,label,argv):
        profile.require_admission()  # No process/output before lifecycle qualification.
        if (self.groups or self.active_launch is not None or self.launch_number>=len(profile.LABELS)
                or label!=profile.LABELS[self.launch_number]):raise RuntimeError('fixed one-rule order, no retries')
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
    ap.add_argument('--source-plan-sha256',required=True);args=ap.parse_args()
    raw=profile.bounded(args.plan,131072)
    if exact.sha(args.plan)!=args.source_plan_sha256:raise RuntimeError('immutable disabled plan pin')
    plan=exact.strict_json(raw);profile.validate_plan(plan)
    # Deliberately no lock, root preparation, output, child, or operator env mutation.
    profile.require_admission()


if __name__=='__main__':main()
