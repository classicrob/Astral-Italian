#!/usr/bin/env python3
import argparse,json,subprocess,sys
from common import HERE,ROOT,REPO,prep,run,write
p=argparse.ArgumentParser();p.add_argument('label');p.add_argument('stage');p.add_argument('--warm');p.add_argument('--extra',default='{}');p.add_argument('--script');p.add_argument('--seconds',type=int,default=180);a=p.parse_args()
app,meta=prep();out=ROOT/'runs'/a.label
r=run(app,out,stage=a.stage,script=a.script or REPO/'native/tools/perf_sweep_move.script',warm=ROOT/'runs'/a.warm if a.warm else None,cold=not a.warm,extra=json.loads(a.extra),seconds=a.seconds)
s=subprocess.run([sys.executable,'-B',str(HERE/'gate533.py'),str(out)]+(['--warm'] if a.warm else []),capture_output=True,text=True)
try:g=json.loads(s.stdout)
except ValueError:g={'result':'FAIL','error':s.stderr or s.stdout}
g['process']=r;g['frozen_build']={k:v for k,v in meta.items() if k!='resource_tree'};g['passed']=g['result']=='PASS' and all(r[k] for k in ('completed','no_faults','binary_unchanged','seed_unchanged')) and g.get('gameplay_frames',0)>0 and g.get('menu_frames',0)>0
write(out/'RESULT.json',g);print(json.dumps(g));sys.exit(not g['passed'])
