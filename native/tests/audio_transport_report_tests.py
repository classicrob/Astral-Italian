#!/usr/bin/env python3
"""Pure Python, no native fixture, audio or subprocess."""
import importlib.util
from pathlib import Path
import unittest
s=importlib.util.spec_from_file_location('transport',Path(__file__).resolve().parents[1]/'tools/audio_transport_report.py')
m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
CFG='[audio-workgroup-config] schema=2 boundary_clock=steady_clock_ns pre_ticks=2000 max_post_ticks=500\n'
def log(rows):
    result=CFG+f'[audio-workgroup-history] begin trigger_mach=3000 end_mach=3400 rows={len(rows)} drops=0 unstable=0 clipped=0 retention_unknown=0 rejected=0 rejected_omitted=0\n'
    for seq,kind,ns,ticket,obj in rows:
        result+=f'[audio-workgroup-history] event seq={seq} mach=2500 kind={kind} thread=123 value=0 steady_ns={ns} ticket={ticket} object={obj:x}\n'
    return result+f'[audio-workgroup-history] end rows={len(rows)}\n'
ROWS=[(i,k,n*1000000,1,10 if k in (17,18,19,20,21) else 20) for i,(k,n) in enumerate(
    [(17,10),(19,11),(20,21),(22,22),(23,23),(24,25),(25,26),(30,27),(31,28),(32,29),(33,30),
     (34,31),(26,32),(27,33),(28,40),(29,41),(21,42),(18,43)],1)]
class Tests(unittest.TestCase):
    def test_nested_spans_are_separate(self):
        r=m.analyze(log(ROWS));self.assertTrue(r['trace_valid']);self.assertFalse(r['acceptance_claimed'])
        c=r['callbacks'][0];self.assertTrue(c['complete_entry_exit'])
        for key,wanted in [('device_lock_wait',10),('stream_get_lock_wait',2),('fill',6),('conversion',7),('put',4)]:
            self.assertEqual(c['spans'][key][0]['ms'],wanted)
        self.assertIsNone(c['metadata']['cpu_error_number'])
    def test_partial_window_is_explicit(self):
        r=m.analyze(log(ROWS[1:-1]));self.assertTrue(r['trace_valid'])
        self.assertFalse(r['callbacks'][0]['complete_entry_exit']);self.assertIsNone(r['callbacks'][0]['entry_ns'])
    def test_ring_level_and_shortfall_keep_actual_callback_ticket(self):
        rows=[(1,17,1000000,7,10),(2,6,2000000,7,0),(3,7,3000000,7,0),(4,18,4000000,7,10)]
        text=log(rows).replace('kind=6 thread=123 value=0','kind=6 thread=123 value='+str((1722<<32)|186))
        text=text.replace('kind=7 thread=123 value=0','kind=7 thread=123 value=120')
        c=m.analyze(text)['callbacks'][0]
        self.assertEqual(c['ring_samples'],[{'steady_ns':2000000,'level':1722,'requested':186}])
        self.assertEqual(c['shortfall_frames'],120)
    def test_overlapping_dump_deduplicates_only_exact_rows(self):
        original=log(ROWS)
        self.assertEqual(m.analyze(original+original)['unique_rows'],len(ROWS))
        self.assertFalse(m.analyze(original+original.replace('value=0','value=1',1))['trace_valid'])
    def test_loss_and_bounds_are_not_ignored(self):
        for a,b in [('drops=0','drops=1'),('retention_unknown=0','retention_unknown=1'),('end_mach=3400','end_mach=3600'),
                    ('steady_ns=10000000','steady_ns=0'),('thread=123','thread=456')]:
            bad=log(ROWS).replace(a,b,1)
            self.assertFalse(m.analyze(bad)['trace_valid'],b)
        self.assertFalse(m.analyze(log(ROWS).replace('end rows=18','end rows=17'))['trace_valid'])
if __name__=='__main__':unittest.main()
