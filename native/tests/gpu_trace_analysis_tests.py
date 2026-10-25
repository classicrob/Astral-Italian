#!/usr/bin/env python3
"""Pure Python fake-clock trace/CSV proof; no native fixtures or compilation."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('trace_report', Path(__file__).resolve().parents[1] / 'tools/gpu_trace_report.py')
report = importlib.util.module_from_spec(spec)
spec.loader.exec_module(report)


def event(name, ns, **kwargs):
    result = dict(type='boundary', event=name, ns=ns, thread=1, sequence=0,
                  packet=0, ticket=0, submission=0, span=0, csv=0, route=0, subject=0, detail=0, value=0)
    result.update(kwargs)
    return result


def fixture():
    events = [event('csv_seed', 1000), event('csv_interval_open', 1000, csv=1),
              event('packet_open', 1001, packet=7), event('capture_request', 1100, packet=7, ticket=9),
              event('csv_interval_close', 2001000, csv=1), event('csv_interval_open', 2001000, csv=2),
              event('map_callback_enter', 2001100, ticket=9), event('readback_cpu_complete', 2001200, ticket=9),
              event('csv_interval_close', 4001000, csv=2)]
    for sequence, e in enumerate(events, 1): e['sequence'] = sequence
    return [dict(type='header', schema=1, clock='steady_clock_ns'), *events,
            dict(type='footer', drops=0, pending_callbacks=0, io_errors=0, complete=True)]


class TraceAnalysisTests(unittest.TestCase):
    rows = [dict(frame='1', interval_ms='2.000'), dict(frame='2', interval_ms='2.000')]
    def test_late_callback_origin_is_not_observation(self):
        events, intervals, errors, _ = report.join(fixture(), self.rows)
        self.assertEqual(errors, [])
        callback = next(x for x in events if x['event'] == 'map_callback_enter')
        self.assertEqual(callback['origin_csv_frame'], 1)
        self.assertEqual(callback['observed_csv_frame'], 2)
        self.assertEqual(callback['origin_packet'], 7)
        self.assertEqual(len(intervals), 2)

    def test_refuse_drop_wrong_duration_duplicate_missing(self):
        for mutation in ['drop', 'duration', 'duplicate', 'missing', 'unknown-ticket']:
            data = fixture()
            if mutation == 'drop': data[-1]['drops'] = 1
            elif mutation == 'duration': data[-2]['ns'] += 10000
            elif mutation == 'duplicate': data.insert(-1, dict(data[2]))
            elif mutation == 'missing': data.pop(2)
            else: data[7]['ticket'] = 100
            self.assertTrue(report.join(data, self.rows)[2], mutation)

    def test_spans_cross_intervals_without_summing(self):
        events, intervals, _, _ = report.join(fixture(), self.rows)
        result = report.worst_rows(events, intervals, self.rows)
        spans = result['worst'][0]['overlapping_spans']
        self.assertTrue(any(x['begin'] == 'capture_request' for x in spans))
        self.assertNotIn('total', result)

    def test_schema2_retains_exact_csv_join_and_does_not_invent_backend_end(self):
        data = fixture(); data[0]['schema'] = 2
        events, _, errors, _ = report.join(data, self.rows)
        self.assertEqual(errors, [])
        audit = report.scheduling_audit(events)
        self.assertEqual(audit['errors'], [])
        self.assertIn('inseparable', audit['backend_observation'])
        self.assertFalse(audit['acceptance'])

    def test_fixed_window_no_late_frame_trimming(self):
        rows = [dict(interval_ms='100'), dict(interval_ms='100'), dict(interval_ms='100')]
        result = report.compare(rows, rows, .25)
        self.assertEqual(result['off']['frames'], 3)
        self.assertEqual(result['off']['gt50'], 3)
        self.assertFalse(result['acceptance'])
        with self.assertRaises(ValueError): report.compare(rows, rows, 1)

if __name__ == '__main__': unittest.main()
