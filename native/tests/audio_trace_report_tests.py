#!/usr/bin/env python3
"""Pure synthetic timestamps; never invokes audio/native/compiler processes."""
import importlib.util
from pathlib import Path
import unittest
spec=importlib.util.spec_from_file_location('audio_trace',Path(__file__).resolve().parents[1]/'tools/audio_trace_report.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
def read(begin,end,missing=3):
    return dict(event='audio_device_ring_shortfall',ns=end,thread=7,subject=55,value=missing,
        audio=dict(kind='ring-read',begin_ns=begin,end_ns=end,missing_frames=missing,requested_frames=5,copied_frames=5-missing))
def report(begin,end,count):
    return dict(event='audio_underrun_report_exchange',ns=end,thread=8,subject=55,value=count,
        audio=dict(kind='report-counter-exchange',begin_ns=begin,end_ns=end,missing_frames=count,report_source='periodic'))
def callback(span,begin,end=None,thread=7,previous_entry=0,previous_exit=0):
    entering=end is None; ns=begin if entering else end
    return dict(event='audio_device_callback_entry' if entering else 'audio_device_callback_exit',
        ns=ns,thread=thread,subject=55,value=0,host={'cpu':2,'self_qos':33},
        audio=dict(kind='device-callback-entry' if entering else 'device-callback-exit',
            begin_ns=begin,end_ns=ns,callback_span=span,ring_level_sample_ns=ns+1,
            ring_level_frames=100,previous_entry_ns=previous_entry,previous_exit_ns=previous_exit,
            active_callbacks_before=0 if entering else 1,protocol_error=False,realtime_api_status=1))
class Tests(unittest.TestCase):
    def test_callback_gap_duration_and_migration(self):
        events=[callback(1,10),callback(1,10,20),
            callback(2,70,thread=8,previous_entry=10,previous_exit=20),callback(2,70,80,thread=8)]
        r=m.summarize(events,0)
        self.assertEqual(r['errors'],[])
        self.assertEqual(r['callbacks'][1]['entry_gap_ns'],60)
        self.assertEqual(r['callbacks'][1]['since_previous_exit_ns'],50)
        self.assertEqual(r['callbacks'][1]['duration_ns'],10)
        self.assertFalse(r['callbacks'][1]['effective_realtime_policy_known'])
    def test_callback_tail_is_censored_and_unmatched_exit_fails(self):
        r=m.summarize([callback(1,10)],0)
        self.assertEqual(r['errors'],[])
        self.assertTrue(r['callbacks'][0]['right_censored'])
        self.assertIn('callback exit without entry',m.summarize([callback(1,10,20)],0)['errors'])
    def test_overlap_clock_chain_and_foreign_read_fail(self):
        e=callback(1,10);e['audio']['protocol_error']=True
        self.assertTrue(m.summarize([e],0)['errors'])
        e=callback(1,10,20);e['audio']['ring_level_sample_ns']=19
        self.assertIn('callback clock bracket mismatch',m.summarize([callback(1,10),e],0)['errors'])
        e=read(1,2);e['audio']['callback_span']=5
        self.assertIn('shortfall outside linked device callback',m.summarize([e],0)['errors'])
        self.assertIn('callback entry-chain mismatch',m.summarize([callback(1,10),callback(1,10,20),callback(2,30)],0)['errors'])
    def test_exact_count_not_pinned_to_report_frame(self):
        r=m.summarize([read(15,16),report(30,31,3)],0,[(0,20,1),(20,40,2)])
        self.assertEqual(r['errors'],[])
        window=r['rings'][0]['reports'][0]
        self.assertEqual(window['possible_report_frames'],[1,2])
        self.assertEqual(window['read_events'][0]['possible_frames'],[1])
        self.assertFalse(window['exact_single_underrun_frame_claimed'])
    def test_counter_exchange_race_remains_ambiguous(self):
        # Atomic read increment and report exchange each occur inside these
        # overlapping brackets. Either report can own all3 frames.
        for first,last in [(3,0),(0,3)]:
            r=m.summarize([read(9,12),report(10,11,first),report(20,21,last)],0)
            self.assertEqual(r['errors'],[])
            self.assertEqual(r['rings'][0]['reported_frames'],3)
    def test_inconsistent_counts_fail(self):
        self.assertTrue(m.summarize([read(1,2),report(3,4,4)],0)['errors'])
    def test_read_crossing_frame_boundary_exposes_both(self):
        r=m.summarize([read(19,21),report(30,31,3)],0,[(0,20,1),(20,40,2)])
        self.assertEqual(r['rings'][0]['reports'][0]['read_events'][0]['possible_frames'],[1,2])
    def test_bad_clock_or_fabricated_read_arithmetic_fails(self):
        e=read(1,2);e['ns']=3
        self.assertIn('audio clock bracket mismatch',m.summarize([e],0)['errors'])
        e=read(1,2);e['audio']['copied_frames']=0
        self.assertIn('invalid ring-read shortfall',m.summarize([e],0)['errors'])
    def test_equal_clock_ticks_remain_boundary_ambiguous(self):
        for first,last in [(3,0),(0,3)]:
            r=m.summarize([read(10,11),report(10,11,first),report(20,21,last)],0)
            self.assertEqual(r['errors'],[])
        self.assertEqual(m.frame_candidates(20,20,[(0,20,1),(20,40,2)]),[1,2])
if __name__=='__main__':unittest.main()
