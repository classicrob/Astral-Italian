#!/usr/bin/env python3
"""Server's 23 Cappy cases, isolated behind the M1 native lock; trim each completed case."""
import importlib.util,json,os,sys
from common import ROOT,REPO,native_lock,prep,disk,trim,sha,write
p=REPO/'native/tests/cappy_spin_switch_tests.py'
spec=importlib.util.spec_from_file_location('cappy_reference',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
original=m.run
def run(*args,**kwargs):
 disk()
 result=original(*args,**kwargs)
 # Caller inspects app.log for extra cases. Raw caches/user can go immediately.
 out=args[2]/args[3]
 trim(out)
 import shutil
 shutil.rmtree(out/'user',ignore_errors=True)
 return result
m.run=run
app,meta=prep();out=ROOT/'runs'/(sys.argv[1] if len(sys.argv)>1 else 'cappy')
assert not out.exists(),out
sys.argv=[str(p),'--app',str(app/'Contents/MacOS/Petari'),'--disc',str(m.Path.home()/'MacioGalaxy2-gamedata/SB4E01-extracted'),'--out',str(out)]
with native_lock():
 disk();write(out/'launch-m1.json',{'app_sha256':meta['perf_exe_sha256'],'frozen_exe_sha256':meta['source_exe_sha256'],'nice':os.getpriority(os.PRIO_PROCESS,0),'reference_runner_sha256':sha(p),'input_driver':'PETARI_INPUT_SCRIPT'})
 m.main()
