#!/usr/bin/env python3
"""Pure source/control audit negatives; no C++ compiler or app launch."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from tape_phase_audit import POWER_PROTOCOL, POWER_BOUNDARY, power_events, load_meta, event_offset, audit_inputs


def fixture():
    def event(i,kind,row,ack):
        return dict(id=i,kind=kind,phase_id=0,after_route_row=row,before_input_frame=row+1,
                    boundary=POWER_BOUNDARY,callback_observed=ack,neutral_elapsed_seconds=180 if ack else 0)
    return dict(schema=2,complete=True,seams=12,input_phases=[dict(first_input_frame=3,end_input_frame_exclusive=8)],
                session_control_protocol=POWER_PROTOCOL,session_control_error='',
                session_events=[event(0,'neutral-tail-begin',3,False),event(1,'power-request',5,True)])


class PowerAudit(unittest.TestCase):
    def test_valid_explicit_events_and_legacy_label(self):
        self.assertEqual(len(power_events(fixture())),2)
        self.assertEqual(power_events(dict(schema=2)),[])

    def test_missing_ambiguous_early_late_order_and_unknown_versions(self):
        changes=[('session_events',[]),('session_control_protocol','unknown'),('session_control_error','ambiguous')]
        for field,value in changes:
            bad=fixture();bad[field]=value
            with self.subTest(field=field),self.assertRaises(ValueError):power_events(bad)
        for field,value in [('before_input_frame',7),('after_route_row',2),('after_route_row',8),
                            ('boundary','host-clock-guess'),('callback_observed',False),('phase_id',1),('id',0),
                            ('kind','reset-guess'),('neutral_elapsed_seconds',float('nan'))]:
            bad=fixture();bad['session_events'][1][field]=value
            with self.subTest(field=field,value=value),self.assertRaises(ValueError):power_events(bad)
        bad=fixture();bad.pop('session_control_protocol')
        with self.assertRaises(ValueError):power_events(bad)
        bad=fixture();bad['session_events'].append(copy.deepcopy(bad['session_events'][1]))
        with self.assertRaises(ValueError):power_events(bad)

    def test_shutdown_raw_bytes_and_connection_are_recorded_not_replaced(self):
        meta=fixture()
        report=dict(finished=True,anchor_policy='game-defined-scene-input-v2',input_phases=[dict(frame_offset=1)],
                    segments=[dict(scene_anchor_frame_offset=1)])
        canonical=[dict(frame=9,ordinal=0,channel=0,capacity=120,returned=0,error=-2,probe=-1,device=253,samples=[])]
        actual=[dict(canonical[0],frame=10)]
        mapping=[dict(actual_frame=10,actual_ordinal=0,source='recorded-shutdown',canonical_frame=9,
                      canonical_ordinal=0,phase=0,channel=0,capacity=120,returned=0,error=-2,samples=0)]
        self.assertEqual(audit_inputs(canonical,actual,mapping,report,meta)['exact_recorded_reads'],1)
        bad=copy.deepcopy(actual);bad[0]['device']=1
        with self.assertRaises(ValueError):audit_inputs(canonical,bad,mapping,report,meta)
        bad=copy.deepcopy(mapping);bad[0]['canonical_frame']=10
        with self.assertRaises(ValueError):audit_inputs(canonical,actual,bad,report,meta)
        with self.assertRaises(ValueError):audit_inputs(canonical,actual,mapping,report)
        with self.assertRaises(ValueError):audit_inputs(canonical,actual,[],report,meta)

    def test_duplicate_json_ordering_cannot_be_silently_selected(self):
        with tempfile.TemporaryDirectory(prefix='power-json-unit-') as temp:
            path=Path(temp)/'meta.json';text=json.dumps(fixture())
            path.write_text(text.replace('"after_route_row": 5','"after_route_row": 5, "after_route_row": 5'))
            with self.assertRaises(ValueError):load_meta(path)

    def test_event_uses_same_approved_play_ready_offset_as_samples_and_state(self):
        report=dict(input_phases=[dict(frame_offset=1)],segments=[dict(play_ready_state_check=dict(
            checked=True,recorded_frame=4,frame_offset=3))])
        event=fixture()['session_events'][1]
        self.assertEqual(event_offset(report,event),3)
        event['after_route_row']=3;self.assertEqual(event_offset(report,event),1)


if __name__=='__main__':unittest.main()
