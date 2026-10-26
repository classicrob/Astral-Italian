import importlib.util
from pathlib import Path
import unittest
SPEC = importlib.util.spec_from_file_location("output", Path(__file__).resolve().parents[1] / "tools/audio_output_au_report.py")
M = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(M)


def good():
    return ("[audio-backend] driver=coreaudio sink=output-au subtype=DefaultOutput\n"
            + "[audio] level 100/200 frames, largest request 186, underrun 0 frames,\n" * 50
            + "[audio-workgroup] errors=0 over175ms=0 history_invalid=0 observing=0 adopted=1 qualifying=1\n"
            + "[audio-au] generation=1 final=1 device_rate=44100 guest_rate=32000 "
              "underrun_unit=guest-equivalent-cumulative-ceil raw_shortfall_device_frames=0 faults=0 timeline_faults=0 "
              "invalid_timestamps=0 overloads=0 healthy=1 ring_min_ms=42 callbacks=100 requested_device_frames=100 "
              "copied_device_frames=100 consumed_device_frames=100 source_pulled=100 source_accepted=100 device_produced=100\n"
            + "[audio-au-final] complete=1 raw_shortfall_device_frames=0 guest_equivalent_frames=0 health_errors=0 "
              "healthy=1 transitions=0 default_device_setter_calls=0\n")


class Tests(unittest.TestCase):
    def test_complete_zero_audio_requires_own_native_health(self):
        result = M.evaluate(iter(good().splitlines()), False)
        self.assertTrue(result["evidence_valid"], result)
        self.assertFalse(result["acceptance_claimed"])
        self.assertEqual(result["ring_min_ms"], 42)

    def test_full_ring_cannot_hide_timeline_or_native_faults(self):
        for a, b in (("timeline_faults=0", "timeline_faults=1"), ("raw_shortfall_device_frames=0", "raw_shortfall_device_frames=1"),
                     ("healthy=1", "healthy=0"), ("invalid_timestamps=0", "invalid_timestamps=1"),
                     ("copied_device_frames=100", "copied_device_frames=99"), ("health_errors=0", "health_errors=1"),
                     ("underrun 0", "underrun 1"), ("source_accepted=100", "source_accepted=101")):
            self.assertFalse(M.evaluate(good().replace(a, b, 1), False)["evidence_valid"], b)

    def test_incomplete_and_malformed_evidence_refused(self):
        for text in (good().split("[audio-au-final]")[0], good().replace("callbacks=100", "callbacks=broken"),
                     good().replace("[audio-backend]", "[not-real]"), good().replace("[audio]", "[not-a-report]", 1),
                     good() + "[audio-au-recovery] attempt=1\n"):
            self.assertFalse(M.evaluate(text, False)["evidence_valid"])

    def test_history_required_only_when_requested_but_loss_always_refused(self):
        self.assertFalse(M.evaluate(good(), True)["evidence_valid"])
        prefix = "[audio-workgroup-config] schema=2 clock=mach_absolute_time pre_ticks=2000 max_post_ticks=500\n"
        history = ("[audio-workgroup-history] begin trigger_mach=3000 end_mach=3400 rows=1 drops=0 unstable=0 "
                   "clipped=0 retention_unknown=0 rejected=0 rejected_omitted=0\n"
                   "[audio-workgroup-history] event seq=1 mach=2500 kind=43 thread=0 value=256 steady_ns=123 ticket=1 object=1\n"
                   "[audio-workgroup-history] end rows=1\n")
        text = prefix + good().replace("observing=0", "observing=1") + history
        self.assertTrue(M.evaluate(text, True)["evidence_valid"])
        for a, b in (("drops=0", "drops=1"), ("retention_unknown=0", "retention_unknown=1"),
                     ("end rows=1", "end rows=0"), ("steady_ns=123", "steady_ns=0")):
            self.assertFalse(M.evaluate(text.replace(a, b), True)["evidence_valid"])


if __name__ == "__main__":
    unittest.main()
