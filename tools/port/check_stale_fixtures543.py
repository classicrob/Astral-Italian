#!/usr/bin/env python3
"""Paired MAIN-fixture / fixed-fixture controls on identical production objects.

Caller owns quiet heavy->native slot. No app build/run, recording or user cache.
Use a ROOT GX test build configured on this branch, compile_commands.json ON.
Run --prepare first, then obtain a new quiet receipt and run --run.
"""
import argparse,hashlib,json,os,shlex,subprocess,time
from pathlib import Path

BASE='7fdb1c38c55d3384439524e789199289d6b7bd82'
ROOT=Path(__file__).resolve().parents[2]
# Explicitly approved independent production repairs. These are NOT a main
# production control; baseline fixtures use the same repaired objects as fixed.
PRODUCTION_FIXES={
    '610ded6eb47f5ef4ab5cc35f3bc01f00a17c5c86':('native/platform/dvd/dvd_host.cpp',),
    'ec0d1424ad15fedff82f683cdb6ef356270cff5a':('native/input/remote_model.cpp','native/input/remote_model.hpp'),
    'fd3d120e8057695948a831d036159f2b402751a8':(
        'native/input/mods.cpp','native/include/petari/mods.hpp',
        'native/home_menu/hbm_bridge.cpp','native/home_menu/mods_menu.cpp',
        'native/home_menu/include/petari/mods_menu.hpp','native/home_menu/CMakeLists.txt'),
}
CASES=[
    ('native_platform_audio','petari_platform_audio_tests','native/tests/platform_audio_tests.cpp',[],['AIGetDSPSampleRate']),
    ('native_rfl_boot','petari_rfl_boot_tests','native/tests/rfl_boot_tests.cpp',[],['RFLGetAdditionalInfo']),
    ('native_platform_dvd','petari_platform_dvd_tests','native/tests/platform_dvd_tests.cpp',[],['DVDPause','DVDResume','DVDCancelAsync']),
    ('native_game_read_input','petari_input_tests','native/tests/input_tests.cpp',['--game-read-only'],['remote connected by the rig']),
    ('native_input','petari_input_tests','native/tests/input_tests.cpp',[],['remote connected by the rig']),
    ('native_pipeline_startup','petari_pipeline_startup_tests','native/tools/pipeline_startup_tests.cpp',[],['events, focus or title not restored']),
]
def need(ok,why):
    if not ok:raise RuntimeError(why)
def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
def write(path,data):Path(path).write_text(json.dumps(data,indent=2)+'\n')
def argv(row):return row.get('arguments') or shlex.split(row['command'])
def object_path(row):
    args=argv(row);return (Path(row['directory'])/args[args.index('-o')+1]).resolve()
def compile_control(row,source,out):
    original=argv(row);args=[];i=0
    while i<len(original):
        token=original[i];i+=1
        if token in ('-o','-MF','-MT','-MQ'):i+=1;continue
        if token in ('-MD','-MMD') or token==row['file']:continue
        args.append(token)
    # Preserve quoted relative includes against the actual fixture directory,
    # while the baseline TU itself is an immutable git-show copy in OUT.
    return args+['-iquote'+str(Path(row['file']).parent),str(source),'-o',str(out)]
def link_control(row,target,control_object,exe):
    obj=object_path(row);directory=next(p for p in obj.parents if p.name==target+'.dir')
    cwd=directory.parent.parent;args=shlex.split((directory/'link.txt').read_text());out=[];i=0;replaced=0;removed=0
    while i<len(args):
        token=args[i];i+=1
        if token=='-o':out+=['-o',str(exe)];i+=1;continue
        if token.endswith('.o'):
            path=(cwd/token).resolve()
            if path==obj:token=str(control_object);replaced+=1
            elif path.name.endswith('kpad_test_support.cpp.o'):
                # MAIN control uses the original KPAD object in petari_input.a.
                # No reset wrapper is linked into that baseline executable.
                removed+=1;continue
        out.append(token)
    need(replaced==1,'main fixture object not uniquely replaced')
    need(removed==(1 if target=='petari_input_tests' else 0),'unexpected baseline SDK wrapper linkage')
    return out,cwd
def unavailable_checks(log):
    result=[]
    for line in log.splitlines():
        if line.startswith('NOT RUN '):
            name,separator,reason=line[len('NOT RUN '):].partition(': ')
            need(bool(separator and name and reason),'malformed NOT RUN record')
            need(all(row['name']!=name for row in result),'duplicate NOT RUN record')
            result.append(dict(name=name,status='NOT_RUN',executed=False,passed=False,reason=reason,raw_line=line))
    return result

def donor_unavailable(record,log):
    need(record['exit']==77 and not record['timed_out'],'donor row must be SKIPPED (77), never passed or timed out')
    rows=unavailable_checks(log)
    need(len(rows)==1 and rows[0]['name']=='native_skip_donor_integration','missing/ambiguous donor NOT RUN')
    reason=rows[0]['reason']
    need(all(token in reason for token in ('UNAVAILABLE IN SMG2','native/STUBS.md','docs/port/HOOK_TRANSFER.md')),
         'donor NOT RUN must retain its reason and citations')
    return dict(rows[0],process=record,ctest_status='SKIPPED')

def production_scope(fixes):
    allowed=set();blobs={}
    for fix in fixes:
        need(fix in PRODUCTION_FIXES,'unapproved production-fix commit')
        need(git('merge-base',BASE,fix).decode().strip()==BASE,'production fix not based on pinned main')
        allowed.update(git('diff','--name-only',BASE,fix).decode().splitlines())
        for path in PRODUCTION_FIXES[fix]:
            expected=git('show',fix+':'+path)
            need(git('show','HEAD:'+path)==expected,'production fix blob differs: '+path)
            blobs[path]=hashlib.sha256(expected).hexdigest()
    return allowed,blobs

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build',type=Path,required=True);parser.add_argument('--out',type=Path,required=True)
    phase=parser.add_mutually_exclusive_group(required=True)
    phase.add_argument('--prepare',action='store_true');phase.add_argument('--run',action='store_true')
    parser.add_argument('--production-fix',choices=sorted(PRODUCTION_FIXES),action='append',default=[],
                        help='explicit approved production repair; repeat on BOTH prepare/run; main fixture source is the control, not main production')
    parser.add_argument('--quiet-json',type=Path,help='RUN only: fresh caller quiet receipt after preparation, passed=true/quiet=true')
    a=parser.parse_args();build=a.build.resolve();out=a.out.resolve()
    need(os.getpriority(os.PRIO_PROCESS,0)==0,'controls require nice0')
    need(not git('status','--porcelain').strip(),'commit fixture changes before controls')
    need(subprocess.run(['git','merge-base','--is-ancestor',BASE,'HEAD'],cwd=ROOT).returncode==0,'not based on pinned main')
    need(out!=Path.home() and out!=ROOT,'owned output required')
    need(not out.exists() if a.prepare else (out/'PREPARED.json').is_file(),'prepare needs a new directory; run needs PREPARED.json')
    cache=(build/'CMakeCache.txt').read_text()
    need('CMAKE_HOME_DIRECTORY:INTERNAL='+str(ROOT) in cache,'ROOT test build on this checkout required; standalone is not a main control')
    allowed={p for _,_,p,_,_ in CASES}|{'native/input/CMakeLists.txt',
        'tools/port/check_stale_fixtures543.py','tools/port/test_stale_fixtures543.py','native/docs/STALE-FIXTURES-543.md'}
    production_fixes=sorted(set(a.production_fix))
    production_allowed,production_blobs=production_scope(production_fixes)
    allowed.update(production_allowed)
    changed=set(git('diff','--name-only',BASE,'HEAD').decode().splitlines())
    need(changed<=allowed,'unexpected production/predicate scope: '+str(changed-allowed))
    if a.prepare:
        out.mkdir(mode=0o700,parents=True)
        for name in ('home','tmp','controls'):(out/name).mkdir()
    env={k:v for k,v in os.environ.items() if k in ('PATH','USER','LOGNAME','SHELL','__CF_USER_TEXT_ENCODING')}
    env.update(HOME=str(out/'home'),CFFIXED_USER_HOME=str(out/'home'),TMPDIR=str(out/'tmp')+'/',LANG='C',LC_ALL='C')
    results=[]
    def run(name,args,cwd,limit=300):
        start=time.monotonic()
        with (out/(name+'.log')).open('wb') as log:
            child=subprocess.Popen(args,cwd=cwd,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            timeout=False
            try:code=child.wait(timeout=limit)
            except subprocess.TimeoutExpired:
                timeout=True;child.terminate()
                try:code=child.wait(timeout=10)
                except subprocess.TimeoutExpired:child.kill();code=child.wait()
        return dict(command=list(map(str,args)),cwd=str(cwd),exit=code,timed_out=timeout,seconds=time.monotonic()-start,
                    log=str(out/(name+'.log')),log_sha256=sha(out/(name+'.log')))
    report=dict(base=BASE,source=git('rev-parse','HEAD').decode().strip(),changed_paths=sorted(changed),
                scope='pinned main fixture sources vs fixed fixtures on identical production objects; no app/cache run',
                production_fix_commits=production_fixes,production_fix_blob_sha256=production_blobs,
                production_control='shared repaired production objects; NOT main production' if production_fixes else 'main production unchanged',
                results=results,passed=False)
    # Caller builds fixed targets first; do not compile during a quiet runtime
    # comparison or hide a build failure as a test result.
    rows=json.loads((build/'compile_commands.json').read_text());built={}
    artifacts={}
    if a.prepare:
      for test,target,path,extra,expected in CASES:
        if target not in built:
            matches=[row for row in rows if Path(row['file']).resolve()==ROOT/path]
            need(len(matches)==1,'missing/ambiguous compile command '+path);row=matches[0]
            control=out/'controls'/(target+'.cpp');control.write_bytes(git('show',BASE+':'+path))
            obj=out/'controls'/(target+'.o');exe=out/'controls'/target
            before_compile=run(target+'-main-compile',compile_control(row,control,obj),row['directory'])
            before_link=None
            if before_compile['exit']==0:
                args,cwd=link_control(row,target,obj,exe);before_link=run(target+'-main-link',args,cwd)
            original=object_path(row);directory=next(p for p in original.parents if p.name==target+'.dir')
            fixed=directory.parent.parent/target;need(fixed.is_file(),'build fixed target before proof: '+target)
            built[target]=(before_compile,before_link,str(exe),str(fixed),sha(control),sha(ROOT/path))
            artifacts[str(fixed)]=sha(fixed)
            args,cwd=link_control(row,target,obj,exe)
            for token in args:
                archive=(cwd/token).resolve()
                if token.endswith(('.a','.o')) and archive.is_file():artifacts[str(archive)]=sha(archive)
      prepared=dict(report=report,build=str(build),targets=built,artifacts=artifacts,
                    prepared_at=time.time(),sdk_source_sha256=sha(ROOT/'src/RVL_SDK/kpad/KPAD.c'))
      write(out/'PREPARED.json',prepared)
      print('Prepared MAIN controls; obtain a fresh quiet receipt before --run.',flush=True)
      return 0
    prepared=json.loads((out/'PREPARED.json').read_text())
    need(prepared['report']['source']==report['source'] and prepared['build']==str(build),'source/build changed after preparation')
    need(prepared['report']['production_fix_commits']==production_fixes,'production-fix binding changed after preparation')
    need(a.quiet_json is not None,'--run requires --quiet-json')
    need(prepared['prepared_at']<=a.quiet_json.stat().st_mtime<=time.time() and time.time()-a.quiet_json.stat().st_mtime<=120,'quiet receipt must be fresh and after all control compiles')
    quiet=json.loads(a.quiet_json.read_text());need(quiet.get('passed') is True or quiet.get('quiet') is True,'caller quiet receipt failed')
    need(max(os.getloadavg()[:2])<=8,'current load is not quiet')
    need(not (out/'REPORT.json').exists(),'one runtime proof per output; preserve old result')
    artifacts=prepared['artifacts']
    for path,digest in artifacts.items():need(sha(path)==digest,'linked object/archive/fixed executable changed: '+path)
    report.update(quiet_sha256=sha(a.quiet_json),prepared_sha256=sha(out/'PREPARED.json'),artifacts=artifacts)
    write(out/'REPORT.json',report)
    for test,target,path,extra,expected in CASES:
        bc,bl,before,fixed,before_sha,after_sha=prepared['targets'][target]
        br=run(test+'-main-run',[str(before),*extra],out) if bl and bl['exit']==0 else None
        fr=run(test+'-fixed-run',[str(fixed),*extra],out)
        evidence=br or bl or bc
        text=Path(evidence['log']).read_text(errors='replace')
        before_expected=evidence['exit']!=0 and not evidence['timed_out'] and all(s in text for s in expected)
        unavailable=unavailable_checks(Path(fr['log']).read_text(errors='replace'))
        pair=dict(test=test,unavailable_checks=unavailable,after_scope='executed checks only; unavailable checks never credited',before_fixture_sha256=before_sha,after_fixture_sha256=after_sha,
            before_compile=bc,before_link=bl,before_run=br,after_run=fr,
            main_failure_matches=before_expected,after_passed=fr['exit']==0 and not fr['timed_out'])
        pair['passed']=pair['main_failure_matches'] and pair['after_passed'];results.append(pair)
        write(out/'REPORT.json',report);print(test,'PASS' if pair['passed'] else 'FAIL',flush=True)
    for path,digest in artifacts.items():need(sha(path)==digest,'linked object/archive/fixed executable changed during run: '+path)
    # Separate unavailable row: verify it is reported as 77 / NOT RUN even if a
    # later executed input check fails. This component is never credited PASS.
    fixed_input=prepared['targets']['petari_input_tests'][3]
    donor=run('native_skip_donor_integration-unavailable',[str(fixed_input),'--donor-skip-integration'],out)
    report['unavailable_components']=[donor_unavailable(donor,Path(donor['log']).read_text(errors='replace'))]
    full_input=next(row for row in results if row['test']=='native_input')
    if full_input['after_passed']:
        need(any(row['name']=='native_skip_donor_integration' for row in full_input['unavailable_checks']),
             'successful executed input suite silently omitted donor NOT RUN')
    report['passed']=len(results)==6 and all(row['passed'] for row in results)
    write(out/'REPORT.json',report);return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
