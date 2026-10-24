#!/usr/bin/env python3
"""Safety checks for measurement parsing and fail-closed acceptance; no game launch."""
import copy,csv,importlib.util,json,sqlite3,subprocess,sys,tempfile,unittest
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
 def test_candidate_arrival_requires_exact_state_and_rng_readback(self):
  from replay_cases54m import anchor_checks,ANCHOR_POLICY
  report={'anchor_policy':ANCHOR_POLICY,'segments':[{'stage':'RedBlueExGalaxy','play_ready_required':True,'play_ready_state_check':{'checked':True,'passed':True,'position_bit_exact':True,'rng':{'verified':True}}}]}
  self.assertTrue(all(anchor_checks(report,'RedBlueExGalaxy').values()))
  for key in ('checked','passed','position_bit_exact','rng'):
   bad=copy.deepcopy(report);bad['segments'][0]['play_ready_state_check'][key]=False if key!='rng' else {'verified':False}
   self.assertFalse(all(anchor_checks(bad,'RedBlueExGalaxy').values()),key)
  bad=copy.deepcopy(report);bad['segments'][0]['play_ready_required']=False
  self.assertFalse(all(anchor_checks(bad,'RedBlueExGalaxy').values()))
 def test_candidate_interactive_map_requires_explicit_policy_and_anchor_kind(self):
  from replay_cases54m import anchor_checks,ANCHOR_POLICY
  report={'anchor_policy':ANCHOR_POLICY,'segments':[{'stage':'WorldMap01Galaxy','play_ready_required':False,'play_ready_state_check':None}]}
  self.assertTrue(all(anchor_checks(report).values()))
  del report['segments'][0]['play_ready_required'];self.assertFalse(all(anchor_checks(report).values()))
  self.assertFalse(all(anchor_checks({'segments':[]}).values()))
  report['segments'][0]['play_ready_required']=False;report['anchor_policy']='legacy-play-ready'
  self.assertFalse(all(anchor_checks(report).values()))
 def test_bounded_identity_still_requires_every_pointer_row(self):
  from replay_cases54m import anchor_checks,replay_accounting,ANCHOR_POLICY
  report={'anchor_policy':ANCHOR_POLICY,'interactive_position_bit_exact':True,'segments':[{'stage':'FileSelect','play_ready_required':False,'rows_compared':10,'identity_rows_skipped':1,'tape_frames':10,'pointer_rows_compared':10,'identity_recorded_frame':225,'identity_replay_frame':224,'identity_expected_replay_frame':225,'identity_frame_offset':-1,'identity_offset_limit':30,'identity_offset_checked':True,'identity_offset_passed':True,'max_position_error':0}]}
  self.assertTrue(all(anchor_checks(report,require_identity=True).values()));self.assertTrue(replay_accounting(report,True))
  bad=copy.deepcopy(report);bad['segments'][0]['pointer_rows_compared']=9
  self.assertFalse(all(anchor_checks(bad,require_identity=True).values()));self.assertFalse(replay_accounting(bad,True))
  bad=copy.deepcopy(report);bad['segments'][0].update(identity_replay_frame=256,identity_frame_offset=31)
  self.assertFalse(all(anchor_checks(bad,require_identity=True).values()))
  bad=copy.deepcopy(report);del bad['interactive_position_bit_exact']
  self.assertFalse(all(anchor_checks(bad,require_identity=True).values()))
  bad=copy.deepcopy(report);bad['segments'][0]['rows_compared']=9
  self.assertFalse(replay_accounting(bad,True))
 def test_pointer_coverage_requires_actual_game_buffer_counters(self):
  from replay_cases54m import game_pointer_coverage
  report={'segments':[{'stage':'WorldMap01Galaxy','pointer_rows_compared':100,'pointer_valid_rows_compared':30,'pointer_distinct_positions_compared':3}]}
  self.assertTrue(game_pointer_coverage(report)['passed'])
  for field,value in (('pointer_valid_rows_compared',29),('pointer_distinct_positions_compared',2),('pointer_valid_rows_compared',101),('pointer_distinct_positions_compared',31),('pointer_valid_rows_compared',True)):
   bad=copy.deepcopy(report);bad['segments'][0][field]=value
   self.assertFalse(game_pointer_coverage(bad)['passed'],(field,value))
  for field in ('pointer_valid_rows_compared','pointer_distinct_positions_compared'):
   bad=copy.deepcopy(report);del bad['segments'][0][field]
   self.assertFalse(game_pointer_coverage(bad)['passed'],field)
  self.assertFalse(game_pointer_coverage({'segments':[]})['passed'])
 def test_pointer_observation_keeps_ninety_seconds_and_neutral_provenance(self):
  from replay_cases54m import pointer_observation_checks
  report={'observation':{'requested_seconds':90,'clock':'steady_clock; quit scheduling only','start_monotonic_seconds':1000.,'stop_monotonic_seconds':1090.,'elapsed_seconds':90.,'completed':True,'post_roll':{'started':True,'start_route_row':1801,'end_route_row':5401,'start_elapsed_seconds':30.,'elapsed_seconds':60.,'loops':3600,'input':'neutral','input_source':'Headless::Replay::neutral','neutral_read_calls':14400}}}
  self.assertTrue(all(pointer_observation_checks(report,{'seconds':90.5}).values()))
  self.assertFalse(all(pointer_observation_checks(report,{'seconds':89.9}).values()))
  for key,value in (('elapsed_seconds',89.99),('completed',False),('stop_monotonic_seconds',None),('requested_seconds',30)):
   bad=copy.deepcopy(report);bad['observation'][key]=value
   self.assertFalse(all(pointer_observation_checks(bad,{'seconds':90.5}).values()),key)
  for key,value in (('input','recorded'),('input_source','host'),('neutral_read_calls',0),('elapsed_seconds',59.),('started',False)):
   bad=copy.deepcopy(report);bad['observation']['post_roll'][key]=value
   self.assertFalse(all(pointer_observation_checks(bad,{'seconds':90.5}).values()),key)
  self.assertFalse(all(pointer_observation_checks({}, {'seconds':90.5}).values()))
if __name__=='__main__':unittest.main()
