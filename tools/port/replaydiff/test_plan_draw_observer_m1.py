import copy
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path
from plan_draw_observer_m1 import plan,PINS

class PlanTests(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup)
        self.home_patch=patch('plan_draw_observer_m1.Path.home',return_value=Path(self.tmp.name).resolve());self.home_patch.start();self.addCleanup(self.home_patch.stop)
        self.root=Path(self.tmp.name)/'MacioGalaxy2-gamedata';self.root.mkdir()
        self.source=self.root/'source';folder=self.source/'native/app/visual';folder.mkdir(parents=True)
        for name in ('draw_observer.cpp','draw_observer_json.cpp','draw_observer_tests.cpp','draw_pilot_bridge.cpp','draw_pilot_bridge_tests.cpp','draw_snapshot.cpp','draw_snapshot_no_xfb.cpp','draw_pilot_host.cpp','draw_observer.hpp','draw_snapshot_no_xfb.hpp','draw_pilot_bridge.hpp'):(folder/name).write_text('synthetic source-only plan input\n')
        self.context=dict(inputs=copy.deepcopy(PINS['inputs']),entry=dict(directory='/pinned-build',file='/pinned-encoded/GameSystem.cpp',arguments=[PINS['inputs'][0]['path'],'-DGALAXY_NATIVE=1','-nostdinc++','-nostdlib++','-std=gnu++17','-ffp-contract=off','-fno-strict-aliasing','-fno-rtti','-isysroot','/pinned-sdk','-o','GameSystem.cpp.o','-c','/pinned-encoded/GameSystem.cpp']))
    def make(self,context=None):return plan(context or self.context,self.source,self.root/'fresh-output')
    def test_plan_has_no_execution_grant_and_exact_bounded_jobs(self):
        result=self.make();self.assertEqual(len(result['commands']),14)
        self.assertEqual(result['limits']['wall_seconds'],120);self.assertEqual(result['limits']['parallel_jobs'],1)
        self.assertEqual(result['status'],'SOURCE_PLAN_ONLY_EXECUTION_BLOCKED');self.assertEqual(result['controls_status'],'UNRUN')
        self.assertIn('BLOCKED',result['runner']);self.assertIsNone(result['qualified_frames'])
        self.assertFalse((self.root/'fresh-output').exists())
        for command in result['commands']:
            if command['label'].startswith('compile-'):
                self.assertIn('-MD',command['argv']);self.assertNotIn('-MMD',command['argv'])
            if command['label'].startswith('link-'):
                self.assertEqual(sum(arg.endswith('.a') for arg in command['argv']),3);self.assertIn('-nostdlib++',command['argv'])
    def test_foreign_compiler_or_runtime_rejected(self):
        c=copy.deepcopy(self.context);c['entry']['arguments'][0]='/usr/bin/clang++'
        with self.assertRaises(ValueError):self.make(c)
        c=copy.deepcopy(self.context);c['inputs'][2]['sha256']='0'*64
        with self.assertRaises(ValueError):self.make(c)
    def test_missing_fp_or_native_policy_rejected(self):
        for token in ('-ffp-contract=off','-DGALAXY_NATIVE=1','-nostdlib++'):
            c=copy.deepcopy(self.context);c['entry']['arguments'].remove(token)
            with self.assertRaises(ValueError):self.make(c)
    def test_ambiguous_command_and_missing_source_rejected(self):
        c=copy.deepcopy(self.context);c['entry']['arguments'][1:1]=['-o','foreign']
        with self.assertRaises(ValueError):self.make(c)
        (self.source/'native/app/visual/draw_snapshot.cpp').unlink()
        with self.assertRaises(ValueError):self.make()
    def test_existing_output_cannot_be_reused(self):
        (self.root/'fresh-output').mkdir()
        with self.assertRaises(ValueError):self.make()

if __name__=='__main__':unittest.main()
