#!/usr/bin/env python3
"""Explicit dispatch-only clean-build proof, used by hookproof/proofland.

The ordinary shell paths remain unchanged. Dispatch mode requires committed,
clean BASE..HEAD inputs and never consumes the ordinary object-equality cert.

Examples (clean committed changes, no concurrent writers):
  DISPATCH_PROOF=1 HOOKPROOF_JOBS=2 tools/team/hookproof.sh build/proof-1 BASE HEAD
  DISPATCH_PROOF=1 HOOKPROOF_JOBS=2 DISPATCH_PROOF_DRYRUN=1 tools/team/proofland.sh
The second command performs the full proof/strict gate and skips land.sh. Both
complete snapshot trees are removed on success or failure; logs/receipts remain.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import signal
import subprocess
import sys


class Refused(RuntimeError):
    pass


def allowed_path(path):
    p=PurePosixPath(path)
    if p.is_absolute() or '..' in p.parts or str(p)!=path:return False
    return (path in ('config/SB4E01/symbols.txt','tools/team/dispatchcheck-allow.txt')
            or path.startswith('include/Game/')
            or (path.startswith('src/Game/') and p.suffix in ('.cpp','.hpp','.h','.inc')))


def check_environment(env):
    if env.get('DISPATCH_PROOF')!='1':raise Refused('DISPATCH_PROOF=1 required')
    modes=[k for k,v in env.items() if v and (k.startswith('REMOTE_PROOF_') or 'MARIO_52' in k)]
    if modes:raise Refused('dispatch proof cannot combine with: '+', '.join(sorted(modes)))
    if env.get('DISPATCH_PROOF_DRYRUN','') not in ('','0','1'):
        raise Refused('DISPATCH_PROOF_DRYRUN must be 0 or 1')
    try:jobs=int(env.get('HOOKPROOF_JOBS','4'))
    except ValueError:raise Refused('HOOKPROOF_JOBS must be a positive integer')
    if jobs<1:raise Refused('HOOKPROOF_JOBS must be a positive integer')
    return jobs


def command(argv, log=None, env=None, check=True, inherit=False):
    stream=open(log,'wb') if log else None if inherit else subprocess.PIPE
    try:
        p=subprocess.Popen(argv,stdout=stream,stderr=subprocess.STDOUT if log else None,
                           env=env,start_new_session=True)
        try:output=p.communicate()[0]
        except BaseException:
            os.killpg(p.pid,signal.SIGTERM)
            try:p.wait(timeout=10)
            except subprocess.TimeoutExpired:
                os.killpg(p.pid,signal.SIGKILL);p.wait()
            raise
        if check and p.returncode:raise Refused(f'command failed ({p.returncode}): {argv}; log={log}')
        return p.returncode,output or b''
    finally:
        if log:stream.close()


def git(*args):return command(['git',*args])[1]


def revision(ref):return git('rev-parse','--verify',ref+'^{commit}').decode().strip()


def changed_paths(base,head,merge_base=False):
    diff=base+'...'+head if merge_base else base+'..'+head
    paths=[p.decode() for p in git('diff','--no-renames','--name-only','-z',diff).split(b'\0') if p]
    for path in paths:
        if not allowed_path(path):raise Refused('dispatch path refused: '+path)
    if not paths:raise Refused('no dispatch changes')
    return paths


def clean():
    dirty=git('status','--porcelain','--untracked-files=all').decode().splitlines()
    if dirty:raise Refused('worktree must be clean: '+dirty[0])


def blob(rev,path):
    rc,data=command(['git','ls-tree','-z',rev,'--',path])
    if not data:return None
    meta,name=data.rstrip(b'\0').split(b'\t',1)
    mode,kind,oid=meta.decode().split()
    if name.decode()!=path or kind!='blob' or mode not in ('100644','100755'):
        raise Refused('non-regular dispatch input: '+path)
    return git('show',rev+':'+path),int(mode,8)&0o777


def apply(rev,paths):
    for path in paths:
        value=blob(rev,path);p=Path(path)
        if p.is_symlink():raise Refused('symlink input: '+path)
        if value is None:
            if p.exists():p.unlink()
        else:
            if any(q.is_symlink() for q in p.parents):raise Refused('symlink parent: '+path)
            p.parent.mkdir(parents=True,exist_ok=True)
            p.write_bytes(value[0]);p.chmod(value[1])


def verify_inputs(rev,paths):
    for path in paths:
        expected=blob(rev,path);p=Path(path)
        if expected is None:
            if p.exists() or p.is_symlink():raise Refused('deleted input reappeared: '+path)
        elif not p.is_file() or p.is_symlink() or p.read_bytes()!=expected[0] or p.stat().st_mode&0o777!=expected[1]:
            raise Refused('phase input changed during build: '+path)


def sha(path):return hashlib.sha1(Path(path).read_bytes()).hexdigest()


def ensure_space(extra=0):
    free=shutil.disk_usage(Path.cwd()).free
    if free<50*1024**3+extra:raise Refused(f'dispatch full proof needs 50 GiB free plus snapshots; available {free} bytes')


def objects():return sorted(Path('build/SB4E01/src').rglob('*.o'))


def snapshot(out,phase,rev):
    tree=out/phase;files=objects();size=sum(p.stat().st_size for p in files)
    if len(files)<2000:raise Refused(f'{phase}: incomplete object tree ({len(files)} objects)')
    ensure_space(size)
    manifest=[]
    for p in files:
        if p.is_symlink():raise Refused('object symlink: '+str(p))
        q=tree/p;q.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,q)
        manifest.append(hashlib.sha256(q.read_bytes()).hexdigest()+'  '+str(p))
    (out/(phase+'.objects.sha256')).write_text('\n'.join(manifest)+'\n')
    cfg=tree/'config/SB4E01';cfg.mkdir(parents=True)
    for name in ('symbols.txt','splits.txt','build.sha1'):
        (cfg/name).write_bytes(git('show',rev+':config/SB4E01/'+name))
    (tree/'orig').symlink_to((Path.cwd()/'orig').resolve(),target_is_directory=True)
    print(f'{phase}: saved {len(files)} objects, {size} bytes',flush=True)
    return dict(objects=len(files),bytes=size)


def original_hash(rev):
    want=git('show',rev+':config/SB4E01/build.sha1').decode().split()[0]
    got=sha('build/SB4E01/main.dol')
    if got!=want:raise Refused(f'AFTER DOL hash {got} != original {want}')
    return got


def select_control(paths,head,out):
    sources=[p for p in paths if p.endswith('.cpp') and blob(head,p) is not None]
    override=os.environ.get('HOOKPROOF_CONTROL_SOURCE')
    if override:
        if sources:raise Refused('control override is only for batches without an edited .cpp')
        if not override.startswith('src/Game/') or not override.endswith('.cpp') or not allowed_path(override):
            raise Refused('invalid control source: '+override)
        if blob(head,override) is None:raise Refused('control source is not tracked: '+override)
        obj='build/SB4E01/'+str(PurePosixPath(override).with_suffix('.o'))
        text=command(['ninja','-t','deps',obj])[1].decode();(out/'control-deps.txt').write_text(text)
        lines=text.rstrip().splitlines()
        match=re.fullmatch(re.escape(obj)+r': #deps (\d+), deps mtime \d+ \(VALID\)',lines[0]) if lines else None
        if not match or len(lines)-1!=int(match[1]) or any(not l.startswith('    ') for l in lines[1:]):
            raise Refused('control source needs a VALID Ninja dependency record')
        deps={Path(l[4:]).resolve() for l in lines[1:]}
        if Path(override).resolve() not in deps or not deps&{Path(p).resolve() for p in paths}:
            raise Refused('control source dependency closure does not intersect changed inputs')
        return override
    if not sources:raise Refused('no edited .cpp: set HOOKPROOF_CONTROL_SOURCE to a tracked dependent source')
    return sources[0]


def prove(out,base,head):
    jobs=check_environment(os.environ);clean()
    if revision('HEAD')!=head:raise Refused('HEAD differs from requested after revision')
    paths=changed_paths(base,head)
    # Validate both phase inputs before removing any output or starting a build.
    for rev in (base,head):
        for path in paths:blob(rev,path)
    out=out.resolve();build=(Path.cwd()/'build').resolve()
    if not out.is_relative_to(build) or out==build or out.is_symlink():raise Refused('proof outdir must be below build/')
    if out.exists():raise Refused('proof outdir already exists; select a fresh outdir: '+str(out))
    ensure_space(max(2*1024**3,2*sum(p.stat().st_size for p in objects())))
    out.mkdir(parents=True)
    receipt=dict(base=base,head=head,jobs=jobs,paths=paths,status='FAILED',phases={})
    phasepaths=sorted(set(paths)|{'config/SB4E01/symbols.txt','config/SB4E01/splits.txt'})
    env=dict(os.environ,MG2_INTERACTIVE='1')
    try:
        for phase,rev in (('before',base),('after',head)):
            apply(rev,phasepaths)
            for obj in objects():obj.unlink()
            rc,_=command(['nice','-n','10','sh','tools/team/nj.sh','-j',str(jobs),'-k','0'],
                         out/f'build-{phase}.log',env=env,check=False)
            receipt['phases'][phase]={'build_rc':rc}
            if rc:raise Refused(phase+' clean build failed')
            verify_inputs(rev,phasepaths)
            receipt['phases'][phase].update(snapshot(out,phase,rev))
        receipt['after_dol_sha1']=original_hash(head)
        rc,_=command([sys.executable,'tools/team/dispatchproof.py','--before',str(out/'before'),
                      '--after',str(out/'after'),'--out',str(out/'dispatchproof.json')],out/'dispatchproof.log',check=False)
        if rc or 'DISPATCH PROOF OK' not in (out/'dispatchproof.log').read_text().splitlines():
            raise Refused('dispatch image proof failed: '+str(out/'dispatchproof.log'))
        ctl=select_control(paths,head,out);obj=Path('build/SB4E01')/Path(ctl).with_suffix('.o')
        original=Path(ctl).read_bytes();h0=sha(obj)
        try:
            Path(ctl).write_bytes(original+b'\nextern const int hookproof_control_marker = 0x1234;\n')
            rc1,_=command(['nice','-n','10','ninja','-j',str(jobs),str(obj)],out/'build-control.log',env=env,check=False)
            h1=sha(obj) if obj.exists() else None
        finally:Path(ctl).write_bytes(original)
        rc2,_=command(['nice','-n','10','ninja','-j',str(jobs),str(obj)],out/'build-control-restore.log',env=env,check=False)
        h2=sha(obj) if obj.exists() else None
        receipt['control']={'source':ctl,'compile_rc':rc1,'restore_rc':rc2,'before':h0,'perturbed':h1,'restored':h2}
        if rc1 or rc2 or not h1 or h1==h0 or h2!=h0:raise Refused('perturbation control failed')
        rc,_=command([sys.executable,'tools/team/dispatchcheck.py','--root','.','--quiet','--allow',
                      'tools/team/dispatchcheck-allow.txt','--strict'],out/'dispatchcheck.log',check=False)
        receipt['oracle_rc']=rc
        if rc:raise Refused('strict dispatch oracle failed')
        original_hash(head);clean()
        if revision('HEAD')!=head:raise Refused('HEAD changed during proof')
        receipt['status']='DISPATCH PROOF OK'
        print('DISPATCH PROOF OK',flush=True)
        return receipt
    except BaseException as exc:
        receipt['error']=str(exc)
        raise
    finally:
        try:apply(head,phasepaths)
        finally:
            receipt['deleted_snapshots']={}
            for phase in ('before','after'):
                tree=out/phase
                if tree.exists():
                    size=sum(p.stat().st_size for p in tree.rglob('*') if p.is_file() and not p.is_symlink())
                    receipt['deleted_snapshots'][phase]=size
                    shutil.rmtree(tree);print(f'{phase}: deleted snapshot ({size} bytes)',flush=True)
            (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')


def land(out):
    check_environment(os.environ);clean();changed_paths('main','HEAD',merge_base=True)
    who=git('branch','--show-current').decode().strip()+'-dispatch-proofland'
    owner=command([sys.executable,'tools/team/landlock.py','acquire',who])[1].decode().strip()
    try:
        rc,_=command(['git','rebase','main'],check=False)
        if rc:
            command(['git','rebase','--abort'],check=False);raise Refused('rebase conflict; resolve before proofland')
        base,head=revision('main'),revision('HEAD')
        changed_paths(base,head);clean()
        # Keep the native build lock through both proof and final land.sh.
        lock=str(Path.home()/'MacioGalaxy2-team/slots/native.lock')
        argv=['lockf','-k',lock,sys.executable,__file__,'locked',str(out),base,head,owner]
        command(argv,inherit=True)
    finally:command([sys.executable,'tools/team/landlock.py','release'],check=False)


def finish_landing(out,base,head,owner,receipt):
    if receipt.get('status')!='DISPATCH PROOF OK':raise Refused('no successful dispatch proof receipt')
    if revision('main')!=base or revision('HEAD')!=head:raise Refused('proven revisions changed')
    clean();changed_paths(base,head)
    if os.environ.get('DISPATCH_PROOF_DRYRUN')=='1':
        receipt['landing']='SKIPPED: DISPATCH_PROOF_DRYRUN=1'
        print('DISPATCH PROOF DRY RUN OK (land.sh skipped)',flush=True)
    else:
        env=dict(os.environ,LAND_LOCK_HELD=owner,TEAM_BUILD_JOBS=str(check_environment(os.environ)))
        receipt['landing']='STARTED'
        (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
        command(['sh','tools/team/land.sh'],env=env,inherit=True)
        receipt['landing']='COMPLETE'
    (out/'receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('mode',choices=('hook','land','locked'));ap.add_argument('out',nargs='?',default='build/dispatch-proofland-'+str(os.getpid()))
    ap.add_argument('base',nargs='?');ap.add_argument('head',nargs='?',default='HEAD');ap.add_argument('owner',nargs='?')
    a=ap.parse_args()
    def interrupted(signum,frame):raise SystemExit(128+signum)
    for sig in (signal.SIGTERM,signal.SIGHUP):signal.signal(sig,interrupted)
    try:
        check_environment(os.environ)
        if a.mode=='land':land(Path(a.out));return
        if not a.base:raise Refused('dispatch hook proof requires committed BASE [HEAD]')
        base,head=revision(a.base),revision(a.head)
        if a.mode=='locked':
            lock=Path.home()/'MacioGalaxy2-team/land.lock/pid'
            if not a.owner or not lock.exists() or lock.read_text().strip()!=a.owner:raise Refused('landing lock owner differs')
            if revision('main')!=base:raise Refused('main changed before proof')
        receipt=prove(Path(a.out),base,head)
        if a.mode=='locked':finish_landing(Path(a.out),base,head,a.owner,receipt)
    except Refused as exc:
        print('DISPATCH PROOF REFUSED: '+str(exc),file=sys.stderr);sys.exit(1)


if __name__=='__main__':main()
