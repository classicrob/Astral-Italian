#!/usr/bin/env python3
"""Server's 23 Cappy cases, isolated behind the M1 native lock; trim each completed case."""
import argparse,importlib.util,json,os,re,sys
from common import ROOT,REPO,native_lock,prep,disk,trim,sha,write,wait_quiet,Path,ID
p=REPO/'native/tests/cappy_spin_switch_tests.py'
spec=importlib.util.spec_from_file_location('cappy_reference',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
original=m.run
def run(*args,**kwargs):
 with native_lock():
  disk()
  wait_quiet(args[2]/args[3]/'quiet.json')
  result=original(*args,**kwargs)
  # Caller inspects app.log for extra cases. Raw caches/user can go immediately.
  out=args[2]/args[3]
  trim(out)
  import shutil
  shutil.rmtree(out/'user',ignore_errors=True)
  return result
m.run=run
parser=argparse.ArgumentParser();parser.add_argument('label',nargs='?',default='cappy');parser.add_argument('--only');parser.add_argument('--app',type=Path);parser.add_argument('--sha');parser.add_argument('--source');parser.add_argument('--build-kind',choices=('dev-m1','server-candidate'));args=parser.parse_args()
assert re.fullmatch('[A-Za-z0-9_-]+',args.label),'safe unique label required'
if args.app:
 import plistlib
 app=args.app.resolve();assert app.is_relative_to(ROOT/'apps') and args.sha and args.source and args.build_kind
 assert sha(app/'Contents/MacOS/Petari')==args.sha
 with (app/'Contents/Info.plist').open('rb') as f:assert plistlib.load(f)['CFBundleIdentifier']==ID
 meta={'perf_exe_sha256':args.sha,'source_exe_sha256':None,'source':args.source,'build_kind':args.build_kind}
else:
 app,meta=prep();meta['build_kind']='frozen54'
out=ROOT/'runs'/args.label
assert not out.exists(),out
sys.argv=[str(p),'--app',str(app/'Contents/MacOS/Petari'),'--disc',str(m.Path.home()/'MacioGalaxy2-gamedata/SB4E01-extracted'),'--out',str(out)]+(['--only',args.only] if args.only else [])
disk();write(out/'launch-m1.json',{'app_sha256':meta['perf_exe_sha256'],'frozen_exe_sha256':meta['source_exe_sha256'],'source':meta.get('source'),'build_kind':meta['build_kind'],'nice':os.getpriority(os.PRIO_PROCESS,0),'selected_cases':args.only,'reference_runner_sha256':sha(p),'input_driver':'PETARI_INPUT_SCRIPT'})
m.main()
