#!/usr/bin/env python3
"""Safety checks for measurement parsing and fail-closed acceptance; no game launch."""
import csv,importlib.util,json,subprocess,sys,tempfile,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
from receipts54m import latency

class GateValidation(unittest.TestCase):
 def case(self,values,log='',warm=False):
  with tempfile.TemporaryDirectory(prefix='m1gate-test-') as raw:
   root=Path(raw);(root/'app.log').write_text(log)
   fields=['phase','unfocused','interval_ms','present_ms']
   with (root/'frames.csv').open('w') as f:
    w=csv.DictWriter(f,fieldnames=fields);w.writeheader()
    for phase,iv,present in values:w.writerow({'phase':phase,'unfocused':'0','interval_ms':iv,'present_ms':present})
   p=subprocess.run([sys.executable,'-B',str(HERE/'gate533.py'),str(root)]+(['--warm'] if warm else []),capture_output=True,text=True)
   return p.returncode,json.loads(p.stdout)
 def test_gameplay_stall_preserved(self):
  code,r=self.case([('menu',16.7,16.7),('gameplay',101,101)],'[stall] frame 42 101.0 ms phase=gameplay cause=other stage YosshiHomeGalaxy\n')
  self.assertEqual(code,1);self.assertEqual(r['gameplay_over100'][0][:2],[42,101.0])
 def test_hold_and_menu_limit_preserved(self):
  code,r=self.case([('menu',26,26)]*100+[('gameplay',16.7,16.7)],'[stall] frame 2 3001.0 ms phase=loading\n')
  self.assertEqual(code,1);self.assertTrue(r['holds_over3s']);self.assertEqual(r['menu_p99'],26)
 def test_one_percent_is_failure(self):
  code,r=self.case([('menu',16.7,16.7)]+[('gameplay',16.7,16.7)]*99+[('gameplay',25,25)],warm=True)
  self.assertEqual(code,1);self.assertEqual(r['late_present_pct'],1.0)
 def test_below_one_percent_is_pass(self):
  code,r=self.case([('menu',16.7,16.7)]+[('gameplay',16.7,16.7)]*100+[('gameplay',25,25)],warm=True)
  self.assertEqual(code,0);self.assertLess(r['late_present_pct'],1)
 def test_latency_extracts_frames_after_read(self):
  samples=latency('[latency] press: 60 ms (frame 88, 2 frames after the read), on screen 70.12 ms\n[latency] press: 60 ms (frame 89, 3 frames after the read), on screen 86.7 ms')
  self.assertEqual(samples,[(2,70.12),(3,86.7)])
if __name__=='__main__':unittest.main()
