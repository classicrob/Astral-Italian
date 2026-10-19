#include "Game/Player/MarioBlown.hpp"
#include "Game/Util.hpp"
#include "Game/Player/FireMarioBall.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Util/MathUtil.hpp"
#include <revolution/types.h>

bool Mario::blown(const TVec3f& rVec) {
    if (getCurrentStatus() == MarioStatus_Blown) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_13) {
        return false;
    }

    stopWalk();

    mBlown->vecKillActor240(rVec);

    mMovementStates.mHighWord |= 0x00080000U;
    mMovementStates.mLowWord &= 0xFFDFFFFFU;
    _430 = 3;

    return true;
}

// NONMATCHING: independent initialization scheduling.
MarioBlown::MarioBlown(MarioActor* pActor) : MarioState(pActor, MarioStatus_Blown), mTimer(), _14() {
    _18.zero();
    _24 = false;
    _25 = false;
}

// NONMATCHING: independent timer initialization order.
bool MarioBlown::start() {
    mTimer = 0;
    _14 = 0;
    _24 = false;
    _25 = false;

    changeAnimation("壁ヒット", "基本");
    playSound("声壁体当たり");
    playSound("壁衝突");

    playEffectTrans("壁ヒット", getPlayer()->getWallPos());

    getPlayer()->mMovementStates.mLowWord &= 0xBFFFFFFFU;
    getPlayer()->mMovementStates.mLowWord |= Mario::MovementStates::Jumping;
    getPlayer()->mMovementStates.mLowWord &= 0xFFEFFFFFU;

    _18 += -mActor->mAppliedGravityVec * mActor->getConst().getTable()->mJumpHeightBlown;
    getPlayer()->setJumpVec(_18);
    addVelocity(_18);
    getPlayer()->lockGroundCheck(this, true);
    return true;
}

bool MarioBlown::update() {
    mTimer++;
    if (getPlayerMode() == PlayerMode_Teresa) {
        return false;
    }
    if (getPlayer()->_960 == 25 || getPlayer()->_960 == 31) {
        return false;
    }
    bool (MarioBlown::*steps[])() = {&MarioBlown::updateAir, &MarioBlown::updateGround};
    return (this->*steps[_14])();
}

bool MarioBlown::close() {
    if (!_24) {
        getPlayer()->stopJump();
    }

    stopAnimation("壁ヒット");

    if (_25) {
        stopAnimation("壁ヒット着地", "基本");
    }

    mActor->setBlendMtxTimer(6);
    getPlayer()->unlockGroundCheck(this);

    if (!_24) {
        getPlayer()->mMovementStates.mHighWord &= 0xFFFFFDFFU;
    }

    return true;
}

f32 MarioBlown::vecKillActor240(const TVec3f& v) { return MR::vecKillElement(v, mActor->mAppliedGravityVec, &_18); }

void MarioBlown::applyAirVelocity() {
    addVelocity(_18);
    _18 += mActor->mAppliedGravityVec * mActor->getConst().getTable()->mGravityBlown;
}

// NONMATCHING: equivalent boolean result materialization.
bool MarioBlown::isGrounded() {
    bool grounded = true;
    if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U) && !MR::isNearZero(getPlayer()->mVerticalSpeed)) {
        grounded = false;
    }
    return grounded;
}

void MarioBlown::land() {
    getPlayer()->mMovementStates.mLowWord &= ~Mario::MovementStates::Jumping;
    getPlayer()->mMovementStates.mLowWord |= 0x00040000U;
}

void MarioBlown::applyGroundVelocity() {
    addVelocity(_18);
    _18.mult(0.95f);
}

void MarioBlown::limitVelocity() {
    f32 vertical = MR::vecKillElement(_18, mActor->mAppliedGravityVec, &_18);
    if (_18.length() > 10.0f) {
        _18.mult(0.5f);
    }
    _18 += mActor->mAppliedGravityVec * MR::clamp(vertical, 0.0f, 40.0f);
}

// NONMATCHING: literal pool order changes string offsets, preserving all values.
bool MarioBlown::updateAir() {
    applyAirVelocity();
    if (mTimer > 300) {
        return false;
    }
    if (mTimer > 120) {
        changeAnimation("中ダメージ空中");
    }
    if (mTimer > 60) {
        TVec3f horizontal;
        if (MR::isNearZero(MR::vecKillElement(mActor->getLastMove(), mActor->mAppliedGravityVec, &horizontal))) {
            return false;
        }
    }
    if (isGrounded()) {
        if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            land();
        }
        if (getPlayer()->damagePolygonCheck(getPlayer()->getGroundPolygon())) {
            getPlayer()->mFlags20.mWord |= 0x00000200U;
            _24 = true;
            return false;
        }
        playSound("吹っ飛び倒れ");
        changeAnimation("壁ヒット着地");
        playEffect("共通壁ヒット着地");
        MR::vecKillElement(_18, mActor->mAppliedGravityVec, &_18);
        _14 = 1;
        mTimer = 0;
    }
    limitVelocity();
    getPlayer()->setJumpVec(_18);
    return true;
}

bool MarioBlown::updateGround() {
    if (!isGrounded()) {
        _25 = true;
        return false;
    }
    applyGroundVelocity();
    if (!isAnimationRun("壁ヒット着地")) {
        return false;
    }
    if (mTimer > 15 && checkTrgA()) {
        getPlayer()->stopJump();
        getPlayer()->tryJump();
        _24 = true;
        return false;
    }
    limitVelocity();
    getPlayer()->setJumpVec(_18);
    return true;
}
