"""DISABLED <=60s chosen-tool proposal. No compile, app or fixture repetition.

Requires an independently reviewed, explicitly enabled plan AND new allocation.
"""
import argparse
import fcntl
import hashlib
import json
import os
from pathlib import Path
import sys
import time
import traceback

import dispatch_full_budget as budget
import dispatch_exec_gate as gate

ROOT = Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-gate-check-v9-r1'
COMMANDS = [[sys.executable,'-c','import os; print("SOLP_EXEC_IDENTITY",os.getpid(),os.getpgrp(),os.getsid(0),os.getuid())'],
            ['ninja','--version'], ['git','--version']]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plan',type=Path,required=True)
    parser.add_argument('--execute-plan-sha256',required=True)
    args=parser.parse_args()
    raw=args.plan.read_bytes();plan=json.loads(raw)
    if gate.digest(args.plan)!=args.execute_plan_sha256 or plan['execution_enabled'] is not True:
        raise RuntimeError('chosen-tool execution remains disabled/unreviewed')
    if plan['root']!=str(ROOT) or plan['commands']!=COMMANDS or plan['total_seconds']!=60:
        raise RuntimeError('chosen-tool scope drift')
    for name,pin in plan['source_files'].items():
        if Path(name).name!=name or gate.digest(Path(__file__).with_name(name))!=pin:
            raise RuntimeError('chosen-tool source pin drift')
    context=budget.launch_context(ROOT)
    if context!=plan['launch_context'] or ROOT.exists() or os.getpriority(os.PRIO_PROCESS,0)<19:
        raise RuntimeError('context/fresh output/nice19 mismatch')
    if budget.shutil.disk_usage(ROOT.parent).free<50*1024**3:
        raise RuntimeError('disk floor')
    lock=(Path.home()/'MacioGalaxy2-team/slots/native.lock').open('a')
    fcntl.lockf(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
    start=time.monotonic();error=None;job=None
    os.umask(0o077);ROOT.mkdir(mode=0o700);(ROOT/'tmp').mkdir();work=ROOT/'work';work.mkdir()
    (ROOT/'invocation.json').write_bytes(raw)
    os.environ.clear();os.environ.update(context['env'])
    try:
        with budget.FullBudgetJob(work,20,ROOT,50*1024**3,64*1024**2,0.05,launch=context) as job:
            for index,argv in enumerate(COMMANDS):
                job.run(argv,log='command-%d.log'%index)
            launches=sorted(work.glob('launch-*'))
            if len(launches)!=3:raise RuntimeError('missing actual launch records')
            first=json.loads((launches[0]/'identity.json').read_text())
            reported=(work/'command-0.log').read_text().strip().split()
            if reported!=['SOLP_EXEC_IDENTITY',*(str(first[k]) for k in ('pid','pgid','sid','uid'))]:
                raise RuntimeError('post-exec PID/group/session/UID differs from verified gate')
            if (work/'command-1.log').read_text().strip()!='1.13.2':
                raise RuntimeError('chosen Ninja version mismatch')
            if not (work/'command-2.log').read_bytes().strip():
                raise RuntimeError('missing actual Git output')
    except BaseException as exc:
        error={'error':repr(exc),'traceback':traceback.format_exc()}
        # Unknown ownership receives NO further signal or group probe. Gates
        # that never exec independently expire10s; failed state stays failed.
        until=min(start+59,time.monotonic()+10.1)
        while time.monotonic()<until:time.sleep(min(0.05,max(0.0,until-time.monotonic())))
    finally:
        try:
            elapsed=time.monotonic()-start
            allocated=budget.bounded_allocated(ROOT)
            report={'status':'CHOSEN_TOOL_COMPONENT_PASS' if error is None and elapsed<=60 and allocated<=64*1024**2 else 'FAIL_CLOSED',
                    'seconds':elapsed,'allocated_bytes':allocated,'error':error,
                    'full_proof_authorized':False,'plan_sha256':hashlib.sha256(raw).hexdigest(),
                    'remaining_registered_groups':list(job.groups) if job is not None else [],
                    'scope':'three exact commands; no compile or general descendant/daemon management'}
            (ROOT/'completion.json').write_text(json.dumps(report,indent=2)+'\n')
        finally:fcntl.lockf(lock,fcntl.LOCK_UN);lock.close()
    return 0 if report['status']=='CHOSEN_TOOL_COMPONENT_PASS' else 1


if __name__=='__main__':raise SystemExit(main())
