"""Owned-session, source-bound GX gates; private evidence only."""
import argparse,atexit,csv,hashlib,json,math,os,re,signal,sqlite3,subprocess,time
from pathlib import Path
from cache_gate52 import inspect as inspect_cache
from visual_gate52 import validate_review
from unlocked_load52 import load_witness
HOME=Path.home();TEAM=HOME/'MacioGalaxy2-team/scratch-port/solL'
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inventory(root):
 files={};attrs={}
 if not root.exists():return {'files':files,'xattrs':attrs}
 for p in [root,*sorted(root.rglob('*'))]:
  assert not p.is_symlink(),str(p)
  rel='.' if p==root else p.relative_to(root).as_posix()
  if p.is_file():files[rel]=digest(p)
  names=subprocess.check_output(['/usr/bin/xattr',str(p)],text=True).splitlines()
  values={n:''.join(subprocess.check_output(['/usr/bin/xattr','-p','-x',n,str(p)],text=True).split()).lower() for n in sorted(names)}
  if values:attrs[rel]=values
 return {'files':files,'xattrs':attrs}
def csvrows(p):return list(csv.DictReader(p.open())) if p.exists() else []
def valid_rows(rows,stage,scenario=1):
 return [r for r in rows if r['stage']==stage and r['scenario']==str(scenario) and r['valid']=='1' and r.get('dead','0')=='0'
         and all(math.isfinite(float(r[c])) for c in ('x','y','z'))]
def cappy_check(name,text):
 throws=text.count('[cappy] throw (ground)');caught=text.count('[cappy] caught')
 transitions=re.findall(r'\[spin-switch\].*source=(\w+) before=(\d) after=(\d)',text)
 if name=='cappy-fluffy':return throws==2 and caught==2 and 'fly' in text and 'return' in text
 if name=='cappy-panel':return throws==1 and caught==1 and transitions==[('cappy','0','1'),('spin','1','0')]
 raise ValueError(name)
def main():
 parser=argparse.ArgumentParser();parser.add_argument('app',type=Path);parser.add_argument('sha256');parser.add_argument('label');parser.add_argument('--isolated-cache',action='store_true');parser.add_argument('--source',required=True);parser.add_argument('--renderer-key',required=True);parser.add_argument('--cases',default='all');parser.add_argument('--modes',default='fresh,warm,functional');args=parser.parse_args();SOURCE=args.source
 assert os.environ.get('SOL_L_PROMOTION_WINDOW')=='PORT_APPROVED'
 assert subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()==SOURCE
 exe=args.app/'Contents/MacOS/Petari';assert digest(exe)==args.sha256
 root=HOME/'MacioGalaxy2-gamedata/solL'/args.label;root.mkdir(mode=0o700)
 fixture_root=HOME/'MacioGalaxy2-gamedata/solL/build52-save-fixture-r3';fixture=fixture_root/'fixture';save=json.loads((fixture_root/'receipt.json').read_text())
 assert inventory(fixture)=={k:save[k] for k in ('files','xattrs')}
 assert inventory(Path(save['source']))=={k:save[k] for k in ('files','xattrs')}
 key=(Path.cwd()/'build/macos-arm64-release/native/gx/pipeline_cache_build_identity.key').read_text().strip()
 assert key==args.renderer_key and re.fullmatch('[0-9a-f]{64}',key)
 real=HOME/'Library/Caches/AstralItalian'
 def foreign():
  return {p.relative_to(real).as_posix():digest(p) for p in sorted(real.rglob('*')) if p.is_file() and key not in p.relative_to(real).parts[0]}
 def caches(path):
  return [inspect_cache(p,key) for p in sorted(path.glob('*.db')) if key in p.name] if path.exists() else []
 initial_foreign=foreign();initial_paths=list(real.glob('*'+key+'*')) if real.exists() else []
 (root/'cache-branch.json').write_text(json.dumps({'key':key,'real_namespace_initially_absent':not initial_paths,'first_real_may_be_true_fresh':not initial_paths,'later_true_fresh_private':True,'foreign':initial_foreign},indent=2)+'\n')
 active=None;results=[]
 def state(phase,out):
  (TEAM/'STATE').write_text(json.dumps({'host':os.uname().nodename,'source_commit':SOURCE,'binary_sha256':args.sha256,'phase':phase,'output':str(out),'owner_pid':os.getpid(),'child_pid':active.pid if active and active.poll() is None else None,'resume_action':'USER INCOMING stops owned app; interrupted case unaccepted, never auto retry'})+'\n')
 def stop():
  nonlocal active
  if active and active.poll() is None:
   active.send_signal(signal.SIGTERM)
   try:active.wait(timeout=20)
   except subprocess.TimeoutExpired:os.killpg(active.pid,signal.SIGKILL);active.wait()
 def finish():
  stop();subprocess.run(['bb','thread','tell','thr_xemjeg9f54','WINDOW CAN CLOSE: all owned52 apps exited. '+str(len(results))+'/14 receipts; '+ '; '.join(r['case']+'/'+r['mode']+':'+('PASS' if r['passed'] else 'FAIL') for r in results)+'. Private '+str(root)],check=False)
 atexit.register(finish)
 def interrupted(signum,frame):stop();raise SystemExit(128+signum)
 for sig in (signal.SIGTERM,signal.SIGINT,signal.SIGHUP):signal.signal(sig,interrupted)
 cases=[]
 for name,stage,seconds,copied,hook in [('yoshi','YosshiHomeGalaxy',60,False,False),('flip','RedBlueExGalaxy',60,False,False),('fluffy','MokumokuValleyGalaxy',60,False,False),('unlocked-stage-adapter','MarioFaceShipGalaxy',90,True,False),('worldmap','WorldMap01Galaxy',90,True,False),('return','YosshiHomeGalaxy',120,True,True)]:
  cases.extend((name,stage,seconds,copied,hook,mode) for mode in ('fresh','warm'))
 cases.extend([('cappy-fluffy','MokumokuValleyGalaxy',90,False,False,'functional'),('cappy-panel','RedBlueExGalaxy',90,False,False,'functional')])
 for routecase in ('normal-route','normal-boot-ui'):
  if routecase in args.cases.split(','):cases.append((routecase,'MarioFaceShipGalaxy',240,True,False,'fresh'))
 if args.cases!='all':cases=[c for c in cases if c[0] in args.cases.split(',')]
 cases=[c for c in cases if c[-1] in args.modes.split(',')]
 assert cases,'No selected gates'
 for name,stage,seconds,copied,hook,mode in cases:
  out=root/(name+'-'+mode);out.mkdir();user=out/'user'
  if copied:
   subprocess.run(['/usr/bin/ditto','--rsrc','--extattr',str(fixture),str(user)],check=True)
   assert inventory(user)=={k:save[k] for k in ('files','xattrs')}
  else:user.mkdir()
  first_real_fresh=mode=='fresh' and not list(real.glob('*'+key+'*'))
  cache=(root/(name+'-cache')) if args.isolated_cache else (real if mode=='warm' or first_real_fresh else out/'cache')
  if cache!=real and mode!='warm':assert not cache.exists()
  before=caches(cache)
  if mode=='warm':assert any(r['rows'] for r in before),'Warm namespace absent'
  assert foreign()==initial_foreign
  env={k:v for k,v in os.environ.items() if not k.startswith(('PETARI_','ASTRAL_'))}
  env.update(PETARI_TRACE_BOOT='1',PETARI_MODS='none',PETARI_CACHE_DIR=str(cache),PETARI_FRAME_CSV=str(out/'frames.csv'),PETARI_ROUTE_RECORD=str(out/'route.csv'),PETARI_CAPTURE_DIR=str(out/'captures'),PETARI_CAPTURE_LOG=str(out/'app.log'),PETARI_FRAME_STATS='1',PETARI_EDITOR_MOUSE_TRACE='1',PETARI_PIPELINE_DIAG='1',SDL_AUDIO_DRIVER='dummy',LANG='C',LC_ALL='C')
  audio=mode=='warm' and name in ('worldmap','flip','fluffy')
  if audio:
   env.pop('SDL_AUDIO_DRIVER',None);env.update(PETARI_AUDIO_DIAG='1',PETARI_SMOKE_REAL_AUDIO='1')
  script=out/'input.txt';script.write_text('120\n10 W\n1200\n' if name=='worldmap' else '180\n')
  if name in ('normal-route','normal-boot-ui'):script.write_text((TEAM/'worldmap-recorded-ship2.txt').read_text())
  if hook:env['PETARI_TEST_RETURN_TO_SHIP']='180:0'
  env.update(PETARI_INPUT_SCRIPT=str(script),PETARI_INPUT_SCRIPT_LOG=str(out/'input.csv'),PETARI_INPUT_SCRIPT_STEADY='240' if hook else '120')
  if name.startswith('cappy-'):
   script.write_text((TEAM/'cappy52-inputs'/('fluffy-throw-return.txt' if name=='cappy-fluffy' else 'flipswap-r-then-f.txt')).read_text())
   (user/'mods.txt').write_text('OdysseyMovement=off\nCappy=on\n' if name=='cappy-fluffy' else 'OdysseyMovement=on\nCappy=on\n')
   env.update(PETARI_MODS='' if name=='cappy-fluffy' else 'OdysseyMovement,Cappy',PETARI_CAPPY_TRACE='1',PETARI_ODYSSEY_TRACE='1',PETARI_INPUT_SCRIPT_STEADY='60',PETARI_XFB_DUMP=str(out/'captures/xfb'),PETARI_XFB_DUMP_LABELS='input-script',PETARI_XFB_DUMP_EVERY='6',PETARI_XFB_DUMP_SPAN='600',PETARI_SURFACE_DUMP=str(out/'captures/surface'),PETARI_SURFACE_DUMP_LABELS='input-script',PETARI_SURFACE_DUMP_EVERY='6',PETARI_SURFACE_DUMP_SPAN='600',TMPDIR=str(out/'tmp')+'/')
   (out/'tmp').mkdir();(out/'captures/xfb').mkdir(parents=True);(out/'captures/surface').mkdir()
   if name=='cappy-fluffy':env['PETARI_ODYSSEY_MOVEMENT']='1'
  command=[str(exe),'--disc',str(HOME/'MacioGalaxy2-gamedata/SB4E01-extracted'),'--user',str(user),'--stage',stage,'--scenario','2' if name in ('normal-route','normal-boot-ui') else '1']
  if name=='normal-boot-ui':command=command[:5]
  state('PROMOTION52_BEFORE_'+name+'_'+mode,out)
  (out/'launch.json').write_text(json.dumps({'source':SOURCE,'binary_sha256':args.sha256,'command':command,'environment':{k:v for k,v in env.items() if k.startswith('PETARI_') or k in ('SDL_AUDIO_DRIVER','TMPDIR')}},indent=2)+'\n')
  with (out/'app.log').open('wb') as log:
   active=subprocess.Popen(command,env=env,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
   state('PROMOTION52_RUNNING_'+name+'_'+mode,out);start=time.monotonic();print(json.dumps({'event':'app-spawn','case':name,'mode':mode,'pid':active.pid,'output':str(out),'pointer_receipt_required':name in ('worldmap','normal-route','normal-boot-ui')}),flush=True)
   while active.poll() is None and time.monotonic()-start<seconds:time.sleep(.25)
   alive=active.poll() is None;stop()
  text=(out/'app.log').read_text(errors='replace');route=csvrows(out/'route.csv');valid=valid_rows(route,stage,2 if name in ('normal-route','normal-boot-ui') else 1)
  frames=csvrows(out/'frames.csv');phases={}
  for r in frames:phases[r['phase']]=phases.get(r['phase'],0)+1
  faults=[line for line in text.splitlines() if re.search(r'FATAL|AddressSanitizer|UndefinedBehaviorSanitizer|runtime error:|more than 2 GiB|unhandled tcg| in ".*" on line \d+\.',line)]
  hooks=[line for line in text.splitlines() if '[return-to-ship-test]' in line];inputs=csvrows(out/'input.csv')
  checks={'alive_at_bound':alive,'clean_exit':active.returncode==0,'menu_frames':phases.get('menu',0)>0,'gameplay_frames':phases.get('gameplay',0)>=120,'requested_stage_valid':len(valid)>=120,'no_faults':not faults,'input_started':bool(inputs),'foreign_untouched':foreign()==initial_foreign,'binary_unchanged':digest(exe)==args.sha256}
  if not hook:checks['hook_off']=not hooks
  if name=='worldmap':
   checks['walk_and_idle']=bool(valid) and any(abs(float(r[c])-float(valid[0][c]))>.01 for r in valid for c in ('x','y','z')) and '[input-script] done' in text
   pointer=[r for r in valid if r.get('pointer_valid')=='1']
   checks['host_pointer_rows']=len(pointer)>=30 and len({(r.get('pointer_x'),r.get('pointer_y')) for r in pointer})>=3
   evidence=out/'host-pointer.json'
   checks['host_pointer_observation']=False
   if evidence.exists():
    observed=json.loads(evidence.read_text());checks['host_pointer_observation']=observed.get('binary_sha256')==args.sha256 and observed.get('source')==SOURCE and observed.get('ordinary_cua_input') is True and observed.get('mapping_observed') is True and observed.get('observed_pid')==active.pid and start<=observed.get('observed_monotonic',-1)<=start+seconds and all(observed.get(k) for k in ('window_points','image_rect_points','before_screenshot','after_screenshot'))
   checks['existing_unlocked_file1']=any(r.get('power_stars')=='242' for r in valid) and 'FileSelector.OpenFile' in text and 'FileSelector.Create' not in text
  elif name in ('normal-route','normal-boot-ui'):
   mapped=valid_rows(route,'WorldMap01Galaxy')
   checks['normal_route_scene_chain']=len(valid)>=120 and len(mapped)>=120 and next((i for i,r in enumerate(route) if r['stage']=='MarioFaceShipGalaxy'),len(route)) < next((i for i,r in enumerate(route) if r['stage']=='WorldMap01Galaxy'),-1)
   checks['unlocked_map']=load_witness(name,text,mapped,command)
   pointer=[r for r in mapped if r.get('pointer_valid')=='1']
   checks['normal_route_pointer']=len(pointer)>=30 and len({(r.get('pointer_x'),r.get('pointer_y')) for r in pointer})>=3
   evidence=out/'host-pointer.json'
   observed=json.loads(evidence.read_text()) if evidence.exists() else {}
   checks['normal_route_host_observation']=observed.get('binary_sha256')==args.sha256 and observed.get('source')==SOURCE and observed.get('ordinary_cua_input') is True and observed.get('mapping_observed') is True and observed.get('observed_pid')==active.pid and start<=observed.get('observed_monotonic',-1)<=start+seconds and observed.get('normal_route_observed') is True and all(observed.get(k) for k in ('window_points','image_rect_points','before_screenshot','after_screenshot'))
   try:checks['normal_route_visual_acceptance']=validate_review(out,{'source':SOURCE,'binary_sha256':args.sha256,'case':name,'mode':mode},active.pid,start,start+seconds)
   except (ValueError,OSError,TypeError,KeyError):checks['normal_route_visual_acceptance']=False
  elif hook:
   ship=[r for r in route if r['stage']=='MarioFaceShipGalaxy' and r['scenario']=='2' and r['valid']=='1']
   fired=[line for line in hooks if 'request' in line and 'invalid' not in line]
   checks['canonical_return_ready']=len(valid)>=180 and len(ship)>=120 and len(fired)==1
   first=inputs[0] if inputs else None
   near=lambda r:first and all(abs(float(r[c])-float(first[c]))<.1 for c in ('x','y','z'))
   checks['idle_started_on_ship']=any(near(r) for r in ship) and not any(near(r) for r in valid)
   checks['input_done']='[input-script] done after 180 frames' in text
  elif name.startswith('cappy-'):
   checks['ordinary_feature_events']=cappy_check(name,text);checks['valid_non_dead_finite_300']=len(valid)>=300;checks['input_done']='[input-script] done' in text
  else:checks['input_done']='[input-script] done after 180 frames' in text
  if name=='unlocked-stage-adapter':checks['existing_unlocked_file1']=any(r.get('power_stars')=='242' for r in valid) and 'FileSelector.OpenFile' in text and 'FileSelector.StartupCreate' not in text and 'FileSelector.Create' not in text
  audio_underruns=[int(x) for x in re.findall(r'underrun (\d+) frames',text)]
  if audio:checks['real_audio_zero_underruns']=len(audio_underruns)>=50 and sum(audio_underruns)==0 and 'SDL audio underrun:' not in text
  after=caches(cache);checks['matching_key_cache_written']=any(r['rows'] for r in after)
  record={'case':name,'mode':mode,'passed':all(checks.values()),'checks':checks,'source':SOURCE,'binary_sha256':args.sha256,'renderer_key':key,'cache':str(cache),'true_fresh_cache':mode=='fresh' and (cache!=real or first_real_fresh),'before_cache':before,'after_cache':after,'seconds':seconds,'exit':active.returncode,'frame_counts':phases,'valid_requested_rows':len(valid),'hook_lines':hooks,'input_rows':len(inputs),'faults':faults,'real_audio':audio,'audio_underrun_frames':sum(audio_underruns) if audio else None,'adapter':('ordinary normal-boot file-select UI on PRIVATE_COPY' if name=='normal-boot-ui' else 'direct --user PRIVATE_COPY --stage; normal-boot UI not driven') if copied else None,'visual_acceptance_pending':name.startswith('cappy-') or (name in ('normal-route','normal-boot-ui') and not checks.get('normal_route_visual_acceptance',False))}
  (out/'receipt.json').write_text(json.dumps(record,indent=2)+'\n');results.append(record);(root/'results.json').write_text(json.dumps(results,indent=2)+'\n')
  active=None;state('PROMOTION52_COMPLETE_'+name+'_'+mode,out)
  print(json.dumps({'case':name,'mode':mode,'passed':record['passed'],'failed_checks':[k for k,v in checks.items() if not v],'valid_rows':len(valid)}),flush=True)
  if not record['passed']:raise SystemExit(1)
 assert inventory(fixture)=={k:save[k] for k in ('files','xattrs')}
 assert inventory(Path(save['source']))=={k:save[k] for k in ('files','xattrs')}
 print(str(len(results))+' selected runtime gates PASS; selected cases only; normal-route/Cappy visual review remain separate promotion requirements',flush=True)
if __name__=='__main__':main()
