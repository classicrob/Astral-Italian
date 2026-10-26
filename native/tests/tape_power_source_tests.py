#!/usr/bin/env python3
"""Source guard: real power delivery order and finish never samples game state."""
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parents[2]


def body(text,name):
    first=text.index('{',text.index(name));depth=0
    for last in range(first,len(text)):
        depth+=(text[last]=='{')-(text[last]=='}')
        if depth==0:return text[first:last+1]
    raise AssertionError('missing function end')


class PowerSource(unittest.TestCase):
    def test_teardown_uses_game_thread_gate_before_any_scene_pointer(self):
        source=body((ROOT/'native/app/smoke_observe_safety.hpp').read_text(),'bool canObserveScene(')
        names=['!controller->mIsResetProcessing','controller->isSceneInitializeState(end)','controller->mScene']
        self.assertEqual([source.index(n) for n in names],sorted(source.index(n) for n in names))
        reset=body((ROOT/'src/Game/System/GameSystemSceneController.cpp').read_text(),'void GameSystemSceneController::prepareReset()')
        self.assertLess(reset.index('mIsResetProcessing = true'),reset.index('requestChangeNerve'))
        change=body((ROOT/'src/Game/System/GameSystemSceneController.cpp').read_text(),'void GameSystemSceneController::requestChangeScene()')
        self.assertLess(change.index('mSceneInitializeState = SceneInitializeState_NotInit'),change.index('requestChangeNerve'))

    def test_all_shipping_tape_and_always_on_observers_use_lifetime_gate(self):
        smoke=(ROOT/'native/app/smoke_game.cpp').read_text()
        for name in ('Observation observeGame(', 'void stallContext(', 'void recordRouteFrame('):
            source=body(smoke,name)
            self.assertIn('canObserveScene(',source)
            self.assertNotIn('mScene &&',source)
        for file in ('tape_scene.cpp','progress_observe.cpp','play_ready.cpp'):
            source=(ROOT/'native/app'/file).read_text()
            self.assertIn('Smoke::canObserveScene(',source)
            self.assertNotIn('mScene != nullptr &&',source)
        recorder=body(smoke,'void recordRouteFrame(')
        self.assertIn('state.initEnd = Smoke::canObserveScene(',recorder)
        self.assertIn('seam.initEnd = Smoke::canObserveScene(',recorder)

    def test_replay_capture_inherits_policy_from_parsed_tape_only(self):
        source=body((ROOT/'native/app/tape_play_hooks.cpp').read_text(),'bool configure(')
        self.assertIn('if (gCaptureReplay) TapeRecord::setNeutralTailPolicy(gSession->meta().neutralTailPolicy)',source)
        self.assertNotIn('PETARI_TAPE_NEUTRAL',source)

    def test_real_callback_is_between_record_and_actual_flag_acknowledgement(self):
        source=body((ROOT/'native/app/platform_host.cpp').read_text(),'void requestQuit()')
        names=['TapeRecord::powerRequest()','TapePlay::beforePowerRequest()',
               'Platform::Power::pressPowerButton()','TapeScene::powerRequestObserved()',
               'TapeRecord::acknowledgePower(observed)','TapePlay::acknowledgePower(observed)']
        self.assertEqual([source.index(n) for n in names],sorted(source.index(n) for n in names))

    def test_native_producer_binds_policy_during_script_initialization(self):
        source=(ROOT/'native/app/input_script_probe.cpp').read_text()
        self.assertIn('if (s.recordNative) TapeRecord::setNeutralTailPolicy(TapeNeutral::Policy::NativeControls)',source)

    def test_native_script_tail_and_real_power_are_inside_recorder_seam(self):
        source=body((ROOT/'native/app/smoke_game.cpp').read_text(),'void recordRouteFrame(')
        self.assertLess(source.index('TapeRecord::beginControlSeam()'),source.index('Smoke::inputScriptRecordSeam('))
        self.assertLess(source.index('Smoke::inputScriptRecordSeam('),source.index('TapeRecord::endControlSeam()'))
        source=body((ROOT/'native/app/input_script_probe.cpp').read_text(),'void inputScriptRecordSeam(')
        self.assertIn('Input::mouseMoved(p.x,p.y)',source)
        self.assertIn('Input::mouseLeft()',source)
        self.assertLess(source.index('TapeRecord::neutralTailBegin()'),source.index('s.tail.began(now(),true)'))
        self.assertIn('s.tail.powerAllowed(now(),true)',source)
        self.assertIn('Host::requestQuit()',source)

    def test_finish_never_observes_actor_writes_row_or_increments_seam(self):
        source=body((ROOT/'native/app/tape_record.cpp').read_text(),'void finish()')
        for forbidden in ['observe(', 'getMario', 'writer.write', '++gRow', 'gRow++', 'beforeRead(']:
            self.assertNotIn(forbidden,source)
        self.assertIn('Input::KpadRecord::close()',source)
        self.assertIn('std::fflush(nullptr)',source)
        self.assertIn('writeSidecar(true)',source)

    def test_power_flag_observation_is_real_game_state(self):
        source=body((ROOT/'native/app/tape_scene.cpp').read_text(),'bool powerRequestObserved()')
        self.assertIn('SingletonHolder<GameSystemResetAndPowerProcess>::get()',source)
        self.assertIn('process->mIsValidPowerOff',source)


if __name__=='__main__':unittest.main()
