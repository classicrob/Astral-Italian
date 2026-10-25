"""Private perf copies and owned GX sessions. No host input or user-cache writes."""
import contextlib, csv, fcntl, hashlib, json, os, platform, re, shutil, signal, subprocess, time
from pathlib import Path
HOME = Path.home()
ROOT = HOME / 'MacioGalaxy2-gamedata/port53/m1g'
FROZEN = HOME / 'MacioGalaxy2-team/scratch-release/frozen54'
FIXTURE = HOME / 'MacioGalaxy2-gamedata/fixtures/unlocked-242/fixture'
DISC = HOME / 'MacioGalaxy2-gamedata/SB4E01-extracted'
LOCK = HOME / 'MacioGalaxy2-team/slots/native.lock'
HEAVY = HOME / 'MacioGalaxy2-team/slots/m1-heavy.lock'
HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
ID = 'dev.petari.Petari.perfm1a'
FLOOR = 50 * 1024**3

def sha(path):
 h=hashlib.sha256()
 with Path(path).open('rb') as f:
  for b in iter(lambda:f.read(1048576),b''):h.update(b)
 return h.hexdigest()

def write(path, data):
 path=Path(path);path.parent.mkdir(parents=True,exist_ok=True)
 path.write_text(json.dumps(data,indent=2)+'\n')

def disk():
 free=shutil.disk_usage(HOME).free
 if free<FLOOR:raise RuntimeError(f'disk guard: {free} bytes free, need {FLOOR}')
 return free

def tree(path):
 return {str(p.relative_to(path)):sha(p) for p in sorted(Path(path).rglob('*')) if p.is_file()}

def prep():
 disk(); build=json.loads((FROZEN/'BUILD.json').read_text())
 assert sha(FROZEN/'Petari.app/Contents/MacOS/Petari')==build['binary_sha256']
 dest=ROOT/'apps/frozen54-perfm1a'; app=dest/'Petari.app'; meta=dest/'PREP.json'
 if dest.exists():
  data=json.loads(meta.read_text());assert sha(app/'Contents/MacOS/Petari')==data['perf_exe_sha256'];return app,data
 dest.mkdir(parents=True)
 before=tree(FROZEN/'Petari.app')
 subprocess.run(['/usr/bin/ditto',str(FROZEN/'Petari.app'),str(app)],check=True)
 subprocess.run(['/usr/libexec/PlistBuddy','-c',f'Set :CFBundleIdentifier {ID}',str(app/'Contents/Info.plist')],check=True)
 subprocess.run(['/usr/bin/codesign','--force','--deep','-s','-',str(app)],check=True,capture_output=True)
 assert before==tree(FROZEN/'Petari.app')
 data={'source_app':str(FROZEN/'Petari.app'),'source_exe_sha256':build['binary_sha256'],
       'perf_exe_sha256':sha(app/'Contents/MacOS/Petari'),'source':build['source'],'renderer_key':build['renderer_key'],
       'bundle_id':ID,'resource_tree':tree(app/'Contents/Resources'),'frozen_tree_sha256':hashlib.sha256(json.dumps(before,sort_keys=True).encode()).hexdigest()}
 write(meta,data);return app,data

def prep_dev(src,expected,source):
 src=Path(src).resolve();assert src.is_relative_to(REPO/'build') and re.fullmatch('[0-9a-f]{40}',source)
 assert sha(src/'Contents/MacOS/Petari')==expected
 dest=ROOT/'apps'/('dev-input-'+expected[:12]+'-perfm1a');app=dest/'Petari.app'
 if not dest.exists():
  disk();dest.mkdir(parents=True);subprocess.run(['/usr/bin/ditto',str(src),str(app)],check=True)
  subprocess.run(['/usr/libexec/PlistBuddy','-c',f'Set :CFBundleIdentifier {ID}',str(app/'Contents/Info.plist')],check=True)
  subprocess.run(['codesign','--force','--deep','-s','-',str(app)],check=True,capture_output=True)
  write(dest/'PREP.json',{'build_kind':'dev-m1','source':source,'source_app':str(src),'source_exe_sha256':expected,'perf_exe_sha256':sha(app/'Contents/MacOS/Petari'),'bundle_id':ID,'resources_identical':tree(app/'Contents/Resources')==tree(src/'Contents/Resources'),'frozen54_acceptance':False})
 meta=json.loads((dest/'PREP.json').read_text());assert meta['source']==source and meta['source_exe_sha256']==expected and meta['resources_identical'] and sha(app/'Contents/MacOS/Petari')==meta['perf_exe_sha256']
 return app,meta

def prep_candidate(directory):
 directory=Path(directory).resolve();assert directory.is_relative_to(ROOT/'candidates')
 build=json.loads((directory/'BUILD.json').read_text());src=directory/'Petari.app'
 expected=build['binary_sha256'];source=build['source']
 assert re.fullmatch('[0-9a-f]{40}',source) and sha(src/'Contents/MacOS/Petari')==expected
 dest=ROOT/'apps'/('candidate-'+expected[:12]+'-perfm1a');app=dest/'Petari.app'
 if not dest.exists():
  disk();dest.mkdir(parents=True);subprocess.run(['/usr/bin/ditto',str(src),str(app)],check=True)
  subprocess.run(['/usr/libexec/PlistBuddy','-c',f'Set :CFBundleIdentifier {ID}',str(app/'Contents/Info.plist')],check=True)
  subprocess.run(['codesign','--force','--deep','-s','-',str(app)],check=True,capture_output=True)
  write(dest/'PREP.json',{'build_kind':'server-candidate','source':source,'source_app':str(src),'source_exe_sha256':expected,'perf_exe_sha256':sha(app/'Contents/MacOS/Petari'),'bundle_id':ID,'resources_identical':tree(app/'Contents/Resources')==tree(src/'Contents/Resources')})
 meta=json.loads((dest/'PREP.json').read_text())
 assert meta['source']==source and meta['source_exe_sha256']==expected and meta['resources_identical'] and sha(app/'Contents/MacOS/Petari')==meta['perf_exe_sha256']
 assert tree(app/'Contents/Resources')==tree(src/'Contents/Resources')
 return app,{**meta,'renderer_key':build['renderer_key'],'candidate_directory':str(directory),'build_metadata':build,'frozen54_acceptance':False}

def campaign_build(candidate_directory=None):
 return prep_candidate(candidate_directory) if candidate_directory else prep()

@contextlib.contextmanager
def native_lock():
 LOCK.parent.mkdir(parents=True,exist_ok=True)
 with HEAVY.open('a') as heavy,LOCK.open('a') as lock:
  fcntl.lockf(heavy,fcntl.LOCK_EX)
  fcntl.lockf(lock,fcntl.LOCK_EX)
  try:yield
  finally:
   fcntl.lockf(lock,fcntl.LOCK_UN);fcntl.lockf(heavy,fcntl.LOCK_UN)

def quiet_snapshot():
 processes=subprocess.check_output(['ps','-axo','pid,ppid,ni,%cpu,time,comm'],text=True).splitlines()[1:]
 compilers=[]
 for row in processes:
  if 'MTLCompilerService' not in row:continue
  pid,ppid,nice,cpu,used,command=row.split(None,5)
  fields=used.split(':');total=0.
  for field in fields:total=total*60+float(field)
  compilers.append({'pid':int(pid),'cpu_time':used,'cpu_seconds':total,'command':command})
 return {'load':os.getloadavg(),'metal_compilers':compilers,
         'top_processes':sorted(processes,key=lambda s:float(s.split()[3]),reverse=True)[:12]}

def wait_quiet(evidence,seconds=900,limit=8):
 """Keep camera load real; never wait for camera PIDs or kill a compiler."""
 start=time.monotonic();samples=[];initial=quiet_snapshot();previous=initial
 while True:
  time.sleep(10)
  sample=quiet_snapshot();sample['elapsed_seconds']=round(time.monotonic()-start,3);samples.append(sample)
  old={p['pid']:p for p in previous['metal_compilers']};new={p['pid']:p for p in sample['metal_compilers']}
  sample['compiler_cpu_deltas']=[{'pid':pid,'before_cpu_seconds':old[pid]['cpu_seconds'] if pid in old else None,
    'after_cpu_seconds':new[pid]['cpu_seconds'] if pid in new else None,
    'delta_seconds':new[pid]['cpu_seconds']-old[pid]['cpu_seconds'] if pid in old and pid in new else None,
    'idle':pid not in new or (pid in old and 0<=new[pid]['cpu_seconds']-old[pid]['cpu_seconds']<.05)} for pid in sorted(old.keys()|new.keys())]
  ready=sample['load'][0]<=limit and sample['load'][1]<=limit and all(p['idle'] for p in sample['compiler_cpu_deltas'])
  write(evidence,{'quiet':ready,'limit_1min_load':limit,'limit_5min_load':limit,'wait_limit_seconds':seconds,
        'initial_snapshot':initial,
        'compiler_policy':'PORT: two CPU-time samples 10s apart; each resident service must gain <0.05s. Never kill.',
        'background_policy':'Front Door camera ffmpeg/python retained as real background load; no process-specific wait.',
        'samples':samples})
  if ready:return sample
  if time.monotonic()-start>=seconds:raise RuntimeError('quiet precondition failed: 1/5-minute load or lingering MTLCompilerService; see '+str(evidence))
  previous=sample

def clear_metal():
 base=Path(subprocess.check_output(['/usr/bin/getconf','DARWIN_USER_CACHE_DIR'],text=True).strip())
 assert base.is_absolute() and base!=Path('/')
 target=base/ID
 assert target.name=='dev.petari.Petari.perfm1a'
 if target.exists():shutil.rmtree(target)

def rows(path):
 if not Path(path).exists():return []
 with Path(path).open() as f:return list(csv.DictReader(f))

FAULT = re.compile(r'FATAL|AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|more than 2 GiB|unhandled tcg| in ".*" on line \d+\.')

def run(app,out,*,stage=None,scenario=1,script=None,extra=None,warm=None,copied=False,seconds=180,bound=False,cold=False,nice=0,load_workers=0,fixture=None,verify_fixture_attrs=False,startup_observe=False,boot_probe=False, boot_probe_m1_screen=False):
 """The lock covers cache clearing, app lifetime and cleanup; terminate only this Popen PID."""
 out=Path(out);assert not out.exists(),out
 assert out.resolve().is_relative_to(ROOT.resolve()) and out.resolve()!=ROOT.resolve(),'run output must stay inside owned m1g'
 app=Path(app);exe=app/'Contents/MacOS/Petari'
 assert nice in (0,19) and 0<=load_workers<=4
 assert not verify_fixture_attrs or copied
 assert ID == subprocess.check_output(['/usr/libexec/PlistBuddy','-c','Print :CFBundleIdentifier',str(app/'Contents/Info.plist')],text=True).strip()
 with native_lock():
  disk();out.mkdir(parents=True);wait_quiet(out/'quiet.json')
  user=out/'user';cache=out/'cache'
  seed=Path(fixture) if fixture else FIXTURE
  assert seed.resolve() in (FIXTURE.resolve(),(ROOT/'fixtures/unlocked-242/fixture').resolve())
  seed_before=tree(seed) if copied else None;seed_inventory=None
  if copied:
   certified=json.loads((seed.parent/'receipt.json').read_text())
   assert seed_before==certified['files'],'unlocked fixture differs from receipt'
   if verify_fixture_attrs:
    from promotion54m import inventory
    seed_inventory=inventory(seed);assert seed_inventory=={k:certified[k] for k in ('files','xattrs')}
   subprocess.run(['/usr/bin/ditto','--rsrc','--extattr',str(seed),str(user)],check=True)
   assert tree(user)==seed_before,'private save copy differs from fixture'
   if verify_fixture_attrs:assert inventory(user)==seed_inventory,'private save xattrs differ from certified local fixture'
  else:user.mkdir()
  if warm:shutil.copytree(Path(warm)/'cache',cache)
  else:cache.mkdir()
  if cold:clear_metal()
  env={k:v for k,v in os.environ.items() if not k.startswith(('PETARI_','ASTRAL_','DYLD_','THRPROBE_'))}
  env.update(SDL_AUDIO_DRIVER='dummy',PETARI_MODS='none',PETARI_CACHE_DIR=str(cache),PETARI_FRAME_CSV=str(out/'frames.csv'),PETARI_FRAME_STATS='1',PETARI_ROUTE_RECORD=str(out/'route.csv'),PETARI_PIPELINE_DIAG='0',LANG='C',LC_ALL='C',PYTHONDONTWRITEBYTECODE='1')
  if script:
   env.update(PETARI_INPUT_SCRIPT=str(script),PETARI_INPUT_SCRIPT_STEADY='60',PETARI_INPUT_SCRIPT_QUIT='1',PETARI_INPUT_SCRIPT_LOG=str(out/'input.csv'))
  env.update(extra or {})
  if boot_probe:env['PETARI_BOOT_SPLASH_CAPTURE']=str(out/'boot-splash.png')
  command=[str(exe),'--disc',str(DISC),'--user',str(user)]
  if stage:command+=['--stage',stage,'--scenario',str(scenario)]
  if nice:command=['/usr/bin/nice','-n',str(nice),*command]
  before=sha(exe);start=time.monotonic(); timed_out=False
  info={'command':command,'environment':{k:v for k,v in env.items() if k.startswith(('PETARI_','SDL_','THRPROBE_','DYLD_'))},'app_sha256':before,'host':platform.node(),'machine':platform.machine(),'nice':os.getpriority(os.PRIO_PROCESS,0),'cold_metal_cleared':cold,'warm_from':str(warm) if warm else None,'free_bytes':disk(),'load':os.getloadavg(),'start_utc':time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime()),'bound_seconds':seconds,'bound_intentional':bound}
  write(out/'launch.json',info)
  loads=[]
  try:
   for _ in range(load_workers):loads.append(subprocess.Popen(['/usr/bin/yes'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL))
   info.update(requested_app_nice=nice,controlled_cpu_load_pids=[p.pid for p in loads],controlled_cpu_load_workers=load_workers)
   with (out/'app.log').open('wb') as log:
    observer=None
    if startup_observe:
     from startup542 import Observer
     observer=Observer(out,stage)
    if observer:observer.popen_before()
    p=subprocess.Popen(command,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
    if observer:
     observer.spawned(p.pid)
     info['popen_clocks']={k:observer.result[k] for k in ('popen_call_before','popen_return_after')}
    responsiveness=None
    if boot_probe:
     from boot_response542 import Probe
     responsiveness=Probe(out,p.pid,app,m1_screenshot=boot_probe_m1_screen)
    info['pid']=p.pid
    if nice:time.sleep(.05)
    try:info['child_nice_at_spawn']=os.getpriority(os.PRIO_PROCESS,p.pid)
    except ProcessLookupError:info['child_nice_at_spawn']=None
    write(out/'launch.json',info)
    try:p.wait(timeout=seconds)
    except subprocess.TimeoutExpired:
     timed_out=True;p.terminate()
     try:p.wait(timeout=20)
     except subprocess.TimeoutExpired:p.kill();p.wait()
    except BaseException:
     if p.poll() is None:
      p.terminate()
      try:p.wait(timeout=20)
      except subprocess.TimeoutExpired:p.kill();p.wait()
     raise
  finally:
   for worker in loads:
    if worker.poll() is None:worker.terminate()
   for worker in loads:
    try:worker.wait(timeout=5)
    except subprocess.TimeoutExpired:worker.kill();worker.wait()
   write(out/'controlled-load.json',{'owned_pids':[p.pid for p in loads],'all_reaped':all(p.poll() is not None for p in loads)})
  if startup_observe and observer:observer.finish()
  if boot_probe and responsiveness:responsiveness.finish()
  text=(out/'app.log').read_text(errors='replace')
  res={'exit':p.returncode,'timed_out':timed_out,'seconds':round(time.monotonic()-start,3),'binary_unchanged':sha(exe)==before,'seed_unchanged':not copied or tree(seed)==seed_before and (not verify_fixture_attrs or inventory(seed)==seed_inventory),'no_faults':not FAULT.search(text),'load_end':os.getloadavg(),'completed':p.returncode==0 and (bound or not timed_out)}
  if copied:res.update(fixture=str(seed),all_fixture_xattrs_verified=verify_fixture_attrs)
  write(out/'process.json',res)
  shutil.rmtree(user)
  return res

def trim(out,cache=True,captures=True):
 out=Path(out).resolve();assert out.is_relative_to(ROOT.resolve()) and out!=ROOT.resolve(),'cleanup refuses paths outside owned m1g runs'
 for name in (['cache'] if cache else [])+(['captures','tmp','dump'] if captures else []):
  p=out/name
  if p.exists():
   if p.is_dir():shutil.rmtree(p)
   else:p.unlink()
 disk()
