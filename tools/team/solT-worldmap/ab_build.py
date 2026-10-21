import datetime,fcntl,hashlib,json,os,shutil,subprocess,sys,time,tempfile
from pathlib import Path
H=Path.home();O=H/'toolchains/sol-t-validation/worldmap51';V=O.parent;R=H/'MacioGalaxy2-team/scratch-port/solT/worldmap51-source';D=H/'MacioGalaxy2-wt/game-wm-route-diag';B=R/'build/gx-llvm20-worldmap51';diag='606d58aa45bc8c9c553ca218cb7da98ff126949c'
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
def git(*args,cwd=R):return subprocess.check_output(['git',*args],cwd=cwd)
def state(phase,pid='none'):
 line=f'UTC={datetime.datetime.now(datetime.timezone.utc).isoformat()} phase={phase} inputs={kind};source{head};diag{diag}+overlay1048d955+toolchainfbd11526 output={O} PID={pid} controllerPID={os.getpid()} resume=ab_build.py-receipts-before-launch\n';(V/'STATE').write_text(line);(O/'STATE').write_text(line)
kind=sys.argv[1];assert kind in ('tu13','combined')
target={'tu13':'046a4f0cf8263743913f4abc41d3cd3dcdce7a76','combined':'87b326a0fb80b2378cb5d953941939e78dd349da'}[kind]
parent={'tu13':'60e739bc2c5e7e462c86ff13101c5a14c03dd8ad','combined':'046a4f0cf8263743913f4abc41d3cd3dcdce7a76'}[kind]
old=json.loads((O/('route-diag-receipt.json' if kind=='tu13' else 'route-ab-tu13-receipt.json')).read_text())
assert git('rev-parse','HEAD').decode().strip()==parent
assert hashlib.sha256(git('diff','HEAD')).hexdigest()==old['source_diff_sha256']
assert sha(R/'native/cmake/M1LLVM.cmake')==old['toolchain_sha256']
paths=git('diff-tree','--no-commit-id','--name-only','-r',target).decode().splitlines()
assert set(paths)=={'config/SB4E01/symbols.txt','src/Game/System/WorldMapResultTU'+('13' if kind=='tu13' else '14')+'.cpp'}
bridge=[dict(path=n,base_sha256=sha(R/n),target_sha256=hashlib.sha256(git('show',target+':'+n)).hexdigest()) for n in paths]
subprocess.run(['git','switch','--detach',target],cwd=R,check=True)
assert hashlib.sha256(git('diff','HEAD')).hexdigest()==old['source_diff_sha256'],'overlay changed'
assert sha(R/'native/cmake/M1LLVM.cmake')==old['toolchain_sha256']
head=git('rev-parse','HEAD').decode().strip()
with tempfile.TemporaryDirectory(prefix='wm-diag-index-',dir=O) as td:
 env=dict(os.environ,GIT_INDEX_FILE=str(Path(td)/'index'))
 subprocess.run(['git','read-tree','HEAD'],cwd=R,env=env,check=True)
 subprocess.run(['git','add','-A','--','src','include','libs','native'],cwd=R,env=env,check=True)
 effective_tree=subprocess.check_output(['git','write-tree'],cwd=R,env=env,text=True).strip()
source_diff_sha=hashlib.sha256(git('diff','HEAD')).hexdigest()
cpp=[n for n in paths if n.endswith('.cpp')]
def snapshot():
 result={}
 for n in cpp:
  objects=list(B.rglob(Path(n).name+'.o'))
  assert len(objects)==1,(n,objects)
  obj=objects[0];gen=B/'legacy-sjis'/n
  result[n]=dict(object=str(obj),object_sha256=sha(obj),object_mtime_ns=obj.stat().st_mtime_ns,generated=str(gen),generated_sha256=sha(gen) if gen.exists() else None,generated_mtime_ns=gen.stat().st_mtime_ns if gen.exists() else None)
 return result
before=snapshot();(O/('route-ab-'+kind+'-object-before.json')).write_text(json.dumps(before,indent=2))
state('PRE-LOCK-DIAGNOSTIC');lock=(H/'MacioGalaxy2-team/slots/native.lock').open('a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB);os.setpriority(os.PRIO_PROCESS,0,19)
def run(target):
 name='route-ab-'+kind+'-'+target;log=O/(name+'.log');state('PRE-SPAWN-'+name)
 with log.open('x') as f:
  cmd=['cmake','--build',str(B),'--target',target,'--parallel','2'];f.write(repr(cmd)+'\n');f.flush()
  p=subprocess.Popen([sys.executable,str(R/'native/tools/bounded_run.py'),'540',*cmd],cwd=R,stdout=f,stderr=subprocess.STDOUT);state('RUNNING-'+name,p.pid);rc=p.wait()
 state('COMPLETE-'+name+'-exit'+str(rc));print(name,rc,flush=True)
 assert rc==0,(name,rc)
try:
 run('petari_legacy_sources');run('petari');after=snapshot();(O/('route-ab-'+kind+'-object-after.json')).write_text(json.dumps(after,indent=2))
 for n,v in after.items():
  assert v['object_sha256']!=before[n]['object_sha256'],n
  if v['generated_mtime_ns'] is not None:assert v['object_mtime_ns']>=v['generated_mtime_ns'],n
 log=O/('route-ab-'+kind+'-petari.log');assert 'Linking CXX executable' in log.read_text()
 app=B/'native/app/Petari.app';exe=app/'Contents/MacOS/Petari';assert sha(exe)!=old['binary_sha256']
 data=exe.read_bytes()
 for marker in (b'[wm-consume]',b'[wm-route-align]',b'[wm-body]',b'[wm-submit]'):assert marker in data,marker
 saved=O/('route-ab-'+kind+'-Petari.app');assert not saved.exists();shutil.copytree(app,saved,symlinks=True);exe=saved/'Contents/MacOS/Petari'
 receipt=dict(label=f'M1 iteration: {head} + overlay 1048d955 + toolchain fbd11526',source_sha=head,production_base=old['production_base'],native_fix='5878fad8',diagnostic_source=diag,diagnostic_input_driver=old['diagnostic_input_driver'],source_root=str(R),build_dir=str(B),binary_path=str(exe),binary_sha256=sha(exe),full_link=True,incremental_from=old['binary_sha256'],source_bridge=bridge,source_status=git('status','--short').decode(),effective_tree=effective_tree,source_diff_sha256=source_diff_sha,object_freshness_receipt=str(O/('route-ab-'+kind+'-object-after.json')),object_freshness_receipt_sha256=sha(O/('route-ab-'+kind+'-object-after.json')),toolchain_sha256=sha(R/'native/cmake/M1LLVM.cmake'),compiler_sha256=old['compiler_sha256'],logs={t:sha(O/('route-ab-'+kind+'-'+t+'.log')) for t in ('petari_legacy_sources','petari')},object_freshness_verified=True,dispatch_arm=kind,log_paths={t:str(O/('route-ab-'+kind+'-'+t+'.log')) for t in ('petari_legacy_sources','petari')},diagnostic='mode/camera/body; separately reviewed dispatch candidate; ordinal is presentation choice only',wii_object_proof='pending; non-native token exclusion only')
 (O/('route-ab-'+kind+'-receipt.json')).write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({k:receipt[k] for k in ('source_sha','binary_sha256','object_freshness_verified')}),flush=True)
finally:fcntl.flock(lock,fcntl.LOCK_UN);lock.close();state('RELEASED-DIAGNOSTIC-BUILD')
