#include "Game/Player/MarioWall.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JMath/JMath.hpp>
#include <cstring>
#ifdef GALAXY_NATIVE
// OdysseyMovement mod (docs/dev/ODYSSEY_MOVEMENT.md): SMO's wall slide.
#include <petari/odyssey_move.hpp>
extern "C" bool petari_mod_enabled(int mod);
#endif

MarioWall::MarioWall(MarioActor* pActor) : MarioState(pActor, MarioStatus_Wall) {
    _14 = 0;
    _18 = 0;
    _1C = 0;
    _1D = 0;
    _1E = 0;
    _20 = 0.0f;
    _24.zero();
    _30.zero();
}

void MarioWall::initTriangleJump() {
    Mario* player = getPlayer();
    MR::vecKillElement(player->mJumpVec, getGravityVec(), &_30);
    MR::normalizeOrZero(&_30);
    _14 = 0;
}

u32 MarioWall::isCancel() {
    if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
        _1C = 0;
    }

    if (_1C && ((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
        if (_24.dot(getPlayer()->getWallNorm()) < mActor->getConst().getTable()->mWallStickCancelAngle) {
            _1C = 0;
        }
    }

    return _1C;
}

#pragma auto_inline off

#pragma dont_inline on

// NONMATCHING: register allocation, stack temporaries and literal-pool layout.
bool MarioWall::start() {
    _18 = 0;
    _20 = 0.0f;
    changeAnimation("壁くっつき");
    startPadVib(0UL);
    getPlayer()->mMovementStates.mHighBits.mBit23 = 0;
    getPlayer()->mFlags24.mHighBits.mBit23 = 0;
    _1D = 0;
    initTriangleJump();
    _1E = 0;

    if (getPlayerMode() == PlayerMode_Ice && getPlayer()->getWallPolygon()->mSensor->isType(0x51)) {
        MarioActor* actor = mActor;
        TVec3f offset(getPlayer()->getWallNorm());
        offset *= 30.0f;
        const TVec3f& normal = getPlayer()->getWallNorm();
        TVec3f position = getPlayer()->getWallPos().operator-(offset);
        actor->createIceWall(position, normal);
        TVec3f effectOffset(getPlayer()->getWallNorm());
        effectOffset *= 15.0f;
        TVec3f effectPosition = getPlayer()->getWallPos().operator-(effectOffset);
        playEffectRT("氷壁ジャンプ", getPlayer()->getWallNorm(), effectPosition);
        playSound("スケート着地");
        _1E = 1;
    }

    MarioActor* actor = mActor;
    if (!actor->_944) {
        actor->_945 = 0;
        actor->_974 = 0;
    }
    actor->_944 = 0;
    getPlayer()->fn_803A2420();
    return true;
}

#pragma dont_inline reset

// NONMATCHING: register allocation, stack temporaries and literal-pool layout.
bool MarioWall::update() {
    bool release = false;
    if (mActor->_334) {
        return false;
    }

    if (MR::getKarikariClingNum()) {
        return false;
    }

    if ((isStatusActiveID(MarioStatus_Rabbit) || mActor->_37C - getPlayer()->_558 < 6) && startJump()) {
        return false;
    }

    if (((getPlayer()->mDrawStates.mWord >> 25) & 1U)) {
        if (_1D) {
            TVec3f horizontal;
            MarioActor* actor = mActor;
            if (MR::vecKillElement(actor->getLastMove(), getGravityVec(), &horizontal) < 1.0f) {
                release = true;
            }
        }

        getPlayer()->mMovementStates.mLowBits.mBit30 = 1;
        _1D = 1;
    } else {
        _1D = 0;
    }

    _14++;
    if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U) && !release) {
        if (getPlayer()->mVerticalSpeed < 80.0f) {
            if (!isOnSlipGround()) {
                getPlayer()->setFrontVecKeepUp(getPlayer()->getWallNorm());
                changeAnimation("着地");
                changeAnimationInterpoleFrame(1);
                mActor->setBlendMtxTimer(4);
            }

            release = true;
        } else {
            getPlayer()->mMovementStates.mLowBits.mBit30 = 0;
        }
    }

#ifdef GALAXY_NATIVE
    // OdysseyMovement: SMO's slide lasts until the wall or the fall ends; holding
    // the stick away from the wall lets go after 15 frames.
    const bool odyssey = petari_mod_enabled(2) && getPlayerMode() == 0 && !_1E;
    if (odyssey ? _18 > 15 : _14 >= getActor()->getConst().getTable()->mWallReleaseTime + getActor()->getConst().getTable()->mWallStickTime) {
        release = true;
    }
#else
    if (_14 >= getActor()->getConst().getTable()->mWallReleaseTime + getActor()->getConst().getTable()->mWallStickTime) {
        release = true;
    }
#endif

    if (_14 >= 3 && !((getPlayer()->mMovementStates.mLowWord >> 23) & 1U) && !((getPlayer()->mMovementStates.mHighWord >> 13) & 1U)) {
        release = true;
    }

    if (release) {
        if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            _1C = 1;
            TVec3f normal(getPlayer()->getWallNorm());
            _24 = normal;
            getPlayer()->setFrontVecKeepUp(-normal);
        }

        if (isOnSlipGround()) {
            addVelocityAfter(getPlayer()->getWallNorm() * 10.0f);
        } else {
            addVelocityAfter(getPlayer()->getWallNorm() * 30.0f);
        }

        return false;
    }

    if (checkTrgA() && startJump()) {
        if (_1E) {
            playSound("スケートジャンプ");
        }

        return false;
    }

    if (mActor->isRequestRush()) {
        if (isPlayerModeFire()) {
            mActor->updateFireSpin();
        } else {
            getPlayer()->mMovementStates.mHighBits.mBit20 = 0;
            if (isPlayerModeCloud()) {
                changeAnimation("雲スピン空中");
            } else {
                changeAnimation("空中ひねり");
            }
            getPlayer()->tryWallPunch();
        }
        getPlayer()->setWallCancel();
        return false;
    }

    if (!getPlayer()->fixWallingPosition(false)) {
        getPlayer()->setWallCancel();
        return false;
    }

    if (getPlayer()->checkWallCode("NoAction", true)) {
        getPlayer()->setWallCancel();
        return false;
    }

    f32 blend = 0.9f;
    f32 speed = getActor()->getConst().getTable()->mWallDropSpeedNormal;
    u8 side = getPlayer()->checkStickWallSide();
    if (side == 1) {
        side = 0;
    }

    switch (side) {
    case 1:
        blend = 0.7f;
        if (_14 > getActor()->getConst().getTable()->mWallStickTime) {
            _14 = getActor()->getConst().getTable()->mWallStickTime;
        }

        _18 = 0;
        speed = getActor()->getConst().getTable()->mWallDropSpeedStop;
        changeAnimation("壁くっつき");
        stopEffect("共通壁手擦り");
        break;
    case 2:
        if (_1E) {
            if (_14 < getActor()->getConst().getTable()->mWallStickTimeIce - 15) {
                _14 = getActor()->getConst().getTable()->mWallStickTimeIce - 15;
            }
        } else if (_14 < 165) {
            _14 = 165;
        }

        if (!_18) {
            _18 = 1;
        }

        stopEffect("共通壁手擦り");
        break;
    case 0:
        if (_14 < getActor()->getConst().getTable()->mWallStickTime) {
            stopEffect("共通壁手擦り");
        } else {
            if (!isAnimationRun("壁くっつき")) {
                changeAnimation("壁すべり");
            }

            playSound("スリップ");
            playEffect("共通壁手擦り");
        }

        break;
    }

    if (_18) {
        _18++;
    }

    if (_14 < getActor()->getConst().getTable()->mWallStickTime) {
        blend = 1.0f;
    }

    if (_1E) {
        if (_14 > getActor()->getConst().getTable()->mWallStickTimeIce) {
            _1C = 1;
            _24 = getPlayer()->getWallNorm();
            return false;
        }

        return true;
    }

#ifdef GALAXY_NATIVE
    if (odyssey) {
        // SMO: held for 3 frames, then gravity 0.5 up to 35.
        _20 = PetariNative::Odyssey::stepWallSlide(_20, static_cast< int >(_14));
        if (_14 == PetariNative::Odyssey::Const::WallKeepFrame + 1) {
            changeAnimation("壁すべり");
        }
        if (_14 > PetariNative::Odyssey::Const::WallKeepFrame) {
            playSound("スリップ");
            playEffect("共通壁手擦り");
        }
    } else {
        _20 = _20 * blend + speed * (1.0f - blend);
    }
#else
    _20 = _20 * blend + speed * (1.0f - blend);
#endif
    addVelocity(getPlayer()->_75C, -_20);
    if (side <= 1) {
        f32 ratio;
        if (_14 < getActor()->getConst().getTable()->mWallStickTime) {
            ratio = 0.0f;
        } else {
            ratio = 1.0f - (_14 - getActor()->getConst().getTable()->mWallStickTime) * getActor()->getConst().getTable()->mWallSideMoveRatio;
        }

        getPlayer()->moveWallSlide(MR::clamp(ratio, 0.0f, 1.0f));
    }

    return true;
}

bool MarioWall::close() {
    stopAnimation("壁くっつき");
    stopAnimation("壁すべり");
    if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
        changeAnimation(nullptr, "基本");
    }

    stopEffect("共通壁手擦り");
    getPlayer()->resetTornado();
    getPlayer()->mMovementStates.mHighBits.mBit7 = 0;
    return true;
}

bool MarioWall::startJump() {
    if (getPlayer()->isInhibitWall()) {
        return false;
    }

    TVec3f velocity(getPlayer()->getWallNorm());
    _24 = velocity;
    if (!((getPlayer()->mDrawStates.mWord >> 28) & 1U) && _14 < 15 && !MR::isNearZero(_30)) {
        f32 dot = velocity.dot(-_30);
        if (dot < mActor->getConst().getTable()->mWallTriJumpMargin && dot > 0.0f) {
            Mtx rotation;
            TVec3f axis;
            axis.cross(-_30, velocity);
            PSMTXRotAxisRad(rotation, &axis, MR::acos(dot));
            PSMTXMultVec(rotation, &velocity, &velocity);
        }
    }

    velocity *= mActor->getConst().getTable()->mWallJumpPowerXZ;
    velocity += getGravityVec() * -mActor->getConst().getTable()->mWallJumpPowerY;
    getPlayer()->tryWallJump(velocity, true);
    playEffect("共通壁ジャンプ");
    _1C = 1;
    getPlayer()->mMovementStates.mHighBits.mBit20 = 0;
    return true;
}

// NONMATCHING: register allocation, stack temporaries and literal-pool layout.
bool MarioWall::startBackJump(u32 type) {
    if (getPlayer()->isInhibitWall()) {
        return false;
    }

    TVec3f velocity(getPlayer()->getWallNorm());
    if (((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && velocity.dot(getPlayer()->mJumpVec) > 0.0f) {
        return false;
    }

    _24 = velocity;
    velocity *= mActor->getConst().getTable()->mWallBackJumpPowerXZ;
    velocity += getGravityVec() * -mActor->getConst().getTable()->mWallBackJumpPowerY;
    getPlayer()->tryWallJump(velocity, false);
    switch (type) {
    case 0: {
        playSound("トランポリンジャンプ大");
        Mario* player = getPlayer();
        player->_402 = 0;
        player = getPlayer();
        player->_428 = 60;
        break;
    }

    case 1:
        playEffectRTZ("水壁ヒット", _24, getPlayer()->getWallPos());
        playSound("水弾かれ");
        break;
    case 2:
        playSound("トランポリンジャンプ大");
        break;
    }

    startPadVib(2);
    _1C = 1;
    getPlayer()->mMovementStates.mHighBits.mBit20 = 1;
    return true;
}

#pragma auto_inline on

bool MarioWall::notice() {
    return false;
}
