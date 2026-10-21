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
if __name__=='__main__':unittest.main()
