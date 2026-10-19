#include "Game/Player/MarioMagic.hpp"
#include "Game/Util.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioModule.hpp"
#include "Game/Player/Yoshi.hpp"

void Mario::stopPunch() {
    if (isStatusActive(MarioStatus_Magic)) {
        closeStatus(mMagic);
    }

    MarioActor* actor = mActor;

    if (!actor->_944) {
        actor->_945 = 0;
        actor->_974 = 0;
    }

    actor->_944 = 0;
}

void Mario::startMagic() {
    if (!mMovementStates.jumping) {
        if (!mActor->_468) {
            if (!mMovementStates._23) {
                if (!isStatusActive(MarioStatus_Slider)) {
                    if (isSkatableFloor()) {
                        doSkate();
                    } else {
                        clearSlope();
                        changeStatus(mMagic);
                        stopAnimationUpper(nullptr);
                        _10._1 = 1;
                    }
                }
            }
        }
    }
}

MarioMagic::MarioMagic(MarioActor* pActor) : MarioState(pActor, MarioStatus_Magic) {
    _12 = 0;
}

// NONMATCHING: literal pool placement only.
bool MarioMagic::start() {
    if (isPlayerModeCloud()) {
        changeAnimation("雲スピン");
    } else {
        changeAnimation("地上ひねり");
    }
    stopEffect("パンチブラー左");
    stopEffect("パンチブラー右");
    if (getPlayer()->_960 == 0x25) {
        playEffect("芝生スピン");
    } else {
        playEffect("共通地上スピン");
    }
    playSound("スピンジャンプ");
    if (getPlayer()->isPlayerModeYoshi()) {
        MR::startActionSound(mActor->fn_803B97F0(), "HeadShake", -1, -1, -1);
    } else {
        playSound("声スピン");
    }
    startPadVib(2);
    _12 = 0;
    return true;
}

// NONMATCHING: literal pool placement only.
bool MarioMagic::close() {
    stopEffect("スピンライト");

    if (_12 < 0x1A) {
        playEffect("スピンライト消去");
    }

    return true;
}

// NONMATCHING: literal pool placement only.
bool MarioMagic::update() {
    if (mActor->isRequestJump()) {
        getPlayer()->tryJump();
        return false;
    } else if (!isAnimationRun("地上ひねり") && !isAnimationRun("雲スピン")) {
        return false;
    }

    _12++;

    if (_12 == 25) {
        stopEffect("スピンライト");
        playEffect("スピンライト消去");
    }

    if (getPlayer()->mMovementStates.jumping) {
        getPlayer()->procJump(false);
    } else {
        getPlayer()->fn_803A0BE0();
    }

    getPlayer()->updateWalkSpeed();
    return true;
}
