#define JGEOMETRY_VEC3_NEG_NOINLINE
#include "Game/MapObj/MapPartsFloatingForce.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/SpringValue.hpp"

namespace NrvMapPartsFloatingForce {
    NEW_NERVE(HostTypeWait, MapPartsFloatingForce, Wait);
    NEW_NERVE(HostTypeMove, MapPartsFloatingForce, Move);
    NEW_NERVE(HostTypeMoveSpring, MapPartsFloatingForce, MoveSpring);
    NEW_NERVE(HostTypeMoveReturn, MapPartsFloatingForce, MoveReturn);
    NEW_NERVE(HostTypeMoveEnd, MapPartsFloatingForce, MoveEnd);
};  // namespace NrvMapPartsFloatingForce

SpringStep::SpringStep(f32 cond, f32 speed, f32 angle) : mSpringValue() {
    mCondition = cond;
    mSpeed = speed;
    mAngle = angle;
    mSpringValue = new SpringValue(cond, 0.0f, speed, angle, 0.0f);
}

void SpringStep::addSpringVelocity(f32 vel) {
    mSpringValue->mVelocity += vel;
}

void SpringStep::setSpringBaseValue(f32 val) {
    mCondition = val;
    mSpringValue->setParam(static_cast< f64 >(val), 0.0f, mSpeed, mAngle, 0.0f);
}

MapPartsFloatingForce::MapPartsFloatingForce(LiveActor* pActor)
    : MapPartsFunction(pActor, "浮力"), mSpringStep(), mObjectName(), mMaxOffset(), mRotateAngle(), mRotateSpeed(), mReturnMargin(10.0f), mRotateAccelType(), mBasePos(gZeroVec),
      mUpDirection(0.0f, 1.0f, 0.0f), mCurOffset(), mVerticalSpeed() {
}

void MapPartsFloatingForce::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE(MapPartsFloatingForce, HostTypeWait));
    MR::getMapPartsArgRotateAccelType(&mRotateAccelType, rIter);

    if (mRotateAccelType == 1) {
        initForSpringMotion(rIter);
    } else {
        initForNormalMotion(rIter);
    }

    MR::initUseStageSwitchReadA(mHost, rIter);
}

void MapPartsFloatingForce::setObjectName(const char* pName) {
    mObjectName = pName;
}

void MapPartsFloatingForce::initForNormalMotion(const JMapInfoIter& rIter) {
    s32 conditionType = 0;
    MR::getMapPartsArgMoveConditionType(&conditionType, rIter);
    mMaxOffset = conditionType;
    MR::getMapPartsArgRotateSpeed(&mRotateSpeed, rIter);
    MR::getMapPartsArgRotateAngle(&mRotateAngle, rIter);
}

void MapPartsFloatingForce::initForSpringMotion(const JMapInfoIter& rIter) {
    s32 conditionType = 0;
    MR::getMapPartsArgMoveConditionType(&conditionType, rIter);
    mMaxOffset = conditionType;
    f32 rotateSpeed = 0.0f;
    MR::getMapPartsArgRotateSpeed(&rotateSpeed, rIter);
    rotateSpeed *= 0.001f;
    f32 rotateAngle = 0.0f;
    MR::getMapPartsArgRotateAngle(&rotateAngle, rIter);
    rotateAngle *= 0.001f;
    s32 rotateAxis = 0;
    MR::getMapPartsArgRotateAxis(&rotateAxis, rIter);
    mSpringKick = 0.1f * rotateAxis;
    mSpringStep = new SpringStep(-conditionType, rotateSpeed, rotateAngle);
}

void MapPartsFloatingForce::start() {
    mBasePos.set(mHost->mPosition);
    TPos3f baseMtx;
    baseMtx.set(mHost->getBaseMtx());
    baseMtx.getYDir(mUpDirection);
    MR::normalize(&mUpDirection);
    if (mRotateAccelType != 1) {
        setNerve(GET_NERVE(MapPartsFloatingForce, HostTypeMove));
    }
}

void MapPartsFloatingForce::updateVelocity() {
    if (MR::isOnPlayer(MR::getBodySensor(mHost))) {
        if (mMaxOffset <= mCurOffset) {
            mVerticalSpeed = MR::converge< f32 >(mVerticalSpeed, 0.0f, 0.05f);
        } else {
            mVerticalSpeed = mVerticalSpeed + (0.0001f * mRotateSpeed);
        }
    } else {
        if (mCurOffset <= 0.0f) {
            if (!MR::isNearZero(mVerticalSpeed, 0.001f)) {
                MR::startActionSoundAuto(mHost, "MapPartsFloaterMoveUpEnd", -1, -1, -1);
                mVerticalSpeed = 0.0f;
            }
        } else {
            mVerticalSpeed = mVerticalSpeed - (0.0001f * mRotateAngle);
        }
    }

    mVerticalSpeed *= 0.98f;
    mVerticalSpeed = MR::clamp(mVerticalSpeed, -50.0f, 50.0f);
}

void MapPartsFloatingForce::updateVelocitySpring() {
    f32 velocity = mSpringStep->mSpringValue->mVelocity;
    bool isSwitchOn = false;
    if (MR::isValidSwitchA(mHost) && MR::isOnSwitchA(mHost)) {
        isSwitchOn = true;
    }

    if (isSwitchOn) {
        velocity *= 3.0f;
    }

    mMoveVelocity.set(mUpDirection * velocity);
    if (MR::isNearZero(mMoveVelocity)) {
        mMoveVelocity.zero();
    }

    mSpringStep->mSpringValue->update();
}

bool MapPartsFloatingForce::tryOn() {
    bool isSwitchOn = false;

    if (MR::isValidSwitchA(mHost) && MR::isOnSwitchA(mHost)) {
        isSwitchOn = true;
    }

    if (isSwitchOn) {
        return false;
    }

    if (MR::isOnPlayer(MR::getBodySensor(mHost))) {
        setNerve(GET_NERVE(MapPartsFloatingForce, HostTypeMoveSpring));
        MR::startActionSoundAuto(mHost, "MapPartsFloaterRidden", -1, -1, -1);
        return true;
    }

    return false;
}

bool MapPartsFloatingForce::tryReturn() {
    bool isSwitchOn = false;

    if (MR::isValidSwitchA(mHost) && MR::isOnSwitchA(mHost)) {
        isSwitchOn = true;
    }

    if (!isSwitchOn) {
        if (MR::isOnPlayer(MR::getBodySensor(mHost))) {
            return false;
        }
    }

    setNerve(GET_NERVE(MapPartsFloatingForce, HostTypeMoveReturn));
    return true;
}

void MapPartsFloatingForce::exeWait() {
    if (isFirstStep()) {
        mCurOffset = 0.0f;
    }

    if (mRotateAccelType == 1) {
        tryOn();
    }
}

void MapPartsFloatingForce::exeMove() {
    updateVelocity();
    mMoveVelocity.set(-mUpDirection * mVerticalSpeed);
    mCurOffset += mVerticalSpeed;
    mCurOffset = MR::clamp(mCurOffset, 0.0f, mMaxOffset);
    LiveActor* pHost = mHost;
    MR::setPosition(pHost, mBasePos - mUpDirection * mCurOffset);

    if (mRotateAccelType == 2 && mMaxOffset <= mCurOffset) {
        MR::tryRumblePadWeak(this, 0);
        setNerve(GET_NERVE(MapPartsFloatingForce, HostTypeMoveEnd));
    }

    if (mObjectName != nullptr) {
        if (mVerticalSpeed > 0.1f) {
            MR::startActionSoundAuto(mHost, "MapPartsFloaterMoveDown", static_cast< s32 >(1000.0f * __fabsf(mVerticalSpeed)), -1, -1);
        } else if (mVerticalSpeed < -0.1f) {
            MR::startActionSoundAuto(mHost, "MapPartsFloaterMoveUp", static_cast< s32 >(1000.0f * __fabsf(mVerticalSpeed)), -1, -1);
        }
    }
}

void MapPartsFloatingForce::exeMoveSpring() {
    if (isFirstStep()) {
        mSpringStep->setSpringBaseValue(-mMaxOffset);
        mSpringStep->addSpringVelocity(-mSpringKick);
    }

    updateVelocitySpring();
    mHost->mVelocity.set(mMoveVelocity);
    tryReturn();
}

void MapPartsFloatingForce::exeMoveReturn() {
    if (isFirstStep()) {
        mSpringStep->setSpringBaseValue((mHost->mPosition - mBasePos).length());
    }

    updateVelocitySpring();
    mHost->mVelocity.set(mMoveVelocity);
    if (mObjectName != nullptr && 0.1f != mVerticalSpeed) {
        MR::startActionSoundAuto(mHost, "MapPartsFloaterMoveUp", static_cast< s32 >(1000.0f * __fabsf(mVerticalSpeed)), -1, -1);
    }

    if (MR::isNearZero(mBasePos - mHost->mPosition) && MR::isNearZero(mMoveVelocity)) {
        MR::setPosition(mHost, mBasePos);
        setNerve(GET_NERVE(MapPartsFloatingForce, HostTypeWait));
    } else {
        tryOn();
    }
}

void MapPartsFloatingForce::exeMoveEnd() {
    if (MR::isOnPlayer(MR::getBodySensor(mHost))) {
        if (mMaxOffset - mReturnMargin > mCurOffset) {
            mVerticalSpeed = 0.0f;
            mMoveVelocity = mUpDirection * mVerticalSpeed;
            mCurOffset = mMaxOffset - mReturnMargin;
            mVerticalSpeed = MR::converge< f32 >(mVerticalSpeed, 0.0f, 0.05f);
        } else {
            mVerticalSpeed = -mReturnMargin / 5.0f;
        }
    } else {
        mVerticalSpeed = 0.0f;
        mMoveVelocity = mUpDirection * mVerticalSpeed;
        setNerve(GET_NERVE(MapPartsFloatingForce, HostTypeMove));
    }

    mVerticalSpeed *= 0.98f;
    mVerticalSpeed = MR::clamp(mVerticalSpeed, -50.0f, 50.0f);
    mMoveVelocity.set(-mUpDirection * mVerticalSpeed);
    mCurOffset += mVerticalSpeed;
    mCurOffset = MR::clamp(mCurOffset, 0.0f, mMaxOffset);
    LiveActor* pHost = mHost;
    MR::setPosition(pHost, mBasePos - mUpDirection * mCurOffset);
}

MapPartsFloatingForce::~MapPartsFloatingForce() {
}
