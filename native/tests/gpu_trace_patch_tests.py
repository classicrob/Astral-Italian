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


def operation_tokens(changed):
    # This erasure is deliberately limited to the existing trace gates, includes
    # and startup configuration. No MonitorWait code is removed here.
    changed = changed.replace('#include <petari/gpu_trace.hpp>\n', '').replace('#include "gpu_trace_dawn.hpp"\n', '')
    changed = changed.replace('namespace GpuTrace = PetariNative::GpuTrace;\n', '')
    changed = changed.replace('#include "interrupt_trace.hpp"\n', '')
    changed = changed.replace('#include <petari/gpu_trace_audio.hpp>\n', '')
    changed = changed.replace('PetariNative::GpuTrace::Audio::takeUnderruns(ring)', 'ring.takeUnderrunFrames()')
    changed = changed.replace('PetariNative::GpuTrace::Audio::takeUnderruns(ring, "shutdown")', 'ring.takeUnderrunFrames()')
    changed = changed.replace('PetariNative::GpuTrace::Audio::read(ring, buffer, static_cast<std::size_t>(count))',
                              'ring.read(buffer, static_cast<std::size_t>(count))')
    # The enabled branch uses the SAME untimed std::condition_variable and
    # predicate. Keep the literal original fallback as the erasure target.
    changed = changed.replace('    if (PetariNative::GpuTrace::enabled.load(std::memory_order_relaxed)) {\n        PetariNative::GpuTrace::detail::invokeEnabled([&] { InterruptTrace::wait(cv, lock, predicate); });\n    } else {\n        cv.wait(lock, predicate);\n    }', '    cv.wait(lock, predicate);')
    changed = changed.replace('    PetariNative::GpuTrace::configure(std::getenv("PETARI_PIPELINE_DIAG"), std::getenv("PETARI_GPU_TRACE"),\n        std::getenv("PETARI_GPU_TRACE_BINDING_SHA256"));\n', '')
    changed = changed.replace('PresentStats::enabled() || PetariNative::GpuTrace::enabled.load(std::memory_order_relaxed)', 'PresentStats::enabled()')
    changed = strip_gates(changed)
    return re.sub(r'\s+', '', changed)

def committed_source(revision, path):
    return subprocess.check_output(['git', 'show', revision + ':' + path], cwd=ROOT, text=True)

# PORT approved these functional monitor changes independently of tracing. Only
# this one path gets that exact, immutable native baseline. Keep the earlier
# trace-only erasure proof as well; never skip the path or remove MonitorWait
# lines by name/pattern. Any ungated token outside this approved revision fails.
MONITOR_PATH = 'native/platform/os/os_thread.cpp'
TRACE_ONLY_REVISION = 'e2f71c485c794bd9283f58f962a1baff08045379'
APPROVED_MONITOR_REVISION = '0970356053d0d1f3a52bba72711151f6b339a365'
APPROVED_WORKGROUP_REVISION = '472665264e094e3b31c9483089a190a46175c27b'
WORKGROUP_PATHS = {'native/platform/os/os_thread.cpp', 'native/platform/os/os_internal.hpp',
                   'native/platform/os/os_interrupts.cpp', 'native/audio/sdl_sink.cpp'}

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

    def test_trace_headers_do_not_leak_framework_types_or_enter_nested_namespace(self):
        host = (ROOT / 'native/include/petari/gpu_trace_host.hpp').read_text()
        self.assertNotIn('#include <CoreFoundation/', host)
        self.assertNotIn('#include <IOKit/', host)
        self.assertNotIn('CFTypeRef', host)
        worker = (ROOT / 'native/gx/pipeline_worker.inc').read_text()
        self.assertNotIn('#include <petari/gpu_trace', worker)
        fixture = (ROOT / 'native/tools/pipeline_worker_tests.cpp').read_text()
        self.assertEqual(fixture.count('#include <petari/gpu_trace.hpp>'), 1)
        before_fixture = fixture[:fixture.index('#include <petari/gpu_trace.hpp>')]
        self.assertEqual(before_fixture.count('{'), before_fixture.count('}'))
        self.assertLess(fixture.index('#include <petari/gpu_trace.hpp>'), fixture.index('namespace absl'))
        pipeline = load('trace_pipeline', ROOT / 'native/gx/patch_aurora_pipeline.py')
        source = (Path(sys.argv[1]) / 'lib/gfx/pipeline_cache.cpp').read_text()
        sys.path.insert(0, str(ROOT / 'native/gx'))
        try:
            generated = pipeline.patch(source)
        finally:
            sys.path.pop(0)
        self.assertEqual(generated.count('#include <petari/gpu_trace.hpp>'), 1)
        before = generated[:generated.index('#include <petari/gpu_trace.hpp>')]
        self.assertEqual(before.count('{'), before.count('}'), 'trace include must be at file scope')
        header = (ROOT / 'native/include/petari/gpu_trace.hpp').read_text()
        self.assertIn('return ::PetariNative::FrameTelemetry::nowNs();', header)

    def test_off_hooks_preserve_native_operation_tokens(self):
        # Strip only our gated statements/includes. Every native operation must
        # match the pinned baseline, including the separately approved K/R delta.
        # git show is read-only; this test compiles/runs no fixture.
        for path in ['native/gx/efb_recording.inc', 'native/gx/efb_frame.inc',
                     'native/gx/efb_snapshot.cpp', 'native/platform/gx_sync/gx_sync.cpp',
                     'native/platform/os/os_thread.cpp', 'native/platform/vi/vi_host.cpp',
                     'native/app/frame_seam.cpp', 'native/app/smoke_game.cpp',
                     'native/app/app_main.cpp', 'native/app/latency_report.cpp',
                     'native/app/frontend_prepare.cpp', 'native/platform/os/os_internal.hpp',
                     'native/platform/os/os_alarm.cpp', 'native/platform/os/os_interrupts.cpp', 'native/gx/pipeline_worker.inc',
                     'native/audio/sdl_sink.cpp']:
            original = subprocess.check_output(['git', 'show', '7fcc610c:' + path], cwd=ROOT, text=True)
            changed = (ROOT / path).read_text()
            if path == MONITOR_PATH:
                trace_only = committed_source(TRACE_ONLY_REVISION, path)
                self.assertEqual(re.sub(r'\s+', '', original), operation_tokens(trace_only),
                                 'original trace-only OS hook-erasure proof')
                original = committed_source(APPROVED_MONITOR_REVISION, path)
            if path in WORKGROUP_PATHS:
                # The separately approved functional workgroup arm is not trace
                # code. Pin it exactly; do not erase it by a name/pattern.
                # Keep the old trace-only erasure proof independently below.
                original = committed_source(APPROVED_WORKGROUP_REVISION, path)
            self.assertEqual(operation_tokens(original), operation_tokens(changed), path)

    def test_workgroup_baseline_retains_original_trace_erasure(self):
        for path in WORKGROUP_PATHS:
            base = APPROVED_MONITOR_REVISION if path == MONITOR_PATH else '7fcc610c'
            self.assertEqual(operation_tokens(committed_source(base, path)),
                             operation_tokens(committed_source('de3dfa77', path)), path)
        baseline = committed_source(APPROVED_WORKGROUP_REVISION, MONITOR_PATH)
        expected = operation_tokens(baseline)
        self.assertNotEqual(expected, operation_tokens(baseline.replace('AudioWorkgroup::guestSafePoint',
                                                                       'AudioWorkgroup::rendererSafePoint')))

    def test_monitor_baseline_rejects_ungated_changes(self):
        approved = committed_source(APPROVED_MONITOR_REVISION, MONITOR_PATH)
        expected = operation_tokens(approved)
        # Both the old semaphore path and the approved experimental path remain
        # protected. An ungated trace operation must also remain visible.
        for before, after in [
                ('semaphore_signal(wake)', 'semaphore_signal(MACH_PORT_NULL)'),
                ('MonitorWait::waitCritical(ns, forcedPending)', 'MonitorWait::waitCritical(ns, false)'),
                ('MonitorWait::configure();', 'MonitorWait::configure(); PetariNative::GpuTrace::nextId();')]:
            self.assertEqual(approved.count(before), 1, before)
            self.assertNotEqual(expected, operation_tokens(approved.replace(before, after)), before)
        self.assertEqual(expected, operation_tokens(approved + '\nPETARI_GPU_TRACE(PetariNative::GpuTrace::nextId());\n'))

    def test_interrupt_wait_erasure_does_not_hide_a_changed_fallback(self):
        path = 'native/platform/os/os_internal.hpp'
        approved = committed_source(APPROVED_WORKGROUP_REVISION, path)
        changed = (ROOT / path).read_text()
        self.assertEqual(operation_tokens(approved), operation_tokens(changed))
        changed = changed.replace('        cv.wait(lock, predicate);', '        cv.wait(lock, [] { return true; });')
        self.assertNotEqual(operation_tokens(approved), operation_tokens(changed))
        helper = (ROOT / 'native/platform/os/interrupt_trace.hpp').read_text()
        self.assertIn('cv.wait(lock, [&]', helper)
        self.assertNotIn('wait_until(', helper)
        self.assertNotIn('condition_variable_any', helper)
        self.assertNotIn('sleep_for(', helper)
        self.assertLess(helper.index('if (!initial) cvReacquired'), helper.index('const bool ready = predicateFn();'))
        self.assertIn('if (!ready) cvReleasePending', helper)
        host = (ROOT / path).read_text()
        guarded = host[host.index('if (PetariNative::GpuTrace::enabled.load'):]
        self.assertLess(guarded.index('enabled.load'), guarded.index('invokeEnabled'))

    def test_audio_trace_preserves_ring_arguments_and_off_fallback(self):
        path = 'native/audio/sdl_sink.cpp'
        changed = (ROOT / path).read_text()
        original = committed_source(APPROVED_WORKGROUP_REVISION, path)
        self.assertEqual(operation_tokens(original), operation_tokens(changed))
        wrong = changed.replace('Audio::read(ring, buffer, static_cast<std::size_t>(count))',
                                'Audio::read(ring, buffer, static_cast<std::size_t>(count + 1))')
        self.assertNotEqual(operation_tokens(original), operation_tokens(wrong))
        helper = (ROOT / 'native/include/petari/gpu_trace_audio.hpp').read_text()
        self.assertIn('} else {\n        copied = ring.read(output, count);\n    }', helper)
        self.assertIn('return copied;', helper)
        self.assertIn('return ring.takeUnderrunFrames();', helper)
        self.assertIn('[[gnu::noinline]] inline std::uint64_t reportEnabled', helper)
        guarded = helper[helper.index('inline std::size_t read('):helper.index('template<class Ring>\n[[gnu::noinline]]', helper.index('inline std::size_t read('))]
        self.assertLess(guarded.index('enabled.load'), guarded.index('invokeEnabled'))
        self.assertLess(guarded.index('invokeEnabled'), guarded.index('nowNs()'))
        self.assertNotIn('std::fwrite', helper)
        self.assertNotIn('std::fprintf', helper)
        self.assertNotIn('setRealtime', helper)
        self.assertIn('[[gnu::noinline]] inline void callbackBeginEnabled', helper)
        self.assertIn('[[gnu::noinline]] inline void callbackEndEnabled', helper)
        self.assertEqual(changed.count('PETARI_GPU_TRACE(PetariNative::GpuTrace::Audio::callbackBeginEnabled'), 1)
        self.assertEqual(changed.count('PETARI_GPU_TRACE(PetariNative::GpuTrace::Audio::callbackEndEnabled'), 2)
        fill = changed[changed.index('void SDLCALL fill('):changed.index('void stop(')]
        self.assertLess(fill.index('callbackBeginEnabled'), fill.index('HostAllocationScope'))

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
        self.assertIn('detail::invokeEnabled([&]() { __VA_ARGS__; })', macro)
        self.assertLess(macro.index('enabled.load'), macro.index('invokeEnabled'))
        self.assertIn('[[gnu::noinline]] inline void invokeEnabled', header)
        helper = header[header.index('namespace detail {'):header.index('} // namespace detail')]
        self.assertIn('body();', helper)
        self.assertNotIn('std::function', helper)
        self.assertNotIn('new ', helper)

if __name__ == '__main__':
    unittest.main(argv=[sys.argv[0]])
