import argparse,datetime,fcntl,hashlib,json,os,re,shutil,subprocess,tempfile,time
from pathlib import Path
H=Path.home();V=H/'toolchains/sol-t-validation';O=V/'worldmap51';P=H/'MacioGalaxy2-gamedata/solT-worldmap51';P.mkdir(mode=0o700,exist_ok=True)
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
a=argparse.ArgumentParser();a.add_argument('--receipt',default=str(O/'build-receipt.json'));a.add_argument('--case',default='pristine-unlocked-idle-camera');a.add_argument('--seconds',type=int,default=210);a.add_argument('--diag',action='store_true');a.add_argument('--normal-boot',action='store_true');a.add_argument('--input-script');a.add_argument('--scene-span',action='store_true');a.add_argument('--host-boot',action='store_true');args=a.parse_args()
if args.host_boot:args.normal_boot=True
r=json.loads(Path(args.receipt).read_text());R=Path(r['source_root']);exe=Path(r['binary_path']);assert sha(exe)==r['binary_sha256'];assert args.seconds<=540
if r.get('object_freshness_verified'):
 assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()==r['source_sha'],'source HEAD changed'
 assert hashlib.sha256(subprocess.check_output(['git','diff','HEAD'],cwd=R)).hexdigest()==r['source_diff_sha256'],'effective source diff changed'
 assert sha(R/'native/cmake/M1LLVM.cmake')==r['toolchain_sha256']
 assert sha(H/'toolchains/LLVM-20.1.8-macOS-ARM64/bin/clang++')==r['compiler_sha256']
 f=Path(r['object_freshness_receipt']);assert sha(f)==r['object_freshness_receipt_sha256']
 for v in json.loads(f.read_text()).values():
  assert sha(v['object'])==v['object_sha256']
  if v['generated_sha256']:assert sha(v['generated'])==v['generated_sha256']
 for target,digest in r['logs'].items():assert sha(Path(r.get('log_paths',{}).get(target,str(O/('route-diag-'+target+'.log')))))==digest

out=Path(tempfile.mkdtemp(prefix=args.case+'-',dir=P));seed=H/'MacioGalaxy2-gamedata/unlocked-save-copy-20261020';shutil.copytree(seed,out/'user');seedhash={str(p.relative_to(seed)):sha(p) for p in seed.rglob('*') if p.is_file()}
for n in ['cache','tmp','captures/xfb','captures/surface']:(out/n).mkdir(mode=0o700,parents=True,exist_ok=True)
script=out/'keyboard.txt';script.write_text('5400\n120 Q\n120\n120 E\n120\n60 C\n120\n120 Z\n120\n120 X\n600\n')
if args.input_script: shutil.copyfile(args.input_script,script)
def state(phase,pid='none'):
 line=f'UTC={datetime.datetime.now(datetime.timezone.utc).isoformat()} label="{r["label"]}" phase={phase} inputs={args.receipt};seed={seed};script={script} output={out} PID={pid} controllerPID={os.getpid()} resume=inspect-private-result-and-owned-bounded-run;new-user-each-run\n';(V/'STATE').write_text(line);(O/'STATE').write_text(line)
state('PRE-LOCK-WORLDMAP');lock=(H/'MacioGalaxy2-team/slots/native.lock').open('a')
try:fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
except BlockingIOError:state('LOCK-BUSY-NO-APP');raise SystemExit('native.lock held')
os.setpriority(os.PRIO_PROCESS,0,19)
try:
 env={k:v for k,v in os.environ.items() if not k.startswith(('PETARI_','ASTRAL_'))}
 env.update(PETARI_MODS='none',PETARI_TRACE_BOOT='1',PETARI_CACHE_DIR=str(out/'cache'),PETARI_ROUTE_RECORD=str(out/'route.csv'),PETARI_FRAME_CSV=str(out/'frames.csv'),PETARI_FRAME_STATS='1',PETARI_INPUT_SCRIPT=str(script),PETARI_INPUT_SCRIPT_LOG=str(out/'input.csv'),PETARI_INPUT_SCRIPT_STEADY='60',TMPDIR=str(out/'tmp')+'/',LANG='C',LC_ALL='C')
 for n in ['XFB','SURFACE']:
  env['PETARI_'+n+'_DUMP']=str(out/'captures'/n.lower());env['PETARI_'+n+'_DUMP_LABELS']='boot,input-script';env['PETARI_'+n+'_DUMP_EVERY']='120';env['PETARI_'+n+'_DUMP_SPAN']='18000'
 for key,value in r.get('diagnostic_environment',{}).items():
  assert key.startswith('PETARI_FIFO_') and isinstance(value,str),'non-worker diagnostic environment rejected'
  env[key]=value
 if args.diag:env['PETARI_WM_DIAG']='1'
 if args.scene_span:env['PETARI_INPUT_SCRIPT_SCENE_SPAN']='1'
 if args.normal_boot and not args.host_boot:env.update(PETARI_SMOKE='observe',PETARI_SMOKE_FRAMES='30000',PETARI_SMOKE_BACKGROUND='0',PETARI_SMOKE_IGNORE_PHYSICAL='1')
 cmd=['python3',str(R/'native/tools/bounded_run.py'),str(args.seconds),str(exe),'--disc',str(H/'MacioGalaxy2-gamedata/SB4E01-extracted'),'--user',str(out/'user'),'--stage','WorldMap01Galaxy','--scenario','1']
 if args.normal_boot:cmd=cmd[:-4]
 start=time.time();provenance=dict(build_receipt=r,command=cmd,environment={k:v for k,v in env.items() if k.startswith('PETARI_')},start_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),start_epoch=start,seed_hashes=seedhash,pointer_click='HOST INPUT REQUESTED; boot/map interaction requires observed evidence' if args.host_boot else 'NOT RUN (OS permission)',alternate_window_size='pending review' if args.host_boot else 'NOT RUN (OS permission)',diagnostic=args.diag,normal_boot=args.normal_boot,scene_span=args.scene_span,host_boot=args.host_boot,cache_initial_files=[])
 (out/'command.json').write_text(json.dumps(provenance,indent=2)+'\n');state('PRE-SPAWN-WORLDMAP')
 with (out/'app.log').open('x') as log:
  p=subprocess.Popen(cmd,cwd=R,env=env,stdout=log,stderr=subprocess.STDOUT);state('RUNNING-WORLDMAP',p.pid);print(str(out),flush=True);rc=p.wait()
 state('COMPLETED-WORLDMAP')
 lines=(out/'app.log').read_text(errors='replace').splitlines();fault=next((s for s in lines if re.search(r'FATAL|indexed XF load from unmapped|malloc:|Assertion.*failed|runtime error:',s,re.I)),None)
 if fault is None:fault=next((line for line in lines if line.startswith('signal: SIG')),None)
 fault_context=[]
 if fault:
  end=lines.index(fault)
  if fault.startswith('signal: SIG'):end=next((i for i,line in enumerate(lines[:end]) if '==== Petari native crash report' in line),end)
  fault_context=lines[max(0,end-3):end]+[fault]

 assert all(sha(seed/k)==v for k,v in seedhash.items()),'seed changed'
 captures={n:len(list((out/'captures'/n).glob('*.png'))) for n in ['surface','xfb']}
 result=dict(label=r['label'],binary_sha256=sha(exe),binary_unchanged=sha(exe)==r['binary_sha256'],returncode=rc,elapsed_seconds=round(time.time()-start,2),fault=fault,captures=captures,private_output=str(out),seed_unchanged=True,pointer_click='HOST INPUT REQUESTED; boot/map interaction requires observed evidence' if args.host_boot else 'NOT RUN (OS permission)',alternate_window_size='pending review' if args.host_boot else 'NOT RUN (OS permission)',diagnostic=args.diag,last_lines=(fault_context if fault else lines[-4:]))
 result['cache_final']={str(f.relative_to(out/'cache')):dict(bytes=f.stat().st_size,sha256=sha(f)) for f in (out/'cache').rglob('*') if f.is_file()}
 result['route_sha256']=sha(out/'route.csv') if (out/'route.csv').exists() else None
 result['input_sha256']=sha(out/'input.csv') if (out/'input.csv').exists() else None
 (out/'result.json').write_text(json.dumps(result,indent=2)+'\n');(O/'latest-result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
finally:fcntl.flock(lock,fcntl.LOCK_UN);lock.close();state('RELEASED-WORLDMAP')
