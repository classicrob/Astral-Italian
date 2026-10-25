#!/usr/bin/env python3
"""Pure synthetic analysis tests; no compiler or native subprocess."""
import importlib.util
from pathlib import Path
import unittest
root=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('mutex_report',root/'native/tools/interrupt_mutex_trace_report.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
def event(name,ns,seq=1,thread=1,**extra):
    return dict(event=name,ns=ns,sequence=seq,thread=thread,span=1,held_ns=0,
        mutex=dict(address=123,cv=0,explicit_pending_requests=0,cv_waits_reacquire_unknown=0),**extra)
class Tests(unittest.TestCase):
    def test_held_lower_bound_does_not_manufacture_free_time(self):
        a=event('interrupt_mutex_acquired',10);b=event('interrupt_mutex_cv_release_pending',40,2);b['held_ns']=30
        r=m.summarize([a,b],0,100)
        self.assertEqual(r['errors'],[])
        self.assertEqual(r['coverage'][0]['observed_held_lower_bound_ns'],30)
        self.assertEqual(r['coverage'][0]['not_covered_by_held_lower_bounds_ns'],70)
        self.assertEqual(r['coverage'][0]['uncovered_classification'],'UNKNOWN, not FREE')
        self.assertFalse(r['pure_cv_wake_observed'])
    def test_overlapping_known_owners_rejected(self):
        a=event('interrupt_mutex_acquired',10);b=event('interrupt_mutex_release_begin',40,2);b['held_ns']=30
        x=event('interrupt_mutex_acquired',20,thread=2);y=event('interrupt_mutex_release_pending',50,2,2);y['held_ns']=30
        self.assertIn('overlapping definite mutex ownership',m.summarize([a,x,b,y],0,100)['errors'])
    def test_gap_and_producer_error_rejected(self):
        self.assertIn('producer sequence gap',m.summarize([event('unrelated',1,seq=2)],0,100)['errors'])
        self.assertIn('producer mutex protocol error',m.summarize([event('interrupt_mutex_protocol_error',1)],0,100)['errors'])
    def test_duration_mismatch_rejected(self):
        a=event('interrupt_mutex_acquired',10);b=event('interrupt_mutex_release_begin',40,2)
        self.assertIn('held duration mismatch',m.summarize([a,b],0,100)['errors'])
    def test_mach_cpu_delta_does_not_invent_remote_cpu(self):
        def sample(ns,seq,cpu):
            return event('assigned_unresumed_mach_sample',ns,seq,subject=77,
                mach=dict(target_port=5,publication_before=8,publication_after=8,generation_before=3,generation_after=3,sample_begin_ns=ns-1,
                    sample_end_ns=ns,state=1,suspend_count=0,error=0,user_cpu_ns=cpu,system_cpu_ns=0,
                    remote_cpu=-1,stable_assigned_unresumed=True))
        a,b=sample(10,1,100),sample(20,2,100)
        r=m.summarize([a,b],0,100);self.assertEqual(r['errors'],[])
        self.assertEqual(r['cpu_time_deltas'][0]['cpu_time_delta_ns'],0)
        b['mach']['remote_cpu']=1
        self.assertIn('remote CPU identity fabricated',m.summarize([a,b],0,100)['errors'])
    def test_unstable_mach_sample_never_used_for_deltas(self):
        e=event('assigned_unresumed_mach_sample',10,subject=7,mach=dict(target_port=5,publication_before=8,publication_after=8,generation_before=3,
            generation_after=4,sample_begin_ns=9,sample_end_ns=10,state=1,error=0,user_cpu_ns=0,
            system_cpu_ns=0,remote_cpu=-1,stable_assigned_unresumed=False))
        self.assertEqual(m.summarize([e],0,100)['cpu_time_deltas'],[])
        e['mach']['stable_assigned_unresumed']=True
        self.assertIn('unstable sample labelled stable',m.summarize([e],0,100)['errors'])
    def test_in_progress_publication_cannot_be_certified(self):
        e=event('assigned_unresumed_mach_sample',10,subject=7,mach=dict(target_port=5,generation_before=3,
            generation_after=3,publication_before=9,publication_after=9,sample_begin_ns=9,sample_end_ns=10,
            state=1,error=0,user_cpu_ns=0,system_cpu_ns=0,remote_cpu=-1,stable_assigned_unresumed=True))
        self.assertIn('in-progress publication labelled stable',m.summarize([e],0,100)['errors'])
if __name__=='__main__':unittest.main()
