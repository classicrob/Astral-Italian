#!/usr/bin/env python3
"""Safety checks for measurement parsing and fail-closed acceptance; no game launch."""
import csv,importlib.util,json,sqlite3,subprocess,sys,tempfile,unittest
from pathlib import Path
HERE=Path(__file__).resolve().parent
from receipts54m import audio_observation,latency

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
 def test_cache_parser_handles_both_storage_tables(self):
  from promotion54m import inspect_cache
  with tempfile.TemporaryDirectory(prefix='m1cache-test-') as raw:
   for table in ('pipeline_cache','cache'):
    path=Path(raw)/(table+'.db')
    with sqlite3.connect(path) as db:
     db.executescript('CREATE TABLE petari_cache_build(build_key); CREATE TABLE '+table+'(value);')
     db.execute('INSERT INTO petari_cache_build VALUES(?)',('abc',));db.execute('INSERT INTO '+table+' VALUES(1)')
    result=inspect_cache(path,'abc');self.assertEqual(result['rows'],1);self.assertEqual(result['table'],table)
    with self.assertRaises(AssertionError):inspect_cache(path,'wrong-build')
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
 def test_audio_requires_real_reports_and_zero_underruns(self):
  prefix='[thread] main thread priority: 37 -> 47 (raised)\n'
  good=prefix+'underrun 0 frames\n'*45
  self.assertTrue(audio_observation(good,10)['passed'])
  self.assertFalse(audio_observation(prefix,10)['passed'])
  self.assertFalse(audio_observation(good+'underrun 1 frames\n',10)['passed'])
  self.assertFalse(audio_observation(good,0)['passed'])
 def test_latency_extracts_frames_after_read(self):
  samples=latency('[latency] press: 60 ms (frame 88, 2 frames after the read), on screen 70.12 ms\n[latency] press: 60 ms (frame 89, 3 frames after the read), on screen 86.7 ms')
  self.assertEqual(samples,[(2,70.12),(3,86.7)])
if __name__=='__main__':unittest.main()
