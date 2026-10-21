import copy,types,unittest
import dispatch_fullcheck as f
class References(unittest.TestCase):
 def setUp(self):
  self.rows={};self.plans={}
  for scope,unit,names,positions in [('TU13','one.o',['start','end'],[16,48]),('TU14','two.o',['wait','zoom'],[32,64])]:
   refs=[(unit,'.data#0',8+i*16,1,0,'sym',n) for i,n in enumerate(names)]
   self.plans[scope]={'symbols':dict.fromkeys(names),'references':refs}
   self.rows[unit]={'named symbols':tuple((n,18,0,'.text#0',v,8) for n,v in zip(names,positions)),'relocations for .data#0':tuple((r[2],r[3],r[4],('sym',r[6])) for r in refs)}
 def audit(self):return f.check_all(self.rows,self.plans,None)
 def test_exact_owned_slots(self):self.assertEqual(len(self.audit()),4)
 def test_aliases(self):
  for target,addend in [(('local','.text#0',16),0),(('local','.text#0',12),4),(('local','.text#0',20),0),(('sym','start'),4)]:
   with self.subTest(target=target):
    rows=copy.deepcopy(self.rows);self.rows['one.o']['relocations for .text#0']=((0,1,addend,target),)
    with self.assertRaises(ValueError):self.audit()
    self.rows=rows
 def test_external_named_alias(self):
  self.rows['one.o']['named symbols']+=(('alias',18,0,'.text#0',16,8),)
  self.rows['third.o']={'relocations for .data#0':((0,1,0,('sym','alias')),)}
  with self.assertRaises(ValueError):self.audit()
 def test_external_direct(self):
  self.rows['third.o']={'relocations for .data#0':((0,1,0,('sym','start')),)}
  with self.assertRaises(ValueError):self.audit()
 def test_wrong_type_addend_missing_and_duplicate(self):
  for mutant in [((8,10,0,('sym','start')),(24,1,0,('sym','end'))),((8,1,4,('sym','start')),(24,1,0,('sym','end'))),((8,1,0,('sym','start')),),((8,1,0,('sym','start')),(24,1,0,('sym','start')))]:
   with self.subTest(mutant=mutant):
    rows=copy.deepcopy(self.rows);self.rows['one.o']['relocations for .data#0']=mutant
    with self.assertRaises(ValueError):self.audit()
    self.rows=rows
 def test_same_offset_different_section_is_not_alias(self):
  self.rows['one.o']['relocations for .text#0']=((0,1,0,('local','.data#0',16)),)
  self.assertEqual(len(self.audit()),4)
 def test_duplicate_definition(self):
  self.rows['third.o']={'named symbols':(('start',18,0,'.text#0',16,8),)}
  with self.assertRaises(ValueError):self.audit()
 def test_no_subset_as_full_proof(self):
  with self.assertRaises(ValueError):f.compare_trees(self.rows,self.rows,self.plans,dict.fromkeys(self.plans),dict.fromkeys(self.plans),None)
class ConfigAlias(References):
 def test_config_only_external_alias_is_rejected(self):
  for scope in self.plans.values():scope['symbol_map']={n:{'after':[0x2000+i*32,8]} for i,n in enumerate(scope['symbols'])}
  self.rows['external.o']={'relocations for .text#0':((0,10,0,('sym','map_only_alias')),)}
  with self.assertRaises(ValueError):f.check_all(self.rows,self.plans,None,{'map_only_alias':[('.text',0x2004)]},'after')
 def test_config_parser_does_not_skip_unknown_records(self):
  self.assertEqual(f.parse_config_symbols('alias = .text:0x2000; // type:label\n'),{'alias':[('.text',0x2000)]})
  with self.assertRaises(ValueError):f.parse_config_symbols('unknown record')
class Combined(References):
 def fixture(self):
  before=copy.deepcopy(self.rows);after=copy.deepcopy(self.rows);maps={phase:{} for phase in ('before','after')}
  for scope,plan in self.plans.items():
   unit='one.o' if scope=='TU13' else 'two.o';rows=before[unit]['named symbols'];plan['units']=None
   plan['symbols']={}
   plan['symbol_map']={}
   for i,row in enumerate(rows):
    new=(row[0],*row[1:4],rows[1-i][4],row[5])
    after[unit]['named symbols']=tuple(new if r[0]==row[0] else r for r in after[unit]['named symbols'])
    plan['symbols'][row[0]]={'before':list(row[1:]),'after':list(new[1:])}
    plan['symbol_map'][row[0]]={'before':[0x8000+row[4],8],'after':[0x8000+new[4],8]}
    maps['before'][row[0]]=[('.text',0x8000+row[4])];maps['after'][row[0]]=[('.text',0x8000+new[4])]
  for unit in ('one.o','two.o'):
   before[unit]['named symbols']=tuple(sorted(before[unit]['named symbols']));after[unit]['named symbols']=tuple(sorted(after[unit]['named symbols']))
  for i in range(2424):before['strict'+str(i)+'.o']=after['strict'+str(i)+'.o']={'section .text#0':b'exact'}
  for plan in self.plans.values():plan['units']=list(before)
  dp=types.SimpleNamespace(SCOPES={'TU13':{'object':'one.o'},'TU14':{'object':'two.o'}},validate_plan=lambda *a:None,binding_check=lambda *a:None)
  return before,after,maps,dp
 def test_both_envelopes_and_all2424(self):
  a,b,m,dp=self.fixture();r=f.compare_trees(a,b,self.plans,dict.fromkeys(self.plans),dict.fromkeys(self.plans),dp,m);self.assertEqual(r['strict_objects'],2424)
  b['strict0.o']={'section .text#0':b'bad'}
  with self.assertRaises(ValueError):f.compare_trees(a,b,self.plans,dict.fromkeys(self.plans),dict.fromkeys(self.plans),dp,m)
 def test_missing_actual_config_map_rejects(self):
  a,b,m,dp=self.fixture();m['after'].pop('start')
  with self.assertRaises(ValueError):f.compare_trees(a,b,self.plans,dict.fromkeys(self.plans),dict.fromkeys(self.plans),dp,m)
if __name__=='__main__':unittest.main()
