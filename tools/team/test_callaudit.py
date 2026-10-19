import unittest,struct,json,hashlib,tempfile
from pathlib import Path
from callaudit import compare,calls,catalog,branch_destination,ELF
P=Path(__file__).parent
DOL=b'\0'*256
def sample_catalog():
 with tempfile.TemporaryDirectory() as td:
  path=Path(td)/'symbols.txt'
  path.write_text('x = .text:0x80000000; // type:function size:0x4\ny = .text:0x80000010; // type:function size:0x4\n')
  return catalog(path)
CAT=sample_catalog()
class Fake:
 def __init__(self,words,rels):
  self.f={'f':('f',0,len(words)*4,1)};self.s=[(0,0),(0,1)];self.c=[b'',struct.pack('>'+str(len(words))+'I',*words)];self.rel={(1,o):(10,(n,0,0,0),a)for o,n,a in rels}
class TestCalls(unittest.TestCase):
 def test_order_vs_multiset(self):
  t=Fake([0x48000001,0x48000001],[(0,'x',0),(4,'y',0)]);s=Fake([0x48000001,0x48000001],[(0,'y',0),(4,'x',0)]);cat=sample_catalog();cat['f']={'address':0,'size':8};r=compare(t,s,'f',cat,b'\0'*256);self.assertFalse(r['ordered_direct_calls_equal']);self.assertTrue(r['direct_call_multisets_equal'])
 def test_addends_and_multiplicity(self):
  t=Fake([0x48000001,0x48000001],[(0,'x',4),(4,'x',4)]);s=Fake([0x48000001,0x48000001],[(0,'x',4),(4,'x',8)]);cat=sample_catalog();cat['f']={'address':0,'size':8};r=compare(t,s,'f',cat,b'\0'*256);self.assertEqual(r['target_only'],[{'callee':'x','addend':4,'count':1}])
 def test_no_fn_prefix_aliasing(self):
  e=Fake([0x48000001],[(0,'fn_123',0)]);r,u,_=calls(e,'f',CAT,DOL);self.assertEqual(r[0]['identity'],['fn_123',0])
 def test_indirect_explicit_tail_excluded(self):
  e=Fake([0x4e800421,0x48000000],[]);r,u,ind=calls(e,'f',CAT,DOL);self.assertFalse(r);self.assertEqual(len(ind),1)
 def test_unresolved_raw_call(self):
  e=Fake([0x48000401],[]);r,u,_=calls(e,'f',CAT,DOL);self.assertTrue(u);self.assertNotIn('identity',r[0])
 def test_branch_AA_LK(self):
  self.assertEqual(branch_destination(0x48000005,0x80000000),0x80000004);self.assertEqual(branch_destination(0x48000007,0x80000000),4);self.assertIsNone(branch_destination(0x48000000,0x80000000))
 def test_zero_call_pair(self):
  t=Fake([0x4e800020],[]);cat=sample_catalog();cat['f']={'address':0,'size':4};r=compare(t,t,'f',cat,b'\0'*256);self.assertTrue(r['ordered_direct_calls_equal']);self.assertTrue(r['ordered_identity_comparison_complete'])
 def test_unresolved_pairs_not_equal(self):
  t=Fake([0x48000401],[]);cat=sample_catalog();cat['f']={'address':0,'size':4};r=compare(t,t,'f',cat,b'\0'*256);self.assertFalse(r['ordered_direct_calls_equal']);self.assertFalse(r['ordered_identity_comparison_complete'])
 def test_duplicate_registered_names_preserved(self):
  with tempfile.TemporaryDirectory()as td:
   f=Path(td)/'symbols.txt';f.write_text('f = .text:0x80000000; // type:function size:0x4\nf = .text:0x80000010; // type:function size:0x4\n');c=catalog(f);self.assertEqual(len(c.duplicates['f']),2);self.assertEqual(len(c.exact),2)
 def test_NOBITS_refused(self):
  e=Fake([0x48000001],[]);e.s[1]=(0,8);e.c[1]=b'';r,u,_=calls(e,'f',CAT,DOL);self.assertFalse(r);self.assertEqual(u[0]['reason'],'NOBITS function refused')
if __name__=='__main__':unittest.main()
