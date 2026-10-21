"""Live observed disk/growth budget for the isolated proof root.

Single-thread SIGALRM supervision; no background observer thread changes the
reviewed preexec contract. Checks happen before child spawn, while waiting, during
Python work and on exit. This is sampled supervision, not a filesystem quota.
"""
import os,shutil,signal,time,json,traceback
from pathlib import Path
import dispatch_preflight as p

class DiskBudgetExceeded(RuntimeError):pass
class ProofInterrupted(RuntimeError):pass

def allocated_bytes(root):
 """lstat allocation count; never follow symlinks, tolerate only vanished files."""
 total=0;todo=[Path(root)]
 while todo:
  directory=todo.pop()
  try:
   with os.scandir(directory) as items:
    for item in items:
     try:
      stat=item.stat(follow_symlinks=False)
      if item.is_symlink():continue
      total+=stat.st_blocks*512
      if item.is_dir(follow_symlinks=False):todo.append(Path(item.path))
     except FileNotFoundError:continue
  except FileNotFoundError:continue
 return total

class LiveBudgetJob(p.OwnedJob):
 def __init__(self,work,seconds,root,min_free,max_growth,interval,baseline=None):
  super().__init__(work,seconds);self.root=Path(root);self.min_free=min_free;self.max_growth=max_growth;self.interval=interval;self.baseline=baseline;self.samples=0;self.peak_growth=0;self.lowest_free=None;self.failure=None;self.max_scan_seconds=0;self.identities={};self.fixture_identity_path=None;self.operation_stage='idle'
 def __enter__(self):
  super().__enter__()
  try:
   if self.baseline is None:self.baseline=allocated_bytes(self.root)
   for signum in (signal.SIGTERM,signal.SIGINT,signal.SIGHUP):signal.signal(signum,self.interrupt)
   signal.signal(signal.SIGALRM,self.tick);self.check();signal.setitimer(signal.ITIMER_REAL,self.interval)
   return self
  except BaseException as error:
   self.__exit__(type(error),error,error.__traceback__);raise
 def remaining(self):
  # CPython3.15 _wait_kqueue catches OSError; signal exceptions must not derive
  # from it or a budget/interrupt raised in kqueue.control can be swallowed.
  try:return super().remaining()
  except TimeoutError as error:
   self.failure='global work deadline';raise DiskBudgetExceeded(self.failure) from error
 def interrupt(self,signum,frame):raise ProofInterrupted('owned proof signal '+str(signum))
 def journal(self,event):
  event={'monotonic':time.monotonic(),'controller_pid':os.getpid(),'stage':self.operation_stage,**event}
  path=self.work/'syscall-journal.jsonl'
  fd=os.open(path,os.O_WRONLY|os.O_CREAT|os.O_APPEND|os.O_NOFOLLOW,0o600)
  try:
   payload=(json.dumps(event,sort_keys=True)+'\n').encode();written=os.write(fd,payload)
   if written!=len(payload):raise RuntimeError('incomplete syscall journal write')
  finally:os.close(fd)
 def observe_fixture(self):
  # Future disposable fixture supplies its ACTUAL self-observed identity.
  # No UID is inferred from the parent or from start_new_session=True.
  if self.fixture_identity_path is None or not self.fixture_identity_path.exists():return
  data=json.loads(self.fixture_identity_path.read_text());required={'pid','pgid','sid','uid'}
  if set(data)!=required or not all(type(data[k]) is int for k in required):raise RuntimeError('invalid fixture identity')
  if data['pid'] not in self.groups:raise RuntimeError('identity is not registered child')
  old=self.identities.get(data['pid'])
  if old is not None and old!=data:raise RuntimeError('child identity changed')
  if old is None:
   # Preserve immutable observation and compare actual parent PGID/SID reads.
   observed={'pgid':self.operation('getpgid-observe',data['pid'],lambda:os.getpgid(data['pid'])),'sid':self.operation('getsid-observe',data['pid'],lambda:os.getsid(data['pid']))}
   if observed['pgid']!=data['pgid'] or observed['sid']!=data['sid']:raise RuntimeError('fixture identity mismatch')
   self.identities[data['pid']]=dict(data);self.journal({'event':'actual_child_identity','identity':dict(data)})
 def operation(self,name,pid,call):
  self.operation_stage=name;identity=self.identities.get(pid)
  self.journal({'event':'intent','operation':name,'pid':pid,'requested_pgid':pid,'actual_identity':identity,'identity_complete':identity is not None})
  try:value=call()
  except BaseException as error:
   self.journal({'event':'error','operation':name,'pid':pid,'errno':getattr(error,'errno',None),'error':repr(error),'traceback':traceback.format_exception(type(error),error,error.__traceback__)})
   raise
  self.journal({'event':'result','operation':name,'pid':pid,'value':value});return value
 def alive(self,pgid):
  try:self.operation('killpg0',pgid,lambda:os.killpg(pgid,0));return True
  except ProcessLookupError:return False
 def stop_group(self,proc):
  # Same signal mechanism/order as reviewed parent; EPERM remains fatal uncertainty.
  observation_error=None
  try:self.observe_fixture()
  except BaseException as error:
   observation_error=error;self.journal({'event':'identity_observation_error','error':repr(error),'traceback':traceback.format_exception(type(error),error,error.__traceback__)})
  for sig,grace in ((signal.SIGTERM,1.0),(signal.SIGKILL,3.0)):
   try:self.operation('killpg-'+signal.Signals(sig).name,proc.pid,lambda:os.killpg(proc.pid,sig))
   except ProcessLookupError:pass
   self.events.append({'pgid':proc.pid,'signal':int(sig)})
   end=time.monotonic()+grace
   while self.alive(proc.pid) and time.monotonic()<end:
    self.operation('poll',proc.pid,proc.poll);time.sleep(0.02)
   if not self.alive(proc.pid):break
  self.operation('wait-reap',proc.pid,lambda:proc.wait(timeout=1))
  if self.alive(proc.pid):raise RuntimeError('owned group remains')
  del self.groups[proc.pid];self.events.append({'pgid':proc.pid,'leader_reaped_and_group_absent':True})
  if observation_error is not None:raise RuntimeError('identity observation failed; no lifecycle qualification') from observation_error
 def check(self):
  self.remaining();self.observe_fixture();start=time.monotonic();free=shutil.disk_usage(self.root).free;size=allocated_bytes(self.root);self.remaining()
  self.max_scan_seconds=max(self.max_scan_seconds,time.monotonic()-start);growth=max(0,size-self.baseline);self.samples+=1;self.peak_growth=max(self.peak_growth,growth);self.lowest_free=free if self.lowest_free is None else min(self.lowest_free,free)
  if free<self.min_free:self.failure='free-space floor crossed';raise DiskBudgetExceeded(self.failure)
  if growth>self.max_growth:self.failure='owned allocated-byte growth crossed';raise DiskBudgetExceeded(self.failure)
 def tick(self,signum,frame):
  # One-shot rearm AFTER check: a slow directory scan never reenters itself.
  self.check();signal.setitimer(signal.ITIMER_REAL,self.interval)
 def run(self,*args,**kwargs):
  self.check();return super().run(*args,**kwargs)
 def __exit__(self,typ,value,tb):
  signal.setitimer(signal.ITIMER_REAL,0);late=None
  # Failed path: no recursive disk scan before child termination/reap.
  if typ is None and self.failure is None:
   try:self.check()
   except BaseException as error:late=error
  try:return_value=super().__exit__(typ or (type(late) if late else None),value or late,tb)
  finally:
   p.dump(self.work/'disk-budget.json',{'root':str(self.root),'initial_allocated_bytes':self.baseline,'minimum_free_bytes':self.min_free,'maximum_owned_growth_bytes':self.max_growth,'sampling_seconds':self.interval,'samples':self.samples,'peak_observed_growth_bytes':self.peak_growth,'lowest_observed_free_bytes':self.lowest_free,'max_scan_seconds':self.max_scan_seconds,'failure':self.failure,'cleanup_order':'all owned PGIDs stopped/reaped/absence checked BEFORE this metadata; no output deletion','semantics':'sampled observed enforcement; not an instantaneous filesystem quota'})
  if late and typ is None:raise late
  return return_value
