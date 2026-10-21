"""Isolated exact-nine gate for five fixed tools-v1 command profiles."""
import json
import os
from pathlib import Path
import signal
import stat
import sys
import time

import dispatch_exec_gate as original
import dispatch_platform_exact as exact
import dispatch_tools_profile as profile


def check(request,checkpoint=None):
    fields={'nonce','argv','cwd','env','target','target_sha256','gate_sha256','python_sha256',
            'operator','base_env','full_env_sha256','source_files','profile','plan_reference'}
    if type(request) is not dict or set(request)!=fields:raise RuntimeError('platform request fields')
    nonce=request['nonce']
    if type(nonce) is not str or len(nonce)!=32 or any(c not in '0123456789abcdef' for c in nonce):raise RuntimeError('platform nonce')
    reference=request['plan_reference']
    if type(reference) is not dict or set(reference)!={'path','sha256'}:raise RuntimeError('tools plan reference')
    path=Path(reference['path'])
    if path.is_symlink() or path.stat().st_size>131072 or exact.sha(path)!=reference['sha256']:raise RuntimeError('immutable bounded tools plan')
    plan=exact.strict_json(path.read_bytes());profile.validate_plan(plan)
    if plan['execution_enabled'] is not True:raise RuntimeError('tools plan disabled')
    if request['profile'] not in profile.LABELS:raise RuntimeError('fixed tools profile required')
    if request['profile']=='compiler_rule':raise RuntimeError('compiler profile child-group lifecycle unqualified')
    index=profile.LABELS.index(request['profile']);argv=profile.commands(plan)[index]
    if (request['source_files']!=plan['source_files'] or request['operator']!=plan['operator']
            or request['base_env']!=plan['base_env'] or request['full_env_sha256']!=plan['full_env_sha256']):
        raise RuntimeError('request/operator/plan joins')
    if exact.sha(__file__)!=request['gate_sha256'] or exact.sha(sys.executable)!=request['python_sha256']:
        raise RuntimeError('platform gate/interpreter drift')
    op=request['operator']
    if type(op) is not dict or set(op)!={'path','sha256'}:raise RuntimeError('operator reference shape')
    env,digest=exact.load_operator(op['path'],op['sha256'],request['base_env'])
    if request['env']!=env or request['full_env_sha256']!=digest:raise RuntimeError('request/operator full mapping join')
    exact.exact_env(dict(os.environ),env)
    if str(Path.cwd().resolve())!=request['cwd']:raise RuntimeError('exact platform cwd mismatch')
    target=Path(request['target']);mode=target.stat().st_mode
    if (not target.is_absolute() or str(target.resolve())!=request['target'] or not stat.S_ISREG(mode)
            or mode&(stat.S_ISUID|stat.S_ISGID) or exact.sha(target)!=request['target_sha256']
            or target.resolve()!=Path(argv[0]).resolve()):raise RuntimeError('exact profile target drift')
    if request['argv']!=argv or request['target']!=str(Path(argv[0]).resolve()):raise RuntimeError('only exact fixed-profile argv/target allowed')
    if request['cwd']!=str(profile.ROOT/'fixture'):raise RuntimeError('fixed tools fixture cwd')
    if checkpoint is not None:
        original.publish(checkpoint,{'operator_sha256':op['sha256'],'full_env_sha256':digest,
                                     'exact_nine_key_equal':True,'cwd_equal':True,'profile':request['profile']})
    return str(target)


def main():
    if len(sys.argv)!=3:raise RuntimeError('platform request path/SHA required')
    signal.signal(signal.SIGALRM,signal.SIG_DFL);signal.alarm(10)
    path=Path(sys.argv[1]);pin=sys.argv[2]
    if path.is_symlink() or exact.sha(path)!=pin:raise RuntimeError('immutable platform request pin')
    request=exact.strict_json(path.read_bytes())
    target=check(request,path.parent/'before-identity-context.json')
    original.publish(path.parent/'identity.json',{'pid':os.getpid(),'pgid':os.getpgrp(),'sid':os.getsid(0),'uid':os.getuid()})
    # Same target shape accepted parent already joins to immutable request SHA.
    original.publish(path.parent/'target.json',{'pid':os.getpid(),'request_sha256':pin,'target':target,
                                              'target_sha256':request['target_sha256'],'cwd':request['cwd']})
    until=time.monotonic()+8;ack=path.parent/'ack.json'
    while not ack.exists():
        if time.monotonic()>=until:raise RuntimeError('platform acknowledgement deadline')
        time.sleep(.01)
    if ack.is_symlink() or exact.strict_json(ack.read_bytes())!={'pid':os.getpid(),'request_sha256':pin}:
        raise RuntimeError('platform acknowledgement mismatch')
    if exact.sha(path)!=pin:raise RuntimeError('platform request changed after ack')
    check(request,path.parent/'after-ack-context.json')
    signal.alarm(0);os.execv(target,request['argv'])


if __name__=='__main__':main()
