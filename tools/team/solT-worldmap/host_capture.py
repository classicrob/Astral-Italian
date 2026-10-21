import contextlib,hashlib,io,json,runpy,struct,subprocess,sys,time
from pathlib import Path
case=Path(sys.argv[1]).resolve();label=sys.argv[2];assert label.replace('-','').isalnum();assert case.parent==Path.home()/'MacioGalaxy2-gamedata/solT-worldmap51';c=json.loads((case/'command.json').read_text());exe=c['build_receipt']['binary_path'];rows=subprocess.check_output(['ps','-axo','pid=,args='],text=True).splitlines();pids=[int(x.strip().split(None,1)[0]) for x in rows if len(x.strip().split(None,1))==2 and x.strip().split(None,1)[1].startswith(exe+' ')];assert len(pids)==1;p=pids[0];sys.argv=['host_input.py','query',str(p)];buf=io.StringIO()
with contextlib.redirect_stdout(buf):runpy.run_path(str(Path.home()/'toolchains/sol-t-validation/worldmap51/host_input.py'),run_name='__main__')
meta=json.loads(buf.getvalue());assert meta['screen_capture_access'];window=meta['windows'][0];dest=case/('host-'+label+'.png');start=time.time();cmd=['/usr/sbin/screencapture','-x','-l',str(window['window_id']),str(dest)];result=subprocess.run(cmd,capture_output=True,text=True);receipt=dict(metadata=meta,capture_epoch=start,elapsed_from_app_start=start-c['start_epoch'],command=cmd,returncode=result.returncode,error=result.stderr.strip(),capture=str(dest),frame_limit='host capture timestamp; no exact game-frame claim')
if dest.exists():
 data=dest.read_bytes();receipt['image_dimensions']=struct.unpack('>II',data[16:24]);receipt['sha256']=hashlib.sha256(data).hexdigest()
(case/('host-'+label+'.json')).write_text(json.dumps(receipt,indent=2)+'\n')
