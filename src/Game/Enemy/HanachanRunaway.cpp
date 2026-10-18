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

HanachanRunaway::HanachanRunaway(TubeSlider* pSlider) : LiveActor("暴走ハナちゃん（チューブスライダー）") {
    mTubeSlider = pSlider;
    mAnimScaleCtrl = nullptr;
    mIsStrong = false;
    mPartsCount = 5;
    mParts = nullptr;
    mDir.set(0.0f, 0.0f, 0.0f);
    mDir.y = 1.0f;
    mRailCoord = 0.0f;
    mRotation = 0.0f;
    mSpeedA = 100;
    mSpeedB = 20;
    mIsPointed = false;
    mPrevRailCoord = 0.0f;
}

HanachanRunaway::~HanachanRunaway() {
}

void HanachanRunaway::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mSpeedA);
    MR::getJMapInfoArg1NoInit(rIter, &mSpeedB);
    MR::getJMapInfoArg2NoInit(rIter, &mPartsCount);
    MR::getJMapInfoArg3NoInit(rIter, &mIsStrong);
    TubeSliderFunction::initDefaultPosOnTube(this, rIter, &mRailCoord, &mRotation, &mDir, 0.0f, mTubeSlider);
    MR::initActor(this, rIter, "HanachanHead", "TubeSlider", nullptr, false);
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
        HanachanRunawayParts* pPart = new HanachanRunawayParts(this, reinterpret_cast<const char*>(i), nullptr);
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

    s16 frameMax = MR::getBckFrameMax(this, "Walk");
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        LiveActor* pPart = mParts->getActor(i);
        if (mIsStrong) {
            MR::startAction(pPart, "AngryDash");
            MR::setBckRate(pPart, 3.0f);
        } else {
            MR::startAction(pPart, "WalkNormal");
            MR::setBckRate(pPart, 1.0f);
        }
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
    updateTubeParts();
    mRailCoord += dRail;
    mRotation += dRot;
    MR::repeatDegree(&mRotation);
    updateTubeParts();
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

// NONMATCHING: the SMG2 speed-to-rail-coord curve is approximated from the target's float constants (~42%).
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

    TVec3f relVel = *MR::getPlayerVelocity();
    relVel.sub(mVelocity);
    f32 relSpeed = PSVECMag(&relVel);
    f32 clampSpeed = MR::getLinerValueFromMinMax(relSpeed, 27.0f, 34.0f, 0.0f, 70.0f);
    f32 lowest = 2.3f / micDist;
    f32 pushBack = -100.0f + 1000.0f * (1000.0f / (1000.0f + (mRailCoord - mPrevRailCoord)));
    if (pushBack < 50.0f) {
        pushBack = 50.0f;
    }
    if (playerDist == 1000.0f && mIsPointed) {
        mRailCoord += lowest * clampSpeed;
    }
    mPrevRailCoord = mRailCoord;
}

void HanachanRunaway::exeWhistle() {
    MR::isFirstStep(this);
    mRailCoord += 0.1f * static_cast<f32>(-mSpeedA);
    mRotation += 0.1f * static_cast<f32>(mSpeedB);
    MR::repeatDegree(&mRotation);
    updateTubeParts();
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
            LiveActor* pPart = mParts->getActor(i);
            MR::emitEffect(pPart, "PointerTouch");
            MR::setBckRate(pPart, 0.0f);
        }
    }

    MR::startDPDFreezeLevelSound(this);
    MR::zeroVelocity(this);
    if (!isStarPointerPointing() && MR::isGreaterStep(this, 5)) {
        mAnimScaleCtrl->startAnim();
        MR::deleteEffect(this, "PointerTouch");
        f32 rate = mIsStrong ? 3.0f : 1.0f;
        MR::setBckRate(this, rate);
        for (s32 i = 0; i < mPartsCount - 1; i++) {
            LiveActor* pPart = mParts->getActor(i);
            MR::setBckRate(pPart, rate);
        }
        setNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvWhistle));
    }
}

void HanachanRunaway::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mAnimScaleCtrl);
}

void HanachanRunaway::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorRide(pReceiver)) {
    }
}

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

// SMG2 fn_80194540: moves the head and every part along the tube. NONMATCHING: approximated.
f32 HanachanRunaway::updateTubeParts() {
    f32 radius = TubeSliderFunction::getRadiusAtRailCoord(mRailCoord, mTubeSlider);
    f32 step = 2.0f * radius * 3.1415927f / 360.0f;
    f32 offset = step * 0.0f;
    for (s32 i = 0; i < mPartsCount - 1; i++) {
        LiveActor* pPart = mParts->getActor(i);
        TVec3f pos(0.0f, 0.0f, 0.0f);
        TubeSliderFunction::unknown(pPart, &pos, mTubeSlider, mRailCoord - offset, mRotation, 0.0f);
        offset += step;
    }
    return radius;
}

// Parts ---------------------------------------------------------------------------------------------

HanachanRunawayParts::HanachanRunawayParts(HanachanRunaway* pOwner, const char* pIndex, char* pUnused) : LiveActor("暴走ハナちゃん体（チューブスライダー）") {
    mOwner = pOwner;
    mIndex = reinterpret_cast<s32>(pIndex);
    mBaseMtx.identity();
}

HanachanRunawayParts::~HanachanRunawayParts() {
}

void HanachanRunawayParts::init(const JMapInfoIter& rIter) {
    const char* pName = (mIndex & 1) == 0 ? "HanachanBody" : "HanachanBodyS";
    MR::initActor(this, rIter, pName, "TubeSlider", nullptr, false);
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

// Nerve ----------------------------------------------------------------------------------------------

void NrvHanachanRunaway::HanachanRunawayNrvWait::execute(Spine* pSpine) const {
    HanachanRunaway* pActor = reinterpret_cast<HanachanRunaway*>(pSpine->mExecutor);
    if (MR::isValidSwitchA(pActor) && MR::isOnSwitchA(pActor)) {
        pActor->setNerve(GET_HANACHAN_RUNAWAY_NERVE(HanachanRunawayNrvWhistle));
    }
}
