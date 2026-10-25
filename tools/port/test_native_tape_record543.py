#!/usr/bin/env python3
import copy
import json
from pathlib import Path
import unittest
from native_tape_record543 import audit_tail,POLICY,LEGACY_POLICY
from tape_phase_audit import bootstrap
from test_tape_power_audit import fixture

class NativeTail(unittest.TestCase):
    def reads(self):
        sample=bootstrap(0,120)['samples'][0]
        return [dict(bootstrap(0,120),frame=4,ordinal=0,returned=3,samples=[sample]*3),
                dict(bootstrap(0,120),frame=5,ordinal=0,returned=4,samples=[sample]*4)]
    def test_every_native_sample_and_connection_constant_counts_may_advance(self):
        proof=audit_tail(fixture(),self.reads(),POLICY)
        self.assertEqual(proof['samples'],7);self.assertEqual(proof['sample_exemptions'],[])
    def test_legacy_still_requires_full_single_bootstrap(self):
        with self.assertRaises(ValueError):audit_tail(fixture(),self.reads(),LEGACY_POLICY)
    def reject_sample(self,at,value):
        rows=self.reads();sample=bytearray(rows[-1]['samples'][-1]);sample[at]=value;rows[-1]['samples'][-1]=bytes(sample)
        with self.assertRaises(ValueError):audit_tail(fixture(),rows,POLICY)
    def test_button(self):self.reject_sample(3,1)
    def test_stick_off_rest_no_deadzone(self):self.reject_sample(99,1)
    def test_pointer_valid(self):self.reject_sample(94,1)
    def test_changed_accel_even_without_speed(self):self.reject_sample(12,1)
    def test_one_differing_sample_field_not_exempted(self):self.reject_sample(236,1)
    def test_accel_event_on_first_sample_cannot_define_rest(self):
        for at in (28,120,176):
            rows=self.reads()
            for row in rows:
                for i,old in enumerate(row['samples']):
                    sample=bytearray(old);sample[at]=0x3f;sample[at+1]=0x80;row['samples'][i]=bytes(sample)
            with self.subTest(at=at),self.assertRaises(ValueError):audit_tail(fixture(),rows,POLICY)
    def test_connection_change(self):
        rows=self.reads();rows[-1]['device']=0
        with self.assertRaises(ValueError):audit_tail(fixture(),rows,POLICY)
    def test_unknown_policy(self):
        with self.assertRaises(ValueError):audit_tail(fixture(),self.reads(),'guessed-neutral')
    def test_policy_has_no_sample_field_exemptions(self):
        p=json.loads(Path(__file__).with_name('native-neutral-controls-v1.json').read_text())
        self.assertEqual(p['policy'],POLICY);self.assertEqual(p['tolerance'],0);self.assertEqual(p['sample_field_exemptions'],[])

if __name__=='__main__':unittest.main()
