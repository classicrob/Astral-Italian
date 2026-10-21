import json,sys,tempfile,types,unittest,contextlib,os
from pathlib import Path
from unittest.mock import patch
import dispatch_budget as b
@contextlib.contextmanager
def retained_fixture():
 base=Path.home()/'MacioGalaxy2-team/scratch-port/solP/dispatch-budget-source-controls'
 base.mkdir(parents=True,exist_ok=True)
 root=Path(tempfile.mkdtemp(prefix='fixture-',dir=base));root.chmod(0o700)
 try:yield str(root)
 except BaseException:
  print('FAILED FIXTURE RETAINED: '+str(root),file=sys.stderr);raise
 else:print('SUCCESS FIXTURE RETAINED: '+str(root),file=sys.stderr)

def child_source(identity,target,grow):
 return ("import os,json,time;from pathlib import Path;"
  "v={'pid':os.getpid(),'pgid':os.getpgrp(),'sid':os.getsid(0),'uid':os.getuid()};"
  "tmp=Path(%r);fd=os.open(tmp,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600);"
  "os.write(fd,json.dumps(v).encode());os.close(fd);os.rename(tmp,%r);"
  "Path(%r).write_bytes(b'x'*%d);time.sleep(10)"%(str(identity)+'.tmp',str(identity),str(target),grow))

class LiveControls(unittest.TestCase):
 def invoke(self,root,work,**kwargs):return b.LiveBudgetJob(work,3,root,kwargs.get('min_free',0),kwargs.get('max_growth',1024**2),0.02)
 def check_reaped(self,work):
  report=json.loads((work/'owned-process-cleanup.json').read_text());self.assertTrue(report['all_owned_groups_absent']);self.assertFalse(report['remaining_pgids']);self.assertFalse(report['failures'])
  if report['events']:
   journal=[json.loads(line) for line in (work/'syscall-journal.jsonl').read_text().splitlines()]
   self.assertTrue(any(row['event']=='actual_child_identity' for row in journal))
  for row in report['events']:
   try:b.os.killpg(row['pgid'],0)
   except ProcessLookupError:pass
   else:self.fail('owned group remains')
 def test_growth_crossing_stops_real_python_child(self):
  with retained_fixture() as d:
   root=Path(d);work=root/'work';work.mkdir();target=root/'grow'
   with self.assertRaises(b.DiskBudgetExceeded):
    with self.invoke(root,work,max_growth=4096) as job:
     job.fixture_identity_path=root/'child-identity.json';job.run([sys.executable,'-c',child_source(job.fixture_identity_path,target,65536)])
   self.check_reaped(work);self.assertEqual(json.loads((work/'disk-budget.json').read_text())['failure'],'owned allocated-byte growth crossed')
 def test_synthetic_floor_crossing_stops_real_python_child(self):
  with retained_fixture() as d:
   root=Path(d);work=root/'work';work.mkdir();ready=root/'ready'
   def disk(path):return types.SimpleNamespace(free=0 if ready.exists() else 1000000)
   with patch.object(b.shutil,'disk_usage',side_effect=disk):
    with self.assertRaises(b.DiskBudgetExceeded):
     with self.invoke(root,work,min_free=1) as job:
      job.fixture_identity_path=root/'child-identity.json';job.run([sys.executable,'-c',child_source(job.fixture_identity_path,ready,0)])
   self.check_reaped(work);self.assertEqual(json.loads((work/'disk-budget.json').read_text())['failure'],'free-space floor crossed')
 def test_python_work_crossing(self):
  with retained_fixture() as d:
   root=Path(d);work=root/'work';work.mkdir()
   with self.assertRaises(b.DiskBudgetExceeded):
    with self.invoke(root,work,max_growth=4096):
     (root/'package-output').write_bytes(b'x'*65536);b.time.sleep(0.2)
   self.check_reaped(work)
 def test_packaging_continues_original_baseline(self):
  with retained_fixture() as d:
   root=Path(d);work=root/'package';work.mkdir();baseline=b.allocated_bytes(root);(root/'old-build').write_bytes(b'x'*65536)
   with self.assertRaises(b.DiskBudgetExceeded):
    with b.LiveBudgetJob(work,3,root,0,4096,0.02,baseline=baseline):self.fail('existing build growth must not be rebased away')
   self.check_reaped(work)
class OwnershipSourceModels(unittest.TestCase):
 def setup_model(self,root):
  work=root/'work';work.mkdir();job=b.LiveBudgetJob(work,3,root,0,1024**2,0.02)
  job.fixture_identity_path=root/'identity.json';job.groups[707]=types.SimpleNamespace(pid=707)
  return job,work
 def test_matching_report_and_kernel_but_wrong_session_leader_rejects(self):
  with retained_fixture() as d:
   job,work=self.setup_model(Path(d));job.fixture_identity_path.write_text(json.dumps({'pid':707,'pgid':808,'sid':808,'uid':501}))
   with patch.object(b.os,'getpgid',return_value=808),patch.object(b.os,'getsid',return_value=808),patch.object(b.os,'killpg') as kill:
    with self.assertRaises(RuntimeError):job.observe_fixture()
    self.assertEqual(job.ownership[707]['status'],'CONTRADICTED');self.assertNotIn(707,job.identities)
    with self.assertRaises(RuntimeError):job.stop_group(job.groups[707])
    kill.assert_not_called()
 def test_missing_identity_is_not_verified_and_does_not_signal(self):
  with retained_fixture() as d:
   job,work=self.setup_model(Path(d))
   with patch.object(b.os,'killpg') as kill:
    job.observe_fixture();self.assertEqual(job.ownership[707]['status'],'MISSING')
    with self.assertRaises(RuntimeError):job.stop_group(job.groups[707])
    kill.assert_not_called()
 def test_verified_observation_requires_both_kernel_values(self):
  with retained_fixture() as d:
   job,work=self.setup_model(Path(d));job.fixture_identity_path.write_text(json.dumps({'pid':707,'pgid':707,'sid':707,'uid':501}))
   with patch.object(b.os,'getpgid',return_value=707),patch.object(b.os,'getsid',return_value=707):job.observe_fixture()
   self.assertEqual(job.ownership[707]['status'],'VERIFIED');self.assertEqual(job.identities[707]['uid'],501)
 def test_cleanup_failure_never_reports_success(self):
  with retained_fixture() as d:
   job,work=self.setup_model(Path(d));failure=RuntimeError('mock cleanup failure')
   with patch.object(b.signal,'setitimer'),patch.object(b.p.OwnedJob,'__exit__',side_effect=failure):
    with self.assertRaises(RuntimeError):job.__exit__(RuntimeError,failure,None)
   data=json.loads((work/'disk-budget.json').read_text());self.assertEqual(data['cleanup']['status'],'FAILED_OR_UNCERTAIN');self.assertFalse(data['cleanup']['all_owned_groups_absence_confirmed']);self.assertEqual(data['cleanup']['remaining_groups'][0]['registered_leader_pid'],707);self.assertIsNotNone(data['cleanup']['error']);self.assertIn('NOT established',data['cleanup_order'])

class LeaderReapSourceModels(unittest.TestCase):
 # Pure models: every group syscall and child wait is mocked; NO process spawn.
 def model(self,mode):
  with retained_fixture() as d:
   root=Path(d);work=root/'work';work.mkdir();job=b.LiveBudgetJob(work,3,root,0,1024**2,0.02)
   calls=[];clock=[0.0];state={'killed':False,'waits':0}
   def wait(timeout):
    calls.append(('wait',timeout));state['waits']+=1
    if mode=='never' or (mode=='ignore' and state['waits']==1):
     clock[0]+=timeout;raise b.subprocess.TimeoutExpired('model',timeout)
    return -9 if state['killed'] else -15
   proc=types.SimpleNamespace(pid=707,wait=wait);job.groups[707]=proc
   def killpg(pid,sig):
    calls.append(('killpg',sig));self.assertEqual(pid,707)
    if sig==b.signal.SIGKILL:state['killed']=True
    if sig==0:
     self.assertTrue(any(c[0]=='wait' for c in calls[:-1]))
     if mode=='eperm':raise PermissionError(1,'model EPERM')
     if mode!='worker' or state['killed']:raise ProcessLookupError()
   def sleep(seconds):clock[0]+=seconds
   with patch.object(b.os,'killpg',side_effect=killpg),patch.object(b.time,'monotonic',side_effect=lambda:clock[0]),patch.object(b.time,'sleep',side_effect=sleep):
    if mode in ('eperm','never'):
     with self.assertRaises((PermissionError,RuntimeError)):job.stop_group(proc)
     self.assertIn(707,job.groups)
    else:job.stop_group(proc);self.assertNotIn(707,job.groups)
   self.assertLessEqual(clock[0],4.0)
   return calls,job.events
 def test_term_reap_before_probe(self):
  calls,events=self.model('term');self.assertEqual([c[0] for c in calls],['killpg','wait','killpg']);self.assertIn({'pgid':707,'leader_reaped_status':-15},events)
 def test_term_ignored_escalates_before_probe(self):
  calls,events=self.model('ignore');self.assertEqual([c[0] for c in calls],['killpg','wait','killpg','wait','killpg']);self.assertEqual(calls[2],('killpg',b.signal.SIGKILL))
 def test_reaped_leader_surviving_worker_requires_kill(self):
  calls,events=self.model('worker');self.assertIn(('killpg',b.signal.SIGKILL),calls);self.assertEqual(calls[-2][0],'wait');self.assertEqual(calls[-1],('killpg',0))
 def test_eperm_after_reap_stays_uncertain(self):
  calls,events=self.model('eperm');self.assertEqual([c[0] for c in calls],['killpg','wait','killpg']);self.assertFalse(any(e.get('leader_reaped_and_group_absent') for e in events))
 def test_kill_timeout_no_unreaped_probe(self):
  calls,events=self.model('never');self.assertEqual([c[0] for c in calls],['killpg','wait','killpg','wait'])

if __name__=='__main__':unittest.main()
