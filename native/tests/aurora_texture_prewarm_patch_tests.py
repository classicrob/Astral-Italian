#!/usr/bin/env python3
"""The HUD texture prewarm patch (native/gx/patch_aurora_texture_prewarm.py): it applies on top of the timing patch,
queues and drains before end_frame's stats, and refuses a source it does not recognise."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
AURORA = Path(sys.argv.pop(1))
TIMING = ROOT / 'native/gx/patch_aurora_texture_timing.py'
PREWARM = ROOT / 'native/gx/patch_aurora_texture_prewarm.py'


def run(script, source, output):
    return subprocess.run([sys.executable, str(script), str(source), str(output)], capture_output=True, text=True)


class TexturePrewarmPatch(unittest.TestCase):
    def test_applies_after_timing_patch(self):
        with tempfile.TemporaryDirectory() as tmp:
            gx = Path(tmp) / 'lib/gx'
            gx.mkdir(parents=True)
            timed, patched = gx / 'texture.cpp', Path(tmp) / 'patched.cpp'
            self.assertEqual(run(TIMING, AURORA / 'lib/gx/texture.cpp', timed).returncode, 0)
            self.assertEqual(run(PREWARM, timed, patched).returncode, 0)
            text = patched.read_text()
            self.assertIn('extern "C" void petari_gx_texture_prewarm(const void* texObj)', text)
            self.assertIn('extern "C" void petari_gx_texture_prewarm_cancel()', text)
            self.assertIn('PETARI_PREWARM_MS', text)  # per-frame time budget
            self.assertIn('kPetariPrewarmLifeFrames', text)  # stale batches are dropped
            end = text.index('void end_frame() noexcept {')
            self.assertLess(end, text.index('petari_drain_texture_prewarm();', end))
            self.assertLess(text.index('static void petari_drain_texture_prewarm()'), end)
            self.assertLess(text.index('resolve_static_texture(obj);', text.index('petari_drain_texture_prewarm()')), end)
            # In place, as the build runs it.
            self.assertEqual(run(PREWARM, timed, timed).returncode, 0)
            self.assertEqual(timed.read_text(), text)

    def test_refuses_unknown_source(self):
        with tempfile.TemporaryDirectory() as tmp:
            bad = Path(tmp) / 'texture.cpp'
            bad.write_text('#include <list>\nvoid something_else() {}\n')
            result = run(PREWARM, bad, Path(tmp) / 'out.cpp')
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('anchor mismatch', result.stderr)
            self.assertFalse((Path(tmp) / 'out.cpp').exists())


if __name__ == '__main__':
    unittest.main()
