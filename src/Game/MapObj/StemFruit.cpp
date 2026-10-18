#pragma auto_inline off
#include "Game/MapObj/StemFruit.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"

// PLACEHOLDER(objects): fn_80422310 (0x40 bytes, no args, MR Yoshi helper); resolve when core names fn_80422310
extern "C" bool fn_80422310();
// PLACEHOLDER(objects): fn_804228B0 (0x40 bytes, no args, MR Yoshi helper); resolve when core names fn_804228B0
extern "C" void fn_804228B0();
// PLACEHOLDER(objects): fn_80422880 (msg == 253 test, Yoshi message); resolve when core names fn_80422880
extern "C" bool fn_80422880(u32 msg);

namespace NrvStemFruit {
    NEW_NERVE(StemFruitNrvWait, StemFruit, Wait);
    NEW_NERVE(StemFruitNrvTrampled, StemFruit, Trampled);
    NEW_NERVE(StemFruitNrvSpin, StemFruit, Spin);
    NEW_NERVE(StemFruitNrvShake, StemFruit, Shake);
    NEW_NERVE(StemFruitNrvGet, StemFruit, Get);
    NEW_NERVE(StemFruitNrvComeOff, StemFruit, ComeOff);
    NEW_NERVE(StemFruitNrvPull, StemFruit, Pull);
    NEW_NERVE(StemFruitNrvPullEnd, StemFruit, PullEnd);
    NEW_NERVE(StemFruitNrvComeOffAlt, StemFruit, ComeOffAlt);
}

namespace NrvStemFruitChild {
    NEW_NERVE(StemFruitChildNrvAppear, StemFruitChild, Appear);
    NEW_NERVE(StemFruitChildNrvWait, StemFruitChild, Wait);
    NEW_NERVE(StemFruitChildNrvNearPlayer, StemFruitChild, NearPlayer);
    NEW_NERVE(StemFruitChildNrvGravity, StemFruitChild, Gravity);
    NEW_NERVE(StemFruitChildNrvBreak, StemFruitChild, Break);
    NEW_NERVE(StemFruitChildNrvHipDropCrash, StemFruitChild, HipDropCrash);
}

StemFruit::StemFruit(const char* pName) : LiveActor(pName) {
    mChild = nullptr;
    mTimer = 0;
}

void StemFruit::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, getParam()->mModelName, false);

    TVec3f offset;
    offset.setAll<f32>(0.0f);
    initSound(4, getParam()->mModelName, nullptr, offset);
    initEffectKeeper(0, nullptr, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::offCalcGravity(this);

    TPos3f mtx;
    MR::makeMtxTR(mtx, this);
    mtx.getYDir(mGravity);
    JGeometry::negateInternal(&mGravity.x, &mGravity.x);
    MR::connectToSceneItem(this);
    initNerve(GET_NERVE(StemFruit, StemFruitNrvWait), 0);

    mChild = new StemFruitChild(this, rIter);

    MR::initUseStageSwitchWriteDead(this, rIter);
    if (!MR::isValidSwitchDead(this)) {
        slot0CC();
    }
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void StemFruit::slot0CC() {
}

void StemFruit::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startAction(mChild, "Wait");
        MR::validateClipping(this);
    }
}

void StemFruit::exeTrampled() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mChild, "Trampled");
    }
    if (MR::isActionEnd(mChild)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeSpin() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mChild, "Spin");
        MR::startActionSound(this, "SpinRotate", -1, -1, -1);
    }
    if (MR::isActionEnd(mChild)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeShake() {
    if (MR::isFirstStep(this)) {
        MR::startAction(mChild, "Shake");
    }
    if (MR::isActionEnd(mChild)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeGet() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ComeOff");
        MR::startAction(mChild, "Get");
        MR::invalidateClipping(this);
    }
    updateChildState();
}

void StemFruit::exeComeOff() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ComeOff");
        MR::invalidateClipping(this);
    }
    updateChildState();
}

void StemFruit::exePull() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Pull");
        MR::startAction(mChild, "Pull");
        MR::invalidateClipping(this);
    }

    MtxPtr pMtxA = getBaseMtx();
    MtxPtr pMtxB = getBaseMtx();
    MtxPtr pMtxC = getBaseMtx();
    TVec3f dir(pMtxC[0][2], pMtxB[1][2], pMtxA[2][2]);
    MR::turnDirectionToPlayerDegreeHorizon(this, &dir, 180.0f);

    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, dir);
    mtx.getEuler(mRotation);
    mRotation = mRotation * 57.2957763671875f;
}

void StemFruit::exePullEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PullEnd");
        MR::startAction(mChild, "PullEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
    }
}

void StemFruit::exeComeOffAlt() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "ComeOff");
        MR::invalidateClipping(this);
    }
    updateChildState();
}

bool StemFruit::tryStartTrampled() {
    if (isNerveActive()) {
        mTimer = 30;
        setNerve(GET_NERVE(StemFruit, StemFruitNrvTrampled));
        return true;
    }
    return false;
}

bool StemFruit::tryStartSpin() {
    if (isNerveActive() && !isNerve(GET_NERVE(StemFruit, StemFruitNrvSpin))) {
        mTimer = 30;
        setNerve(GET_NERVE(StemFruit, StemFruitNrvSpin));
        return true;
    }
    return false;
}

bool StemFruit::tryStartShake() {
    if (isNerve(GET_NERVE(StemFruit, StemFruitNrvWait)) && mTimer == 0) {
        mTimer = 30;
        setNerve(GET_NERVE(StemFruit, StemFruitNrvShake));
        return true;
    }
    mTimer = 30;
    return false;
}

void StemFruit::setNerveGet() {
    setNerve(GET_NERVE(StemFruit, StemFruitNrvGet));
}

void StemFruit::onGetSwitch() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    if (getParam()->mFlag24 != 0) {
        onEat();
    }
}

void StemFruit::onEat() {
}

bool StemFruit::tryStartComeOff() {
    if (!isNerveActive()) {
        return false;
    }
    setNerve(GET_NERVE(StemFruit, StemFruitNrvComeOff));
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    } else {
        onEatEnd();
    }
    return true;
}

bool StemFruit::tryStartPull() {
    if (isNerveActive()) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvPull));
        return true;
    }
    return false;
}

bool StemFruit::tryStartPullEnd() {
    if (!isNerve(GET_NERVE(StemFruit, StemFruitNrvPull))) {
        return false;
    }
    setNerve(GET_NERVE(StemFruit, StemFruitNrvPullEnd));
    return true;
}

bool StemFruit::tryStartComeOffAlt() {
    if (isNerveActive()) {
        setNerve(GET_NERVE(StemFruit, StemFruitNrvComeOffAlt));
        return true;
    }
    return false;
}

void StemFruit::setNerveWait() {
    setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
}

void StemFruit::endPull() {
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
    setNerve(GET_NERVE(StemFruit, StemFruitNrvGet));
}

void StemFruit::control() {
    if (mTimer != 0) {
        mTimer--;
    }
}

void StemFruit::updateChildState() {
    if (getParam()->mFlag08 != 0) {
        if (MR::isDead(mChild)) {
            if (MR::isGreaterEqualStep(this, getParam()->mRespawnStep)) {
                mChild->appear();
                setNerve(GET_NERVE(StemFruit, StemFruitNrvWait));
            }
        }
    } else {
        if (MR::isActionEnd(this)) {
            MR::validateClipping(this);
        }
    }
}

bool StemFruit::isNerveActive() const {
    return isNerve(GET_NERVE(StemFruit, StemFruitNrvWait)) || isNerve(GET_NERVE(StemFruit, StemFruitNrvTrampled)) ||
           isNerve(GET_NERVE(StemFruit, StemFruitNrvSpin)) || isNerve(GET_NERVE(StemFruit, StemFruitNrvShake));
}

// ---------------------------------------------------------------------------
// StemFruitChild (the fruit's sprout/pick-up child, 0xBC bytes)
// ---------------------------------------------------------------------------

StemFruitChild::StemFruitChild(StemFruit* pParent, const JMapInfoIter& rIter) : LiveActor("\x8e\xc0") {
    mParent = pParent;
    mFixedPos = nullptr;
    mBaseScale.x = 1.0f;
    mBaseScale.y = 1.0f;
    mBaseScale.z = 1.0f;
    mYoshiLockOn = nullptr;
    mCycleTimer = 0.0f;
    mPullPos.x = 0.0f;
    mPullPos.y = 0.0f;
    mPullPos.z = 0.0f;

    MR::initActor(this, rIter, pParent->getParam()->mChildModelName, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    MR::connectToSceneItem(this);
    initEffectKeeper(0, nullptr, false);
    initHitSensor(1);

    TVec3f zero(0.0f, 0.0f, 0.0f);
    MR::addHitSensor(this, "Body", 0x4E, 0x20, pParent->getParam()->mHitSensorRadius, zero);

    s32 starRadius = pParent->getParam()->mStarPointerRadiusS32;
    MR::initStarPointerTarget(this, static_cast<f32>(starRadius), TVec3f(0.0f, pParent->getParam()->mStarPointerOffsetY, 0.0f));

    TVec3f zeroLock;
    zeroLock.setAll<f32>(0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, nullptr, zeroLock);
    MR::offCalcGravity(this);
    mGravity.set(pParent->mGravity);

    mFixedPos = new FixedPosition(pParent, "FruitPos", TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));

    mBaseScale.set(mScale);
    TVec3f zeroSound;
    zeroSound.setAll<f32>(0.0f);
    initSound(4, "StemFruit", nullptr, zeroSound);
    initNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait), 0);
    makeActorAppeared();
    MR::useStageSwitchAwake(this, rIter);
}

void StemFruitChild::appear() {
    LiveActor::appear();
    mScale.x = 0.0f;
    mScale.y = 0.0f;
    mScale.z = 0.0f;
    setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvAppear));
    MR::hideModel(this);
}

void StemFruitChild::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::showModel(this);
    }

    f32 step = static_cast<f32>(getNerveStep());
    TVec3f diff = mBaseScale - mScale;
    diff.scale(step);
    diff.scale(1.0f / 30.0f);
    mScale.add(diff);

    if (MR::isGreaterEqualStep(this, 30)) {
        setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait));
        mParent->setNerveWait();
    }
}

void StemFruitChild::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }
    if (MR::isPlayerElementModeYoshi()) {
        if (mParent->getParam()->mFlag10 != 0) {
            YoshiFunction::tryYoshiPull(mYoshiLockOn);
        } else {
            YoshiFunction::tryYoshiEat(mYoshiLockOn);
        }
    }
    if (MR::isStarPointerPointing2PAttachTarget(this)) {
        mParent->tryStartShake();
    }
    mCycleTimer += mParent->getParam()->mTimerStep;
}

void StemFruitChild::exeNearPlayer() {
    if (MR::isFirstStep(this)) {
        mPullPos.set(*MR::getPlayerPos());
    }

    f32 dist = PSVECDistance(&mPullPos, &mPosition);
    if (MR::isNearPlayer(this, dist + 75.0f)) {
        fn_80422310();
        setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvGravity));
        mParent->setNerveGet();
        MR::startActionSound(mParent, "PulledOut", -1, -1, -1);
    }
}

void StemFruitChild::exeGravity() {
    if (MR::isFirstStep(this)) {
    }
    mGravity.set(*MR::getPlayerGravity());
}

void StemFruitChild::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        MR::startActionSound(mParent, "Break", -1, -1, -1);
    }
    if (MR::isActionEnd(this)) {
        kill();
    }
}

void StemFruitChild::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) || MR::isSensorYoshi(pReceiver)) {
        if (isNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait))) {
            if (mParent->getParam()->mFlag11 != 0) {
                if (YoshiFunction::tryYoshiPushEat(reinterpret_cast<YoshiLockOnTarget*>(const_cast<StemFruitParam*>(mParent->getParam())))) {
                    return;
                }
            }
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool StemFruitChild::receiveMsgPush(HitSensor*, HitSensor*) {
    if (isNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait))) {
        return mParent->tryStartShake();
    }
    return false;
}

bool StemFruitChild::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait))) {
        return false;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        mParent->tryStartSpin();
        return true;
    }
    if (MR::isMsgYoshiEat(msg)) {
        if (mParent->tryStartComeOff()) {
            setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvBreak));
        }
        return true;
    }
    if (MR::isMsgYoshiProjectile(msg)) {
        if (mParent->tryStartComeOff()) {
            setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvBreak));
        }
        return true;
    }
    if (MR::isMsgJetTurtleAttack(msg)) {
        if (mParent->tryStartComeOff()) {
            setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvBreak));
        }
        return true;
    }
    if (mParent->getParam()->mFlag11 != 0) {
        if (YoshiFunction::tryYoshiPushEat(reinterpret_cast<YoshiLockOnTarget*>(const_cast<StemFruitParam*>(mParent->getParam())))) {
            if (MR::isMsgSupportTicoSpin(msg)) {
                return mParent->tryStartSpin();
            }
            return false;
        }
    }
    if (MR::isMsgPlayerTrample(msg)) {
        return mParent->tryStartTrampled();
    }
    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        return mParent->tryStartSpin();
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        if (mParent->getParam()->mFlag12 != 0) {
            if (mParent->tryStartComeOffAlt()) {
                setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvHipDropCrash));
            }
        } else {
            if (mParent->tryStartTrampled()) {
                MR::sendMsgAwayJump(pSender, pReceiver);
            }
        }
        return true;
    }
    return false;
}

bool StemFruitChild::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait))) {
        if (MR::isMsgYoshiStartTongue(msg)) {
            if (mParent->getParam()->mFlag10 != 0) {
                setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvNearPlayer));
                mParent->tryStartPull();
                return true;
            }
            setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvGravity));
            mParent->setNerveGet();
            MR::startActionSound(mParent, "PulledOutInstant", -1, -1, -1);
            return true;
        }
        if (mParent->getParam()->mFlag11 != 0) {
            if (fn_80422880(msg)) {
                return true;
            }
            if (MR::isMsgYoshiAutoEatStart(msg)) {
                setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvGravity));
                mParent->setNerveGet();
                MR::startActionSound(mParent, "AutoEatStart", -1, -1, -1);
                return true;
            }
        }
        if (MR::isMsgRushEnd(msg)) {
            if (mParent->tryStartComeOff()) {
                setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvBreak));
            }
            return true;
        }
        return false;
    }
    if (isNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvNearPlayer))) {
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait));
            mParent->tryStartPullEnd();
            return true;
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            setNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvWait));
            mParent->tryStartPullEnd();
            return true;
        }
    }
    if (isNerve(GET_NERVE(StemFruitChild, StemFruitChildNrvGravity))) {
        if (MR::isMsgYoshiLoopTongue(msg)) {
            return true;
        }
        if (MR::isMsgYoshiReleaseEaten(msg)) {
            updateSensorOffset();
            return true;
        }
        if (mParent->getParam()->mFlag11 != 0) {
            if (MR::isMsgYoshiAutoEatEnd(msg)) {
                updateSensorOffset();
                return true;
            }
        }
        if (MR::isMsgYoshiCancelEaten(msg)) {
            mParent->endPull();
            kill();
            return true;
        }
        return false;
    }
    return false;
}

void StemFruitChild::updateSensorOffset() {
    mParent->onGetSwitch();
    MR::startActionSound(mParent, "EatEnd", -1, -1, -1);
    const StemFruitParam* pParam = mParent->getParam();
    if (pParam->mEffectName == nullptr || MR::emitEffect(this, pParam->mEffectName) == 0) {
        fn_804228B0();
    }
    kill();
}
