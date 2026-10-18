#include "Game/Player/PlayerEventKillNoDirector.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

EventKillNoDirector::EventKillNoDirector() : EventSequence(6) {
    addEventOnTime("レイアウト非表示", static_cast<EventFunc1>(&EventKillNoDirector::endCometEvent), 0);
    addEventOnTime("サウンドA", static_cast<EventFunc1>(&EventKillNoDirector::init), 0);
    addEventOnTime("ＭＩＳＳ表示", static_cast<EventFunc1>(&EventKillNoDirector::showMissLayout), 15);
    addEventInTime("ワイプ開始", static_cast<EventFunc1>(&EventKillNoDirector::doCloseWipe), 70, 600);
    addEventOnTime("残機を引く", static_cast<EventFunc1>(&EventKillNoDirector::decLeft), 90);
    addEventInPhase("終了", static_cast<EventFunc1>(&EventKillNoDirector::finish), 2);
}

void EventKillNoDirector::showMissLayout(u16, u16) {
    MR::startMissLayout();
}

void EventKillNoDirector::init(u16, u16) {
    MR::setCubeBgmChangeInvalid();
    MR::clearBgmQueue();
    MR::stopStageBGM(10);
    MR::stopSubBGM(10);
    MR::fn_8005BAD0();
    MR::setSoundVolumeSetting(2, 20);
    MR::startSubBGM("BGM_MISS", false);
}

void EventKillNoDirector::finish(u16, u16) {
    MR::requestEndMissDemo();
    stopSequence();
}
