#!/usr/bin/env python3
"""Owned standalone native-input build/tests, serialized with M1 measurements."""
import argparse,os,re,subprocess,sys
from common import *

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');a=p.parse_args();assert re.fullmatch('[A-Za-z0-9_-]+',a.label)
 out=ROOT/'build-reports'/a.label;out.mkdir(parents=True,exist_ok=False);build=ROOT/'build-input';results=[]
 env={**os.environ,'M1_COMPAT':'1','PYTHONDONTWRITEBYTECODE':'1'}
 commands=[['cmake','-S',REPO/'native/input','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_OSX_DEPLOYMENT_TARGET=15.0','-DBUILD_TESTING=ON','-DPETARI_SDL3_INCLUDE_DIR='+str(HOME/'MacioGalaxy2-team/scratch-port/gate53/deps/sdl-src-patched/include')],
           ['cmake','--build',build,'--parallel','6'],['ctest','--test-dir',build,'--output-on-failure']]
 with native_lock():
  disk();wait_quiet(out/'quiet.json')
  for i,cmd in enumerate(commands):
   print('START',i,flush=True)
   with (out/(str(i)+'.log')).open('w') as log:code=subprocess.run(list(map(str,cmd)),stdout=log,stderr=subprocess.STDOUT,env=env).returncode
   results.append({'command':list(map(str,cmd)),'exit':code});write(out/'RESULT.json',{'steps':results,'passed':len(results)==3 and all(r['exit']==0 for r in results),'source_head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip(),'build':str(build)})
   print('END',i,code,flush=True)
   if code:return code
 return 0

if __name__=='__main__':sys.exit(main())
