#!/usr/bin/env python3
"""Pure Python; no compile, subprocess fixtures, devices, or cache access."""
import importlib.util
from pathlib import Path
import unittest
spec = importlib.util.spec_from_file_location("report", Path(__file__).resolve().parents[1] / "tools/audio_workgroup_report.py")
report = importlib.util.module_from_spec(spec)
spec.loader.exec_module(report)
BASE = "[audio-backend] driver=coreaudio\n" + "[audio] level 2/3 frames, largest request 4, underrun 0 frames, AI replayed 0 blocks\n" * 50
STATUS = "[audio-workgroup] mode=async errors=0 over175ms=0 history_invalid=0 observing=1 adopted=1 qualifying=1 guest=1000 host_generation=7 guest_host=2000 renderer_host=3000 completed=1 samples=560 sessions=1 guest_effective_computation_us=1000 renderer_effective_computation_us=1000\n"
CONFIG = "[audio-workgroup-config] schema=1 clock=mach_absolute_time pre_ticks=2000\n"
DUMP = "[audio-workgroup-history] begin trigger_mach=3000 rows=1 drops=0 unstable=0 clipped=0\n[audio-workgroup-history] event seq=1 mach=2000 kind=4 thread=3000 value=20\n[audio-workgroup-history] end rows=1\n"

class Tests(unittest.TestCase):
    def test_valid_is_not_acceptance(self):
        result = report.evaluate(BASE + CONFIG + STATUS + DUMP)
        self.assertTrue(result["evidence_valid"])
        self.assertFalse(result["acceptance_claimed"])
    def test_underrun_is_failure(self):
        self.assertFalse(report.evaluate((BASE + CONFIG + STATUS).replace("underrun 0", "underrun 1", 1))["evidence_valid"])
        self.assertFalse(report.evaluate(BASE + CONFIG + STATUS + "SDL audio underrun: 2 silent frames\n")["evidence_valid"])
    def test_count_not_lowered(self):
        self.assertFalse(report.evaluate(BASE.replace("[audio] level", "[other] level", 1)+CONFIG+STATUS)["evidence_valid"])
    def test_adoption_required(self):
        for bad in ("adopted=0", "errors=1", "over175ms=1", "history_invalid=1", "qualifying=0"):
            key = bad.split("=")[0]
            old = key + ("=1" if key in ("adopted", "qualifying") else "=0")
            self.assertFalse(report.evaluate(BASE+CONFIG+STATUS.replace(old,bad))["evidence_valid"], bad)
    def test_off_is_explicit(self):
        self.assertTrue(report.evaluate(BASE,"off",False)["evidence_valid"])
        self.assertFalse(report.evaluate(BASE+STATUS,"off",False)["evidence_valid"])
    def test_history_failures(self):
        for before, after in (("drops=0","drops=1"),("clipped=0","clipped=1"),("unstable=0","unstable=1"),
                              ("end rows=1","end rows=2"),("mach=2000","mach=9000"),("kind=4","kind=99")):
            self.assertFalse(report.evaluate(BASE+CONFIG+STATUS+DUMP.replace(before,after))["evidence_valid"],after)
        self.assertFalse(report.evaluate(BASE+CONFIG+STATUS+DUMP.split("[audio-workgroup-history] end")[0])["evidence_valid"])
    def test_real_driver_required(self):
        self.assertFalse(report.evaluate(BASE.replace("driver=coreaudio","driver=dummy")+CONFIG+STATUS)["evidence_valid"])
    def test_observation_is_not_assumed(self):
        self.assertFalse(report.evaluate(BASE+CONFIG+STATUS.replace("observing=1","observing=0"))["evidence_valid"])
        self.assertTrue(report.evaluate(BASE+STATUS.replace("observing=1","observing=0"),"async",False)["evidence_valid"])

if __name__ == "__main__": unittest.main()
