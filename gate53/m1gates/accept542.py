#!/usr/bin/env python3
"""Exact 9ec3d907/072 acceptance commands: per-arm heavy->native, no outer lock.

One invocation works on M1/M4 with explicit tier, owned IDs and lock paths.
Nonzero threshold receipts are retained; runner/identity failures stop the plan.
Warm pairs always run prime immediately followed by second launch with no clear.
"""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

HERE=Path(__file__).resolve().parent
ROOT=Path.home()/'MacioGalaxy2-gamedata/port53/m1g'

def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--candidate-dir',type=Path,required=True)
 p.add_argument('--baseline-dir',type=Path,required=True)
 p.add_argument('--tier',choices=('m1','m4'),required=True)
 p.add_argument('--out',type=Path,required=True)
 p.add_argument('--disc',type=Path,required=True)
 p.add_argument('--heavy-lock',type=Path,required=True)
 p.add_argument('--native-lock',type=Path,required=True)
 p.add_argument('--baseline-id',required=True)
 p.add_argument('--candidate-id',required=True)
 p.add_argument('--spots',action='store_true')
 p.add_argument('--plan-only',action='store_true',help='Write exact command plan without preparing copies or launching anything')
 args=p.parse_args();out=args.out.expanduser().resolve()
 assert out.is_relative_to(ROOT.resolve()) and out!=ROOT.resolve() and not out.exists()
 for own in (args.baseline_id,args.candidate_id):assert re.fullmatch(r'dev\.petari\.Petari\.perf[A-Za-z0-9_-]+',own)
 out.mkdir(parents=True)
 apps={};common=[sys.executable,'-B']
 for build,source,own in (('541',args.baseline_dir,args.baseline_id),('542',args.candidate_dir,args.candidate_id)):
  target=out/'apps'/build
  cmd=common+[str(HERE/'prepare_title542.py'),'--candidate-dir',str(source),'--out',str(target),'--bundle-id',own]
  if not args.plan_only: subprocess.run(cmd,check=True)
  apps[build]=(target/'Petari.app',target/'PREP.json',own)
 plan=[]
 def add(kind,build,name,extra,warm_from=None):
  app,prep,own=apps[build];folder=ROOT/'runs'/f'{out.name}-{build}-{name}'
  if kind=='direct':
   cmd=common+[str(HERE/'cold542m.py'),'--app',str(app),'--prep',str(prep),'--label',f'{out.name}-{build}-{name}',
     '--arm',str(extra),'--tier',args.tier,'--bundle-id',own,'--disc',str(args.disc),
     '--native-lock',str(args.native_lock),'--heavy-lock',str(args.heavy_lock)]
   stages=('YosshiHomeGalaxy','MokumokuValleyGalaxy','MarioFaceShipGalaxy','IslandFleetGalaxy','VsKoopaLv1Galaxy')
   stage,rep=(stages[extra%3],extra//3+1) if extra<9 else (stages[extra-6],1)
   folder=ROOT/'runs'/f'{out.name}-{build}-{name}-{stage}-r{rep}'
   cmd=['/usr/bin/lockf','-k',str(args.heavy_lock),*cmd]
  else:
   cmd=common+[str(HERE/'title542m.py'),'--app',str(app),'--prep',str(prep),'--out',str(folder),
       '--label',f'{out.name}-{build}-{name}','--tier',args.tier,'--bundle-id',own,'--disc',str(args.disc),
       '--heavy-lock',str(args.heavy_lock),'--native-lock',str(args.native_lock),'--keep-cache']
   if warm_from:cmd+=['--warm-from',str(warm_from)]
   if build=='542':cmd+=['--require-frontend-prepresent']
  if build=='541':cmd+=['--baseline']
  else:cmd+=['--require-boot-responsiveness']
  plan.append(dict(kind=kind,build=build,name=name,folder=str(folder),command=cmd))
  return folder
 for i in range(9):
  add('direct','541',f'cold-{i}',i);add('direct','542',f'cold-{i}',i)
 for rep in (1,2,3):
  add('title','541',f'title-r{rep}',None);add('title','542',f'title-r{rep}',None)
 if args.spots:
  for i in (9,10):add('direct','542',f'spot-{i}',i)
 for build in ('541','542'):
  prime=add('title',build,'warm-prime-r1',None)
  add('title',build,'warm-second-r1',None,warm_from=prime)
 (out/'COMMANDS.json').write_text(json.dumps(dict(plan=plan,tier=args.tier,comparison_window_seconds=300,
  baseline='07223918',candidate='9ec3d907',warm='replacement prime / immediate second launch, exact same private cache + Metal namespace',
  direct_scoring='gate533 unchanged + gate542m explicit tier / target deadline counts / frontend-ready / OS probe; actual receipts retain all',
  no_outer_lock=True),indent=2)+'\n')
 if args.plan_only: return 0
 results=[]
 for arm in plan:
  (out/'DRIVER.json').write_text(json.dumps(dict(current=arm,results=results,started=time.time()),indent=2)+'\n')
  with (out/f'{arm["build"]}-{arm["name"]}.log').open('wb') as log:
   status=subprocess.run(arm['command'],stdout=log,stderr=subprocess.STDOUT).returncode
  folder=Path(arm['folder']);path=folder/'RESULT.json';process=folder/'process.json'
  if not path.exists() or not process.exists():
   (out/'STOPPED.json').write_text(json.dumps(dict(arm=arm,exit=status,reason='missing actual receipt; no replacement run'),indent=2)+'\n');return 2
  result=json.loads(path.read_text());proc=json.loads(process.read_text())
  results.append(dict(arm=arm,result=str(path),passed=result['passed'],exit=status,process=proc))
  if not proc.get('normal_exit',proc.get('completed',False)) and not arm['name'].startswith('spot-'):
   (out/'STOPPED.json').write_text(json.dumps(dict(arm=arm,exit=status,reason='abnormal/timed out arm; preserve and report'),indent=2)+'\n');return 2
 (out/'DRIVER.json').write_text(json.dumps(dict(complete=True,results=results,finished=time.time(),acceptance_claimed=False),indent=2)+'\n')
 print(out,flush=True)
 return 0
if __name__=='__main__':raise SystemExit(main())
