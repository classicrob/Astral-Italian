#!/usr/bin/env python3
"""Light model checks only; does not compile or replace actual ELF controls."""
import copy,types,unittest
import dispatch_preflight as p
class Check(unittest.TestCase):
 def test_envelopes(self):
  for scope in p.EXPECT:
   for identity in p.EXPECT[scope]:
    name=('Map' if scope=='TU13' else 'End')+identity+'CFP'
    old,new=p.EXPECT[scope][identity]
    before={'named symbols':((name,18,0,'.text#0',*old),),'section .text#0':b'exact','relocations':('exact',)}
    after=copy.deepcopy(before);after['named symbols']=((name,18,0,'.text#0',*new),)
    ob=types.SimpleNamespace(summarize=lambda x:copy.deepcopy({'a':before,'b':after}[x]))
    dp=types.SimpleNamespace(SCOPES={scope:{'symbols':[name]}})
    self.assertTrue(p.envelope(ob,'a','b',scope,dp))
    for key in ['section .text#0','relocations']:
     save=after[key];after[key]=b'changed'
     with self.assertRaises(AssertionError):p.envelope(ob,'a','b',scope,dp)
     after[key]=save
    for field in [1,2,3,4,5]:
     save=after['named symbols'];row=list(save[0]);row[field]='wrong';after['named symbols']=(tuple(row),)
     with self.assertRaises(AssertionError):p.envelope(ob,'a','b',scope,dp)
     after['named symbols']=save
 def test_config(self):
  maps={'TU13':{'symbols':{'Start':((1,8),(2,8)),'End':((2,8),(1,8))}},'TU14':{'symbols':{'Wait':((4,12),(3,8)),'Zoom':((3,8),(4,12))}}}
  dp=types.SimpleNamespace(SCOPES=maps)
  def text(phase):return 'strict remainder\n'+''.join(n+' = .text:0x%08X; // type:function size:0x%X\n'%v[phase] for s in maps.values() for n,v in s['symbols'].items())
  a,b=text(0),text(1);self.assertTrue(p.config_envelope(a,b,dp))
  for bad in [b.replace('strict','changed'),b.replace('0x00000003','0x00000009'),b.replace('size:0xC','size:0x8'),b+b.splitlines(keepends=True)[1]]:
   with self.assertRaises(AssertionError):p.config_envelope(a,bad,dp)

class Lifecycle(unittest.TestCase):
 def test_leader_exit_does_not_prove_worker_exit(self):
  from unittest.mock import Mock,patch
  from pathlib import Path
  job=p.OwnedJob(Path('.'));proc=Mock(pid=707);proc.poll.return_value=0;proc.wait.return_value=0;job.groups[707]=proc
  state={'alive':True};calls=[]
  def killpg(pid,sig):
   calls.append(sig)
   if not state['alive']:raise ProcessLookupError()
   if sig==p.signal.SIGKILL:state['alive']=False
  ticks=iter(range(100))
  with patch.object(p.os,'killpg',side_effect=killpg),patch.object(p.time,'monotonic',side_effect=lambda:next(ticks)),patch.object(p.time,'sleep'):
   job.stop_group(proc)
  self.assertIn(p.signal.SIGTERM,calls);self.assertIn(p.signal.SIGKILL,calls);self.assertFalse(job.groups);proc.wait.assert_called_once()
 def test_term_int_hup_unwind(self):
  import tempfile
  from pathlib import Path
  for sig in (p.signal.SIGTERM,p.signal.SIGINT,p.signal.SIGHUP):
   with tempfile.TemporaryDirectory() as d:
    with self.assertRaises(InterruptedError):
     with p.OwnedJob(Path(d),2):p.os.kill(p.os.getpid(),sig)
    result=p.json.loads((Path(d)/'owned-process-cleanup.json').read_text());self.assertTrue(result['all_owned_groups_absent']);self.assertIn('InterruptedError',result['execution_error'])
 def test_deadline_covers_python_postcompile_work(self):
  import tempfile
  from pathlib import Path
  with tempfile.TemporaryDirectory() as d:
   with self.assertRaises(TimeoutError):
    with p.OwnedJob(Path(d),0.02):p.time.sleep(0.2)
   result=p.json.loads((Path(d)/'owned-process-cleanup.json').read_text());self.assertIn('TimeoutError',result['execution_error'])

if __name__=='__main__':unittest.main()
