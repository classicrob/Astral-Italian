"""Ordinary host input only; run beside the owned bounded app. Never opens a file or changes stage state."""
import csv,json,subprocess,sys,time
from pathlib import Path
case=Path(sys.argv[1]).resolve();c=json.loads((case/'command.json').read_text());helper=Path.home()/'toolchains/sol-t-validation/worldmap51/host_action.py';deadline=time.monotonic()+150
def stage():
 p=case/'route.csv'
 if not p.exists():return ''
 with p.open() as f:rows=list(csv.DictReader(f))
 return next((r.get('stage','') for r in reversed(rows) if r.get('valid')=='1'),'')
def act(*args):
 if stage() not in ('','FileSelect'):return False
 subprocess.run([sys.executable,str(helper),str(case),*map(str,args)],check=True);return True
def until(seconds):
 while time.time()-c['start_epoch']<seconds:
  if time.monotonic()>deadline:raise SystemExit('host boot deadline')
  time.sleep(.25)
act('move',10,10)
for t in (18,25):until(t);act('key',36,500)
until(32)
for attempt in range(3):
 if stage() not in ('','FileSelect'):break
 print('ordinary FileSelect1/Start attempt',attempt+1,flush=True)
 act('move',347,438);time.sleep(2);act('key',49,160)
 time.sleep(4);act('move',869,682);time.sleep(2);act('key',49,160)
 for _ in range(12):
  if stage() not in ('','FileSelect'):break
  time.sleep(1)
 if stage() not in ('','FileSelect'):break
print(json.dumps({'observed_stage':stage(),'no_state_or_file_open_api':True}),flush=True)
if stage() not in ('MarioFaceShipGalaxy','WorldMap01Galaxy'):raise SystemExit('ordinary host boot did not reach saved ship')
