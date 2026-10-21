import json
from pathlib import Path
import tempfile
import unittest

from kpad_records import digest
from yoshi_native import finalized_inputs, frozen_bundle


class FollowupTests(unittest.TestCase):
    def fixture(self, root):
        capture=root/'capture';capture.mkdir()
        for name in ('observed-inputs.jsonl','observed-inputs.kpd','inputs.jsonl','oracle.csv'):
            (capture/name).write_bytes(('synthetic '+name).encode())
        binding=dict(frames=3,path=str(capture/'observed-inputs.jsonl'),sha256=digest(capture/'observed-inputs.jsonl'),
                     kpd_sha256=digest(capture/'observed-inputs.kpd'))
        result=dict(status='verified diagnostic only',delivered_input=binding,
                    input_export_sha256=digest(capture/'inputs.jsonl'),oracle_sha256=digest(capture/'oracle.csv'))
        manifest=dict(status='captured; repeat qualification required',frames=4,
                      input_export=dict(sha256=result['input_export_sha256']),
                      sync=dict(stage='YosshiHomeGalaxy',scenario=1,rng_forced=False,source_frame=10),
                      raw_input_suffix=dict(source_start=11))
        (root/'result.json').write_text(json.dumps(result))
        (capture/'manifest.json').write_text(json.dumps(manifest))
        return result,manifest

    def test_actual_export_hashes_and_phase_required(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);self.fixture(root)
            self.assertEqual(finalized_inputs(root)[3],3)
            (root/'capture/inputs.jsonl').write_bytes(b'changed actual export')
            with self.assertRaisesRegex(ValueError,'artifact changed'): finalized_inputs(root)

    def test_original_synthetic_tape_cannot_replace_observed_export(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);result,_=self.fixture(root)
            original=root/'original.jsonl';original.write_bytes(b'synthetic original tape')
            result['delivered_input'].update(path=str(original),sha256=digest(original))
            (root/'result.json').write_text(json.dumps(result))
            with self.assertRaisesRegex(ValueError,'actual normalized'): finalized_inputs(root)

    def test_partial_wrong_phase_and_forced_rng_rejected(self):
        for mode in ('partial','wrongphase','forced','wrongtarget'):
            with self.subTest(mode=mode),tempfile.TemporaryDirectory() as directory:
                root=Path(directory);_,manifest=self.fixture(root)
                if mode=='partial':manifest['status']='incomplete'
                elif mode=='wrongphase':manifest['raw_input_suffix']['source_start']=10
                elif mode=='forced':manifest['sync']['rng_forced']=True
                elif mode=='wrongtarget':manifest['sync']['scenario']=2
                (root/'capture/manifest.json').write_text(json.dumps(manifest))
                with self.assertRaises(ValueError): finalized_inputs(root)

    def test_frozen_binary_runtime_and_support_are_bound(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'launcher').mkdir()
            for name in ('petari_headless','libclang_rt.asan_osx_dynamic.dylib'):(root/name).write_bytes(name.encode())
            support=root/'launcher/test.py';support.write_bytes(b'synthetic support')
            (root/'identity.json').write_text(json.dumps(dict(source='synthetic',binary_sha256=digest(root/'petari_headless'),
                                                            asan_sha256=digest(root/'libclang_rt.asan_osx_dynamic.dylib'))))
            (root/'launcher/source.json').write_text(json.dumps(dict(source='synthetic',files={'test.py':digest(support)})))
            self.assertEqual(frozen_bundle(root)[0]['source'],'synthetic')
            support.write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError,'support changed'): frozen_bundle(root)


if __name__=='__main__':unittest.main()
