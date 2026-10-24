#!/usr/bin/env python3
"""Compare Cappy Spin onset on a dev perf app, normal and owned CPU load/nice19."""
import argparse,importlib.util,json,re,sys
from common import *

def observation(log):
 text=log.split('[input-script] start',1)[1];frame=-1;pending=[];onsets=[];states=[]
 for line in text.splitlines():
  match=re.match(r'\[cappy\] f(\d+) ',line)
  if match:
   frame+=1;states.append(int(match[1]));onsets.extend([frame]*len(pending));pending=[]
  elif '[cappy] mario spin (air) starts' in line:pending.append(line)
 toggles=[m.group(1) for m in re.finditer(r'\[spin-switch\] \S+ source=(cappy|spin) before=\d after=\d',text)]
 return {'spin_onset_frames':onsets,'toggle_sources':toggles,'cappy_frames_contiguous':all(b==a+1 for a,b in zip(states,states[1:])),'script_completed':'[input-script] done after 170 frames' in text}

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');p.add_argument('--app',type=Path,required=True);p.add_argument('--sha',required=True);p.add_argument('--source',required=True);a=p.parse_args()
 assert re.fullmatch('[A-Za-z0-9_-]+',a.label);app=a.app.resolve();assert app.is_relative_to(ROOT/'apps') and sha(app/'Contents/MacOS/Petari')==a.sha
 spec=importlib.util.spec_from_file_location('cappy_reference',REPO/'native/tests/cappy_spin_switch_tests.py');ref=importlib.util.module_from_spec(spec);spec.loader.exec_module(ref)
 base=ROOT/'runs'/a.label;base.mkdir(parents=True,exist_ok=False);results=[]
 for delay,target in ((2,42),(4,45),(6,46)):
  keys=base/f'air-{delay}.txt';keys.write_text(ref.script(True,True,delay));arms=[]
  for arm in ('normal','loaded'):
   out=base/f'air-{delay}-{arm}';env={'PETARI_MODS':'OdysseyMovement,Cappy','PETARI_CAPPY_TRACE':'1','PETARI_SHAKE_TRACE':'1','PETARI_SMOKE':'','PETARI_SMOKE_BACKGROUND':'1','PETARI_FAST_BOOT':'1','PETARI_SMOKE_FRAMES':'100000','PETARI_STAGE_GATE_MS':'0','PETARI_INPUT_SCRIPT_LOG':str(out/'input.csv')}
   print('START',delay,arm,flush=True);proc=run(app,out,stage='RedBlueExGalaxy',script=keys,extra=env,seconds=400,nice=19 if arm=='loaded' else 0,load_workers=4 if arm=='loaded' else 0)
   obs=observation((out/'app.log').read_text(errors='replace'));obs.update(arm=arm,process=proc);arms.append(obs);write(out/'OBSERVATION.json',obs);trim(out);print('END',delay,arm,obs['spin_onset_frames'],obs['toggle_sources'],flush=True)
  checks={'same_onset_normal_loaded':arms[0]['spin_onset_frames']==arms[1]['spin_onset_frames'],'server_onset_preserved':all(r['spin_onset_frames']==[target] for r in arms),'spin_only_toggle':all(r['toggle_sources']==['spin'] for r in arms),'frames_and_completion':all(r['cappy_frames_contiguous'] and r['script_completed'] and r['process']['completed'] and r['process']['no_faults'] and r['process']['binary_unchanged'] for r in arms)}
  results.append({'delay':delay,'expected_server_onset':target,'checks':checks,'passed':all(checks.values()),'runs':arms})
  write(base/'RESULT.json',{'build_kind':'dev-m1','source':a.source,'binary_sha256':a.sha,'passed':len(results)==3 and all(r['passed'] for r in results),'results':results,'frozen54_acceptance':False})
 return 0 if all(r['passed'] for r in results) else 1

if __name__=='__main__':sys.exit(main())
