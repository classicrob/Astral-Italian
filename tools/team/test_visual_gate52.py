import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import hashlib,json,tempfile,unittest
from pathlib import Path
from visual_gate52 import validate_review

class VisualGate(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name);self.expected=dict(source='source',binary_sha256='binary',case='normal-boot-ui',mode='fresh')
  self.captures={}
  for n in ('before','after'):
   p=self.root/(n+'.png');p.write_bytes(n.encode());self.captures[str(p)]=hashlib.sha256(p.read_bytes()).hexdigest()
  self.o=dict(source='source',binary_sha256='binary',observed_pid=123,observed_monotonic=20,ordinary_cua_input=True,mapping_observed=True,normal_route_observed=True,before_screenshot=next(iter(self.captures)),after_screenshot=list(self.captures)[1],screenshot_sha256=self.captures)
  self.r=dict(self.expected,accepted=True,capture_sha256=self.captures);self.write()
 def tearDown(self):self.tmp.cleanup()
 def write(self):
  p=self.root/'host-pointer.json';p.write_text(json.dumps(self.o));self.r['pointer_observation_sha256']=hashlib.sha256(p.read_bytes()).hexdigest();(self.root/'visual-review.json').write_text(json.dumps(self.r))
 def check(self):return validate_review(self.root,self.expected,123,10,30)
 def test_valid(self):self.assertTrue(self.check())
 def test_missing(self):
  (self.root/'visual-review.json').unlink()
  with self.assertRaises(ValueError):self.check()
 def test_rejected(self):
  self.r['accepted']=False;self.write()
  with self.assertRaises(ValueError):self.check()
 def test_identity_mismatches(self):
  for field in self.expected:
   with self.subTest(field=field):
    old=self.r[field];self.r[field]='wrong';self.write()
    with self.assertRaises(ValueError):self.check()
    self.r[field]=old
 def test_stale_time(self):
  self.o['observed_monotonic']=1;self.write()
  with self.assertRaises(ValueError):self.check()
 def test_wrong_pid(self):
  self.o['observed_pid']=124;self.write()
  with self.assertRaises(ValueError):self.check()
 def test_mutated_capture(self):
  Path(next(iter(self.captures))).write_bytes(b'changed')
  with self.assertRaises(ValueError):self.check()
 def test_missing_capture(self):
  Path(next(iter(self.captures))).unlink()
  with self.assertRaises(ValueError):self.check()
 def test_unbound_live_observation(self):
  p=self.root/'host-pointer.json';p.write_text(p.read_text()+' ')
  with self.assertRaises(ValueError):self.check()
if __name__=='__main__':unittest.main()
