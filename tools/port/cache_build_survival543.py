#!/usr/bin/env python3
"""Source-bound M4 cache survival proof. Private roots only; no retries or host input.

A cold title/stage -> A warm stage/title -> B stage/title on that exact cache ->
descriptor-ABI negative stage/title. Run from bash at nice 0 in PORT's quiet slot.
This is cache/visual evidence, not an audio or frame-threshold acceptance gate.
"""
from pathlib import Path
import argparse, contextlib, csv, fcntl, hashlib, json, os, platform, plistlib, re
import shutil, sqlite3, struct, subprocess, sys, time, uuid, zlib

BASE='18ce6b65e6b269076716707b8d0686b00f262bc0'
A_SOURCES={BASE,'fc3a671221eb0186169beaef75412daf01a06017'}
B_SOURCE='35b5e378e9ebfa41a9d696d3069a3a5ff8c68003'
NEG_SOURCE='f458b41d66accd22bc84a7550fa153fa57276d93'
PROTOCOL='petari-pipeline-cache-v2'
TITLE_SECONDS=120
STAGE_FRAMES=1800
FAULT=re.compile(r'FATAL|AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|unhandled tcg')

def need(value,reason):
    if not value: raise RuntimeError(reason)

def sha(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as f:
        for chunk in iter(lambda:f.read(1<<20),b''):h.update(chunk)
    return h.hexdigest()

def write(path,value):
    Path(path).write_text(json.dumps(value,indent=2,sort_keys=True)+'\n')

def contains_bytes(path,needle):
    with Path(path).open('rb') as stream:
        tail=b''
        for chunk in iter(lambda:stream.read(1<<20),b''):
            block=tail+chunk
            if needle in block:return True
            tail=block[-len(needle):]
    return False

def inventory(root):
    return {str(p.relative_to(root)):sha(p) for p in sorted(Path(root).rglob('*')) if p.is_file() and not p.is_symlink()}

def readonly(path):
    return sqlite3.connect(Path(path).resolve().as_uri()+'?mode=ro',uri=True)

def cache_snapshot(root):
    """Only the selected private root; never search HOME or legacy user caches."""
    result={}
    for path in sorted(Path(root).glob('*.v2.*.db')):
        need(not path.is_symlink(),'cache DB symlink refused')
        with readonly(path) as db:
            need(db.execute('PRAGMA quick_check').fetchall()==[('ok',)],'corrupt DB '+str(path))
            meta=db.execute('SELECT protocol,kind,identity FROM petari_cache_compatibility').fetchall()
            need(len(meta)==1 and meta[0][0]==PROTOCOL,'invalid cache metadata')
            kind,key=meta[0][1:]
            if kind=='recipe':
                rows=db.execute('SELECT type,hash,config_version,config_size,config,first_frame_used FROM pipeline_cache ORDER BY type,hash').fetchall()
                entries={f'{r[0]}:{r[1]}':dict(version=r[2],size=r[3],bytes_sha256=hashlib.sha256(r[4]).hexdigest(),first_use=r[5]) for r in rows}
            elif kind=='artifact':
                rows=db.execute('SELECT key_hash,full_key,context,version,value,size,encoding,digest FROM artifacts ORDER BY key_hash').fetchall()
                entries={}
                for index,full,ctx,version,value,size,encoding,digest in rows:
                    need(isinstance(full,bytes) and hashlib.sha256(full).hexdigest()==index and ctx==key and version==2,'artifact envelope/key mismatch')
                    need(isinstance(value,bytes) and 0<size<=64*1024*1024 and encoding in (0,1),'artifact size/encoding')
                    entries[index]=dict(full_key_hex=full.hex(),context=ctx,version=version,stored_bytes_sha256=hashlib.sha256(value).hexdigest(),size=size,encoding=encoding,digest=digest)
            else:raise RuntimeError('unknown cache kind')
            result[path.name]=dict(kind=kind,identity=key,count=len(entries),entries=entries)
    return result

def one(pattern,text,label):
    matches=re.findall(pattern,text)
    need(len(matches)==1,f'{label}: expected one observation, got {len(matches)}')
    return matches[0]

def log_evidence(text):
    recipe,backend,device,artifact=one(r'\[pipeline compatibility\] recipe=(\w+) backend=(\w+) device=(\w+) artifact=(\w+)',text,'device context')
    counts=one(r'\[pipeline artifacts\] hits=(\d+) misses=(\d+) rejected=(\d+) stored=(\d+)',text,'artifact counters')
    closed=one(r'\[pipeline artifacts close\] persistence=(-?\d+) checkpoint=(-?\d+) close=(-?\d+)',text,'artifact close')
    need(all(int(status)==0 for status in closed),'artifact checkpoint/close failed')
    recipes=re.findall(r'\[pipeline recipes\] type=(\d+) accepted=(\d+) rejected=(\d+) ABI=(\w+)',text)
    need(recipes and all(r[3]==recipe for r in recipes),'recipe acceptance evidence missing/foreign')
    prep=one(r'\[gx frontend pre-present\] ([^\n]+)',text,'frontend prep')
    fields=dict(re.findall(r'(\w+)=([^ ]+)',prep))
    need(fields.get('result')=='ready' and fields.get('pending')=='0' and fields.get('failed')=='0' and fields.get('manifest_errors')=='0','frontend readiness failed')
    need(int(fields['presented_frames'])>=3,'missing ordinary presented splash frames')
    need(not FAULT.search(text),'native fault')
    return dict(recipe=recipe,backend=backend,device=device,artifact=artifact,
        counters=dict(zip(('hits','misses','rejected','stored'),map(int,counts))),
        artifact_close=dict(zip(('persistence','checkpoint','close'),map(int,closed))),
        recipe_accepted=sum(int(x[1]) for x in recipes),recipe_rejected=sum(int(x[2]) for x in recipes),
        prep=fields,splash_duration_ms=float(one(r'\[gx boot splash frames\][^\n]* elapsed_ms=([\d.]+)',text,'splash duration')),
        observed_artifact_path=one(r'\[pipeline artifacts open\] path=(.+) namespace='+artifact+r' legacy=not-adopted',text,'artifact path'))

def image(path):
    """Strict RGB8 PNG decoder; compare pixels exactly, not compression metadata."""
    data=Path(path).read_bytes();need(data[:8]==b'\x89PNG\r\n\x1a\n','PNG signature')
    at=8;packed=[];header=None;ended=False
    while at<len(data):
        need(at+12<=len(data),'truncated PNG chunk')
        n,kind=struct.unpack_from('>I4s',data,at);at+=8
        need(n<=64*1024*1024 and at+n+4<=len(data),'PNG chunk bound')
        body=data[at:at+n];crc=struct.unpack_from('>I',data,at+n)[0];at+=n+4
        need(zlib.crc32(kind+body)&0xffffffff==crc,'PNG CRC')
        if kind==b'IHDR':need(header is None,'duplicate IHDR');header=struct.unpack('>IIBBBBB',body)
        elif kind==b'IDAT':packed.append(body)
        elif kind==b'IEND':ended=True;break
    need(ended and at==len(data) and header is not None,'PNG termination')
    w,h,depth,ctype,compression,filtering,interlace=header
    need(0<w<=8192 and 0<h<=8192 and depth==8 and ctype==2 and compression==filtering==interlace==0,'PNG format/bounds')
    size=(w*3+1)*h;decoder=zlib.decompressobj();raw=decoder.decompress(b''.join(packed),size+1)
    need(len(raw)==size and decoder.eof and not decoder.unused_data and not decoder.unconsumed_tail,'PNG inflated length')
    out=bytearray();previous=bytearray(w*3)
    for y in range(h):
        p=y*(w*3+1);f=raw[p];row=bytearray(raw[p+1:p+1+w*3]);need(f<=4,'PNG filter')
        for i in range(len(row)):
            a=row[i-3] if i>=3 else 0;b=previous[i];c=previous[i-3] if i>=3 else 0
            if f==1:v=a
            elif f==2:v=b
            elif f==3:v=(a+b)//2
            elif f==4:
                q=a+b-c;pa,pb,pc=abs(q-a),abs(q-b),abs(q-c);v=a if pa<=pb and pa<=pc else b if pb<=pc else c
            else:v=0
            row[i]=(row[i]+v)&255
        out.extend(row);previous=row
    need(len(set(out))>16,'blank/degenerate capture')
    return dict(width=w,height=h,pixels_sha256=hashlib.sha256(out).hexdigest(),file_sha256=sha(path))

def compare_images(reference,actual):
    need(set(reference)==set(actual) and reference,'capture pair missing')
    return all((reference[n]['width'],reference[n]['height'],reference[n]['pixels_sha256'])==
               (actual[n]['width'],actual[n]['height'],actual[n]['pixels_sha256']) for n in reference)

def judge(mode,evidence,before,after,previous=None,reference=None,captures=None):
    c=evidence['counters'];artifact=evidence['artifact'];name='dawn_cache.v2.'+artifact+'.db'
    checks=dict(no_corrupt_entries=c['rejected']==0,no_rejected_recipes=evidence['recipe_rejected']==0,
        artifact_namespace_created=name in after,artifact_path_name=Path(evidence['observed_artifact_path']).name==name)
    if mode in ('cold','negative'):
        checks.update(namespace_was_absent=name not in before,new_requests_miss=c['misses']>0,new_artifacts_stored=c['stored']>0)
    if mode in ('warm','reuse'):
        checks.update(accepted_recipes=evidence['recipe_accepted']>0,full_key_hits=c['hits']>0)
    if mode=='reuse':
        # Aggregate counters are a stronger check: zero stores/misses anywhere in
        # this run implies zero new compilation for every member of A's warmed set.
        checks.update(no_new_artifact_stores=c['stored']==0,no_artifact_misses=c['misses']==0,
            same_context=all(evidence[k]==previous[k] for k in ('recipe','backend','device','artifact')),
            warmed_entries_unchanged=name in before and name in after and before[name]['entries']==after[name]['entries'],
            output_pixels_exact=compare_images(reference,captures))
    if mode=='negative':
        checks.update(descriptor_recipe_miss=evidence['recipe']!=previous['recipe'],
            artifact_context_miss=evidence['artifact']!=previous['artifact'],same_backend_device=all(evidence[k]==previous[k] for k in ('backend','device')),
            old_artifacts_preserved=all(after.get(n)==v for n,v in before.items() if v['kind']=='artifact'))
    return checks

@contextlib.contextmanager
def locks(heavy,native):
    with open(heavy,'a') as h,open(native,'a') as n:
        fcntl.flock(h,fcntl.LOCK_EX)
        try:
            fcntl.flock(n,fcntl.LOCK_EX)
            try:yield
            finally:fcntl.flock(n,fcntl.LOCK_UN)
        finally:fcntl.flock(h,fcntl.LOCK_UN)

def cpu_seconds(value):
    days=0
    if '-' in value:
        day,value=value.split('-',1);days=int(day)
    fields=value.split(':');total=0.
    for field in fields:total=total*60+float(field)
    return total+days*86400

def quiet(path):
    def sample():
        p=subprocess.run(['/bin/ps','-axo','pid=,comm=,time='],capture_output=True,text=True,check=True)
        rows=[]
        for line in p.stdout.splitlines():
            fields=line.strip().split()
            if len(fields)>=3:rows.append(dict(pid=int(fields[0]),command=' '.join(fields[1:-1]),cpu_seconds=cpu_seconds(fields[-1])))
        return dict(load=os.getloadavg(),processes=rows)
    start=sample();time.sleep(10);stop=sample();before={r['pid']:r for r in start['processes']}
    compiler_names={'clang','clang++','cc1','ld','ld64','make','ninja','xcodebuild'}
    active=[r for r in stop['processes'] if Path(r['command']).name in compiler_names]
    metal=[]
    for r in stop['processes']:
        if 'MTLCompilerService' in r['command']:
            delta=r['cpu_seconds']-before[r['pid']]['cpu_seconds'] if r['pid'] in before else None
            metal.append(dict(pid=r['pid'],delta_seconds=delta,idle=delta is not None and 0<=delta<.05))
    result=dict(first=start,second=stop,active_compilers=active,metal_cpu_deltas=metal,
        passed=max(start['load'][:2]+stop['load'][:2])<=8 and not active and all(r['idle'] for r in metal))
    write(path,result);need(result['passed'],'quiet precondition failed; no arm launched')

def prepare_build(directory,out,own_id,expected):
    directory=Path(directory).resolve()
    need('/Library/Caches/AstralItalian' not in str(directory),'user cache input forbidden')
    app=directory/'Petari.app';build=directory/'BUILD.json'
    info=json.loads(build.read_text());source=info.get('source_commit',info.get('source'))
    need(source in expected,'unexpected source commit '+str(source));exe=app/'Contents/MacOS/Petari'
    digest=sha(exe);need(digest==info.get('binary_sha256',info.get('exe_sha256')),'BUILD/exe mismatch')
    out.parent.mkdir(parents=True,exist_ok=True)
    subprocess.run(['/usr/bin/ditto','--rsrc','--extattr',str(app),str(out)],check=True)
    subprocess.run(['/bin/chmod','-R','u+w',str(out)],check=True)
    plist=out/'Contents/Info.plist';value=plistlib.loads(plist.read_bytes());value['CFBundleIdentifier']=own_id;plist.write_bytes(plistlib.dumps(value))
    subprocess.run(['/usr/bin/codesign','--force','--deep','--sign','-',str(out)],check=True,capture_output=True)
    subprocess.run(['/usr/bin/codesign','--verify','--deep','--strict',str(out)],check=True,capture_output=True)
    with readonly(out/'Contents/Resources/pipeline-seeds/__global__.db') as db:
        metadata=db.execute('SELECT protocol,kind,identity FROM petari_cache_compatibility').fetchall()
    need(len(metadata)==1 and metadata[0][:2]==(PROTOCOL,'recipe'),'bundled recipe identity')
    need(metadata[0][2]==info['renderer_key'],'BUILD/seed recipe key mismatch')
    receipt=dict(source=source,build_json=str(build),build_sha256=sha(build),original_app=str(app),original_exe_sha256=digest,
        app=str(out),perf_exe_sha256=sha(out/'Contents/MacOS/Petari'),bundle_id=own_id,recipe=metadata[0][2],
        pfd={p.name:sha(p) for p in (out/'Contents/Resources/pipeline-seeds').glob('*.pfd')})
    write(out.parent/'PREP.json',receipt);return receipt

def run_arm(args,root,label,build,kind,mode,reference):
    out=root/'arms'/label;out.mkdir(parents=True,exist_ok=False)
    cache=root/'cache';user=out/'user';home=root/'home';script=root/'neutral.script'
    if kind=='stage':shutil.copytree(args.fixture,user)
    else:user.mkdir()
    env={k:v for k,v in os.environ.items() if k in ('PATH','USER','LOGNAME','SHELL','__CF_USER_TEXT_ENCODING')}
    env.update(HOME=str(home),CFFIXED_USER_HOME=str(home),TMPDIR=str(root/'tmp')+'/',LANG='C',LC_ALL='C',
        SDL_AUDIO_DRIVER='dummy',PETARI_MODS='none',PETARI_CACHE_DIR=str(cache),PETARI_FRAME_STATS='1',
        PETARI_FRAME_CSV=str(out/'frames.csv'),PETARI_ROUTE_RECORD=str(out/'route.csv'),PETARI_PIPELINE_DIAG='0',PETARI_TRACE_BOOT='0',
        ASTRAL_TAPE='off',PETARI_XFB_DUMP=str(out/'captures'),PETARI_XFB_DUMP_LABELS='boot' if kind=='title' else 'input-script',
        PETARI_XFB_DUMP_EVERY='1200' if kind=='title' else '120',PETARI_XFB_DUMP_SPAN='1201' if kind=='title' else '241')
    command=[str(Path(build['app'])/'Contents/MacOS/Petari'),'--disc',str(args.disc),'--user',str(user)]
    if kind=='stage':
        command+=['--stage',args.stage,'--scenario','1']
        env.update(PETARI_INPUT_SCRIPT=str(script),PETARI_INPUT_SCRIPT_STEADY='60',PETARI_INPUT_SCRIPT_QUIT='1',PETARI_INPUT_SCRIPT_LOG=str(out/'input.csv'))
    else:env.update(PETARI_TITLE_OBSERVE='1',PETARI_TITLE_OBSERVE_MIN_SECONDS=str(TITLE_SECONDS))
    need(os.getpriority(os.PRIO_PROCESS,0)==0,'nice must be 0; launch from bash')
    with locks(args.heavy_lock,args.native_lock):
        quiet(out/'quiet.json');before=cache_snapshot(cache);write(out/'cache-before.json',before)
        metal_before=inventory(cache/'metal')
        started=time.monotonic()
        with (out/'app.log').open('wb') as log:
            proc=subprocess.Popen(command,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
            try:
                launch=dict(command=command,environment=env,build=build,pid=proc.pid,nice=os.getpriority(os.PRIO_PROCESS,proc.pid),
                    private_root=str(root),cache_root=str(cache),metal_namespace=args.metal_namespace,plan_sha256=sha(root/'PLAN.json'),no_host_input=True,
                    popen_monotonic=started,utc=time.time(),kind=kind,mode=mode)
                write(out/'launch.json',launch);need(launch['nice']==0,'child nice !=0')
                try:code=proc.wait(timeout=240)
                except subprocess.TimeoutExpired:
                    proc.terminate()
                    try:proc.wait(timeout=10)
                    except subprocess.TimeoutExpired:proc.kill();proc.wait()
                    write(out/'process.json',dict(timeout=True,exit=proc.returncode,seconds=time.monotonic()-started))
                    raise RuntimeError('arm timeout; owned PID stopped, no next arm')
            finally:
                if proc.poll() is None:
                    proc.terminate()
                    try:proc.wait(timeout=10)
                    except subprocess.TimeoutExpired:proc.kill();proc.wait()
        write(out/'process.json',dict(timeout=False,exit=code,seconds=time.monotonic()-started))
        after=cache_snapshot(cache);write(out/'cache-after.json',after)
        metal_after=inventory(cache/'metal')
        write(out/'metal-cache.json',dict(namespace=args.metal_namespace,private_target=str(cache/'metal'),before=metal_before,after=metal_after))
        need(metal_after,'owned private Metal namespace received no cache files')
    need(code==0,'nonzero app exit')
    need(sha(Path(build['app'])/'Contents/MacOS/Petari')==build['perf_exe_sha256'],'perf exe changed during run')
    text=(out/'app.log').read_text(errors='replace');evidence=log_evidence(text)
    need(Path(evidence['observed_artifact_path']).resolve().parent==cache,'cache path escaped private root')
    need(evidence['recipe']==build['recipe'],'launched recipe key differs from BUILD')
    if kind=='title':
        elapsed=float(one(r'\[graphics preparation\] event=title-observation-complete[^\n]*elapsed_s=([\d.]+)',text,'fixed title completion'))
        need(TITLE_SECONDS<=elapsed<=TITLE_SECONDS+1,'title did not finish in fixed native observation window')
        names=['xfb-boot-1200.png'];need('event=title_first_draw' in text,'no title draw')
    else:
        need(f'[input-script] done after {STAGE_FRAMES} frames' in text,'neutral script did not complete')
        names=['xfb-input-script-120.png','xfb-input-script-240.png']
        with (out/'route.csv').open() as f:rows=list(csv.DictReader(f))
        need(any(r.get('stage')==args.stage and r.get('player_valid',r.get('valid'))=='1' for r in rows),'target gameplay not valid')
    with (out/'route.csv').open() as f: route=list(csv.DictReader(f))
    controls=('move_x','move_y','jump','spin','crouch','shoot','camera_left','camera_right','camera_center','camera_up','camera_down')
    need(route and all(all(float(row[k])==0 for k in controls) for row in route),'non-neutral/host controls observed')
    captures={n:image(out/'captures'/n) for n in names}
    checks=judge(mode,evidence,before,after,reference['evidence'] if reference else None,
        reference['captures'] if reference else None,captures)
    result=dict(label=label,passed=all(checks.values()),checks=checks,evidence=evidence,captures=captures,
        build=build,cache_root=str(cache),metal_namespace=args.metal_namespace,observation='prep/splash duration recorded only; cached process/device recreation is expected')
    write(out/'RESULT.json',result);need(result['passed'],'arm predicate failed: '+label)
    return result

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('a-build-dir','b-build-dir','negative-build-dir','disc','fixture','out','heavy-lock','native-lock'):p.add_argument('--'+n,type=Path,required=True)
    p.add_argument('--stage',default='YosshiHomeGalaxy');args=p.parse_args()
    need(platform.system()=='Darwin','macOS required')
    brand=subprocess.check_output(['/usr/sbin/sysctl','-n','machdep.cpu.brand_string'],text=True).strip();need(brand=='Apple M4 Max','M4-only runner; M1 belongs to SOL A')
    need(os.getpriority(os.PRIO_PROCESS,0)==0,'nice must be0; use bash')
    allowed=Path.home()/'MacioGalaxy2-gamedata/port53/m1g';root=args.out.expanduser().resolve()
    need(root.is_relative_to(allowed.resolve()) and root!=allowed.resolve() and not root.exists(),'new owned output must be below port53/m1g')
    need(args.fixture.resolve().is_relative_to((Path.home()/'MacioGalaxy2-gamedata').resolve()),'fixture must be an explicit gamedata fixture, never a user-cache path')
    need(args.fixture.is_dir() and args.disc.exists(),'fixture/disc missing')
    fixture_inventory=inventory(args.fixture)
    need(fixture_inventory and not any('pipeline_cache' in n or 'dawn_cache' in n or '/cache/' in n for n in fixture_inventory),'fixture contains cache data')
    root.mkdir(parents=True)
    for n in ('cache','home','tmp','apps'):(root/n).mkdir()
    (root/'neutral.script').write_text(f'# no controls, fixed gameplay observation\n{STAGE_FRAMES}\n')
    own_id='dev.petari.Petari.perfcache543'+uuid.uuid4().hex[:12]
    darwin=Path(subprocess.check_output(['/usr/bin/getconf','DARWIN_USER_CACHE_DIR'],text=True).strip())
    namespace=darwin/own_id;need(not namespace.exists() and not namespace.is_symlink(),'owned Metal namespace already exists')
    metal=root/'cache/metal';metal.mkdir();namespace.symlink_to(metal,target_is_directory=True);args.metal_namespace=str(namespace)
    results=[]
    try:
        with locks(args.heavy_lock,args.native_lock):
            a=prepare_build(args.a_build_dir,root/'apps/A/Petari.app',own_id,A_SOURCES)
            b=prepare_build(args.b_build_dir,root/'apps/B/Petari.app',own_id,{B_SOURCE})
            neg=prepare_build(args.negative_build_dir,root/'apps/negative/Petari.app',own_id,{NEG_SOURCE})
        need(a['recipe']==b['recipe'] and neg['recipe']!=a['recipe'],'positive/negative recipe identity relation')
        need(a['original_exe_sha256']!=b['original_exe_sha256'],'B binary must change')
        probe=b'Invalid GX display-copy scaling dimensions (cache proof B)'
        need(contains_bytes(Path(b['original_app'])/'Contents/MacOS/Petari',probe),'B diagnostic probe was optimized out / wrong binary')
        need(not contains_bytes(Path(a['original_app'])/'Contents/MacOS/Petari',probe),'A already contains B probe')
        need(a['pfd']==b['pfd']==neg['pfd'] and len(a['pfd'])==57,'PFD pack changed')
        need(a['pfd'] and inventory(args.fixture)==fixture_inventory,'fixture changed during prep')
        plan=dict(protocol='cache-build-survival543-v1',base=BASE,builds=dict(A=a,B=b,negative=neg),root=str(root),metal_namespace=str(namespace),bundle_id=own_id,
            fixture=str(args.fixture.resolve()),fixture_inventory=fixture_inventory,disc=str(args.disc.resolve()),machine=brand,nice=0,
            title_native_seconds=TITLE_SECONDS,stage=args.stage,stage_frames=STAGE_FRAMES,script_sha256=sha(root/'neutral.script'),script_source_sha256=sha(__file__),
            order=['A-cold-title','A-cold-stage','A-warm-stage','A-warm-title','B-stage','B-title','negative-stage','negative-title'],
            gates='exact pixels; accepted recipes; full-key hits; zero aggregate new stores/misses on B (stronger than only warmed-set check); negative misses/stores',
            duration_policy='title120..121 native seconds after first game loop, stage1800 neutral gameplay frames; exact image pixels,240s hard timeout',
            prep_policy='record only; expected >=3 cached pipeline-object recreation splash frames; no literal no-splash predicate')
        write(root/'PLAN.json',plan)
        reference={}
        order=[('A-cold-title',a,'title','cold'),('A-cold-stage',a,'stage','warm'),('A-warm-stage',a,'stage','warm'),('A-warm-title',a,'title','warm'),('B-stage',b,'stage','reuse'),('B-title',b,'title','reuse'),('negative-stage',neg,'stage','negative'),('negative-title',neg,'title','warm')]
        for label,build,kind,mode in order:
            result=run_arm(args,root,label,build,kind,mode,reference.get(kind))
            results.append(result)
            if label.startswith('A-warm'):reference[kind]=result
            write(root/'DRIVER.json',dict(status='running',results=results))
        need(inventory(args.fixture)==fixture_inventory,'fixture mutated')
        write(root/'DRIVER.json',dict(status='complete',passed=True,results=results,plan_sha256=sha(root/'PLAN.json')))
    except Exception as exc:
        write(root/'STOPPED.json',dict(passed=False,error=str(exc),completed=[r['label'] for r in results],no_retry=True))
        raise
    finally:
        # Unlink only this exact owned symlink; all private data/receipts remain.
        if namespace.is_symlink() and namespace.readlink()==metal:namespace.unlink()
        write(root/'RELEASE.json',dict(driver_pid=os.getpid(),all_owned_children_reaped=True,metal_alias_unlinked=not namespace.is_symlink(),no_jobs_queued=True))

if __name__=='__main__':main()
