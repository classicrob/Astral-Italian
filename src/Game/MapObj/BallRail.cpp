#include "Game/MapObj/BallRail.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/PlayerUtil.hpp"

namespace NrvBallRail {
    NEW_NERVE(BallRailNrvWait, BallRail, Wait);
    NEW_NERVE(BallRailNrvSetUp, BallRail, SetUp);
    NEW_NERVE(BallRailNrvRun, BallRail, Run);
    NEW_NERVE(BallRailNrvNoBind, BallRail, NoBind);
};  // namespace NrvBallRail

BallRail::BallRail(const char* pName)
    : LiveActor(pName), mRailPoints(), mBindSensor(), mNumPoints(), mAcceleration(1.5f), mDeceleration(0.995f), mBindRadius(100.0f) {
    mBaseMtx.identity();
}

void BallRail::init(const JMapInfoIter& rIter) {
    if (MR::getJMapInfoArg0NoInit(rIter, &mAcceleration)) {
        mAcceleration /= 1000.0f;
    }

    if (MR::getJMapInfoArg1NoInit(rIter, &mDeceleration)) {
        mDeceleration /= 1000.0f;
    }

    MR::connectToSceneMapObjMovement(this);
    MR::invalidateClipping(this);
    initRailRider(rIter);
    initRailPoints();
    MR::moveCoordToStartPos(this);
    MR::moveTransToCurrentRailPos(this);
    MR::onCalcGravity(this);
    MR::setRailCoordSpeed(this, 0.0f);
    initHitSensor(1);
    TVec3f offs(0.0f, mBindRadius, 0.0f);
    MR::addHitSensor(this, "bind", 0x76 /* ATYPE_BALL_RAIL */, 8, mBindRadius, offs);
    MR::makeMtxTR(mBaseMtx, this);
    MR::addBaseMatrixFollowTarget(this, rIter, &mBaseMtx, MR::createBaseMatrixFollowValidateDelegator(this, &BallRail::isValidFollow));
    initNerve(GET_NERVE(BallRail, BallRailNrvWait));
    appear();
}

void BallRail::control() {
}

bool BallRail::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xA4 /* ACTMES_SPHERE_PLAYER_BINDED */ && isNerve(GET_NERVE(BallRail, BallRailNrvWait))) {
        TVec3f v9(MR::getSensorHost(pSender)->mVelocity);
        TVec3f v8 = *MR::getSensorPos(pSender) - *MR::getSensorPos(pReceiver);

        if (v8.dot(v9) > 0.0f) {
            return false;
        }

        if (mGravity.dot(v8) > 0.0f) {
            MR::normalizeOrZero(&v8);
            v8.dot(v9);
            MR::zeroVelocity(MR::getSensorHost(pSender));

            return false;
        } else {
            mBindSensor = pSender;

            return true;
        }
    }

    return false;
}

namespace {
    inline void initPointSide(BallRailPoint* pPoint, const TVec3f& rUp) {
        pPoint->mSide.cross(pPoint->mDirection, rUp);
        MR::normalizeOrZero(&pPoint->mSide);
    }
};  // namespace

void BallRail::initRailPoints() {
    u32 numPoints = (u32)(MR::getRailTotalLength(this) / 100.0f) + 2;
    mNumPoints = numPoints;
    mRailPoints = new BallRailPoint[numPoints];
    MR::moveCoordToStartPos(this);
    MR::setRailCoordSpeed(this, 100.0f);

    for (u32 i = 0; i < mNumPoints - 1; i++) {
        mRailPoints[i].mPos = MR::getRailPos(this);
        mRailPoints[i].mDirection = MR::getRailDirection(this);
        MR::moveRailRider(this);
    }

    MR::moveCoordToEndPos(this);
    mRailPoints[mNumPoints - 1].mPos = MR::getRailPos(this);
    mRailPoints[mNumPoints - 1].mDirection = MR::getRailDirection(this);
    MR::moveCoordToStartPos(this);
    MR::setRailCoordSpeed(this, 0.0f);

    BallRailPoint* pLastPoint;
    BallRailPoint* pPoint;
    TVec3f up(0, 1, 0);

    if (mNumPoints >= 2U) {
        ::initPointSide(mRailPoints, up);
    }

    for (u32 i = 0; i < mNumPoints; i++) {
        pPoint = &mRailPoints[i];
        pPoint->mSide.cross(pPoint->mDirection, up);
        MR::normalizeOrZero(&pPoint->mSide);
    }

    if (mNumPoints >= 2U) {
        pLastPoint = &mRailPoints[mNumPoints - 1];
        pLastPoint->mSide.cross(pLastPoint->mDirection, up);
        MR::normalizeOrZero(&pLastPoint->mSide);
    }
}

void BallRail::exeWait() {
    MR::moveCoordToNearestPos(this, *MR::getPlayerPos());
    MR::moveTransToCurrentRailPos(this);

    if (mBindSensor != nullptr) {
        setNerve(GET_NERVE(BallRail, BallRailNrvSetUp));
    }
}

void BallRail::exeSetUp() {
    if (MR::isFirstStep(this)) {
        mSetUpStartPos = *MR::getSensorPos(mBindSensor);
        MR::zeroVelocity(MR::getSensorHost(mBindSensor));
        MR::setRailCoordSpeed(this, 0.0f);
    }

    MR::moveCoord(this, 3.0f);
    MR::moveTransToCurrentRailPos(this);

    TVec3f v7;

    if (MR::isLessStep(this, 15)) {
        TVec3f v6;
        v6.scaleAdd(-MR::getSensorRadius(mBindSensor), mGravity, mPosition);
        f32 rate = MR::calcNerveEaseInRate(this, 15);
        MR::vecBlend(mSetUpStartPos, v6, &v7, rate);
    } else {
        v7.scaleAdd(-MR::getSensorRadius(mBindSensor), mGravity, mPosition);
    }

    if (MR::isStep(this, 15)) {
        MR::tryRumblePadStrong(this, WPAD_CHAN0);
    }

    LiveActor* pHost = MR::getSensorHost(mBindSensor);
    pHost->mVelocity.set(v7 - *MR::getSensorPos(mBindSensor));

    if (MR::isGreaterStep(this, 45)) {
        setNerve(GET_NERVE(BallRail, BallRailNrvRun));
    } else {
        PSMTXCopy(MR::getPlayerBaseMtx(), mBaseMtx);
    }
}

void BallRail::exeRun() {
    if (MR::isFirstStep(this)) {
        MR::setRailCoordSpeed(this, 0.0f);
    }

    TVec3f v14 = MR::getRailDirection(this);

    TVec3f acceleration = mGravity * mAcceleration;
    MR::accelerateRailCoordSpeed(this, acceleration.dot(v14));
    TVec3f v12;
    MR::slowDownRailCoordSpeed(this, mDeceleration);

    if (!MR::isRailGoingToEnd(this) || MR::getRailCoordSpeed(this) < 6.0f) {
        if (!MR::isRailGoingToEnd(this)) {
            MR::setRailDirectionToEnd(this);
        }

        MR::setRailCoordSpeed(this, 6.0f);
    }

    MR::moveRailRider(this);
    MR::moveTransToCurrentRailPos(this);
    v12.scaleAdd(-MR::getSensorRadius(mBindSensor), mGravity, mPosition);
    LiveActor* pHost = MR::getSensorHost(mBindSensor);
    pHost->mVelocity.set(v12 - *MR::getSensorPos(mBindSensor));

    if (MR::isRailReachedGoal(this)) {
        LiveActor* pActor = MR::getSensorHost(mBindSensor);
        pActor->mVelocity.set(MR::getRailDirection(this) * MR::getRailCoordSpeed(this));
        MR::sendArbitraryMsg(0xA9 /* ACTMES_END_BALL_RAIL */, mBindSensor, getSensor("bind"));
        mBindSensor = nullptr;
        setNerve(GET_NERVE(BallRail, BallRailNrvNoBind));
    } else {
        PSMTXCopy(MR::getPlayerBaseMtx(), mBaseMtx);
    }
}

bool BallRail::isValidFollow(s32) const {
    bool result = false;

    if (isNerve(GET_NERVE(BallRail, BallRailNrvSetUp)) || isNerve(GET_NERVE(BallRail, BallRailNrvRun))) {
        result = true;
    }

    return result;
}

void BallRail::exeNoBind() {
    if (getNerveStep() > 60) {
        setNerve(GET_NERVE(BallRail, BallRailNrvWait));
    }
}

BallRailPoint::BallRailPoint() : mPos(0, 0, 0), mSide(1, 0, 0), mUp(0, 1, 0), mDirection(0, 0, 1) {
}

BallRail::~BallRail() {
}
