#!/usr/bin/env python3
"""Separate frozen54 Cappy diagnostic: script/game frames, raw KPAD and shake alarm traces."""
import argparse,importlib.util,json,os,re,sys
from common import *

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');p.add_argument('--delays',default='2,4,6');a=p.parse_args()
 assert re.fullmatch('[A-Za-z0-9_-]+',a.label)
 source=REPO/'native/tests/cappy_spin_switch_tests.py'
 spec=importlib.util.spec_from_file_location('cappy_reference',source);ref=importlib.util.module_from_spec(spec);spec.loader.exec_module(ref)
 app,meta=prep();base=ROOT/'runs'/a.label;base.mkdir(parents=True,exist_ok=False);records=[]
 for delay in map(int,a.delays.split(',')):
  assert delay in (2,4,6,10,20)
  name=f'air_throw_then_spin_{delay}';keys=base/(name+'.keys.txt');keys.write_text(ref.script(True,True,delay));out=base/name
  env={'PETARI_MODS':'OdysseyMovement,Cappy','PETARI_CAPPY_TRACE':'1','PETARI_SHAKE_TRACE':'1','PETARI_ODYSSEY_TRACE_SWING':'1',
       'PETARI_SMOKE':'','PETARI_SMOKE_BACKGROUND':'1','PETARI_FAST_BOOT':'1','PETARI_SMOKE_FRAMES':'100000','PETARI_STAGE_GATE_MS':'0',
       'PETARI_INPUT_SCRIPT_LOG':str(out/'input.csv'),'PETARI_TAPE_RECORD':str(out)}
  print('START',name,flush=True)
  proc=run(app,out,stage='RedBlueExGalaxy',script=keys,extra=env,seconds=400)
  log=(out/'app.log').read_text(errors='replace');toggles=[(m.group(1),int(m.group(2)),int(m.group(3))) for m in ref.LINE.finditer(log)]
  record={'name':name,'process':proc,'toggles':toggles,'diagnostic_only':True,'gate_arm':False,'nice':os.getpriority(os.PRIO_PROCESS,0),
          'frozen_build':{k:v for k,v in meta.items() if k!='resource_tree'},'reference_runner_sha256':sha(source),
          'difference_from_reference':'Added frame/route/input/KPAD/shake traces and graceful script completion; diagnostic, never replaces acceptance.'}
  write(out/'DIAGNOSTIC.json',record);records.append(record);trim(out);print('END',name,toggles,flush=True)
 write(base/'DIAGNOSTIC.json',{'runs':records,'functional_acceptance_claimed':False});return 0

if __name__=='__main__':sys.exit(main())
