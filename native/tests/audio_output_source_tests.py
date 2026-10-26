"""Pure source guards. Release linked-callee proof is independently mandatory."""
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[1]


class Tests(unittest.TestCase):
    def test_render_has_no_host_or_guest_service_path(self):
        render = (ROOT / "audio/output_au_render.cpp").read_text()
        for forbidden in ("thread_local", "HostAllocation", "pthread_", "thread_info", "mach_", "malloc(", "new ",
                          "SDL_", "AudioConverter", "AudioUnitGet", "AudioUnitSet", "::pull", "printf", "fwrite", "mutex", "wait("):
            self.assertNotIn(forbidden, render, forbidden)
        self.assertLess(render.index("const bool on=d->observing"), render.index("d->clock()"))
        self.assertIn("if (!d || !frames)", render)
        self.assertIn("BadTimeline", render)
        self.assertIn("frames>d->maxFrames", render)

    def test_default_is_sdl_and_no_system_device_setter_exists(self):
        sink = (ROOT / "audio/sdl_sink.cpp").read_text()
        self.assertIn('selector && !std::strcmp(selector,"output-au")', sink)
        self.assertIn('else Platform::Audio::setSink({start, stop, nullptr})', sink)
        control = (ROOT / "audio/output_au_control.cpp").read_text()
        self.assertIn("kAudioUnitSubType_DefaultOutput", control)
        self.assertNotIn("AudioObjectSetPropertyData", control)
        self.assertNotIn("SDL_OpenAudio", control)
        self.assertIn("r.globalMinimum", control)

    def test_rt_publisher_does_not_delegate_to_old_observer(self):
        body = (ROOT / "platform/audio/audio_workgroup_rt.cpp").read_text()
        for forbidden in ("pthread_", "thread_local", "sampleHost", "steadyNs()", "mach_absolute_time()", "noteTransport", "event("):
            self.assertNotIn(forbidden, body)
        self.assertIn("audioHistory.push", body)
        self.assertIn("audioHistory.trigger", body)

    def test_actual_production_render_is_guarded_including_first_and_error_paths(self):
        test = (ROOT / "tests/audio_output_render_tests.cpp").read_text()
        self.assertIn("const auto result=render(", test)
        for required in ("petari_au_guard_enter", "violations==0", "publishRenderEvent", "observerClock", "first", "OverLimit", "Reentrant", "BadTimeline"):
            self.assertIn(required.lower(), test.lower())

    def test_interposer_startup_uses_no_tls_and_explicit_armed_boundary(self):
        guard = (ROOT / "tests/audio_output_rt_guard.cpp").read_text()
        self.assertNotIn("thread_local", guard)
        self.assertNotIn("pthread_getspecific", guard)
        self.assertIn("std::atomic<pthread_t> armedThread{nullptr}", guard)
        self.assertIn("if(target) markArmed(target)", guard)
        self.assertIn("__attribute__((noinline)) void markArmed", guard)
        startup = (ROOT / "tests/audio_output_guard_startup_tests.cpp").read_text()
        for required in ("constructor", "mach_timebase_info", "startupPassed", "petari_au_guard_leave()>=3"):
            self.assertIn(required, startup)


if __name__ == "__main__":
    unittest.main()
