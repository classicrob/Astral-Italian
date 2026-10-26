#!/usr/bin/env python3
"""Pure source guards; these do not substitute for server native/codegen tests."""
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[2]
def read(path): return (ROOT / "native" / path).read_text()

class Tests(unittest.TestCase):
    def test_api_is_async_and_members_are_explicit(self):
        cpp = read("platform/audio/audio_workgroup.cpp")
        self.assertIn("AudioWorkIntervalCreate(", cpp)
        self.assertIn("os_workgroup_interval_start(", cpp)
        self.assertIn("os_workgroup_interval_finish(", cpp)
        self.assertNotIn("kAudioDevicePropertyIOThreadOSWorkgroup", cpp)
        self.assertNotIn("kAudioOutputUnitProperty_OSWorkgroup", cpp)
        self.assertIn("identity == callbackCaller", cpp)
        self.assertIn("identity == startCaller", cpp)
        self.assertNotIn("priority ==", cpp)  # readback metadata is not a membership predicate
    def test_tls_helpers_are_outlined_and_public_header_clean(self):
        header = read("platform/include/petari/platform/audio_workgroup.hpp")
        for forbidden in ("thread_local", "CoreFoundation", "CoreAudio/", "mach/", "IOKit"):
            self.assertNotIn(forbidden, header)
        cpp = read("platform/audio/audio_workgroup.cpp")
        for name in ("guestSafePoint", "guestExit", "rendererSafePoint", "rendererExit", "beginCycle", "subframeComplete"):
            self.assertIn("__attribute__((noinline)) void " + name, cpp)
    def test_pause_and_control_keep_original_baton_predicate(self):
        source = read("platform/os/os_thread.cpp")
        body = source[source.index("void waitForCpu("):source.index("void requireRunningOsThread(")]
        self.assertIn("hostWait(host->cv, [&] { return gCurrent.load(std::memory_order_acquire) == self || host->terminated; });", body)
        self.assertIn("if (gCurrent.load(std::memory_order_acquire) == self) break;", body)
        self.assertNotIn("wait_until", body)
        self.assertNotIn("sleep_for", body)
        self.assertIn("pauseProduction(true)", read("platform/audio/ai_host.cpp"))
    def test_dispatchers_and_ring_producer_not_changed_to_joiners(self):
        ai = read("platform/audio/ai_host.cpp")
        irq = ai[ai.index("void* interruptThreadMain("):ai.index("void raiseInterrupt(")]
        self.assertNotIn("Workgroup::", irq)
        dsp = read("platform/audio/dsp_device.cpp")
        irq = dsp[dsp.index("void* interruptMain("):dsp.index("}  // namespace",dsp.index("void* interruptMain("))]
        self.assertNotIn("WG::", irq)
        sink = read("audio/sdl_sink.cpp")
        producer = sink[sink.index("void produce("):sink.index("void SDLCALL fill(")]
        self.assertNotIn("Workgroup::", producer)
    def test_exact_single_read_and_no_rt_io_in_history(self):
        sink = read("audio/sdl_sink.cpp")
        fill = sink[sink.index("void SDLCALL fill("):sink.index("void stop(")]
        self.assertEqual(fill.count("GpuTrace::Audio::read(ring, buffer, static_cast<std::size_t>(count))"),1)
        self.assertIn("WG::noteShortfall(count-copied)",fill)
        self.assertNotIn("takeUnderrunFrames",fill)
        history = read("platform/audio/workgroup_history.hpp")
        for forbidden in ("fprintf", "fwrite", "std::mutex", "new ", "sleep", "while ("):
            self.assertNotIn(forbidden,history)
        self.assertIn("static_assert(std::atomic<std::uint64_t>::is_always_lock_free",history)
    def test_deadlines_use_render_completion_not_dma_registration(self):
        ai = read("platform/audio/ai_host.cpp")
        body = ai[ai.index("void AIInitDMA("):ai.index("uintptr_t AIGetDMAStartAddr(")]
        self.assertNotIn("finish", body)
        self.assertIn("noteRegistration",body)
        dsp = read("platform/audio/dsp_device.cpp")
        dsp = dsp[dsp.index("void renderAudio("):dsp.index("void runPendingCommands(")]
        self.assertLess(dsp.index("d.renderer.finalizeFrame("),dsp.index("WG::subframeComplete()"))
        self.assertLess(dsp.index("WG::subframeComplete()"),dsp.index("pushMails(d, {kDspSync"))

if __name__ == "__main__": unittest.main()
