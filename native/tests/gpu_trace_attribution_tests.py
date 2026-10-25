#!/usr/bin/env python3
"""Pure Python synthetic-clock tests. No compilation or app execution."""
import copy
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('trace_attribution', Path(__file__).resolve().parents[1] / 'tools/gpu_trace_attribution.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

def event(name, ns, **kw):
    result = dict(event=name, ns=ns, thread=1, sequence=ns, subject=0)
    result.update(kw)
    return result

def fixture():
    return [event('baton_grant', 100, subject=7, holder=0, waiter=7, held_ns=0, reason='initial'),
            event('host_wait_begin', 105, primitive='cond-for', deadline_ns=130),
            event('baton_refusal', 110, holder=7, waiter=9, held_ns=10, reason='refusal-system-code'),
            event('baton_grant', 120, subject=9, holder=7, waiter=9, held_ns=20, reason='reassign-unresumed'),
            event('host_wait_return', 150, primitive='cond-for', deadline_ns=130, late_ns=20, wait_ns=45),
            event('baton_release', 160, holder=9, waiter=0, held_ns=40, reason='host-blocking')]

class AttributionTests(unittest.TestCase):
    def test_reassignment_refusal_and_deadline(self):
        result = module.audit(fixture())
        self.assertEqual(result['errors'], [])
        self.assertEqual([x['held_ns'] for x in result['assignments']], [0, 20, 40])
        self.assertEqual(result['waits'][0]['late_ns'], 20)
        self.assertTrue(result['assignments'][1]['wall_time_not_cpu_time'])
        self.assertNotIn('total', result)
        self.assertFalse(result['acceptance'])

    def test_unknown_remote_cpu_and_requested_qos(self):
        data = fixture()
        host = dict(observer=1001, target=2002, sampled_ns=101, cpu=-1, requested_qos=33, priority=47)
        data[0]['host'] = host
        self.assertFalse(module.audit(data)['errors'])
        host['cpu'] = 4
        self.assertIn('remote CPU number fabricated from emitter', module.audit(data)['errors'])

    def test_bad_duration_holder_or_deadline_is_not_admitted(self):
        for index, key, value in [(2, 'held_ns', 11), (3, 'holder', 2), (4, 'late_ns', 0),
                                   (4, 'wait_ns', 44), (4, 'deadline_ns', 131)]:
            data = copy.deepcopy(fixture()); data[index][key] = value
            self.assertTrue(module.audit(data)['errors'], key)

    def test_monitor_deadline_wake_and_backoff_are_not_collapsed(self):
        data = [event('monitor_wait_plan', 100, span=7, value=500000),
                event('host_wait_begin', 101, span=7, primitive='mach-semaphore-timedwait', deadline_ns=500101),
                event('host_wait_return', 2500101, span=7, primitive='mach-semaphore-timedwait', deadline_ns=500101,
                      wait_ns=2500000, late_ns=2000000),
                event('monitor_woke', 2500101, span=7),
                event('forced_preemption_attempt_result', 2500200, span=7, detail=3),
                event('monitor_retry_backoff', 2500300, span=7, value=250000)]
        result = module.audit(data)
        self.assertFalse(result['errors'])
        attempt = result['monitor_attempts'][0]
        self.assertEqual(attempt['wait_return']['late_ns'], 2000000)
        self.assertEqual(attempt['events'][-1]['value'], 250000)
        data[3]['ns'] += 1
        self.assertTrue(module.audit(data)['errors'])

    def test_nested_untimed_and_shutdown_censoring(self):
        data = [event('host_wait_begin', 100, primitive='wii-queue', deadline_ns=0),
                event('host_wait_begin', 110, primitive='cond-untimed', deadline_ns=0),
                event('host_wait_return', 150, primitive='cond-untimed', deadline_ns=0, late_ns=0, wait_ns=40)]
        result = module.audit(data)
        self.assertEqual(result['errors'], [])
        self.assertEqual(result['waits_open_at_shutdown'][0]['primitive'], 'wii-queue')
        self.assertEqual(result['waits'][0]['deadline_ns'], None)
        data[2]['late_ns'] = 1
        self.assertTrue(module.audit(data)['errors'])

if __name__ == '__main__': unittest.main()
