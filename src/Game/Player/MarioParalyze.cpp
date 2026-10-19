#include "Game/Player/Mario.hpp"
#include "Game/Util.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioParalyze.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Player/MarioState.hpp"
#include <revolution/types.h>

MarioParalyze::MarioParalyze(MarioActor* pActor) : MarioState(pActor, MarioStatus_Paralyze), _12(), mTimer(), _16(), mNotDecLife() {
}

bool MarioParalyze::start() {
    changeAnimationNonStop("電気ダメージ");
    playSound("ダメージ");
    playSound("電気ダメージ");
    if (isPlayerModeYoshi()) {
        MR::startActionSound(mActor->getYoshi(), "DamageElec", -1, -1, -1);
    } else {
        playSound("声電気ダメージ");
    }
    playEffect("ビリビリ");

    startPadVib(3);

    if (!mNotDecLife) {
        mActor->decLifeLarge();
        mActor->fn_803BE9E0();
    }

    mNotDecLife = false;
    mTimer = 60;
    _12 = 0;

    return true;
}

bool MarioParalyze::update() {
    if (mTimer != 0) {
        mTimer--;
        if (_12 != 0 && ((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
            addVelocity(getFrontVec(), -3.0f);
        }
    }

    if (mTimer == 0) {
        if (_12 != 0) {
            return false;
        }

        _12 = 1;

        if (!((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
            mTimer = 10;
        } else {
            mTimer = 30;
        }

        if (((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
            changeAnimation("電気ダメージ終了");
            playSound("声電気ダメージ終了");
        }

        if (mActor->mHealth == 0) {
            if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
                mActor->forceGameOverNonStop();
            } else {
                mActor->forceGameOver();
            }

            mActor->changeGameOverAnimation();
        }
    }

    return true;
}

bool MarioParalyze::close() {
    if (mActor->mHealth == 0) {
        if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            mActor->forceGameOverNonStop();
        } else {
            mActor->forceGameOver();
        }

        mActor->changeGameOverAnimation();
    }

    stopAnimation("電気ダメージ");
    stopEffect("ビリビリ");

    _16 = 120;

    if (!getPlayer()->isStatusActive(MarioStatus_Swim) && !((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
        getPlayer()->tryFreeJump(getFrontVec() * -10.0f, true);
    }

    return true;
}
