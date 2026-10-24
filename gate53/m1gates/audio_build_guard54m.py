#!/usr/bin/env python3
"""Guard the not-yet-run audio compiler in an already-loaded campaign.

The compiler's output pipe cannot open until this reader owns heavy->native.
New campaigns lock the compiler directly; this only repairs the running driver.
"""
import argparse,json,os,re,stat,subprocess,time
from common import ROOT,native_lock,write,sha

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('label');p.add_argument('--parent-pid',type=int,required=True);a=p.parse_args()
 assert re.fullmatch('[A-Za-z0-9_-]+',a.label)
 base=(ROOT/'campaigns'/a.label).resolve();assert base.is_relative_to(ROOT/'campaigns') and base.is_dir()
 pipe=base/'audio-probe-build.out';assert not pipe.exists()
 def alive():
  r=subprocess.run(['ps','-p',str(a.parent_pid),'-o','command='],capture_output=True,text=True)
  return r.returncode==0 and '/gate53/m1gates/campaign54m.py '+a.label+' ' in r.stdout
 def ledger():
  try:return json.loads((base/'MANIFEST.json').read_text())['steps']
  except json.JSONDecodeError:return []
 assert alive() and not any(s['name']=='audio-probe-build' for s in ledger())
 os.mkfifo(pipe,0o600);owned=pipe.stat();output=bytearray();record={'parent_pid':a.parent_pid,'label':a.label,'compiler_output_pipe':str(pipe),'guard':'wait for final cadence, then lock heavy->native before opening compiler output','completed':False}
 write(base/'AUDIO-BUILD-GUARD.json',record)
 try:
  while alive() and not any(s['name']=='cadence-immediate-r2' for s in ledger()):time.sleep(1)
  if not alive():record['reason']='parent exited before audio build';return 1
  with native_lock():
   record['locks_acquired_utc']=time.strftime('%Y-%m-%dT%H:%M:%SZ',time.gmtime());write(base/'AUDIO-BUILD-GUARD.json',record)
   fd=os.open(pipe,os.O_RDONLY|os.O_NONBLOCK)
   try:
    while True:
     try:
      chunk=os.read(fd,65536)
      if chunk:output.extend(chunk)
     except BlockingIOError:pass
     step=next((s for s in ledger() if s['name']=='audio-probe-build'),None)
     if step is not None:
      while True:
       try:chunk=os.read(fd,65536)
       except BlockingIOError:break
       if not chunk:break
       output.extend(chunk)
      record.update(completed=True,compiler_exit=step['exit'],manifest_sha256_at_completion=sha(base/'MANIFEST.json'));break
     if not alive():record['reason']='parent exited before build ledger completion';break
     time.sleep(.1)
   finally:os.close(fd)
  return 0 if record['completed'] else 1
 finally:
  current=pipe.stat()
  assert stat.S_ISFIFO(current.st_mode) and (current.st_dev,current.st_ino)==(owned.st_dev,owned.st_ino)
  pipe.unlink();pipe.write_bytes(output)
  write(base/'AUDIO-BUILD-GUARD.json',record)

if __name__=='__main__':raise SystemExit(main())
