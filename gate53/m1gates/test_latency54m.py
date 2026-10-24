import copy,unittest
from latency54m import ledger_checks,script_without_space
from common import REPO

def fixture():
 es=[{'event':'start','ns':1,'clock':'CLOCK_UPTIME_RAW','planned_presses':27,'seed':12345,'gap_min_ms':1000,'gap_max_ms':1600,'hold_ms':60,'symbol_exports_verified':True,'main_executable_file_verified':True}];text=[];plan=1
 for i in range(1,28):
  plan+=1_060_000_000;n=plan+100
  es.extend([{'event':'send','id':i,'ns':n,'native_press_ns':n+1,'return_ns':n+2,'planned_ns':plan,'gap_ms':1000,'stage_before':0},
   {'event':'release','id':i,'ns':n+60_000_002,'return_ns':n+60_000_003},
   {'event':'match','id':i,'ns':n+70_000_000,'matched':True,'stage':0,'native_press_ns':n+1,'report_ns':n+2,'read_ns':n+3,'copy_ns':n+4,'shown_ns':n+5,'present_ns':n+6,'read_frame':i*100,'shown_frame':i*100+2}])
  text.append(f'[latency] press: report 0.0 ms, game read 0.0 ms (frame {i*100}), display copy 0.0 ms (frame {i*100+2}), shown 0.0 ms (frame {i*100+2}, 2 frames after the read), present returned 0.0 ms, on screen 50.0 ms')
 es.append({'event':'quit','ns':es[-1]['ns']+250_000_000,'sent':27,'failed':False,'drain_ms':250})
 return es,'\n'.join(text)

class Latency(unittest.TestCase):
 def test_complete_independent_wall_train(self):
  e,t=fixture();self.assertTrue(ledger_checks(e,t)['passed'])
 def test_missing_actual_read_refuses(self):
  e,t=fixture();e[-2]['matched']=False;e[-2]['read_ns']=0
  d=ledger_checks(e,t);self.assertFalse(d['passed']);self.assertEqual(d['unmatched_send_ids'],[27])
 def test_busy_attempt_is_not_replaced(self):
  e,t=fixture();e.insert(4,{'event':'skip_busy','id':2,'ns':2,'stage':4});self.assertFalse(ledger_checks(e,t)['passed'])
 def test26_reports_refuses(self):
  e,t=fixture();self.assertFalse(ledger_checks(e,'\n'.join(t.splitlines()[:-1]))['passed'])
 def test_shifted_game_frame_schedule_refuses(self):
  e,t=fixture();e[4]['planned_ns']+=16_666_667;self.assertFalse(ledger_checks(e,t)['passed'])
 def test_wrong_latency_refuses(self):
  e,t=fixture();e[3]['shown_frame']+=1;self.assertFalse(ledger_checks(e,t)['passed'])
 def test_no_drain_refuses(self):
  e,t=fixture();e[-1]['ns']=e[-2]['ns'];self.assertFalse(ledger_checks(e,t)['passed'])
 def test_no_script_key_collision_or_timeline_change(self):
  original=(REPO/'native/tools/perf_sweep_move.script').read_text();s=script_without_space(original)
  self.assertNotIn('Space',[x for l in s.splitlines() if not l.startswith('#') for x in l.split()])
  self.assertEqual(sum(int(l.split()[0]) for l in s.splitlines() if l and not l.startswith('#')),970)

if __name__=='__main__':unittest.main()
