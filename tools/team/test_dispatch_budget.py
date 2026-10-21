import json,sys,tempfile,types,unittest,contextlib,shutil,os
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
 else:shutil.rmtree(root)

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
if __name__=='__main__':unittest.main()
