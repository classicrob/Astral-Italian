import datetime,fcntl,hashlib,json,os,shutil,subprocess,sys
from pathlib import Path
H=Path.home();O=H/'toolchains/sol-t-validation/worldmap51';V=O.parent;R=H/'MacioGalaxy2-team/scratch-port/solT/worldmap51-source';B=R/'build/gx-llvm20-worldmap51'
package=json.loads(Path(sys.argv[1]).read_text());sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest();old=json.loads((O/'route-ab-typed-receipt.json').read_text());edge=json.loads((O/'actual-worker-compiled-edge.json').read_text())[0];source=Path(edge['source']);obj=Path(edge['object'])
assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip()==old['source_sha']
assert hashlib.sha256(subprocess.check_output(['git','diff','HEAD'],cwd=R)).hexdigest()==old['source_diff_sha256']
assert sha(R/'native/cmake/M1LLVM.cmake')==old['toolchain_sha256'];assert sha(source)==edge['source_sha256'];assert sha(obj)==edge['object_sha256']
assert package['base_sha256']==edge['source_sha256'];assert sha(package['patched_cpp'])==package['patched_cpp_sha256']
def state(phase,pid='none'):
 s=f'UTC={datetime.datetime.now(datetime.timezone.utc).isoformat()} phase={phase} inputs=8c9430f8+overlay1048d955+toolchainfbd11526;workerpackage={sys.argv[1]} output={O} PID={pid} controllerPID={os.getpid()} resume=worker-build-receipt-before-launch;deadline05UTC\n';(V/'STATE').write_text(s);(O/'STATE').write_text(s)
state('PRE-LOCK-WORKER-DIAG');lock=(H/'MacioGalaxy2-team/slots/native.lock').open('a');fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB);os.setpriority(os.PRIO_PROCESS,0,19)
def run(target):
 name='worker-diag-'+target;log=O/(name+'.log');state('PRE-SPAWN-'+name)
 with log.open('x') as f:
  cmd=['cmake','--build',str(B),'--target',target,'--parallel','2'];f.write(repr(cmd)+'\n');f.flush();p=subprocess.Popen([sys.executable,str(R/'native/tools/bounded_run.py'),'300',*cmd],cwd=R,stdout=f,stderr=subprocess.STDOUT);state('RUNNING-'+name,p.pid);rc=p.wait()
 state('COMPLETE-'+name+'-exit'+str(rc));print(name,rc,flush=True);assert rc==0
try:
 run('petari_legacy_sources');assert sha(source)==edge['source_sha256'],'generator changed expectedbase'
 source.write_bytes(Path(package['patched_cpp']).read_bytes())
 for extra in package.get('adjacent_files',[]):
  assert sha(extra['path'])==extra['sha256'];dest=source.parent/extra['name'];assert not dest.exists();shutil.copyfile(extra['path'],dest)
 run('petari');assert sha(source)==package['patched_cpp_sha256'];assert sha(obj)!=edge['object_sha256'];assert obj.stat().st_mtime_ns>=source.stat().st_mtime_ns
 log=O/'worker-diag-petari.log';assert 'Linking CXX executable' in log.read_text();app=B/'native/app/Petari.app';exe=app/'Contents/MacOS/Petari';assert sha(exe)!=old['binary_sha256'];assert package['binary_marker'].encode() in exe.read_bytes()
 saved=O/'worker-diag-Petari.app';assert not saved.exists();shutil.copytree(app,saved,symlinks=True);exe=saved/'Contents/MacOS/Petari'
 fresh={'worker':dict(object=str(obj),object_sha256=sha(obj),object_mtime_ns=obj.stat().st_mtime_ns,generated=str(source),generated_sha256=sha(source),generated_mtime_ns=source.stat().st_mtime_ns)};fr=O/'worker-diag-object-after.json';fr.write_text(json.dumps(fresh,indent=2))
 receipt=dict(old);receipt.update(binary_path=str(exe),binary_sha256=sha(exe),incremental_from=old['binary_sha256'],worker_package=package,worker_package_manifest=str(Path(sys.argv[1]).resolve()),worker_package_manifest_sha256=sha(sys.argv[1]),diagnostic_environment=package['environment'],diagnostic='worker-only rawopcode/localoffset/history plusunchanged606 game mode; nooriginalDL/caller/absoluteownership',object_freshness_receipt=str(fr),object_freshness_receipt_sha256=sha(fr),logs={t:sha(O/('worker-diag-'+t+'.log')) for t in ('petari_legacy_sources','petari')},log_paths={t:str(O/('worker-diag-'+t+'.log')) for t in ('petari_legacy_sources','petari')},generated_worker_external_delta=True)
 (O/'worker-diag-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(dict(binary_sha256=receipt['binary_sha256'],worker_object=sha(obj),worker_source=sha(source))),flush=True)
finally:fcntl.flock(lock,fcntl.LOCK_UN);lock.close();state('RELEASED-WORKER-DIAG-BUILD')
