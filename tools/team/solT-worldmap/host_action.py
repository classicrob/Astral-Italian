import argparse,csv,datetime,json,subprocess,sys,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('case');p.add_argument('action',choices=['query','activate','move','click','key','size']);p.add_argument('values',nargs='*');a=p.parse_args();case=Path(a.case).resolve();assert case.parent==Path.home()/'MacioGalaxy2-gamedata/solT-worldmap51';command=json.loads((case/'command.json').read_text());exe=command['build_receipt']['binary_path'];processes=subprocess.check_output(['ps','-axo','pid=,args='],text=True).splitlines();ids=[int(line.strip().split(None,1)[0]) for line in processes if len(line.strip().split(None,1))==2 and line.strip().split(None,1)[1].startswith(exe+' ')];assert len(ids)==1,ids;pid=ids[0];helper=Path.home()/'toolchains/sol-t-validation/worldmap51/host_input.py';cmd=[sys.executable,str(helper),a.action];cmd+=([str(pid)]+a.values if a.action in ['query','activate','size'] else a.values)
def tailframe():
 f=case/'route.csv'
 if not f.exists():return None
 with f.open('rb') as stream:
  header=stream.readline().decode().strip().split(',');stream.seek(0,2);end=stream.tell();stream.seek(max(0,end-8192));lines=stream.read().decode(errors='replace').splitlines()
 for line in reversed(lines[:-1]):
  row=next(csv.reader([line]));
  if len(row)==len(header):return dict(zip(header,row))
 return None
before=tailframe();start=time.time();
if a.action in ['move','click','key','size']:subprocess.run([sys.executable,str(helper),'activate',str(pid)],check=True,capture_output=True,text=True)
result=subprocess.run(cmd,check=True,capture_output=True,text=True);after=tailframe();record=dict(utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),epoch=start,elapsed_from_app_start=round(start-command['start_epoch'],3),app_pid=pid,action=a.action,arguments=a.values,result=json.loads(result.stdout),route_before=before,route_after=after,frame_limit='route brackets host event; no claim exact game-consumption frame')
with (case/'host-events.jsonl').open('a') as stream:stream.write(json.dumps(record)+'\n')
print(json.dumps({k:v for k,v in record.items() if k not in ['route_before','route_after']}))
