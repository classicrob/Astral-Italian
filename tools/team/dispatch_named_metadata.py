"""Metadata ONLY: three selected-key stage records; no payload/ack/admission.

Disabled plan and independent allocation required by launcher. Child self-expires
while exporting only the explicitly named key, actual identity and source pins.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import signal
import stat
import sys
import time

from dispatch_platform_context_proposal import KEY, observe_named_only

CAP = 16384
STAGES = ('launcherentry','parentaftereightkeysetup','childentry')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def strict_json(raw):
    def unique(pairs):
        result={}
        for key,value in pairs:
            if key in result:raise ValueError('duplicate metadata JSON key')
            result[key]=value
        return result
    return json.loads(raw,object_pairs_hook=unique)


def selected_only(mapping):
    return {KEY:mapping[KEY]} if KEY in mapping else {}


def identity():
    return {'pid':os.getpid(),'pgid':os.getpgrp(),'sid':os.getsid(0),'uid':os.getuid()}


def source_check(plan):
    base=Path(__file__).resolve().parent
    for name,pin in plan['source_files'].items():
        if Path(name).name!=name or sha(base/name)!=pin:raise RuntimeError('metadata source pin mismatch')
    python=plan['python']
    if str(Path(sys.executable).resolve())!=python['resolved'] or sha(sys.executable)!=python['sha256']:
        raise RuntimeError('metadata interpreter pin mismatch')


def stage_record(stage,selected,plan,plan_sha,parent_pid):
    if stage not in STAGES:raise ValueError('metadata stage')
    return {'schema':1,'stage':stage,'key':KEY,'observation':observe_named_only(selected),
            'identity':identity(),'parent_pid':parent_pid,'plan_sha256':plan_sha,
            'source_files':plan['source_files'],'python':plan['python'],
            'scope':'named-key metadata only; no payload, ack or admission'}


def publish(path,record):
    raw=json.dumps(record,sort_keys=True,ensure_ascii=True).encode()
    if len(raw)>CAP:raise RuntimeError('stage metadata size bound')
    path=Path(path)
    if path.exists() or path.is_symlink():raise RuntimeError('stage metadata exists')
    tmp=path.with_name(path.name+'.tmp')
    fd=os.open(tmp,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600)
    try:
        if os.write(fd,raw)!=len(raw):raise RuntimeError('incomplete stage metadata write')
    finally:os.close(fd)
    os.rename(tmp,path)


def read_stage(path,stage,plan,pin,parent_pid):
    path=Path(path);st=path.lstat()
    if not stat.S_ISREG(st.st_mode) or stat.S_IMODE(st.st_mode)!=0o600 or st.st_uid!=os.getuid() or st.st_size>CAP:raise RuntimeError('stage record type/size')
    data=strict_json(path.read_bytes())
    if set(data)!={'schema','stage','key','observation','identity','parent_pid','plan_sha256','source_files','python','scope'}:
        raise RuntimeError('stage record fields')
    if type(data['schema']) is not int or data['schema']!=1 or data['stage']!=stage or data['key']!=KEY:
        raise RuntimeError('stage record identity')
    if data['plan_sha256']!=pin or data['source_files']!=plan['source_files'] or data['python']!=plan['python'] or data['parent_pid']!=parent_pid:
        raise RuntimeError('stage record source/parent joins')
    observation=data['observation']
    if observation.get('key')!=KEY or type(observation.get('present')) is not bool:raise RuntimeError('named record format')
    if observation['present']:
        if (set(observation)!={'key','present','representation','value_hex','bytes','sha256'}
                or type(observation['bytes']) is not int or not 0<=observation['bytes']<=512
                or type(observation['value_hex']) is not str or len(observation['value_hex'])>1024):
            raise RuntimeError('named record shape/bound')
        raw=bytes.fromhex(observation['value_hex']);value=raw.decode('utf-8',errors='strict')
        if observe_named_only({KEY:value})!=observation:raise RuntimeError('named record exact representation')
    elif observation!={'key':KEY,'present':False}:raise RuntimeError('absent named record shape')
    ident=data['identity']
    if set(ident)!={'pid','pgid','sid','uid'} or not all(type(v) is int for v in ident.values()):raise RuntimeError('actual identity fields')
    return data


def child():
    signal.signal(signal.SIGALRM,signal.SIG_DFL);signal.alarm(10)
    parser=argparse.ArgumentParser();parser.add_argument('--child',action='store_true',required=True)
    parser.add_argument('--plan',type=Path,required=True);parser.add_argument('--plan-sha256',required=True)
    parser.add_argument('--parent-pid',type=int,required=True);args=parser.parse_args()
    # Selection occurs once, before any intentional environment setup in child.
    selected=selected_only(os.environ)
    if args.plan.is_symlink() or sha(args.plan)!=args.plan_sha256:raise RuntimeError('child plan pin/path')
    plan=strict_json(args.plan.read_bytes())
    if plan['execution_enabled'] is not True or os.getppid()!=args.parent_pid:raise RuntimeError('child plan/parent mismatch')
    source_check(plan)
    root=Path(plan['root']);os.umask(0o077)
    if root.is_symlink() or not root.is_dir() or root.stat().st_mode & 0o077:raise RuntimeError('private observer root')
    publish(root/'childentry.json',stage_record('childentry',selected,plan,args.plan_sha256,args.parent_pid))
    # No ack/command exec: retain a brief bounded interval for independent kernel
    # identity observation, then exit normally; alarm remains default and armed.
    time.sleep(1.0)


if __name__=='__main__':child()
