#include "Game/Player/MarioFaint.hpp"
#include "Game/Util.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Util/MathUtil.hpp"
#include <revolution/types.h>


bool Mario::doFlipWeak(const TVec3f& rVec) {
    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    mFaint->mNoDamage = true;

    if (faint(rVec)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->fn_803F7C10();
            mFaint->mNoDamage = false;
        }

        return true;
    }

    mFaint->mNoDamage = false;
    forceStopTornado();
    return false;
}

bool Mario::faint(const TVec3f& rVec) {
    _7C4 = rVec;
    if (!isEnableAddDamage()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    mFaint->setVec(rVec);
    stopWalk();
    forceStopTornado();
    mActor->damageDropThrowMemoSensor();

    if (getCurrentStatus() != MarioStatus_Faint) {
        mMovementStates.mHighWord |= 0x01000000U;
        return true;
    }

    return ((mMovementStates.mHighWord >> 24) & 1U);
}

MarioFaint::MarioFaint(MarioActor* pActor) : MarioState(pActor, MarioStatus_Faint), mTimer(), _14(), _16() {
    _18.x = 0.0f;
    _18.y = 0.0f;
    _18.z = 0.0f;
    mNoDamage = false;
    mTookDamage = false;
}

void MarioFaint::setVec(const TVec3f& rVec) {
    MR::vecKillElement(rVec, mActor->mAppliedGravityVec, &_18);
    _18.setLength(mActor->getConst().getTable()->mSlideDistFaint);
    mTimer = 0;
    _16 = 0;

    if (getPlayer()->isStatusActive(MarioStatus_Hang)) {
        _18.zero();
    }
}

bool MarioFaint::update() {
    mTimer++;

    switch (_16) {
    case 0:
        addVelocity(_18);
        _18.scale(mActor->getConst().getTable()->mFaintFriction1);
        if (mTimer == mActor->getConst().getTable()->mFaintTimer1) {
            _16++;
        }

        break;
    case 1:
        if (!((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
            return false;
        }

        addVelocity(_18);
        _18.scale(mActor->getConst().getTable()->mFaintFriction2);

        if (checkTrgA()) {
            getPlayer()->tryJump();
            return false;
        }

        if (mTimer == mActor->mConst->getTable()->mFaintTimer1 + mActor->getConst().getTable()->mFaintTimer2) {
            return false;
        }

        break;
    }

    return true;
}

// NONMATCHING: stack temporaries, registers and independent floating-point loads.
bool MarioFaint::start() {
    mTimer = 0;
    _16 = 0;
    getPlayer()->mMovementStates.mLowWord &= 0xFFEFFFFFU;
    getPlayer()->mMovementStates.mLowWord &= ~Mario::MovementStates::Jumping;

    if (_18.dot(getPlayer()->mFrontVec) > 0.0f) {
        getPlayer()->setFrontVecKeepUp(_18);
        changeAnimation("後方小ダメージ");
    } else {
        getPlayer()->setFrontVecKeepUp(-_18);
        changeAnimation("前方小ダメージ");
    }

    if (mNoDamage) {
        changeAnimation("ノーダメージ");
    }

    playSound("声小ダメージ");
    playSound("ダメージ");
    playEffect("ダメージ");
    startPadVib(2);

    addVelocity(_18);

    mTookDamage = !mNoDamage;

    if (!mNoDamage) {
        mActor->decLife(0);
        TVec3f reaction(_18);
        reaction.scale(0.5f);
        mActor->fn_803BE8D0(reaction);
        mActor->fn_803B94E0();

        if (mActor->mHealth == 0) {
            if (!((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
                mActor->forceGameOverNonStop();
            } else {
                mActor->forceGameOver();
            }
        }

        return true;
    }

    mNoDamage = false;
    mActor->fn_803B9560();
    return true;
}

bool MarioFaint::close() {
    if (((getPlayer()->getMovementStates().mLowWord >> 30) & 1U)) {
        stopAnimation("後方小ダメージ");
        stopAnimation("前方小ダメージ", "基本");
    }

    if (mTookDamage) {
        _14 = 120;
    }

    return true;
}
