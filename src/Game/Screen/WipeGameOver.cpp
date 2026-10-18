#include "Game/Screen/WipeGameOver.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/EventUtil.hpp"

namespace NrvWipeGameOver {
    NEW_NERVE(WipeGameOverWait, WipeGameOver, Wait);
    NEW_NERVE(WipeGameOverActive, WipeGameOver, Active);
};  // namespace NrvWipeGameOver

WipeGameOver::WipeGameOver() : WipeLayoutBase("ゲームオーバー") {
}

void WipeGameOver::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE(WipeGameOver, WipeGameOverWait));
    initLayoutManager("GameOver", 2);
}

void WipeGameOver::kill() {
    LayoutActor::kill();
    setNerve(GET_NERVE(WipeGameOver, WipeGameOverWait));
}

void WipeGameOver::wipe(s32 frame) {
    if (isNerve(GET_NERVE(WipeGameOver, WipeGameOverActive))) {
        return;
    }

    appear();
    setColor();
    MR::hideLayout(this);
    setNerve(GET_NERVE(WipeGameOver, WipeGameOverActive));
}

void WipeGameOver::forceClose() {
    wipe(-1);
    setColor();
}

void WipeGameOver::forceOpen() {
    MR::hideLayout(this);
    setColor();
    kill();
}

bool WipeGameOver::isOpen() const {
    return MR::isDead(this) || isNerve(GET_NERVE(WipeGameOver, WipeGameOverWait));
}

bool WipeGameOver::isClose() const {
    return isNerve(GET_NERVE(WipeGameOver, WipeGameOverActive)) && MR::isAnimStopped(this, 0);
}

bool WipeGameOver::isWipeIn() const {
    return false;
}

bool WipeGameOver::isWipeOut() const {
    return isNerve(GET_NERVE(WipeGameOver, WipeGameOverActive)) && !MR::isAnimStopped(this, 0);
}

void WipeGameOver::exeWait() {
}

void WipeGameOver::exeActive() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        MR::startAnim(this, "GameOver", 0);
    }
}

void WipeGameOver::setColor() {
    MR::startAnim(this, "Color", 1);
    if (MR::isPowerStarGreenInCurrentStage()) {
        MR::setAnimFrameAndStop(this, 1.0f, 1);
    } else {
        MR::setAnimFrameAndStop(this, 0.0f, 1);
    }
}
