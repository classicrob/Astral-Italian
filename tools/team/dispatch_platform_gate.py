"""Isolated exact nine-key gate for ONE pinned Python identity/mapping payload."""
import json
import os
from pathlib import Path
import signal
import stat
import sys
import time

import dispatch_exec_gate as original
import dispatch_platform_exact as exact


def check(request,checkpoint=None):
    fields={'nonce','argv','cwd','env','target','target_sha256','gate_sha256','python_sha256',
            'operator','base_env','full_env_sha256','source_files'}
    if type(request) is not dict or set(request)!=fields:raise RuntimeError('platform request fields')
    nonce=request['nonce']
    if type(nonce) is not str or len(nonce)!=32 or any(c not in '0123456789abcdef' for c in nonce):raise RuntimeError('platform nonce')
    exact.source_check(request['source_files'])
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
            or target.resolve()!=Path(sys.executable).resolve()):raise RuntimeError('exact Python target drift')
    if request['argv']!=exact.payload(request['target'],op):raise RuntimeError('only exact Python mapping payload allowed')
    if checkpoint is not None:
        original.publish(checkpoint,{'operator_sha256':op['sha256'],'full_env_sha256':digest,
                                     'exact_nine_key_equal':True,'cwd_equal':True})
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
