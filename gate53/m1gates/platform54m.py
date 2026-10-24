#!/usr/bin/env python3
import json,os,re,sqlite3,subprocess,sys
from common import *
app,meta=prep();out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=False);checks={}
for name,cmd,marker in (
 ('weak-imports',[sys.executable,'-B',REPO/'tools/release/check_weak_imports.py',app],'0 weak import(s) missing'),
 ('scan-bundle',[sys.executable,'-B',REPO/'tools/release/scan_bundle.py','--min-macos','15.0',app],'scan_bundle: clean'),
 ('check-bundle',[sys.executable,'-B',REPO/'native/tools/check_app_bundle.py',app],'result: PASS'),
 ('minos',['otool','-l',app/'Contents/MacOS/Petari'],'minos 15.0')):
 r=subprocess.run([str(x) for x in cmd],capture_output=True,text=True);(out/(name+'.log')).write_text(r.stdout+r.stderr);checks[name]=r.returncode==0 and marker in r.stdout
resources=app/'Contents/Resources';seeds=list((resources/'pipeline-seeds').glob('*.db'));build=json.loads((FROZEN/'BUILD.json').read_text())
checks['seed-pack-count']=len(seeds)==build['bundled_seed_dbs']
checks['resource-copy-identical']=tree(resources)==tree(FROZEN/'Petari.app/Contents/Resources')
seedrows=[]
for seed in seeds:
 with sqlite3.connect('file:'+str(seed)+'?mode=ro',uri=True) as db:
  ok=db.execute('PRAGMA integrity_check').fetchone()==('ok',)
  seedrows.append({'path':seed.name,'sha256':sha(seed),'integrity':ok,'rows':db.execute('SELECT count(*) FROM pipeline_cache').fetchone()[0]})
checks['seed-integrity']=bool(seedrows) and all(r['integrity'] for r in seedrows)
write(out/'seed-pack.json',seedrows)
# Revalidate release lineage from git objects, not this shared branch HEAD.
r=subprocess.run(['git','diff','--name-only','a23609ca',build['source'],'--','src','include','libs'],cwd=REPO,capture_output=True,text=True)
checks['wii-source-unchanged']=r.returncode==0 and not r.stdout.strip()
(out/'wii-source-diff.log').write_text(r.stdout+r.stderr)
write(out/'RESULT.json',{'passed':all(checks.values()),'checks':checks,'nice':os.getpriority(os.PRIO_PROCESS,0),'build':build,'perf_exe_sha256':meta['perf_exe_sha256'],'macos':subprocess.check_output(['sw_vers','-productVersion'],text=True).strip()})
sys.exit(not all(checks.values()))
