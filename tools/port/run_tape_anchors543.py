#!/usr/bin/env python3
"""B-owned map tape proof. Plan only until explicit SOL A release; no retries.

The detached driver takes no outer heavy lock. Each child arm owns lockf heavy,
then native, quiet guard, only perfm1b cache, app lifetime and all receipts.
"""
import argparse
import contextlib
import fcntl
import hashlib
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import stat
import subprocess
import sys
import time
import traceback

HERE = Path(__file__).resolve().parent
if not (HERE/'common.py').exists():
    sys.path.insert(0, str(HERE.parents[1]/'gate53/m1gates'))
import common
from capture_schema2_tape import inventory, PRODUCTION_PIPELINE_ENV
from tape_phase_audit import audit_native

ROOT = Path.home()/'MacioGalaxy2-gamedata/port53/m1g'
OWN = 'dev.petari.Petari.perfm1b'
HEAVY = Path.home()/'MacioGalaxy2-team/slots/m1-heavy.lock'
NATIVE = Path.home()/'MacioGalaxy2-team/slots/native.lock'
DISC = Path.home()/'MacioGalaxy2-gamedata/SB4E01-extracted'
SOURCE = ROOT/'tapes/tape-worldmap-54v2'
FIXTURE = ROOT/'fixtures/unlocked-242/fixture'


def write(path, value):
    path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
    temp=path.with_suffix(path.suffix+'.tmp');temp.write_text(json.dumps(value,indent=2)+'\n');temp.replace(path)


def need(value, why):
    if not value: raise RuntimeError(why)


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def cold_metal(evidence):
    # Same exact B namespace as prior authorised gates. Never A/user namespaces.
    base=Path(subprocess.check_output(['/usr/bin/getconf','DARWIN_USER_CACHE_DIR'],text=True).strip()).resolve()
    target=base/OWN
    need(base.is_absolute() and base!=Path('/') and target.name=='dev.petari.Petari.perfm1b', 'bad owned Metal namespace')
    before=common.tree(target) if target.exists() else {}
    if target.exists(): shutil.rmtree(target)
    need(not target.exists(), 'owned cold Metal clear failed')
    write(evidence,dict(policy='owned B perfm1b only; checked Python deletion',target=str(target),before=before,absent=True))


def prepare(source, target, kind, expected):
    need(not target.exists(), 'new perf directory required')
    build=json.loads((source/'BUILD.json').read_text());original=source/'Petari.app'
    need(build['source'].startswith(expected) and sha(original/'Contents/MacOS/Petari')==build['binary_sha256'], 'original build mismatch')
    before=common.tree(original);resources=common.tree(original/'Contents/Resources')
    target.mkdir(parents=True);app=target/'Petari.app'
    subprocess.run(['/usr/bin/ditto',str(original),str(app)],check=True)
    for p in (app,*app.rglob('*')):
        if not p.is_symlink(): p.chmod(p.stat().st_mode|stat.S_IWUSR)
    subprocess.run(['/usr/libexec/PlistBuddy','-c','Set :CFBundleIdentifier '+OWN,str(app/'Contents/Info.plist')],check=True)
    subprocess.run(['/usr/bin/codesign','--force','--deep','-s','-',str(app)],check=True,capture_output=True)
    subprocess.run(['/usr/bin/codesign','--verify','--deep','--strict',str(app)],check=True,capture_output=True)
    need(common.tree(original)==before and common.tree(app/'Contents/Resources')==resources, 'preparation changed original/resources')
    write(target/'PREP.json',dict(build_kind=kind,source_commit=build['source'],original_app=str(original),app=str(app),
        original_exe_sha256=build['binary_sha256'],perf_sha256=sha(app/'Contents/MacOS/Petari'),bundle_id=OWN,
        resource_tree=resources,build=build,original_tree=before,original_unchanged=True,
        reference_is_test_instrument=kind=='server-instrumented-reference',owner='SOL B thr_65jasku4ah'))


def plan(out, controls_only=False, candidate=None, reference072=None, reference002=None, schema2_only=False):
    need(ROOT.resolve() in out.parents and not out.exists(), 'fresh owned output required')
    builds={
        'legacy072':(ROOT/'candidates/07223918','frozen-control','07223918'),
        'legacy002':(ROOT/'candidates/00221126','frozen-control','00221126')}
    if not controls_only:
        need(candidate is not None and reference072 is not None and reference002 is not None,
             'explicit metadata-corrected candidate/reference directories required')
        for key,directory,kind in [('ref072',reference072,'server-instrumented-reference'),
                                   ('ref002',reference002,'server-instrumented-reference'),
                                   ('candidate',candidate,'server-candidate')]:
            builds[key]=(directory,kind,json.loads((directory/'BUILD.json').read_text())['source'])
    out.mkdir(parents=True);harness=out/'harness';harness.mkdir()
    sources=[Path(__file__),HERE/'capture_schema2_tape.py',HERE/'tape_phase_audit.py',HERE/'replay_tape.py',Path(common.__file__),Path(common.__file__).with_name('promotion54m.py')]
    for source in sources: shutil.copy2(source,harness/source.name)
    arms=[]
    def add(name,build,kind,tape,warm=None):
        arms.append(dict(name=name,build=build,kind=kind,tape=str(tape),warm_from=str(warm) if warm else None,
                         out=str(out/'runs'/name),observe_seconds=90 if kind in ('legacy','conversion') else 120))
        return out/'runs'/name
    for build in (() if schema2_only else ('legacy072','legacy002')):
        fresh=add(build+'-fresh',build,'legacy',SOURCE)
        add(build+'-warm',build,'legacy',SOURCE,fresh)
    if schema2_only:
        builds={k:v for k,v in builds.items() if not k.startswith('legacy')}
    if not controls_only:
        canonical072=add('convert-ref072','ref072','conversion',SOURCE)
        canonical002=add('convert-ref002','ref002','conversion',SOURCE)
        for build in ('ref072','ref002'):
            fresh=add('matrix-'+build+'-fresh',build,'schema2',canonical072)
            add('matrix-'+build+'-warm',build,'schema2',canonical072,fresh)
        fresh=add('reciprocal-ref002-to-ref072-fresh','ref072','schema2',canonical002)
        add('reciprocal-ref002-to-ref072-warm','ref072','schema2',canonical002,fresh)
        for label,tape in (('072',canonical072),('002',canonical002)):
            fresh=add('candidate-from'+label+'-fresh','candidate','schema2',tape)
            add('candidate-from'+label+'-warm','candidate','schema2',tape,fresh)
    write(out/'PLAN.json',dict(status='planned-not-launched',builds={k:dict(source=str(v[0]),kind=v[1],expected=v[2]) for k,v in builds.items()},
        arms=arms,nice=0,bundle=OWN,heavy_then_native=[str(HEAVY),str(NATIVE)],no_outer_lock=True,
        no_retries=True,legacy_controls_only=controls_only,schema2_only=schema2_only,
        production_pipeline_env=PRODUCTION_PIPELINE_ENV,canonical_selection='one strict conversion per reference, fail closed; no failure substituted',
        schema2_observation='120s minimum to retain full 90s converted prefix AND a positive neutral tail; original >=90s/50audio/0underrun predicates unchanged',
        source_hashes={p.name:sha(p) for p in harness.iterdir()},acceptance_claimed=False))


def legacy(arm, app, prep, control):
    # Arm process already owns BOTH locks. Common.run's nested lock is disabled.
    common.native_lock=contextlib.nullcontext;common.ID=OWN
    common.clear_metal=lambda:cold_metal(control/'cold-metal.json')
    out=Path(arm['out']);tape=Path(arm['tape'])
    metadata=json.loads((tape/'route.tape.json').read_text())
    need(metadata['schema']==1,'legacy control must use original schema1')
    extra=dict(PETARI_TAPE_PLAY=str(tape),PETARI_TAPE_PLAY_SESSION='1',PETARI_TAPE_PLAY_QUIT='1',
               PETARI_TAPE_PLAY_OBSERVE_SECONDS='90',PETARI_TAPE_PLAY_OUT=str(out/'replay'),PETARI_MODS='OdysseyJumpDive',**PRODUCTION_PIPELINE_ENV)
    if arm['warm_from']: extra.update(PETARI_SMOKE_REAL_AUDIO='1',PETARI_AUDIO_DIAG='1',SDL_AUDIO_DRIVER='coreaudio')
    before=common.tree(app);source_before=common.tree(tape)
    process=common.run(app,out,stage='WorldMap01Galaxy',scenario=1,extra=extra,cold=not arm['warm_from'],warm=arm['warm_from'],
                       copied=True,fixture=FIXTURE,verify_fixture_attrs=True,seconds=600)
    native=json.loads((out/'replay/divergence.json').read_text());error=None
    try: audit_native(native,metadata)
    except (ValueError,KeyError,TypeError) as exc:error=str(exc)
    segments=native.get('segments',[]);maps=[s for s in segments if s['stage']=='WorldMap01Galaxy']
    roll=native.get('observation',{});text=(out/'app.log').read_text(errors='replace')
    audio=[int(n) for n in re.findall(r'underrun (\d+) frames',text)]
    checks=dict(process_normal=process['completed'] and not process['timed_out'],native_exact=error is None,
        strict_completed=native.get('finished') is True and not native.get('stopped') and bool(segments) and all(s.get('completed') and not s.get('diverged') and s.get('max_position_error')==0 for s in segments),
        pointer_coverage=bool(maps) and sum(s.get('pointer_valid_rows_compared',0) for s in maps)>=30 and max((s.get('pointer_distinct_positions_compared',0) for s in maps),default=0)>=3,
        observation=roll.get('completed') is True and roll.get('elapsed_seconds',0)>=90 and process['seconds']>=90,
        app_unchanged=common.tree(app)==before,tape_unchanged=common.tree(tape)==source_before,
        fixture_unchanged=process['seed_unchanged'],no_faults=process['no_faults'])
    checks['child_nice0']=json.loads((out/'launch.json').read_text()).get('child_nice_at_spawn')==0
    if arm['warm_from']:checks['real_audio']=len(audio)>=50 and sum(audio)==0
    write(out/'RESULT.json',dict(passed=all(checks.values()),checks=checks,process=process,error=error,
        tape_format_label='legacy-v1',kind='owned perf copy of immutable game/player legacy control; schema2 capture unavailable',
        original_build=prep,strict_native_report=str(out/'replay/divergence.json'),native_finished=native.get('finished'),native_stopped=native.get('stopped'),audio_reports=len(audio),audio_underrun=sum(audio),
        source_bindings={name:sha(tape/name) for name in ('route.csv','route.kpd','route.tape.json','route.actions')},acceptance_claimed=False))


def arm(out, name):
    need(os.getpriority(os.PRIO_PROCESS,0)==0,'arm nice0 required')
    spec=json.loads((out/'PLAN.json').read_text());a=next(p for p in spec['arms'] if p['name']==name)
    control=out/'arms'/name;control.mkdir(parents=True)
    with NATIVE.open('a') as lock:
        fcntl.lockf(lock,fcntl.LOCK_EX)
        app=out/'apps'/a['build']/'Petari.app';prep_path=app.parent/'PREP.json';prep=json.loads(prep_path.read_text())
        need(sha(app/'Contents/MacOS/Petari')==prep['perf_sha256'],'prepared app differs')
        need(common.tree(app/'Contents/Resources')==prep['resource_tree'],'prepared resources differ')
        original=Path(prep['original_app'])
        need(common.tree(original)==prep['original_tree'],'immutable original differs from PREP')
        with (app/'Contents/Info.plist').open('rb') as f:need(plistlib.load(f)['CFBundleIdentifier']==OWN,'wrong B namespace')
        if a['kind']=='legacy':legacy(a,app,prep,control)
        else:
            common.wait_quiet(control/'quiet.json');common.disk()
            if not a['warm_from']:cold_metal(control/'cold-metal.json')
            else:
                base=Path(subprocess.check_output(['/usr/bin/getconf','DARWIN_USER_CACHE_DIR'],text=True).strip())
                need((base/OWN).is_dir(),'warm Metal namespace missing')
                write(control/'warm-metal.json',dict(target=str(base/OWN),before=common.tree(base/OWN),cleared=False))
            command=[sys.executable,'-B',str(HERE/'capture_schema2_tape.py'),'--app',str(app),'--prep',str(prep_path),
                '--disc',str(DISC),'--tape',a['tape'],'--out',a['out'],'--user-seed',str(FIXTURE),
                '--fixture-receipt',str(FIXTURE.parent/'receipt.json'),'--stage','WorldMap01Galaxy','--scenario','1',
                '--observe-seconds',str(a['observe_seconds']),'--bound','600','--require-pointer-evidence']
            if a['kind']=='conversion':command+=['--allow-incomplete']
            if a['warm_from']:command+=['--warm-from',a['warm_from'],'--real-audio']
            write(control/'command.json',dict(command=command,nice=0,locks=['heavy via parent lockf','native via current PID'],pid=os.getpid()))
            subprocess.run(command,check=False)
        result=Path(a['out'])/'RESULT.json';need(result.exists(),'no actual RESULT; stop, no replacement')
        need(common.tree(original)==prep['original_tree'] and common.tree(app/'Contents/Resources')==prep['resource_tree'],
             'original/prepared resources changed during replay')
        result=json.loads(result.read_text());process=json.loads((Path(a['out'])/'process.json').read_text())
        need(process['exit']==0 and not process.get('timed_out'),'abnormal app exit; stop')
        write(control/'COMPLETE.json',dict(arm=a,passed=result['passed'],process=process,result_sha256=sha(Path(a['out'])/'RESULT.json')))
        return 0 if result['passed'] or a['kind']=='legacy' else 1


def run(out, release):
    need(os.getpriority(os.PRIO_PROCESS,0)==0,'driver nice0 required')
    need(release.is_file(),'explicit SOL A release receipt required')
    write(out/'RUNNING.json',dict(pid=os.getpid(),ppid=os.getppid(),sid=os.getsid(0),nice=0,release=str(release),release_sha256=sha(release),started=time.time()))
    spec=json.loads((out/'PLAN.json').read_text());results=[]
    need({p.name:sha(p) for p in HERE.iterdir() if p.is_file()}==spec['source_hashes'],'frozen harness differs from plan')
    # Preparation also serializes; no measurement/build overlap, no outer cohort lock.
    for key,entry in spec['builds'].items():
        with HEAVY.open('a') as heavy,NATIVE.open('a') as native:
            fcntl.lockf(heavy,fcntl.LOCK_EX);fcntl.lockf(native,fcntl.LOCK_EX)
            common.disk();prepare(Path(entry['source']),out/'apps'/key,entry['kind'],entry['expected'])
    receipt=json.loads((FIXTURE.parent/'receipt.json').read_text())
    need(inventory(FIXTURE)=={k:receipt[k] for k in ('files','xattrs')},'certified fixture differs')
    for a in spec['arms']:
        write(out/'DRIVER.json',dict(status='running',current=a,completed=results,pid=os.getpid(),acceptance_claimed=False))
        command=['/usr/bin/lockf','-k',str(HEAVY),sys.executable,'-B',str(Path(__file__)),'--arm',a['name'],'--out',str(out)]
        with (out/(a['name']+'-driver.log')).open('wb') as log:
            child=subprocess.Popen(command,stdout=log,stderr=subprocess.STDOUT,stdin=subprocess.DEVNULL)
            write(out/'ACTIVE.json',dict(arm=a,pid=child.pid,driver_pid=os.getpid(),command=command,nice=os.getpriority(os.PRIO_PROCESS,child.pid)))
            status=child.wait()
        path=Path(a['out'])/'RESULT.json';complete=out/'arms'/a['name']/'COMPLETE.json'
        if path.is_file() and complete.is_file():results.append(json.loads(complete.read_text()))
        if status!=0 or not complete.is_file():
            write(out/'STOPPED.json',dict(arm=a,status=status,reason='first real failed dependency; preserve, no retries',completed=results));return 2
    write(out/'DRIVER.json',dict(status='complete',completed=results,pid=os.getpid(),finished=time.time(),acceptance_claimed=False))
    return 0


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',required=True,type=Path)
    modes=p.add_mutually_exclusive_group(required=True);modes.add_argument('--plan-only',action='store_true');modes.add_argument('--run',action='store_true');modes.add_argument('--arm');modes.add_argument('--detach',action='store_true')
    p.add_argument('--release-receipt',type=Path)
    scope=p.add_mutually_exclusive_group();scope.add_argument('--legacy-controls-only',action='store_true');scope.add_argument('--schema2-only',action='store_true')
    p.add_argument('--candidate-dir',type=Path);p.add_argument('--reference072-dir',type=Path);p.add_argument('--reference002-dir',type=Path)
    p.add_argument('--notify-thread',help='authorised milestone recipient; one message at final completion/stop only')
    a=p.parse_args();out=a.out.resolve()
    try:
        if a.plan_only:plan(out,a.legacy_controls_only,a.candidate_dir,a.reference072_dir,a.reference002_dir,a.schema2_only);return 0
        if a.arm:return arm(out,a.arm)
        need(a.release_receipt is not None,'release receipt required')
        if a.detach:
            release=a.release_receipt.resolve();need(release.is_file(),'explicit SOL A release required before queueing')
            need(not (out/'DETACHED.json').exists(),'no second launch/resume')
            need(os.getpriority(os.PRIO_PROCESS,0)==0,'detached launch must inherit nice0')
            command=['/usr/bin/nohup',sys.executable,'-B',str(out/'harness'/Path(__file__).name),'--run','--out',str(out),'--release-receipt',str(release)]
            if a.notify_thread:command+=['--notify-thread',a.notify_thread]
            with (out/'detached-driver.log').open('xb') as log:
                child=subprocess.Popen(command,stdout=log,stderr=subprocess.STDOUT,stdin=subprocess.DEVNULL,start_new_session=True,close_fds=True)
            write(out/'DETACHED.json',dict(pid=child.pid,command=command,nice=os.getpriority(os.PRIO_PROCESS,child.pid),start=time.time(),
                release_receipt=str(release),release_sha256=sha(release),method='bash parent; nohup + setsid via start_new_session; all fds detached'))
            print(child.pid,flush=True);return 0
        status=run(out,a.release_receipt.resolve())
        if a.notify_thread:
            need(re.fullmatch(r'thr_[a-z0-9]+',a.notify_thread),'invalid milestone recipient')
            final=out/'DRIVER.json' if status==0 else out/'STOPPED.json'
            value=json.loads(final.read_text());rows=value.get('completed',[])
            text='SOL B543 tape driver '+('COMPLETE' if status==0 else 'STOPPED fail-closed')+'; '+str(len(rows))+' actual completed arms.\n'
            text+='Receipt: '+str(final)+'\n'
            for row in rows:text+=row['arm']['name']+': '+('PASS' if row['passed'] else 'FAIL')+'; '+str(Path(row['arm']['out'])/'RESULT.json')+'\n'
            text+='No acceptance claim by this driver. Legacy controls retain legacy-v1 behavior; no source failure replaced. Detailed mechanism review follows from retained raw reports.\n'
            message=out/'MILESTONE.txt';message.write_text(text)
            notification=subprocess.run(['bb','thread','tell',a.notify_thread,'--message-file',str(message)],capture_output=True,text=True)
            write(out/'NOTIFICATION.json',dict(exit=notification.returncode,stdout=notification.stdout,stderr=notification.stderr,recipient=a.notify_thread))
        return status
    except BaseException as exc:
        if out.is_dir():write(out/('ARM-ERROR-'+a.arm+'.json' if a.arm else 'DRIVER-ERROR.json'),dict(error=str(exc),traceback=traceback.format_exc(),time=time.time()))
        raise


if __name__=='__main__':raise SystemExit(main())
