#include "Game/Enemy/HanachanBig.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"


namespace NrvHanachanBig {
    NEW_NERVE(HanachanBigPartsNrvWalk, HanachanBigParts, Walk);
    NEW_NERVE(HanachanBigNrvWait, HanachanBig, Wait);
    NEW_NERVE(HanachanBigNrvHit, HanachanBig, Hit);
    NEW_NERVE(HanachanBigNrvStandby, HanachanBig, Standby);
}

HanachanBig::HanachanBig(const char* pName)
    : LiveActor(pName), mScaleController(nullptr), mPartsCount(9), mHitCount(0), mIsAngry(false) {}

// NONMATCHING: init loop float register assignment (f30/f31) and mPartsCount store/reload scheduling
void HanachanBig::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        MR::isConnectedWithRail(rIter);
        MR::initDefaultPos(this, rIter);
        initRailRider(rIter);
    }
    MR::connectToSceneEnemyMovement(this);
    initNerve(GET_NERVE(HanachanBig, HanachanBigNrvWait), 0);
    initHitSensor(1);
    TVec3f sensorOffset;
    sensorOffset.x = 0.0f;
    sensorOffset.y = 0.0f;
    sensorOffset.z = 0.0f;
    MR::addHitSensorEnemy(this, "Body", 8, 0.0f, sensorOffset);
    initSound(6, "HanachanBig", nullptr, TVec3f(0.0f));
    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    MR::validateClipping(this);
    MR::setGroupClipping(this, rIter, 32);
    MR::joinToGroupArray(this, rIter, mName, 3);
    s32 partsCount = -1;
    MR::getJMapInfoArg0NoInit(rIter, &partsCount);
    if (partsCount >= 3) {
        mPartsCount = partsCount;
        if (mPartsCount > 13) {
            mPartsCount = 13;
        }
    }
    mRange = -1.0f;
    MR::getJMapInfoArg1NoInit(rIter, &mRange);
    if (mRange < 0.0f) {
        mRange = 100.0f;
    }
    MR::getJMapInfoArg2NoInit(rIter, &mIsAngry);
    for (s32 i = 0; i < mPartsCount; i++) {
        mParts[i] = new HanachanBigParts(this, i, i == 0 ? "\x93\xaa\x83p\x81[\x83" "c" : "\x91\xcc\x83p\x81[\x83" "c");
        MR::setScale(mParts[i], mScale);
        mParts[i]->initWithoutIter();
    }
    MR::moveCoordToNearestPos(this, mPosition);
    MR::moveTransToOtherActorRailPos(mParts[0], this);
    MR::reverseRailDirection(this);
    for (s32 i = 1; i < mPartsCount; i++) {
        f32 a = 0.5f * MR::getSensorRadius(mParts[i], "BodySpace");
        f32 b = 0.5f * MR::getSensorRadius(mParts[i - 1], "BodySpace");
        MR::moveCoord(this, b + a);
        MR::moveTransToOtherActorRailPos(mParts[i], this);
    }
    MR::reverseRailDirection(this);
    MR::setClippingTypeSphere(this, 2000.0f * mScale.x);
    makeActorAppeared();
}

void HanachanBig::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    for (s32 i = 0; i < mPartsCount; i++) {
        MR::offBind(mParts[i]);
        mParts[i]->makeActorAppeared();
        if (mIsAngry) {
            mParts[i]->becomeAngry();
        }
    }
}

void HanachanBig::makeActorDead() {
    LiveActor::makeActorAppeared();
    for (s32 i = 0; i < mPartsCount; i++) {
        mParts[i]->makeActorDead();
    }
}

void HanachanBig::startClipped() {
    LiveActor::startClipped();
    for (s32 i = 0; i < mPartsCount; i++) {
        mParts[i]->startClipped();
    }
}

void HanachanBig::endClipped() {
    LiveActor::endClipped();
    for (s32 i = 0; i < mPartsCount; i++) {
        mParts[i]->endClipped();
    }
}

void HanachanBig::control() {
    mScaleController->updateNerve();
    MR::setPosition(this, mParts[mPartsCount / 2]->mPosition);
}

bool HanachanBig::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body")) {
        return false;
    }
    if (msg == 123) {
        mHitCount = 0;
        if (isNerve(GET_NERVE(HanachanBig, HanachanBigNrvWait))) {
            setNerve(GET_NERVE(HanachanBig, HanachanBigNrvStandby));
            return true;
        }
    }
    if (msg == 122) {
        if (isNerve(GET_NERVE(HanachanBig, HanachanBigNrvStandby))) {
            mScaleController->startAnim();
            f32 rate = 1.0f;
            if (mIsAngry) {
                rate = 3.0f;
            }
            for (s32 i = 0; i < mPartsCount; i++) {
                MR::setBckRate(mParts[i], rate);
            }
            setNerve(GET_NERVE(HanachanBig, HanachanBigNrvWait));
            return true;
        }
    }
    return false;
}

void HanachanBig::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        mHitCount = 0;
    }
    if (isStarPointerPointing()) {
        mHitCount = 0;
        setNerve(GET_NERVE(HanachanBig, HanachanBigNrvHit));
        MR::sendMsgToGroupMember(123, this, getSensor("Body"), "Body");
    }
    moveAlongRail(0.01f * (20.0f * mRange));
}

void HanachanBig::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
        mScaleController->startDpdHitVibration();
        for (s32 i = 0; i < mPartsCount; i++) {
            MR::emitEffect(mParts[i], "Touch");
            MR::setBckRate(mParts[i], 0.0f);
        }
    }
    MR::startDPDFreezeLevelSound(this);
    MR::zeroVelocity(this);
    bool isPointing = false;
    mHitCount++;
    if (isStarPointerPointing()) {
        mHitCount = 0;
        isPointing = true;
    }
    if (isPointing) {
        return;
    }
    if (!MR::isGreaterStep(this, 5)) {
        return;
    }
    if (mHitCount <= 5) {
        return;
    }
    mScaleController->startAnim();
    f32 rate = 1.0f;
    if (mIsAngry) {
        rate = 3.0f;
    }
    for (s32 i = 0; i < mPartsCount; i++) {
        MR::deleteEffect(mParts[i], "Touch");
        MR::setBckRate(mParts[i], rate);
    }
    MR::sendMsgToGroupMember(122, this, getSensor("Body"), "Body");
    setNerve(GET_NERVE(HanachanBig, HanachanBigNrvWait));
}

void HanachanBig::exeStandby() {
    if (!MR::isFirstStep(this)) {
        return;
    }
    for (s32 i = 0; i < mPartsCount; i++) {
        MR::setBckRate(mParts[i], 0.0f);
    }
}

// NONMATCHING: float register allocation (f28/f29 swap) in the body loop
void HanachanBig::moveAlongRail(f32 speed) {
    MR::moveCoord(this, speed);
    f32 railCoord = MR::getRailCoord(this);
    MR::moveTransToOtherActorRailPos(mParts[0], this);
    MR::calcRailDirection(&mParts[0]->mRailDirection, this);
    MR::reverseRailDirection(this);
    const f32 headRatio = 0.9f;
    for (s32 i = 1; i < mPartsCount; i++) {
        f32 a = headRatio * MR::getSensorRadius(mParts[i], "BodySpace");
        f32 b = 0.5f * MR::getSensorRadius(mParts[i - 1], "BodySpace");
        MR::moveCoord(this, b + a);
        MR::moveTransToOtherActorRailPos(mParts[i], this);
        MR::calcRailDirection(&mParts[i]->mRailDirection, this);
    }
    MR::reverseRailDirection(this);
    MR::setRailCoord(this, railCoord);
}

bool HanachanBig::isStarPointerPointing() {
    for (s32 i = 0; i < mPartsCount; i++) {
        if (MR::isStarPointerPointing2PAttachTarget(mParts[i])) {
            return true;
        }
    }
    return false;
}

HanachanBigParts::HanachanBigParts(HanachanBig* pHost, s32 index, const char* pName)
    : LiveActor(pName), mHost(pHost), mScaleController(nullptr), mRailDirection(0.0f, 0.0f, 1.0f), mPartsType(1),
      mPartsIndex(index), mIsAngry(false) {}

void HanachanBigParts::init(const JMapInfoIter& rIter) {
    const char* pName;
    if (mPartsIndex == 0) {
        mPartsType = PartsType_Head;
        pName = "HanachanHeadBig";
    } else if (mPartsIndex % 2 == 1) {
        mPartsType = PartsType_BodyS;
        pName = "HanachanBodySBig";
    } else {
        mPartsType = PartsType_Body;
        pName = "HanachanBodyBig";
    }
    MR::initActor(this, rIter, pName, false);
    mScaleController = new AnimScaleController(nullptr);
    mScaleController->setParamTight();
    initNerve(GET_NERVE(HanachanBig, HanachanBigPartsNrvWalk), 0);
    MR::invalidateClipping(this);
    MR::startBrk(this, "Normal");
    if (mPartsType == PartsType_Head) {
        MR::startBva(this, "normal");
    }
}

// NONMATCHING: target has one extra dead `b` to the epilogue (block layout)
void HanachanBigParts::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            HanachanBig* host = mHost;
            bool flag = host->isNerve(GET_NERVE(HanachanBig, HanachanBigNrvHit)) || host->isNerve(GET_NERVE(HanachanBig, HanachanBigNrvStandby));
            if (!flag) {
                if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    return;
                }
                MR::sendMsgPush(pReceiver, pSender);
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else if (MR::isSensorEnemy(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
            if (!mScaleController->isHitReaction(-1)) {
                mScaleController->startHitReaction();
            }
        }
    }
}

bool HanachanBigParts::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerTrample(msg)) {
        if (mScaleController->isHitReaction(-1)) {
            mScaleController->startHitReaction();
            return true;
        }
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        if (!mScaleController->isHitReaction(-1)) {
            mScaleController->startHitReaction();
            MR::sendMsgAwayJump(pSender, pReceiver);
            return true;
        }
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return false;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        if (!mScaleController->isHitReaction(-1)) {
            mScaleController->startHitReaction();
            return true;
        }
    }
    return false;
}

void HanachanBigParts::control() {
    mScaleController->updateNerve();
}

#pragma auto_inline off
void HanachanBigParts::calcAndSetBaseMtx() {
    MR::fn_8000F880(this, &mRailDirection);
    TVec3f scale;
    scale.mul(mHost->mScaleController->_C, mScale);
    scale.mul(scale, mScaleController->_C);
    MR::setBaseScale(this, scale);
}
#pragma auto_inline on

// NONMATCHING: instruction scheduling of rate multiply (mr r3 position) in first-step block
void HanachanBigParts::exeWalk() {
    if (MR::isFirstStep(this)) {
        f32 frame;
        if (mIsAngry) {
            MR::startAction(this, "AngryDash");
            MR::setBckRate(this, 3.0f);
            s16 frameMax = MR::getBckFrameMax(this, "Walk");
            frame = static_cast<f32>((frameMax + mPartsIndex * 8) % frameMax);
        } else {
            MR::startAction(this, "WalkNormal");
            MR::setBckRate(this, 1.0f);
            s16 frameMax = MR::getBckFrameMax(this, "Walk");
            frame = static_cast<f32>((frameMax + mPartsIndex * 8) % frameMax);
        }
        MR::setBckFrame(this, frame);
        f32 multiplier = 1.0f; f32 rate = mIsAngry ? 3.0f : 1.0f; MR::setBckRate(this, multiplier * rate);
    }
    if (mPartsType != PartsType_Head) {
        return;
    }
    if (!mIsAngry) {
        return;
    }
    if (!(MR::calcDistanceToPlayer(this) <= 2000.0f)) {
        return;
    }
    MR::startActionSound(this, "Whistle", -1, -1, 30);
}

void HanachanBigParts::becomeAngry() {
    if (mPartsType == PartsType_Head) {
        MR::emitEffect(this, "AngrySmoke");
    }
    mIsAngry = true;
}

HanachanBig::~HanachanBig() {}

HanachanBigParts::~HanachanBigParts() {}
