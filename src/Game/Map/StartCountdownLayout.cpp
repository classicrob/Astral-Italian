#include "Game/Map/StartCountdownLayout.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvStartCountdownLayout {
    NEW_NERVE(StartCountdownLayoutNrvCountdown, StartCountdownLayout, Countdown);
    NEW_NERVE(StartCountdownLayoutNrvStart, StartCountdownLayout, Start);
};  // namespace NrvStartCountdownLayout

StartCountdownLayout::StartCountdownLayout() : LayoutActor("スタートカウントダウンレイアウト", true) {
}

void StartCountdownLayout::init(const JMapInfoIter& rIter) {
    MR::connectToSceneLayout(this);
    initLayoutManager("StartCountdown", 1);
    initEffectKeeper(0, nullptr, nullptr);
    initNerve(GET_NERVE(StartCountdownLayout, StartCountdownLayoutNrvCountdown));
    initActionSound(3, "StartCountdown", nullptr);
    MR::registerDemoSimpleCastAll(this);
    kill();
}

void StartCountdownLayout::appear() {
    LayoutActor::appear();
    setNerve(GET_NERVE(StartCountdownLayout, StartCountdownLayoutNrvCountdown));
}

void StartCountdownLayout::exeCountdown() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Count", 0);
        MR::startActionSound(this, "RaceCountDown", -1, -1);
    }

    if (MR::isStep(this, 60) || MR::isStep(this, 120)) {
        MR::startActionSound(this, "RaceCountDown", -1, -1);
    }

    if (MR::isStep(this, 170)) {
        MR::startMinigameBGM();
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE(StartCountdownLayout, StartCountdownLayoutNrvStart));
    }
}

void StartCountdownLayout::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "Go", 0);
        MR::emitEffect(this, "Start");
        MR::startActionSound(this, "RaceStart", -1, -1);
    }

    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}

bool StartCountdownLayout::isStart() const {
    return isNerve(GET_NERVE(StartCountdownLayout, StartCountdownLayoutNrvStart));
}

StartCountdownLayout::~StartCountdownLayout() {
}
