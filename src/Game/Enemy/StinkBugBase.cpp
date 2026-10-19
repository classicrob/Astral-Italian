#include "Game/Enemy/StinkBugBase.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JMath/JMath.hpp>

StinkBugBase::StinkBugBase(const char* pName)
    : LiveActor(pName), mFront(0.0f, 0.0f, 1.0f), mHomePos(0.0f, 0.0f, 0.0f), mHomeFront(0.0f, 0.0f, 1.0f), mMaxTurnDegree(180.0f), mRadius(1000.0f), mTurnPositive(false) {
}

void StinkBugBase::setDashVelocity(f32 velocity) {
    TVec3f result;
    f32 radius = mRadius;
    result.scaleAdd(radius, mFront, mHomePos);
    f32 distance = result.distance(mPosition);
    distance = MR::min(velocity, distance);
    mVelocity.scale(distance, mFront);
}

void StinkBugBase::fixInitPos() {
    mVelocity.zero();
    MR::setPosition(this, mHomePos);
}

void StinkBugBase::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    const char* objectName;
    MR::getObjectName(&objectName, rIter);
    MR::initActor(this, rIter, objectName, false);
    MR::connectToSceneCollisionEnemyNoShadowedMapObjStrongLight(this);
    MR::initLightCtrl(this);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    initEffectKeeper(2, nullptr, false);
    MR::addEffectHitNormal(this, nullptr);
    MR::initUseStageSwitchWriteDead(this, rIter);
    mHomePos.set(mPosition);
    MR::calcFrontVec(&mFront, this);
    mHomeFront.set(mFront);
    TVec3f upVec;
    MR::calcUpVec(&upVec, this);
    mGravity.negate(upVec);
}

#pragma push
#pragma global_optimizer off
bool StinkBugBase::isPlayerInTerritory(f32 arg1, f32 arg2, f32 arg3, f32 arg4) const {
    if (!MR::isNearPlayer(this, mRadius + arg1)) {
        return false;
    }

    const TVec3f& rPos = mPosition;
    TVec3f v1;
    v1.sub(*MR::getPlayerPos(), rPos);

    TVec3f upVec;
    MR::calcUpVec(&upVec, this);

    f32 dot = v1.dot(upVec);

    if (MR::abs(dot) > arg3) {
        return false;
    }

    TVec3f sideVec;
    MR::calcSideVec(&sideVec, this);

    dot = v1.dot(sideVec);

    if (MR::abs(dot) > arg2) {
        return false;
    }

    if (mMaxTurnDegree == 0.0f) {
        return v1.dot(mFront) > 0.0f;
    }

    if (mMaxTurnDegree == 180.0f) {
        return true;
    }

    f32 f1 = MR::abs(MR::sinDegree(mMaxTurnDegree));

    const TVec3f& rPos2 = mPosition;
    TVec3f scaledAdded;
    scaledAdded.scaleAdd(-arg4 / f1, mHomeFront, rPos2);

    TVec3f v3;
    v3.sub(*MR::getPlayerPos(), scaledAdded);

    TVec3f v5 = v3.killElement(mGravity);

    return MR::isNearAngleDegree(v5, mHomeFront, mMaxTurnDegree);
}
#pragma pop

bool StinkBugBase::tryTurnSearch(f32 angle) {
    if (mTurnPositive) {
        MR::rotateVecDegree(&mFront, mGravity, angle);
    } else {
        MR::rotateVecDegree(&mFront, mGravity, -angle);
    }

    if (!MR::isNearAngleDegree(mFront, mHomeFront, mMaxTurnDegree)) {
        MR::clampVecAngleDeg(&mFront, mHomeFront, mMaxTurnDegree);
        mTurnPositive = !mTurnPositive;
        return true;
    }

    return false;
}

bool StinkBugBase::tryTurnDashSign(f32 angle) {
    TVec3f* playerPos = MR::getPlayerPos();
    MR::turnDirectionToTargetUseGroundNormalDegree(this, &mFront, *playerPos, angle);
    MR::clampVecAngleDeg(&mFront, mHomeFront, mMaxTurnDegree);
    return true;
}

bool StinkBugBase::isHitHorn(HitSensor* pSender, HitSensor* pReceiver, f32 arg3) const {
    TVec3f toReceiver;
    toReceiver.sub(*MR::getSensorPos(pReceiver), *MR::getSensorPos(pSender));

    if (toReceiver.dot(mFront) < 0.0f) {
        return false;
    }

    TVec3f up;
    MR::calcUpVec(&up, this);

    TVec3f v2;
    v2.scale(up.dot(toReceiver), up);

    f32 length = v2.length();
    return length <= arg3 + MR::getSensorRadius(pReceiver);
}
