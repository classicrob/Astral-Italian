"""Replay the owned typed case postboot HOST events, with actual mode/node guards. No game-state writes."""
import json,re,subprocess,sys,time
from pathlib import Path
case=Path(sys.argv[1]);ref=Path(sys.argv[2]);base=Path.home()/'toolchains/sol-t-validation/worldmap51';start=json.loads((case/'command.json').read_text())['start_epoch']
events=[json.loads(x) for x in (ref/'host-events.jsonl').read_text().splitlines()]
events=[x for x in events if x['elapsed_from_app_start']>=250 and x['action'] in ('move','size','click')]
assert len([x for x in events if x['action']=='click'])==3
click=0
def state():
 lines=(case/'app.log').read_text(errors='replace').splitlines()
 owners=[x for x in lines if '[wm-owner]' in x]
 if not owners:raise SystemExit('no actual map owner observation')
 return owners[-1],lines
for index,event in enumerate(events):
 while time.time()-start<event['elapsed_from_app_start']:time.sleep(.1)
 if event['action']=='click':
  owner,lines=state();expected=[(1,0),(0,0),(0,1)][click]
  for key,value in zip(('galaxy-map','current'),expected):
   if not re.search(r'(?:^| )'+key+'='+str(value)+r'(?: |$)',owner):raise SystemExit('actual owner readiness differs: '+owner)
  if click==0:
   assert any('[input-script] done' in line for line in lines),'common scripted prefix not complete; refuse matched click'
   n=[x for x in lines if '[wm-node]' in x and 'domain=1 node=0 ' in x][-1];m=re.search(r'clip=\(([^,]+),([^,]+),([^\)]+)\)',n);x,y,w=map(float,m.groups());assert abs(x/w)<.01 and abs(y/w)<.01,'Grand projection moved'
  click+=1
 cmd=[sys.executable,str(base/'host_action.py'),str(case),event['action'],*event['arguments']]
 subprocess.run(cmd,check=True)
 if event['action']=='click' and (index+1==len(events) or events[index+1]['elapsed_from_app_start']-(time.time()-start)>5):
  time.sleep(2)
  subprocess.run([sys.executable,str(base/'capture_wait.py'),str(case),'worker-click'+str(click)],check=False)
  subprocess.run([sys.executable,str(base/'host_action.py'),str(case),'activate'],check=False)
print(json.dumps({'replayed_postboot_host_events':len(events),'actual_state_guards':'Grand0/local0/local1; no stage or actor writes','reference':str(ref)}))
