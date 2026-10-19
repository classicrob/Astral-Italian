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

bool Mario::isWalling() const {
    return getCurrentStatus() == MarioStatus_Wall;
}

void Mario::checkWallStick() {
    if (getPlayerMode() == PlayerMode_Bee) {
        checkBeeStick();
        return;
    }

    if (!checkWallJumpCode() && isEnableStickWall()) {
        if (mActor->_334) {
            TVec3f horizontal;
            f32 vertical = MR::vecKillElement(mJumpVec, mActor->_240, &horizontal);
            horizontal.setLength(-0.25f);
            mJumpVec = mActor->_240 * vertical + horizontal;
            return;
        }

        changeStatus(mWall);
        stopWalk();
        mMovementStates.mLowWord &= ~Mario::MovementStates::Jumping;
        mMovementStates.mLowWord &= 0xFFEFFFFFU;
        mMovementStates.mLowWord &= 0xFDFFFFFFU;
        mMovementStates.mLowWord &= 0xFFFFFEFFU;
        fixWallingPosition(true);
        resetTornado();
    }
}

u8 Mario::checkStickWallSide() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(getWallNorm()) < -0.5f) {
            return 1;
        }

        if (direction.dot(getWallNorm()) > 0.5f && !((mMovementStates.mLowWord >> 2) & 1U)) {
            return 2;
        }
    }

    return 0;
}

s32 Mario::checkStickFrontBack() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(-mFrontVec) < -0.5f) {
            return 1;
        }

        if (direction.dot(-mFrontVec) > 0.5f && !((mMovementStates.mLowWord >> 2) & 1U)) {
            return 2;
        }
    }

    return 0;
}

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

bool Mario::fixWallingPosition(bool immediate) {
    if (!fixWallingDist()) {
        return false;
    }

    fixWallingDir(immediate);
    return true;
}

void Mario::fixWallingDir(bool immediate) {
    bool back = true;
    if (isStatusActive(MarioStatus_SideStep)) {
        back = false;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U) && back) {
        if (immediate) {
            setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0), 0.1f);
        return;
    }

    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        if (immediate) {
            setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0), 0.1f);
    }
}

bool Mario::fixWallingTop() {
    TVec3f side = getWallNorm().cross(getAirGravityVec());
    MR::normalizeOrZero(&side);
    if (MR::isNearZero(side)) {
        return false;
    }

    _75C.cross(getWallNorm(), side);
    MR::normalizeOrZero(&_75C);
    if (MR::isNearZero(_75C)) {
        return false;
    }

    getWallNorm().dot(getAirGravityVec());
    getPlayer()->forceSetHeadVecKeepSide(_75C);
    return true;
}

bool Mario::checkWallFloorCode(u16 code) const {
    if ((((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) && _964[0] == code) {
        return true;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U) && _964[1] == code) {
        return true;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U) && _964[2] == code) {
        return true;
    }

    return false;
}

bool Mario::checkWallCode(const char* pCode, bool frontBackOnly) const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mBackWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mFrontWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mSideWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    return false;
}

bool Mario::checkWallCodeNorm(u16 code, TVec3f* pNormal, bool frontBackOnly) const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mBackWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mBackWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mFrontWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mFrontWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mSideWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mSideWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    return false;
}

void Mario::setWallCancel() {
    mWall->_1C = 1;
    mWall->_24 = getWallNorm();
}

void Mario::keepDistFrontWall() {
    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);
        if (horizontal.length() < 80.0f) {
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()));
        }
    }
}

bool Mario::isEnableStickWall() {
    switch (getPlayerMode()) {
    case PlayerMode_Teresa:
        return false;
    case PlayerMode_Foo:
        if (isStatusActive(MarioStatus_Foo)) {
            return false;
        }

        break;
    }

    if (mWall->isCancel()) {
        return false;
    }

    if (!((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) || ((mMovementStates.mLowWord >> 30) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 20) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 16) & 1U)) {
        return false;
    }

    if (((mDrawStates.mWord >> 1) & 1U)) {
        return false;
    }

    if (!((mMovementStates.mLowWord >> 23) & 1U) && !((mMovementStates.mLowWord >> 6) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 29) & 1U) && mVerticalSpeed < mActor->getConst().getTable()->mWallStickGrHeight) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 10) & 1U) && _4E0 < mActor->getConst().getTable()->mWallStickFrHeight && !getWallPolygon()->mSensor->isType(0x57)) {
        return false;
    }

    if (isAnimationRun("空中ひねり")) {
        if (mActor->_945 < 25) {
            return false;
        }
    } else if (mActor->isPunching() && mActor->_945 < 15) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 10) & 1U) && ((mMovementStates.mLowWord >> 29) & 1U) && ((mMovementStates.mHighWord >> 6) & 1U)) {
        if ((_4A4 - mShadowPos).dot(-*getGravityVec()) < mActor->getConst().getTable()->mWallStickStepHeight) {
            return false;
        }
    }

    TVec3f horizontal;
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        MR::vecKillElement(mJumpVec, mActor->_240, &horizontal);
        if (mBackWallTriangle->getNormal(0)->dot(horizontal) > 0.0f) {
            return false;
        }
    }

    if (isInhibitWall()) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U) && calcPolygonAngleD(mBackWallTriangle) < 80.0f) {
        return false;
    }

    TVec3f side = getWallNorm().cross(getAirGravityVec());
    if (MR::normalizeOrZero(&side)) {
        return false;
    }

    if (!checkWallCode("NotWallSlip", true)) {
        mMovementStates.mHighWord |= 0x00800000U;
    }

    if (checkWallCode("NoAction", true)) {
        return false;
    }

    if ((getPlayerMode() != PlayerMode_Ice || !getWallPolygon()->mSensor->isType(0x57)) && isRising()) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        if (!isAnimationRun("壁ジャンプ") && getPlayer()->checkStickWallSide() != 1) {
            return false;
        }
    } else if (!isAnimationRun("壁ジャンプ") && !((mMovementStates.mLowWord >> 22) & 1U) && getPlayer()->checkStickWallSide() != 1) {
        return false;
    }

    if (((mMovementStates.mHighWord >> 15) & 1U)) {
        return false;
    }

    if (mActor->_468) {
        if (!checkTrgA()) {
            return false;
        }

        mWall->initTriangleJump();
        mWall->startJump();
        return false;
    }

    TVec3f position(mActor->_2AC);
    f32 distance = 120.0f;
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        distance = -120.0f;
    }

    if (!MR::isExistMapCollision(position, mFrontVec * distance)) {
        return false;
    }

    return MR::getKarikariClingNum() == 0;
}

#pragma auto_inline off

#pragma dont_inline on

// NONMATCHING: register allocation, stack temporaries and literal-pool layout.
bool MarioWall::start() {
    _18 = 0;
    _20 = 0.0f;
    changeAnimation("壁くっつき");
    startPadVib(0UL);
    getPlayer()->mMovementStates.mHighWord &= 0xFF7FFFFFU;
    getPlayer()->mFlags24.mHighWord &= 0xFF7FFFFFU;
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

        getPlayer()->mMovementStates.mLowWord |= 0x40000000U;
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
            getPlayer()->mMovementStates.mLowWord &= 0xBFFFFFFFU;
        }
    }

    if (_14 >= getActor()->getConst().getTable()->mWallReleaseTime + getActor()->getConst().getTable()->mWallStickTime) {
        release = true;
    }

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
            getPlayer()->mMovementStates.mHighWord &= 0xFFEFFFFFU;
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

    _20 = _20 * blend + speed * (1.0f - blend);
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
    getPlayer()->mMovementStates.mHighWord &= 0xFFFFFF7FU;
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
    getPlayer()->mMovementStates.mHighWord &= 0xFFEFFFFFU;
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
    getPlayer()->mMovementStates.mHighWord |= 0x00100000U;
    return true;
}

#pragma auto_inline on

bool Mario::fixWallingDist() {
    if (((mMovementStates.mLowWord >> 6) & 1U) & ((mMovementStates.mLowWord >> 23) & 1U)) {
        return true;
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        TVec3f position(_4F4 + *mBackWallTriangle->getNormal(0) * 79.0f);
        f32 distance = MR::vecKillElement(position - mPosHolder->mPos, *mBackWallTriangle->getNormal(0), &position);
        if (!fixWallingTop()) {
            return false;
        }

    } else if (((mMovementStates.mLowWord >> 23) & 1U)) {
        if (!fixWallingTop()) {
            return false;
        }

        if (MR::isExistMapCollision(mPosHolder->mPos, mFrontVec * 100.0f) && mFrontVec.dot(*mFrontWallTriangle->getNormal(0)) < -0.999f) {
            TVec3f position(_4E8 + *mFrontWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f);
            TVec3f correction;
            MR::vecKillElement(position - mPosHolder->mPos, *getGravityVec(), &correction);
            if (!MR::isNearZero(correction)) {
                TVec3f relative(correction + mActor->_288);
                if (MR::abs(relative.x) < MR::abs(correction.x)) {
                    correction.x = relative.x;
                }

                if (MR::abs(relative.y) < MR::abs(correction.y)) {
                    correction.y = relative.y;
                }

                if (MR::abs(relative.z) < MR::abs(correction.z)) {
                    correction.z = relative.z;
                }

                addTrans(correction, "壁補正");
            }
        }
    }

    return true;
}

bool Mario::isInhibitWall() const {
    if (((mDrawStates.mWord >> 29) & 1U)) {
        return true;
    }

    if (checkWallCode("NotWallJump", true)) {
        return true;
    }

    return checkWallCode("NoAction", true);
}

void Mario::tryWallPunch() {
    if (!((mMovementStates.mHighWord >> 20) & 1U) && !checkWallCodeNorm(8, nullptr, false) && (!isStatusActive(MarioStatus_Skate) || !checkWallFloorCode(0x21))) {
        if (isSwimming()) {
            if (((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
                mSwim->hitWall(*mFrontWallTriangle->getNormal(0), mFrontWallTriangle->mSensor);
            }

            if (((getPlayer()->mMovementStates.mLowWord >> 6) & 1U)) {
                mSwim->hitWall(*mBackWallTriangle->getNormal(0), mBackWallTriangle->mSensor);
            }

            if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U)) {
                mSwim->hitWall(*mSideWallTriangle->getNormal(0), mSideWallTriangle->mSensor);
            }
        } else {
            stopWalk();
            TVec3f velocity(getWallNorm() * mActor->getConst().getTable()->mWallSpinFlipGround);
            if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
                velocity *= mActor->getConst().getTable()->mWallSpinFlipAirRatio;
            }

            velocity -= *getGravityVec() * mActor->getConst().getTable()->mWallSpinHopGround;
            tryForcePowerJump(velocity, true);
            mMovementStates.mHighWord |= 0x00100000U;
            startPadVib(2);
            playSound("壁反射");
            playSound("声スピンキャンセル");
            playEffectTrans("壁ヒット", getWallPos());
            if (((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
                sendPunch(mFrontWallTriangle->mSensor, mFrontWallTriangle, true);
            }

            if (((getPlayer()->mMovementStates.mLowWord >> 6) & 1U)) {
                sendPunch(mBackWallTriangle->mSensor, mBackWallTriangle, true);
            }

            if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U)) {
                sendPunch(mSideWallTriangle->mSensor, mSideWallTriangle, true);
            }

            mActor->_EF6 = 30;
        }
    }
}

bool MarioWall::notice() {
    return false;
}
