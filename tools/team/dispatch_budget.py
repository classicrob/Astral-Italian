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
  super().__init__(work,seconds);self.root=Path(root);self.min_free=min_free;self.max_growth=max_growth;self.interval=interval;self.baseline=baseline;self.samples=0;self.peak_growth=0;self.lowest_free=None;self.failure=None;self.max_scan_seconds=0;self.identities={};self.ownership={};self.fixture_identity_path=None;self.operation_stage='idle'
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
 def ownership_state(self,pid,status,**evidence):
  state={'status':status,**evidence};self.ownership[pid]=state
  self.journal({'event':'ownership_state','pid':pid,'ownership':dict(state)})
 def observe_fixture(self):
  # Exact disposable fixture only. No compiler UID/ownership claim is made.
  if self.fixture_identity_path is None:return
  if not self.fixture_identity_path.exists():
   for pid in self.groups:
    previous=self.ownership.get(pid,{}).get('status')
    if previous in ('CONTRADICTED','OBSERVATION_FAILED'):continue
    state='OBSERVATION_FAILED' if previous=='VERIFIED' else 'MISSING'
    if previous!=state:self.ownership_state(pid,state,reason='fixture identity file absent')
   return
  try:
   data=json.loads(self.fixture_identity_path.read_text());required={'pid','pgid','sid','uid'}
   if set(data)!=required or not all(type(data[k]) is int for k in required):raise RuntimeError('invalid fixture identity')
   if data['pid'] not in self.groups:
    for pid in self.groups:self.ownership_state(pid,'CONTRADICTED',reported_identity=data,reason='reported PID is not registered child')
    raise RuntimeError('identity is not registered child')
   pid=data['pid']
   if self.ownership.get(pid,{}).get('status') in ('CONTRADICTED','OBSERVATION_FAILED'):raise RuntimeError('prior ownership contradiction/observation failure is not recoverable in this fixture')
   old=self.identities.get(pid)
   if old is not None and old!=data:
    self.ownership_state(pid,'CONTRADICTED',reported_identity=data,prior_identity=old,reason='immutable child identity changed')
    raise RuntimeError('child identity changed')
   if old is None:
    observed={'pgid':self.operation('getpgid-observe',pid,lambda:os.getpgid(pid)),'sid':self.operation('getsid-observe',pid,lambda:os.getsid(pid))}
    # BOTH independent observations must identify this registered session leader.
    if observed['pgid']!=pid or observed['sid']!=pid or data['pgid']!=pid or data['sid']!=pid:
     self.ownership_state(pid,'CONTRADICTED',observed=observed,reported_identity=data,registered_leader=pid,reason='unexpected process group/session')
     raise RuntimeError('fixture is not the registered new-session group leader')
    self.identities[pid]=dict(data);self.ownership_state(pid,'VERIFIED',observed=observed,identity=dict(data))
    self.journal({'event':'actual_child_identity','identity':dict(data)})
  except BaseException as error:
   for pid in self.groups:
    if self.ownership.get(pid,{}).get('status')!='CONTRADICTED':self.ownership_state(pid,'OBSERVATION_FAILED',reason=repr(error))
   raise
 def require_signal_ownership(self,pid):
  # Missing/failed observations are not verified ownership. In this fixture mode
  # no signal/probe targets an unverified or contradicted requested process group.
  if self.fixture_identity_path is not None and self.ownership.get(pid,{}).get('status')!='VERIFIED':
   self.journal({'event':'signal_blocked_unverified_ownership','pid':pid,'ownership':self.ownership.get(pid,{'status':'MISSING'})})
   raise RuntimeError('fixture process-group ownership is not verified')
 def operation(self,name,pid,call):
  self.operation_stage=name;identity=self.identities.get(pid)
  if name.startswith('killpg'):self.require_signal_ownership(pid)
  self.journal({'event':'intent','operation':name,'pid':pid,'requested_pgid':pid,'actual_identity':identity,'identity_complete':self.ownership.get(pid,{}).get('status')=='VERIFIED','ownership':self.ownership.get(pid,{'status':'UNOBSERVED'})})
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
  self.require_signal_ownership(proc.pid)
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
  cleanup_error=None
  try:return_value=super().__exit__(typ or (type(late) if late else None),value or late,tb)
  except BaseException as error:
   cleanup_error={'error':repr(error),'errno':getattr(error,'errno',None),'traceback':traceback.format_exception(type(error),error,error.__traceback__)}
   raise
  finally:
   base_cleanup=None;base_read_error=None
   try:base_cleanup=json.loads((self.work/'owned-process-cleanup.json').read_text())
   except BaseException as error:base_read_error=repr(error)
   confirmed=(cleanup_error is None and not self.groups and base_cleanup is not None and base_cleanup.get('all_owned_groups_absent') is True and not base_cleanup.get('failures'))
   remaining=[{'registered_leader_pid':pid,'requested_pgid':pid,'ownership':self.ownership.get(pid,{'status':'UNOBSERVED'})} for pid in self.groups]
   cleanup={'status':'CONFIRMED' if confirmed else 'FAILED_OR_UNCERTAIN','all_owned_groups_absence_confirmed':confirmed,'remaining_groups':remaining,'error':cleanup_error,'base_report':base_cleanup,'base_report_read_error':base_read_error}
   order='owned-group cleanup confirmed before this metadata' if confirmed else 'owned-group cleanup attempted before metadata; success/absence NOT established'
   p.dump(self.work/'disk-budget.json',{'root':str(self.root),'initial_allocated_bytes':self.baseline,'minimum_free_bytes':self.min_free,'maximum_owned_growth_bytes':self.max_growth,'sampling_seconds':self.interval,'samples':self.samples,'peak_observed_growth_bytes':self.peak_growth,'lowest_observed_free_bytes':self.lowest_free,'max_scan_seconds':self.max_scan_seconds,'failure':self.failure,'cleanup':cleanup,'cleanup_order':order,'semantics':'sampled observed enforcement; not an instantaneous filesystem quota'})
  if late and typ is None:raise late
  return return_value
