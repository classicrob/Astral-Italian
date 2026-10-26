#!/usr/bin/env python3
"""Source-only exact-anchor/wait-preservation tests against generated sources."""
import importlib.util,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('patch',ROOT/'native/gx/patch_aurora_game_wait.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Tests(unittest.TestCase):
    def test_unknown_or_missing_anchors_refuse(self):
        for kind in ('aurora','fifo','worker','frame','unknown'):
            with self.assertRaises(ValueError):m.patch('',kind)
    def test_final_generated_sources_and_original_waits(self):
        if len(sys.argv)<2 or sys.argv[1].startswith('-'):self.skipTest('requires configured GX generated source directory')
        base=Path(sys.argv[1])
        pairs=[('aurora','present/lib/aurora.cpp'),('fifo','sync/lib/gx/fifo.cpp'),
               ('worker','allocation-safe/lib/gfx/render_worker.cpp'),('frame','ubershader/frame.cpp')]
        for kind,path in pairs:
            text=(base/path).read_text();out=m.patch(text,kind)
            self.assertEqual(out.count('GetCurrentTexture('),text.count('GetCurrentTexture('))
            self.assertEqual(out.count('g_surface.Present()'),text.count('g_surface.Present()'))
            self.assertEqual(out.count('m_notFull.wait('),text.count('m_notFull.wait('))
            self.assertEqual(out.count('sync->cv.wait('),text.count('sync->cv.wait('))
            self.assertNotIn('wait_for(',out.replace('m_notEmpty.wait_for(','')) if kind=='worker' else None
            self.assertIn('game_wait_history.hpp',out)
            with self.assertRaises(ValueError):m.patch(out,kind)
if __name__=='__main__':
    unittest.main(argv=[sys.argv[0]])
