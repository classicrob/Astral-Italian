#!/usr/bin/env python3
"""Pure source tests. No compilation, graphics or subprocess test fixtures."""
import importlib.util
from pathlib import Path
import sys
import subprocess
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]

def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

patcher = load('trace_patch', ROOT / 'native/gx/patch_aurora_gpu_trace.py')
present = load('present_patch', ROOT / 'native/gx/patch_aurora_present.py')


def strip_gates(changed):
    while 'PETARI_GPU_TRACE(' in changed:
        start = changed.index('PETARI_GPU_TRACE(')
        position = start + len('PETARI_GPU_TRACE(')
        depth = 1
        quote = None
        escaped = False
        while depth:
            char = changed[position]
            if quote:
                if escaped: escaped = False
                elif char == chr(92): escaped = True
                elif char == quote: quote = None
            elif char in ('"', "'"): quote = char
            elif char == '(': depth += 1
            elif char == ')': depth -= 1
            position += 1
        assert changed[position] == ';'
        changed = changed[:start] + changed[position + 1:]
    return changed

class TracePatchTests(unittest.TestCase):
    def test_pinned_frame_worker_present(self):
        upstream = Path(sys.argv[1])
        for kind, path in [('frame', 'lib/gfx/frame.cpp'), ('worker', 'lib/gfx/render_worker.cpp'),
                           ('aurora', 'lib/aurora.cpp'), ('fifo', 'lib/gx/fifo.cpp')]:
            source = (upstream / path).read_text()
            if kind == 'aurora': source = present.patch_aurora(source)
            if kind == 'fifo':
                sync = load('trace_sync', ROOT / 'native/gx/patch_aurora_sync.py')
                source = sync.patch_fifo(source)
            result = patcher.patch(source, kind)
            self.assertIn('PETARI_GPU_TRACE(', result)
            original_tokens = re.sub(r'\s+', '', source)
            stripped = result.replace('#include <petari/gpu_trace.hpp>\n', '').replace('#include "gpu_trace_dawn.hpp"\n', '')
            self.assertEqual(re.sub(r'\s+', '', strip_gates(stripped)), original_tokens, kind)
            # Actual GPU API calls and original work closures are neither
            # duplicated nor replaced; new workdone registration lives in helper.
            for literal in ['g_queue.Submit(', 'item->work();', 'item.work();', 'encoder.Finish(']:
                self.assertEqual(result.count(literal), source.count(literal))
            with self.assertRaises(ValueError): patcher.patch(source + source, kind)
            with self.assertRaises(ValueError): patcher.patch(result, kind)

    def test_off_hooks_preserve_native_operation_tokens(self):
        # Strip only our gated statements/includes. Every pre-existing token in
        # these actual native paths must remain exactly the baseline token stream.
        # git show is read-only; this test compiles/runs no fixture.
        for path in ['native/gx/efb_recording.inc', 'native/gx/efb_frame.inc',
                     'native/gx/efb_snapshot.cpp', 'native/platform/gx_sync/gx_sync.cpp',
                     'native/platform/os/os_thread.cpp', 'native/platform/vi/vi_host.cpp',
                     'native/app/frame_seam.cpp', 'native/app/smoke_game.cpp',
                     'native/app/app_main.cpp', 'native/app/latency_report.cpp',
                     'native/app/frontend_prepare.cpp', 'native/platform/os/os_internal.hpp',
                     'native/platform/os/os_alarm.cpp', 'native/gx/pipeline_worker.inc']:
            original = subprocess.check_output(['git', 'show', '7fcc610c:' + path], cwd=ROOT, text=True)
            changed = (ROOT / path).read_text()
            changed = changed.replace('#include <petari/gpu_trace.hpp>\n', '').replace('#include "gpu_trace_dawn.hpp"\n', '')
            changed = changed.replace('namespace GpuTrace = PetariNative::GpuTrace;\n', '')
            changed = changed.replace('    PetariNative::GpuTrace::configure(std::getenv("PETARI_PIPELINE_DIAG"), std::getenv("PETARI_GPU_TRACE"),\n        std::getenv("PETARI_GPU_TRACE_BINDING_SHA256"));\n', '')
            changed = changed.replace('PresentStats::enabled() || PetariNative::GpuTrace::enabled.load(std::memory_order_relaxed)', 'PresentStats::enabled()')
            changed = strip_gates(changed)
            self.assertEqual(re.sub(r'\s+', '', original), re.sub(r'\s+', '', changed), path)

    def test_observer_is_guarded(self):
        source = (ROOT / 'native/gx/efb_frame.inc').read_text()
        self.assertIn('PETARI_GPU_TRACE(\n          PetariNative::GpuTrace::emit("submit_return"', source)
        self.assertIn('observeSubmission(g_queue, PetariNative::GpuTrace::workerContext)', source)
        header = (ROOT / 'native/include/petari/gpu_trace.hpp').read_text()
        macro = header[header.index('#define PETARI_GPU_TRACE'):]
        self.assertEqual(macro.count('enabled.load'), 1)
        self.assertLess(macro.index('enabled.load'), macro.index('__VA_ARGS__'))
        self.assertNotIn('getenv', macro)
        self.assertNotIn('nowNs', macro)

if __name__ == '__main__':
    unittest.main(argv=[sys.argv[0]])
