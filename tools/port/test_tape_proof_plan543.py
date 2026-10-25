#!/usr/bin/env python3
"""Pure Python source admission/hash negatives; no compiler or game process."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from run_tape_anchors543 import tape_binding, verify_tape_binding, qualified_tape, plan
from test_tape_power_audit import fixture as power_fixture
from capture_schema2_tape import sha

class ProofBindings(unittest.TestCase):
    def test_native_plan_has_no_legacy_conversion_and_all_arms_bind_policy(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='native-plan-',dir=root) as temporary:
            base=Path(temporary);builds=[]
            for name in ('candidate','ref072','ref002'):
                d=base/name;d.mkdir();builds.append(d)
                (d/'BUILD.json').write_text(json.dumps(dict(source=name,build_host='SERVER',
                    native_tests_passed=True,candidate_suite_passed=True)))
            out=base/'plan'
            plan(out,candidate=builds[0],reference072=builds[1],reference002=builds[2],schema2_only=True,power_protocol=True,native_script=True)
            spec=json.loads((out/'PLAN.json').read_text())
            self.assertEqual(spec['neutral_tail_policy'],'native-neutral-controls-v1')
            self.assertEqual(len(spec['arms']),11)
            self.assertEqual(spec['arms'][0]['name'],'record-native-ref072')
            self.assertEqual(spec['arms'][0]['kind'],'native-recording')
            self.assertTrue(spec['arms'][0]['tape'].endswith('worldmap-native543.script'))
            self.assertFalse(any(a['kind'] in ('legacy','conversion') or 'tape-worldmap-54v2' in a['tape'] for a in spec['arms']))
            self.assertIn('native-neutral-controls-v1.json',spec['source_hashes'])
            self.assertEqual(spec['builds']['ref072']['kind'],'server-instrumented-reference')
            self.assertEqual(spec['builds']['ref002']['kind'],'server-instrumented-reference')
            self.assertEqual(spec['builds']['candidate']['kind'],'server-candidate')
            for i,field in ((0,'candidate_suite_passed'),(0,'native_tests_passed'),(1,'native_tests_passed'),(2,'native_tests_passed')):
                path=builds[i]/'BUILD.json';original=path.read_text();bad=json.loads(original);bad[field]=False;path.write_text(json.dumps(bad))
                with self.subTest(build=i,field=field),self.assertRaises(RuntimeError):
                    plan(base/f'reject-{i}-{field}',candidate=builds[0],reference072=builds[1],reference002=builds[2],schema2_only=True,power_protocol=True,native_script=True)
                path.write_text(original)

    def test_power_conversion_native_report_is_bound_to_legacy_source(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='power-source-binding-',dir=root) as temporary:
            tape=Path(temporary)/'capture';source=Path(temporary)/'source'
            (tape/'replay').mkdir(parents=True);source.mkdir()
            (source/'route.tape.json').write_text(json.dumps(dict(schema=1)))
            for name in ('route.csv','route.kpd','route.actions'):(tape/name).write_text(name)
            (tape/'route.tape.json').write_text(json.dumps(power_fixture()))
            (tape/'launch.json').write_text(json.dumps(dict(environment=dict(
                PETARI_PIPELINE_DIAG='0',PETARI_TRACE_BOOT='0',PETARI_TAPE_PLAY=str(source)))))
            native=dict(finished=True,stopped='',mode='whole-session',interactive_position_bit_exact=True,
                anchor_policy='interactive-and-play-ready-v1',segments=[dict(tape_frames=3,rows_compared=4,
                identity_rows_skipped=0,pointer_rows_compared=3,identity_offset_checked=True,
                identity_offset_passed=True,identity_offset_limit=30,identity_frame_offset=0)])
            (tape/'replay/divergence.json').write_text(json.dumps(native))
            hashes={str(tape/n):sha(tape/n) for n in ('route.csv','route.kpd','route.actions','route.tape.json','replay/divergence.json')}
            hashes[str(source/'route.tape.json')]=sha(source/'route.tape.json')
            result=dict(passed=True,checks=dict(strict=True),input_proof=dict(passed=True,captured_schema=2,
                actual_payloads_independently_checked=True,source_schema=1,actual_reads=12,
                exact_recorded_reads=4,exact_neutral_reads=8,hashes=hashes))
            (tape/'RESULT.json').write_text(json.dumps(result))
            self.assertEqual(qualified_tape(tape,require_power=True)['source_schema'],1)
            (source/'route.tape.json').write_text(json.dumps(dict(schema=2)))
            with self.assertRaises(RuntimeError):qualified_tape(tape,require_power=True)

    def test_power_plan_has_one_conversion_and_no_failed_reference_legacy_dependency(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='power-plan-unit-',dir=root) as temporary:
            base=Path(temporary)
            builds=[]
            for name in ('candidate','ref072','ref002'):
                directory=base/name;directory.mkdir();builds.append(directory)
                (directory/'BUILD.json').write_text(json.dumps(dict(source=name,build_host='M1',
                    native_tests_passed=True,candidate_suite_passed=True)))
            out=base/'plan'
            plan(out,candidate=builds[0],reference072=builds[1],reference002=builds[2],
                 schema2_only=True,power_protocol=True)
            spec=json.loads((out/'PLAN.json').read_text())
            self.assertTrue(spec['power_protocol'])
            self.assertEqual(len(spec['arms']),11)
            self.assertEqual([a['name'] for a in spec['arms'] if a['kind']=='conversion'],['convert-ref072'])
            self.assertTrue(all(a['observe_seconds']==90 for a in spec['arms']))
            self.assertTrue(all('M1' in b['kind'] for b in spec['builds'].values()))
            for a in spec['arms']:
                if a['name'].startswith('reciprocal'):
                    self.assertEqual(a['tape'],str(out/'runs/matrix-ref002-fresh'))
            (builds[0]/'BUILD.json').write_text(json.dumps(dict(source='bad',build_host='M1',
                native_tests_passed=True,candidate_suite_passed=False)))
            with self.assertRaises(RuntimeError):
                plan(base/'bad',candidate=builds[0],reference072=builds[1],reference002=builds[2],
                     schema2_only=True,power_protocol=True)

    def test_canonical_and_reciprocal_admission_fail_closed(self):
        root=Path.home()/'MacioGalaxy2-gamedata/port53/m1g/solb'
        with tempfile.TemporaryDirectory(prefix='tape-proof-plan-unit-',dir=root) as temporary:
            tape=Path(temporary);(tape/'replay').mkdir()
            for name in ['route.csv','route.kpd','route.actions']:(tape/name).write_text(name)
            (tape/'route.tape.json').write_text(json.dumps(dict(schema=2,complete=True)))
            (tape/'launch.json').write_text(json.dumps(dict(environment=dict(PETARI_PIPELINE_DIAG='0',PETARI_TRACE_BOOT='0'))))
            (tape/'replay/divergence.json').write_text(json.dumps(dict(finished=True,stopped='')))
            binding=tape_binding(tape)
            result=dict(passed=True,checks=dict(strict=True),input_proof=dict(passed=True,captured_schema=2,
                actual_payloads_independently_checked=True,source_schema=1,actual_reads=4,
                exact_recorded_reads=3,exact_neutral_reads=1,
                hashes={str(tape/name):value for name,value in binding['files'].items()}))
            def write(value):(tape/'RESULT.json').write_text(json.dumps(value))
            write(result)
            verify_tape_binding(binding)
            self.assertEqual(qualified_tape(tape)['binding'],binding)
            with self.assertRaises((RuntimeError,ValueError)):qualified_tape(tape,require_power=True)
            with self.assertRaises(RuntimeError):qualified_tape(tape,source_schema=2)
            result['input_proof']['source_schema']=2;write(result)
            self.assertEqual(qualified_tape(tape,source_schema=2)['source_schema'],2)
            for name in binding['files']:
                original=(tape/name).read_bytes();(tape/name).write_bytes(original+b'changed')
                with self.subTest(name=name):
                    with self.assertRaises(RuntimeError):verify_tape_binding(binding)
                    with self.assertRaises((RuntimeError,ValueError)):qualified_tape(tape)
                (tape/name).write_bytes(original)
            for part,key,value in [('root','passed',False),('checks','strict',False),
                    ('input_proof','passed',False),('input_proof','actual_payloads_independently_checked',False)]:
                bad=copy.deepcopy(result);(bad if part=='root' else bad[part])[key]=value;write(bad)
                with self.subTest(part=part,key=key):
                    with self.assertRaises(RuntimeError):qualified_tape(tape)
            write(result)
            (tape/'launch.json').write_text(json.dumps(dict(environment=dict(PETARI_PIPELINE_DIAG='0',PETARI_TRACE_BOOT='1'))))
            with self.assertRaises(RuntimeError):qualified_tape(tape)

if __name__=='__main__':unittest.main()
