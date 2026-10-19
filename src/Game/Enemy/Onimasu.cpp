#include "Game/Enemy/Onimasu.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

void Onimasu_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.000003814697265625f;
    (void)0.0f;
    (void)2.0f;
    (void)1.5707963705062866f;
    (void)-1.0f;
    (void)0.5f;
    (void)3.1415927410125732f;
}

namespace {
    static const f32 sGravityScalar = 1.6f;
    // static const f32 sQuickRatio = ;
    static const f32 sGravityScalarQuick = 6.4f;
    static const f32 sCamShakeDistanceStrong = 2000.0f;
    static const f32 sCamShakeDistanceWeak = 3500.0f;
    static const f32 sSensorRadius = 800.0f;
};  // namespace

namespace NrvOnimasu {
    NEW_NERVE(HostTypeWaitForSwitchOn, Onimasu, WaitForSwitchOn);
    NEW_NERVE(HostTypeWait, Onimasu, Wait);
    NEW_NERVE(HostTypeWaitForStamp, Onimasu, Wait);
    NEW_NERVE(HostTypeJump, Onimasu, Move);
    NEW_NERVE(HostTypeStamp, Onimasu, Stamp);
};  // namespace NrvOnimasu

Onimasu::~Onimasu() {
}

Onimasu::Onimasu(const char* pName) : LiveActor(pName), mStateParam(0), mWaitTime(90), mTimeToNextPoint(30), mWaitTimeQuick(45), mTimeToNextPointQuick(15), mRotateAxis(1.0f, 0.0f, 0.0f), mRailClipPos(gZeroVec) {
    mPointMtx.identity();
    mStartQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mTargetQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mCurrentQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

void Onimasu::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mWaitTime);
    MR::getJMapInfoArg1NoInit(rIter, &mTimeToNextPoint);
    MR::getJMapInfoArg2NoInit(rIter, &mWaitTimeQuick);
    MR::getJMapInfoArg4NoInit(rIter, &mTimeToNextPointQuick);
    MR::initDefaultPos(this, rIter);
    initRailRider(rIter);
    initFromRailPoint();
    MR::initActor(this, rIter, "Onimasu", false);

    MR::connectToSceneCollisionEnemyNoShadowedMapObjStrongLight(this);

    MR::initLightCtrl(this);
    initEffectKeeper(0, nullptr, false);

    MR::setEffectHostMtx(this, "Move", mPointMtx);

    initSound(4, "Onimasu", nullptr, TVec3f(0.0f));

    initHitSensor(1);
    MR::initCollisionParts(this, "Onimasu", MR::addHitSensor(this, "Body", ATYPE_MAP_OBJ_PRESS, 8, ::sSensorRadius, TVec3f(0.0f, 0.0f, 0.0f)),
                           nullptr);

    MR::initAndSetRailClipping(&mRailClipPos, this, 100.0f, 500.0f);

    MR::setGroupClipping(this, rIter, 16);

    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);

    MR::onCalcGravity(this);

    mStateParam = new ActorStateParamScale(this);
    mStateParam->init(rIter);

    if (MR::initUseStageSwitchReadA(this, rIter)) {
        initNerve(GET_NERVE(Onimasu, HostTypeWaitForSwitchOn));
    } else {
        initNerve(GET_NERVE(Onimasu, HostTypeWait));
    }

    MR::useStageSwitchAwake(this, rIter);

    makeActorAppeared();
}

void Onimasu::control() {
    mStateParam->calcHostVelocity();
}

void Onimasu::movement() {
    mStateParam->update();
    LiveActor::movement();
    mStateParam->resetHostVelocity();
}

void Onimasu::initAfterPlacement() {
    collectRailPointInfo();
    MR::moveCoordAndTransToRailStartPoint(this);
    MR::setRailDirectionToEnd(this);

    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR(mtx.toMtxPtr(), this);
    MR::setBaseTRMtx(this, mtx);

    if (MR::isEqualStageName("FactoryGalaxy")) {
        mtx.getXDir(mRotateAxis);
        MR::normalize(&mRotateAxis);
    }

    mtx.getQuat(mStartQuat);
    mTargetQuat.set(mStartQuat);
    mCurrentQuat.set(mTargetQuat);

    if (MR::isEqualStageName("FactoryGalaxy")) {
        f32 turnDirection = 0.0f;
        if (calcTurnDirection(&turnDirection) && turnDirection == -1.0f) {
            setNerve(GET_NERVE(Onimasu, HostTypeWaitForStamp));
        }
    }
}

void Onimasu::calcTargetPose() {
    TQuat4f rotationChange;
    rotationChange.set(0.0f, 0.0f, 0.0f, 1.0f);

    if (MR::isSameDirection(getLastPointNormal(), getNextPointNormal())) {
        f32 turnDirection = 0.0f;
        if (calcTurnDirection(&turnDirection)) {
            TPos3f mtx;
            mtx.identity();
            const f32 pi = MR::pi();
            mtx.makeRotate(getLastPointNormal(), pi * turnDirection);
            mtx.mult(mRotateAxis, mRotateAxis);

            MR::normalize(&mRotateAxis);
        } else {
            TVec3f lastPointPos;
            MR::calcRailPointPos(&lastPointPos, this, getLastPointNo());

            TVec3f nextPointPos;
            MR::calcRailPointPos(&nextPointPos, this, getNextPointNo());

            TVec3f railDir(nextPointPos - lastPointPos);
            MR::normalize(&railDir);

            TVec3f down;
            calcGravityDir(&down);
            mRotateAxis.cross(railDir, down);
            MR::normalize(&mRotateAxis);
        }
    } else {
        TQuat4f normalRotation;
        normalRotation.set(0.0f, 0.0f, 0.0f, 1.0f);
        normalRotation.setRotate(getLastPointNormal(), getNextPointNormal());
        rotationChange.set(normalRotation);
        normalRotation.getRotate(mRotateAxis);
        MR::normalize(&mRotateAxis);
    }

    mStartQuat.set(mTargetQuat);

    TQuat4f quarterTurn;
    quarterTurn.set(0.0f, 0.0f, 0.0f, 1.0f);
    quarterTurn.setRotate(mRotateAxis, MR::pi() / 2.0f);
    rotationChange.mult(quarterTurn);
    mTargetQuat.mult(rotationChange);

    TVec3f startFront;
    mStartQuat.getZDir(startFront);

    TVec3f targetFront;
    mTargetQuat.getZDir(targetFront);

    if (MR::isSameDirection(startFront, targetFront)) {
        TQuat4f adjustment;
        adjustment.setRotate(mRotateAxis, MR::pi() * 0.001f);
        mStartQuat.mult(adjustment);
    }
}

void Onimasu::updatePose() {
    f32 nerveRate = mStateParam->getNerveStepRate() / getTimeToNextPoint();
    if (1.0f < nerveRate) {
        nerveRate = 1.0f;
    }

    mCurrentQuat.set(mStartQuat);
    mCurrentQuat.slerp(mTargetQuat, nerveRate);
    mCurrentQuat.normalize();
}

void Onimasu::calcAndSetBaseMtx() {
    TPos3f baseMtx;
    baseMtx.identity();
    baseMtx.setQT(mCurrentQuat, mPosition);

    MR::setBaseTRMtx(this, baseMtx);
}

void Onimasu::land() {
    emitEffectLand();
    MR::startRumbleWithShakeCameraNormalWeak(this, "中", "弱", ::sCamShakeDistanceStrong, ::sCamShakeDistanceWeak);

    TVec3f railPointPos;
    MR::calcRailPointPos(&railPointPos, this, getNextPointNo());

    MR::setPosition(this, railPointPos);
    mVelocity.zero();

    MR::moveCoordToRailPoint(this, getNextPointNo());
}

bool Onimasu::calcTurnDirection(f32* pTurnDir) const {
    s32 arg0 = -1;
    MR::getCurrentRailPointArg0NoInit(this, &arg0);

    if (arg0 == -1) {
        return false;
    }

    if (arg0 == 0) {
        *pTurnDir = 0.5f;
        return true;
    }

    if (arg0 == 1) {
        *pTurnDir = -0.5f;
        return true;
    }

    if (arg0 == 2) {
        *pTurnDir = -1.0f;
        return true;
    }

    return false;
}

void Onimasu::updateStompVelocity() {
    f32 speed = getGravityScalar() * getTimeToNextPoint() * 0.5f;

    TVec3f gravityDir;
    gravityDir.set(-getNextPointNormal());
    mVelocity.set(-gravityDir * speed);
}

void Onimasu::calcGravityDir(TVec3f* pDir) const {
    pDir->set(-getNextPointNormal());
}

f32 Onimasu::getGravityScalar() const {
    return MR::isGalaxyQuickCometAppearInCurrentStage() ? ::sGravityScalarQuick : ::sGravityScalar;
}

s32 Onimasu::getTimeToNextPoint() const {
    bool quick = MR::isGalaxyQuickCometAppearInCurrentStage();
    return quick ? mTimeToNextPointQuick : mTimeToNextPoint;
}

s32 Onimasu::getWaitTime() const {
    bool quick = MR::isGalaxyQuickCometAppearInCurrentStage();
    return quick ? mWaitTimeQuick : mWaitTime;
}

void Onimasu::emitEffectLand() {
    TVec3f railPointPos;
    MR::calcRailPointPos(&railPointPos, this, getNextPointNo());

    TVec3f offset = (-getNextPointNormal() * ::sSensorRadius) * 0.5f;
    railPointPos.add(railPointPos, offset);

    if (MR::isSameDirection(mRotateAxis, getNextPointNormal())) {
        MR::makeMtxUpNoSupportPos(&mPointMtx, getNextPointNormal(), railPointPos);
    } else {
        MR::makeMtxUpSidePos(&mPointMtx, getNextPointNormal(), mRotateAxis, railPointPos);
    }

    MR::emitEffect(this, "Move");
    MR::startActionSound(this, "OjOnimasuLand");
}

void Onimasu::exeWaitForSwitchOn() {
    if (MR::isOnSwitchA(this)) {
        f32 turnDirection = 0.0f;
        if (calcTurnDirection(&turnDirection) && turnDirection == -1.0f) {
            setNerve(GET_NERVE(Onimasu, HostTypeWaitForStamp));
        } else {
            setNerve(GET_NERVE(Onimasu, HostTypeWait));
        }
    }
}

void Onimasu::exeWait() {
    if (MR::isStep(this, getWaitTime())) {
        if (isNerve(GET_NERVE(Onimasu, HostTypeWaitForStamp))) {
            setNerve(GET_NERVE(Onimasu, HostTypeStamp));
        } else {
            setNerve(GET_NERVE(Onimasu, HostTypeJump));
        }
    }
}

void Onimasu::exeMove() {
    if (mStateParam->isFirstStep()) {
        MR::startActionSound(this, "OjOnimasuJump");

        incrementNextPoint();
        calcTargetPose();
        startMoveInner();
    }

    if (MR::isStep(this, 1)) {
        MR::offCalcGravity(this);
    }

    updatePose();
    updatePoseInner();

    if (mStateParam->isStep(getTimeToNextPoint())) {
        land();

        f32 turnDirection = 0.0f;
        if (calcTurnDirection(&turnDirection) && turnDirection == -1.0f) {
            setNerve(GET_NERVE(Onimasu, HostTypeWaitForStamp));
        } else {
            setNerve(GET_NERVE(Onimasu, HostTypeWait));
        }
    }
}

void Onimasu::exeStamp() {
    if (mStateParam->isFirstStep()) {
        MR::startActionSound(this, "OjOnimasuJump");
        updateStompVelocity();
    }

    TVec3f gravityDir;
    calcGravityDir(&gravityDir);

    mStateParam->calcHostVelocity(gravityDir * getGravityScalar());

    if (mStateParam->isStep(getTimeToNextPoint())) {
        land();
        setNerve(GET_NERVE(Onimasu, HostTypeWait));
    }
}

namespace OnimasuFunction {
    bool getPolygonOnRailPoint(TVec3f* pPosition, TVec3f* pNormal, const LiveActor* pActor, int pointNo) {
        TVec3f railPosition;
        MR::calcRailPointPos(&railPosition, pActor, pointNo);

        TVec3f gravity(0.0f, 1.0f, 0.0f);
        MR::calcGravityVector(pActor, railPosition, &gravity, nullptr, 0);
        TVec3f direction(gravity);
        gravity.setLength(2000.0f);

        Triangle triangle;
        if (!MR::getFirstPolyOnLineToMapExceptSensor(pPosition, &triangle, railPosition, gravity, pActor->getSensor("Body"))) {
            return false;
        }

        pNormal->set(*triangle.getFaceNormal());
        return true;
    }
};  // namespace OnimasuFunction
