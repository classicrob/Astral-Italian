#define JGEOMETRY_VEC3_MUL_NOINLINE

#include "Game/Util/LiveActorUtil.hpp"
#include "Game/MapObj/SnowBall.hpp"
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/MapObj/SnowStep.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Unnamed SMG2 helpers used by this unit (owned by other units).
// Nerve order follows the target singleton addresses; names follow their actual execute bodies.

class SnowBallNrvWait : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeWait();
    }
    static SnowBallNrvWait sInstance;
};
SnowBallNrvWait SnowBallNrvWait::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvFall : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeFall();
    }
    static SnowBallNrvFall sInstance;
};
SnowBallNrvFall SnowBallNrvFall::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvRoll : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeRoll();
    }
    static SnowBallNrvRoll sInstance;
};
SnowBallNrvRoll SnowBallNrvRoll::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvBlow : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeBlow();
    }
    static SnowBallNrvBlow sInstance;
};
SnowBallNrvBlow SnowBallNrvBlow::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvThaw : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeThaw();
    }
    static SnowBallNrvThaw sInstance;
};
SnowBallNrvThaw SnowBallNrvThaw::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvBreak : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeBreak();
    }
    static SnowBallNrvBreak sInstance;
};
SnowBallNrvBreak SnowBallNrvBreak::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvHide : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeHide();
    }
    static SnowBallNrvHide sInstance;
};
SnowBallNrvHide SnowBallNrvHide::sInstance ATTRIBUTE_WEAK;

static f32 calcSensorRadius(f32 radius) NO_INLINE;
static f32 calcSensorRadius(f32 radius) {
    if (radius <= 50.0f) {
        return radius;
    }
    if (radius >= 125.0f) {
        return radius - 25.0f;
    }
    return 16.666666f + (0.6666667f * radius);
}

SnowBall::SnowBall(const char* pName) : LiveActor(pName) {
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mLastStepPos.set(0.0f);
    mBasePos.set(0.0f);
    mPrevPos.set(0.0f);
    mGroundCount = 0;
    mStopCount = 0;
    mSpeedTarget = 100.0f;
    mRadius = 100.0f;
    mBaseRadius = 1.0f;
    mStepCount = 0;
    mCanRespawn = true;
    mBaseMtx.identity();

    MR::createClipAreaDropHolder();
    MR::createSnowStepHolder();
}

SnowBall::~SnowBall() {
}

void SnowBall::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SnowBall", false);
    MR::makeQuatFromRotate(&mQuat, this);
    mBaseRadius = mScale.x;
    mRadius = 100.0f * mBaseRadius;
    mSpeedTarget = mRadius;
    mBasePos.set(mPosition);
    mPrevPos.set(mPosition);
    MR::setAllEffectHostMtx(this, mBaseMtx);
    MR::setBinderExceptSensorType(this, SnowStep::getSensorType());
    MR::onCalcGravity(this);
    initNerve(&SnowBallNrvRoll::sInstance, 0);
    makeActorAppeared();
}

void SnowBall::setRadius(f32 radius) {
    f32 value = radius;
    if (value < 1.0f) {
        value = 1.0f;
    }
    MR::setScale(this, value / 100.0f);
    MR::setBinderRadius(this, value);
    MR::setSensorRadius(this, "Push", calcSensorRadius(mRadius));
    MR::setSensorRadius(this, "Body", mRadius);
    MR::setClippingTypeSphere(this, 1.2f * mRadius);
    MR::setStarPointerTargetRadius3d(this, mRadius);
}

void SnowBall::tryDropSnowStep(const TVec3f& rPos) {
    mStepCount++;
    if (mStepCount > 30 || PSVECDistance(&rPos, &mLastStepPos) > 150.0f) {
        bool isLong = mSpeedTarget - 4.0f < 50.0f;
        f32 speed = isLong ? 400.0f : 200.0f + mSpeedTarget;
        s32 size = isLong ? 40 : 25;
        TVec3f down(-mGravity);
        if (MR::tryAppearSnowStep(rPos, down, size, 240, 120, speed)) {
            MR::startActionSound(this, "Melt", -1, -1, -1);
            mLastStepPos.set(rPos);
            mStepCount = 0;
            mSpeedTarget -= 4.0f;
        }
    }
}

void SnowBall::control() {
    mPrevPos.set(mPosition);
    setRadius(mRadius);
    MR::makeMtxUpNoSupportPos(&mBaseMtx, -mGravity, mPosition + mGravity * mRadius);
}

void SnowBall::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void SnowBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isBlowOrThawNerve()) {
        return;
    }
    if (MR::isSensorPlayer(pReceiver) || MR::isSensorYoshi(pReceiver)) {
        if (MR::isEqualSensor(pSender, this, "Push")) {
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
    } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver) || MR::isSensorNpc(pReceiver) || MR::isSensorPush(pReceiver)) {
        if (!MR::sendMsgToEnemyAttackTrample(pReceiver, pSender)) {
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
        MR::sendArbitraryMsg(0x12, pReceiver, pSender);
    }
}

bool SnowBall::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg) || MR::isMsgStarPieceReflect(msg)) {
        if (isHideBreakOrThawNerve()) {
            TVec3f dir;
            MR::calcSensorHorizonNormalize(&dir, mGravity, pSender, pReceiver);
            MR::addVelocity(this, dir * 5.0f);
            mStopCount = 0;
            return true;
        }
    }
    if (MR::isMsgFireBallAttack(msg)) {
        setNerve(&SnowBallNrvBreak::sInstance);
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg) && isHideBreakOrThawNerve()) {
        MR::addVelocitySeparateHV(this, pSender, pReceiver, 70.0f, 0.0f);
        MR::invalidateClipping(this);
        setNerve(&SnowBallNrvBlow::sInstance);
        mStopCount = 0;
        return true;
    }
    return false;
}

bool SnowBall::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pSender)) {
        if (MR::isEqualSensor(pReceiver, this, "Push") && isHideBreakOrThawNerve()) {
            TVec3f front;
            MR::getPlayerFrontVec(&front);
            front.killElement(front, mGravity);
            MR::normalizeOrZero(&front);

            TVec3f dir;
            MR::calcSensorHorizonNormalize(&dir, mGravity, pSender, pReceiver);
            if (dir.dot(front) > 0.0f && mVelocity.dot(dir) < 20.0f) {
                if (dir.dot(front) > 0.0f) {
                    MR::addVelocity(this, (front - dir) * 0.5f);
                }

                MR::addVelocity(this, dir * 1.0f);
                mStopCount = 0;
                return true;
            }
        }
    }

    if (MR::isSensorEnemy(pSender) || MR::isSensorNpc(pSender)) {
        if (isHideBreakOrThawNerve()) {
            MR::killVelocityToTarget(this, *MR::getSensorPos(pSender));
            return true;
        }
    }

    return false;
}

// ---- helpers (state tests) ----

// fn_803020E0
#pragma dont_inline on
bool SnowBall::isBlowOrThawNerve() const {
    if (isNerve(&SnowBallNrvRoll::sInstance) || isNerve(&SnowBallNrvBlow::sInstance)) {
        return true;
    }
    return false;
}

// fn_80302140
bool SnowBall::isHideBreakOrThawNerve() const {
    if (isNerve(&SnowBallNrvWait::sInstance) || isNerve(&SnowBallNrvFall::sInstance) || isNerve(&SnowBallNrvRoll::sInstance)) {
        return true;
    }
    return false;
}
#pragma dont_inline reset

void SnowBall::breakAndWait() {
    if (!mCanRespawn) {
        kill();
        return;
    }
    MR::hideModel(this);
    MR::invalidateClipping(this);
    MR::invalidateHitSensors(this);
    MR::offBind(this);
    MR::offCalcGravity(this);
    MR::zeroVelocity(this);
    setNerve(&SnowBallNrvHide::sInstance);
}

void SnowBall::launch(const TVec3f& rPos, const TVec3f& rVel, bool canRespawn) {
    mBasePos.set(rPos);
    mCanRespawn = canRespawn;
    resetRoll();
    appear();
    MR::invalidateClipping(this);
    setNerve(&SnowBallNrvFall::sInstance);
    MR::setVelocity(this, rVel);
}

void SnowBall::resetRoll() {
    MR::onCalcGravity(this);
    MR::onBind(this);
    MR::validateHitSensors(this);
    MR::showModel(this);
    MR::resetPosition(this, mBasePos);
    mPrevPos.set(mPosition);
    setNerve(&SnowBallNrvRoll::sInstance);
    MR::zeroVelocity(this);
    mRadius = 100.0f * mBaseRadius;
    mSpeedTarget = mRadius;
    setRadius(mRadius);
    mStepCount = 0;
}

bool SnowBall::updateBindedGroundRoll(bool bindHit) {
    if (MR::isBindedGroundSnow(this)) {
        const TVec3f* pNormal;
        const TVec3f* pVel = &mVelocity;
        pNormal = &MR::getGroundNormal(this);
        TVec3f proj;
        JMAVECScaleAdd(pNormal, pVel, &proj, -pNormal->dot(*pVel));
        f32 len = PSVECMag(&proj);
        mSpeedTarget += len / 50.0f;
        if (mSpeedTarget > 200.0f) {
            mSpeedTarget = 200.0f;
        }
    }

    if (MR::isBindedGroundDamageFire(this)) {
        tryDropSnowStep(MR::getGroundHitPos(this));
    }

    if (MR::isBindedGround(this)) {
        TVec3f negGravity(-mGravity);
        f32 deg = 57.29578f * JMAAcosRadian(MR::getGroundNormal(this).dot(negGravity));
        if (deg > 35.0f) {
            f32 k = MR::normalize(deg, 35.0f, 60.0f);
            MR::addVelocityToGravity(this, 2.0f * k);
            mStopCount = 0;
        }
        mGroundCount = 0;
    } else {
        MR::addVelocityToGravity(this, 2.0f);
        mGroundCount++;
    }

    if (!bindHit) {
        MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f);
    }

    MR::attenuateVelocity(this, (mGroundCount < 5) ? 0.96f : 0.99f);
    mRadius = MR::lerp(mRadius, mSpeedTarget, 0.1f);

    const TVec3f* pUp;
    if (MR::isBindedGround(this)) {
        pUp = &MR::getGroundNormal(this);
    } else {
        TVec3f up(-mGravity);
        pUp = &up;
    }
    MR::rotateQuatRollBall(&mQuat, mVelocity, *pUp, mRadius);
    f32 velLen = MR::calcVelocityLength(this);
    if (velLen < 4.0f) {
        mStopCount++;
        if (mStopCount > 100) {
            mStopCount = 100;
        }
    } else {
        mStopCount = 0;
    }
    if (mStopCount > 60) {
        MR::killVelocityOnGroundH(this);
    }
    if (mRadius < 50.0f) {
        mRadius = 50.0f;
        return true;
    }
    return false;
}

void SnowBall::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Stable");
        MR::validateClipping(this);
        MR::zeroVelocity(this);
    }
    MR::killVelocityOnGroundH(this);
    if (mStopCount <= 0) {
        setNerve(&SnowBallNrvRoll::sInstance);
    }
    MR::attachSupportTicoToTarget(this);
}

void SnowBall::exeFall() {
    MR::addVelocityToGravity(this, 1.2f);
    MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f);
    MR::attenuateVelocity(this, 0.99f);
    TVec3f up(-mGravity);
    MR::rotateQuatRollBall(&mQuat, mVelocity, up, mRadius);
    if (MR::isGreaterStep(this, 10) && MR::isBindedGround(this)) {
        setNerve(&SnowBallNrvRoll::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 300)) {
        makeActorDead();
    }
}

void SnowBall::exeRoll() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Move");
        MR::invalidateClipping(this);
    }
    bool bindHit = MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"));
    if (updateBindedGroundRoll(bindHit)) {
        setNerve(&SnowBallNrvThaw::sInstance);
        return;
    }
    if (mStopCount > 60) {
        if (MR::isBindedGround(this) && !MR::isBindedGroundDamageFire(this)) {
            setNerve(&SnowBallNrvWait::sInstance);
            return;
        }
    }
    MR::attachSupportTicoToTarget(this);
    f32 velLen = MR::calcVelocityLength(this);
    if (mGroundCount < 5 && velLen > 4.0f) {
        MR::startActionSound(this, "Roll", (s32) (3.0f * (velLen * velLen)), -1, -1);
    }
}

void SnowBall::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Move");
        MR::startBlowHitReaction(this);
    }
    bool bindHit = MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"));
    if (MR::sendMsgEnemyAttackRollToBindedSensor(this, getSensor("Body")) && !bindHit) {
        setNerve(&SnowBallNrvBreak::sInstance);
        return;
    }
    if (updateBindedGroundRoll(bindHit)) {
        setNerve(&SnowBallNrvThaw::sInstance);
        return;
    }
    if (MR::sendMsgToBindedSensor(0x12, this, getSensor("Body"))) {
        setNerve(&SnowBallNrvRoll::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 30)) {
        setNerve(&SnowBallNrvRoll::sInstance);
    }
}

void SnowBall::exeThaw() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Thaw");
        MR::zeroVelocity(this);
        mSpeedTarget = mRadius;
    }
    mRadius = MR::calcNerveEaseInOutValue(this, 40, 50.0f, 0.05f);
    if (MR::isGreaterStep(this, 40)) {
        breakAndWait();
    }
}

void SnowBall::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReactionBreak(this);
        MR::startActionSound(this, "Break", -1, -1, -1);
        MR::zeroVelocity(this);
    }
    if (MR::isGreaterStep(this, 5)) {
        breakAndWait();
    }
}

void SnowBall::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::hideModel(this);
    }
    if (MR::isJudgedToClipFrustum50m(mBasePos, 100.0f * mBaseRadius)) {
        resetRoll();
    }
}

