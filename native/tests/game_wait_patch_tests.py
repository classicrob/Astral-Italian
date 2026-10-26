#!/usr/bin/env python3
"""Source-only exact-anchor/wait-preservation tests against generated sources."""
import importlib.util,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('patch',ROOT/'native/gx/patch_aurora_game_wait.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class Tests(unittest.TestCase):
    def test_unknown_or_missing_anchors_refuse(self):
        for kind in ('aurora','fifo','worker','frame','unknown'):
            with self.assertRaises(ValueError):m.patch('',kind)
    def test_frame_generation_is_single_pass(self):
        # Use the current append fragment, not a retained generated TU: that
        # distinction caught the configure regression caused by an early include.
        fragment=(ROOT/'native/gx/efb_frame.inc').read_text()
        self.assertNotIn('game_wait_history',fragment)
        self.assertNotIn('PETARI_GAME_WAIT',fragment)
        # Minimal upstream frame anchors exercise readback's actual composition
        # and both later generators, without CMake, a compiler or a GPU.
        upstream='''void begin() {
  const size_t frameSlot = acquire_frame_slot();
  const auto stagingSlot = acquire_mapped_staging_buffer();
  frame.stagingBuffer = *stagingSlot;
  frame.frameId = g_nextFrameId++;
  begin_pipeline_frame();
    webgpu::gpu_prof::frame_begin(g_framePackets[frameSlot].encoder);
    auto& packet = g_framePackets[frameSlot];
}
void end() {
  const size_t stagingSlot = frame.stagingBuffer;
}
'''
        self.check_frame_chain(upstream)
    def test_pinned_frame_generator_chain(self):
        if len(sys.argv)<3:self.skipTest('requires pinned Aurora source directory')
        source=Path(sys.argv[2])/'lib/gfx/frame.cpp'
        before=source.read_bytes()
        self.check_frame_chain(before.decode())
        self.assertEqual(source.read_bytes(),before)
    def check_frame_chain(self,upstream):
        with tempfile.TemporaryDirectory() as temp:
            base=Path(temp)
            source=base/'original/frame.cpp';source.parent.mkdir();source.write_text(upstream)
            for script,folder,args in (
                ('readback','readback',()), ('geometry','geometry',()),
                ('ubershader','ubershader',('frame',str(base),str(base/'pipeline.hpp'))),
            ):
                output=base/folder/'frame.cpp'
                subprocess.run([sys.executable,str(ROOT/f'native/gx/patch_aurora_{script}.py'),
                                str(source),str(output),*args],check=True,capture_output=True,text=True)
                source=output
                self.assertNotIn('game_wait_history.hpp',source.read_text())
            text=source.read_text();out=m.patch(text,'frame')
        self.assertEqual(out.count('#include <petari/game_wait_history.hpp>'),1)
        self.assertEqual(out.count('::Kind::Capture'),2)
        self.assertEqual(out.count('::Kind::Staging'),2)
        for name in ('Worker','FrameSlot','Submit','RenderSync'):
            self.assertEqual(out.count(f'::Kind::{name}'),1)
        for call in ('acquire_frame_slot();','acquire_mapped_staging_buffer();',
                     'g_queue.Submit(1, &commands);','completion.get();'):
            self.assertEqual(out.count(call),text.count(call))
        self.assertIn('return '+m.N+'::invoke('+m.N+'::Kind::Staging, 0, 1, [&] { return acquire_mapped_staging_buffer(); });',out)
        with self.assertRaisesRegex(ValueError,'already instrumented'):m.patch(out,'frame')
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
