#!/usr/bin/env python3
"""Source-only recorder boundaries. Native state-machine test is built after review."""
from pathlib import Path
import unittest
from tape_power_source_tests import body
ROOT=Path(__file__).resolve().parents[2]
class RecorderSource(unittest.TestCase):
    def test_normal_live_and_replay_do_not_enable_observation(self):
        source=(ROOT/'native/app/tape_scene.cpp').read_text()
        self.assertIn('gRecordObservation=observation && *observation && recording && !TapePlay::meta() && !gCaptureReplay;',source)
        self.assertIn('if (gRecordObservation) Smoke::inputScriptRecordBeforeRead(s);',source)
        self.assertLess(source.index('TapeRecord::beforeRead(s);'),source.index('inputScriptRecordBeforeRead(s);'))
    def test_pre_read_only_counts_no_power_or_input_delivery(self):
        source=body((ROOT/'native/app/input_script_probe.cpp').read_text(),'void inputScriptRecordBeforeRead(')
        for forbidden in ('Host::requestQuit','TapeRecord::neutralTailBegin','Input::keyEvent','mouseLeft','mouseMoved','getMario','getPlayerPos'):
            self.assertNotIn(forbidden,source)
        self.assertIn('s.observationCompletePending=true',source)
    def test_queue_and_power_use_existing_control_scope(self):
        source=body((ROOT/'native/app/smoke_game.cpp').read_text(),'void recordRouteFrame(')
        self.assertLess(source.index('TapeRecord::beginControlSeam()'),source.index('Smoke::inputScriptRecordSeam('))
        self.assertLess(source.index('Smoke::inputScriptRecordSeam('),source.index('TapeRecord::endControlSeam()'))
        source=body((ROOT/'native/app/input_script_probe.cpp').read_text(),'void inputScriptRecordSeam(')
        self.assertIn('if (s.observationCompletePending)',source)
        self.assertIn('s.tail.queue()',source)
        self.assertLess(source.index('TapeRecord::neutralTailBegin()'),source.index('s.tail.powerAllowed('))
        self.assertIn('Tail::Power',source)
    def test_fixed_case_and_no_game_state_access(self):
        source=(ROOT/'native/app/record_normal_observation.hpp').read_text()
        self.assertIn('Packets=14400, MinimumValid=13292',source)
        self.assertIn('s.kind!="controller-normal"',source)
        self.assertIn('!s.firstNormal',source)
        for forbidden in ('#include "Game/','Host::','Input::','getenv','requestQuit','neutralTailBegin'):
            self.assertNotIn(forbidden,source)
    def test_recorder_configuration_rejects_script_or_player(self):
        source=(ROOT/'native/app/input_script_probe.cpp').read_text()
        self.assertIn('std::strcmp(observe,"14400")!=0 || (path && *path) || !s.recordNative',source)
        self.assertIn('!std::getenv("PETARI_TAPE_RECORD") || std::getenv("PETARI_TAPE_PLAY")',source)
        self.assertIn('if (s.observeNative) return true;',source)
        self.assertIn('TapeRecord::setNeutralTailPolicy(TapeNeutral::Policy::NativeControls)',source)
if __name__=='__main__':unittest.main()
