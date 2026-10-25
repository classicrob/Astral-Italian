#!/usr/bin/env python3
"""Pure Python source admission/hash negatives; no compiler or game process."""
import copy
import json
from pathlib import Path
import tempfile
import unittest
from run_tape_anchors543 import tape_binding, verify_tape_binding, qualified_tape

class ProofBindings(unittest.TestCase):
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
