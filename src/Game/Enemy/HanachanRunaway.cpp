#include "Game/Enemy/HanachanRunaway.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Ride/TubeSlider.hpp"
#include "Game/Ride/TubeSliderFunction.hpp"


namespace NrvHanachanRunaway {
    // Wait: switches to Whistle when the switch A is on.
    class HanachanRunawayNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static HanachanRunawayNrvWait sInstance;
    };
    HanachanRunawayNrvWait HanachanRunawayNrvWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(HanachanRunawayNrvWhistle, HanachanRunaway, Whistle);
    NEW_NERVE(HanachanRunawayNrvBindStarPointer, HanachanRunaway, BindStarPointer);
};  // namespace NrvHanachanRunaway

#define GET_HANACHAN_RUNAWAY_NERVE(name) (&NrvHanachanRunaway::name::sInstance)

HanachanRunawayParts::HanachanRunawayParts(HanachanRunaway* pOwner, s32 index) : LiveActor("暴走ハナちゃん体（チューブスライダー）") {
    mOwner = pOwner;
    mIndex = index;
    mBaseMtx.identity();
}

HanachanRunawayParts::~HanachanRunawayParts() {
}

void HanachanRunawayParts::init(const JMapInfoIter& rIter) {
    const char* pName = mIndex % 2 == 0 ? "HanachanBody" : "HanachanBodyS";
    MR::initActor(this, rIter, pName, "TubeSlider", false);
    makeActorAppeared();
}

void HanachanRunawayParts::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mBaseMtx);
    MR::updateBaseScale(this, mOwner->mAnimScaleCtrl);
}

void HanachanRunawayParts::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    mOwner->attackSensor(pSender, pReceiver);
}

bool HanachanRunawayParts::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mOwner->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

HanachanRunaway::HanachanRunaway(TubeSlider* pSlider) : LiveActor("暴走ハナちゃん（チューブスライダー）") {
    mTubeSlider = pSlider;
    mAnimScaleCtrl = nullptr;
    mIsStrong = false;
    mPartsCount = 5;
    mParts = nullptr;
    mDir.set(0.0f, 0.0f, 1.0f);
    mRailCoord = 0.0f;
    mRotation = 1.0f;
    mSpeedA = 100;
    mSpeedB = 20;
    mIsPointed = false;
    mPrevRailCoord = 0.0f;
}

HanachanRunaway::~HanachanRunaway() {
}

// NONMATCHING: Remaining GPR allocation and load scheduling differ.
void HanachanRunaway::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mSpeedA);
    MR::getJMapInfoArg1NoInit(rIter, &mSpeedB);
    MR::getJMapInfoArg2NoInit(rIter, &mPartsCount);
    MR::getJMapInfoArg3NoInit(rIter, &mIsStrong);
    TubeSliderFunction::initDefaultPosOnTube(this, rIter, &mRailCoord, &mRotation, &mDir, 0.0f, mTubeSlider);
    MR::initActor(this, rIter, "HanachanHead", "TubeSlider", false);
    initNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvWhistle), 0);
    if (MR::isValidSwitchA(this)) {
        setNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvWait));
    }
    MR::setClippingFar300m(this);
    MR::validateClipping(this);
    mAnimScaleCtrl = new AnimScaleController(nullptr);
    MR::addEffect(this, "PointerTouch");

    mParts = new LiveActorGroup("体保持", mPartsCount - 1);
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        HanachanRunawayParts* pPart = new HanachanRunawayParts(this, i);
        MR::setPosition(pPart, mPosition);
        pPart->initWithoutIter();
        mParts->registerActor(pPart);
        MR::addEffect(pPart, "PointerTouch");
    }

    if (mIsStrong) {
        MR::startAction(this, "AngryDash");
        MR::setBckRate(this, 3.0f);
        MR::emitEffect(this, "AngrySmoke");
    } else {
        MR::startAction(this, "WalkNormal");
        MR::setBckRate(this, 1.0f);
    }

    for (s32 i = 0; i < mPartsCount - 1; i++) {
        LiveActor* pPart = mParts->getActor(i);
        if (mIsStrong) {
            MR::startAction(pPart, "AngryDash");
            MR::setBckRate(pPart, 3.0f);
        } else {
            MR::startAction(pPart, "WalkNormal");
            MR::setBckRate(pPart, 1.0f);
        }
        s16 frameMax = MR::getBckFrameMax(this, "Walk");
        MR::setBckFrame(pPart, static_cast<f32>((frameMax + i * 8) % frameMax));
    }

    makeActorAppeared();
}

void HanachanRunaway::initAfterPlacement() {
    f32 dRail = 0.1f * static_cast<f32>(-mSpeedA);
    f32 dRot = 0.1f * static_cast<f32>(mSpeedB);
    mRailCoord -= dRail;
    mRotation -= dRot;
    MR::repeatDegree(&mRotation);
    fn_80194540();
    mRailCoord += dRail;
    mRotation += dRot;
    MR::repeatDegree(&mRotation);
    fn_80194540();
}

void HanachanRunaway::startClipped() {
    LiveActor::startClipped();
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        LiveActor* pPart = mParts->getActor(i);
        if (!MR::isDead(pPart)) {
            pPart->startClipped();
        }
    }
}

void HanachanRunaway::endClipped() {
    LiveActor::endClipped();
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        LiveActor* pPart = mParts->getActor(i);
        if (!MR::isDead(pPart)) {
            pPart->endClipped();
        }
    }
}

// NONMATCHING: Remaining FP register allocation and conversion scheduling differ.
void HanachanRunaway::control() {
    mAnimScaleCtrl->updateNerve();
    if (!isNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvBindStarPointer)) && isStarPointerPointing()) {
        setNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvBindStarPointer));
        return;
    }
    f32 micDist = MR::getMicDistance(this);
    f32 playerDist = MR::calcDistanceToPlayer(this);
    if (playerDist <= 450.0f) {
        mIsPointed = true;
    }
    TVec3f relVel = *MR::getPlayerVelocity() - mVelocity;
    f32 relSpeed = PSVECMag(&relVel);
    s32 base = 50;
    f32 extra = 1000.0f * (100.0f * (2.3f / micDist));
    f32 maxSpeed = base + extra < 0.0f ? 0.0f : (base + extra > 1000.0f ? 1000.0f : base + extra);
    f32 volume = MR::getLinerValueFromMinMax(relSpeed, 27.0f, 34.0f, 0.0f, maxSpeed);
    s32 offset = -100;
    f32 pitch = offset + 1000.0f * (50.0f / (50.0f + (micDist - mPrevRailCoord)));
    if (playerDist <= 1000.0f && mIsPointed) {
        s32 level = (s32)pitch < 50 ? 50 : ((s32)pitch > 1000 ? 1000 : (s32)pitch);
        MR::startActionSound(this, "MarioPassLvSlider", (s32)volume, level, -1);
    }
    mPrevRailCoord = micDist;
}

// NONMATCHING: Remaining FP register allocation and load scheduling differ.
void HanachanRunaway::exeWhistle() {
    MR::isFirstStep(this);
    mRailCoord += 0.1f * static_cast<f32>(-mSpeedA);
    mRotation += 0.1f * static_cast<f32>(mSpeedB);
    MR::repeatDegree(&mRotation);
    fn_80194540();
    if (mIsStrong && MR::calcDistanceToPlayer(this) <= 1000.0f) {
        MR::startActionSound(this, "Whistle", -1, -1, 5);
    }
}

void HanachanRunaway::exeBindStarPointer() {
    if (MR::isFirstStep(this)) {
        MR::startDPDHitSound();
        mAnimScaleCtrl->startDpdHitVibration();
        MR::emitEffect(this, "PointerTouch");
        MR::setBckRate(this, 0.0f);
        for (s32 i = 0; i < mPartsCount - 1; i++) {
            MR::emitEffect(mParts->getActor(i), "PointerTouch");
            MR::setBckRate(mParts->getActor(i), 0.0f);
        }
    }

    MR::startDPDFreezeLevelSound(this);
    MR::zeroVelocity(this);
    if (!isStarPointerPointing() && MR::isGreaterStep(this, 5)) {
        mAnimScaleCtrl->startAnim();
        MR::deleteEffect(this, "PointerTouch");
        f32 rate = 1.0f;
        if (mIsStrong) {
            rate = 3.0f;
        }
        MR::setBckRate(this, rate);
        for (s32 i = 0; i < mPartsCount - 1; i++) {
            MR::deleteEffect(mParts->getActor(i), "PointerTouch");
            MR::setBckRate(mParts->getActor(i), rate);
        }
        setNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvWhistle));
    }
}

void HanachanRunaway::calcAndSetBaseMtx() {
    TPos3f mtx;
    MR::makeMtxFrontUpPos(&mtx, mDir, -mGravity, mPosition);
    MR::setBaseTRMtx(this, mtx);
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

#pragma push
#pragma global_optimizer off
void HanachanRunaway::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorRide(pReceiver)) {
        if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            return;
        }
    }
}
#pragma pop

bool HanachanRunaway::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    return MR::isMsgStarPieceReflect(msg);
}

bool HanachanRunaway::isStarPointerPointing() {
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        return true;
    }
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        if (MR::isStarPointerPointing2PAttachTarget(mParts->getActor(i))) {
            return true;
        }
    }
    return false;
}

// NONMATCHING: Remaining FP/GPR allocation, matrix-copy scheduling and temporary slots differ.
void HanachanRunaway::fn_80194540() {
    f32 railStep;
    f32 rotationStep;
    f32 railSpeed = 0.1f * (f32)-mSpeedA;
    f32 rotationSpeed = 0.1f * (f32)mSpeedB;
    TVec3f prev(mPosition);
    TVec3f dir(0.0f);
    TubeSliderFunction::unknown(this, &dir, mTubeSlider, mRailCoord, mRotation, 0.0f);
    mDir = mPosition - prev;
    MR::normalize(&mDir);
    f32 coord = mRailCoord;
    f32 rotation = mRotation;
    f32 circumference = 6.2831855f * (2.0f * TubeSliderFunction::getRadiusAtRailCoord(coord, mTubeSlider));
    railStep = 0.0f;
    if (railSpeed == 0.0f) {
        rotationStep = 3.1415927f * (360.0f * (70.0f / circumference));
    } else if (rotationSpeed == 0.0f) {
        rotationStep = 0.0f;
        railStep = 70.0f;
    } else {
        f32 ratio = __fabsf(__fabsf(railSpeed / ((circumference * rotationSpeed) / 360.0f)));
        f32 step = 70.0f * (1.0f / (1.0f + ratio));
        railStep = step * ratio;
        rotationStep = step * (3.1415927f * (360.0f * (1.0f / circumference)));
    }
    TPos3f mtx;
    mtx.identity();
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        HanachanRunawayParts* part = static_cast<HanachanRunawayParts*>(mParts->getActor(i));
        TVec3f front;
        front.x = 0.0f;
        front.y = 0.0f;
        front.z = 1.0f;
        if (railSpeed > 0.0f) {
            coord -= railStep;
        } else if (railSpeed < 0.0f) {
            coord += railStep;
        }
        if (rotationSpeed > 0.0f) {
            rotation -= rotationStep;
        } else if (rotationSpeed < 0.0f) {
            rotation += rotationStep;
        }
        MR::repeatDegree(&rotation);
        TVec3f previous(part->mPosition);
        TubeSliderFunction::unknown(part, &front, mTubeSlider, coord, rotation, 0.0f);
        if (!MR::isNearZero(part->mPosition - previous, 0.001f)) {
            MR::makeMtxFrontUpPos(&mtx, part->mPosition - previous, -part->mGravity, part->mPosition);
            part->mBaseMtx.set(mtx);
        }
    }
}

// Parts ---------------------------------------------------------------------------------------------

// Nerve ----------------------------------------------------------------------------------------------

void NrvHanachanRunaway::HanachanRunawayNrvWait::execute(Spine* pSpine) const {
    HanachanRunaway* pActor = reinterpret_cast<HanachanRunaway*>(pSpine->mExecutor);
    if (MR::isValidSwitchA(pActor) && MR::isOnSwitchA(pActor)) {
        pActor->setNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvWhistle));
    }
}
