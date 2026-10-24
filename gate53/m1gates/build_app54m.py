#!/usr/bin/env python3
"""Incremental shared app build, only at SOL B's agreed source handoff."""
import argparse,os,re,subprocess,sys
from common import *

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');a=p.parse_args();assert re.fullmatch('[A-Za-z0-9_-]+',a.label)
 out=ROOT/'build-reports'/a.label;out.mkdir(parents=True,exist_ok=False)
 with native_lock():
  disk();wait_quiet(out/'quiet.json')
  source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
  assert not subprocess.check_output(['git','status','--porcelain','--untracked-files=no'],cwd=REPO,text=True).strip(),'commit shared source before build'
  build=REPO/'build/macos-arm64-release-portable';cmd=['cmake','--build',str(build),'--target','petari','--parallel','6']
  print('START app build',source,flush=True)
  with (out/'build.log').open('w') as log:code=subprocess.run(cmd,stdout=log,stderr=subprocess.STDOUT,env={**os.environ,'M1_COMPAT':'1','PYTHONDONTWRITEBYTECODE':'1'}).returncode
  write(out/'RESULT.json',{'command':cmd,'exit':code,'source':source,'build_kind':'dev-m1','passed':False})
  if code:return code
  assert subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()==source,'source commit changed during build'
  assert not subprocess.check_output(['git','status','--porcelain','--untracked-files=no'],cwd=REPO,text=True).strip(),'source changed during build'
  original=build/'native/app/Petari.app';app,meta=prep_dev(original,sha(original/'Contents/MacOS/Petari'),source)
  result={'passed':True,'command':cmd,'source':source,'build_kind':'dev-m1','app':str(app),'build':meta};write(out/'RESULT.json',result);write(ROOT/'apps/active-input-dev.json',result)
  print('END app build',app,flush=True)
 return 0

if __name__=='__main__':sys.exit(main())
