#include "Game/MapObj/SnowBall.hpp"
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/MapObj/SnowStep.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Unnamed SMG2 helpers used by this unit (owned by other units).
// PLACEHOLDER(objects): extern "C" block of unnamed/odd target symbols (relocs must match); resolve when they get real names in symbols.txt
extern "C" {
void fn_80034820(LiveActor*);
}

namespace {
    const char* const cSnowBallName = "SnowBall";
    const char* const cPushSensorName = "Push";
    const char* const cBodySensorName = "Body";
    const char* const cMeltSoundName = "Melt";
    const char* const cBreakSoundName = "Break";
    const char* const cMoveActionName = "Move";
    const char* const cRollSoundName = "Roll";
    const char* const cThawActionName = "Thaw";
}  // namespace

// ---- nerves (sinit order: Hide, Break, Thaw, Blow, Roll, Fall, Wait) ----

class SnowBallNrvHide : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeHide();
    }
    static SnowBallNrvHide sInstance;
};
SnowBallNrvHide SnowBallNrvHide::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvBreak : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeBreak();
    }
    static SnowBallNrvBreak sInstance;
};
SnowBallNrvBreak SnowBallNrvBreak::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvThaw : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeThaw();
    }
    static SnowBallNrvThaw sInstance;
};
SnowBallNrvThaw SnowBallNrvThaw::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvBlow : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeBlow();
    }
    static SnowBallNrvBlow sInstance;
};
SnowBallNrvBlow SnowBallNrvBlow::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvRoll : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeRoll();
    }
    static SnowBallNrvRoll sInstance;
};
SnowBallNrvRoll SnowBallNrvRoll::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvFall : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeFall();
    }
    static SnowBallNrvFall sInstance;
};
SnowBallNrvFall SnowBallNrvFall::sInstance ATTRIBUTE_WEAK;

class SnowBallNrvWait : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SnowBall* pBall = reinterpret_cast<SnowBall*>(pSpine->mExecutor);
        pBall->exeWait();
    }
    static SnowBallNrvWait sInstance;
};
SnowBallNrvWait SnowBallNrvWait::sInstance ATTRIBUTE_WEAK;

// ---- helpers (state tests) ----

// fn_803020E0
#pragma dont_inline on
bool SnowBall::isBlowOrThawNerve() const {
    return isNerve(&SnowBallNrvThaw::sInstance) || isNerve(&SnowBallNrvBlow::sInstance);
}

// fn_80302140
bool SnowBall::isHideBreakOrThawNerve() const {
    return isNerve(&SnowBallNrvHide::sInstance) || isNerve(&SnowBallNrvBreak::sInstance) || isNerve(&SnowBallNrvThaw::sInstance);
}
#pragma dont_inline reset

// fn_803019C0: maps the radius to the sensor radius
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
    MR::initActor(this, rIter, cSnowBallName, nullptr, false);
    MR::makeQuatFromRotate(&mQuat, this);
    mBaseRadius = mScale.x;
    mRadius = 100.0f * mBaseRadius;
    mSpeedTarget = mRadius;
    mBasePos.set(mPosition);
    mPrevPos.set(mPosition);
    MR::setAllEffectHostMtx(this, mBaseMtx);
    MR::onCalcGravity(this);
    MR::setBinderExceptSensorType(this, SnowStep::getSensorType());
    initNerve(&SnowBallNrvThaw::sInstance, 0);
    makeActorAppeared();
}

void SnowBall::control() {
    mPrevPos.set(mPosition);
    setRadius(mRadius);
    TVec3f offset(mGravity);
    offset *= mRadius;
    TVec3f center(mPosition);
    center += offset;
    TVec3f up(-mGravity);
    MR::makeMtxUpNoSupportPos(&mBaseMtx, up, center);
}

void SnowBall::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

void SnowBall::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isBlowOrThawNerve()) {
        return;
    }
    if (MR::isSensorPlayer(pReceiver) || MR::isSensorYoshi(pReceiver)) {
        if (MR::isEqualSensor(pSender, this, cPushSensorName)) {
            MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
        }
    } else if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver) || MR::isSensorNpc(pReceiver)) {
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
            dir *= 5.0f;
            MR::addVelocity(this, dir);
            mStopCount = 0;
            return true;
        }
    }
    if (MR::isMsgFireBallAttack(msg)) {
        setNerve(&SnowBallNrvFall::sInstance);
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg) && isHideBreakOrThawNerve()) {
        MR::invalidateClipping(this);
        MR::addVelocitySeparateHV(this, pSender, pReceiver, 0.0f, 70.0f);
        setNerve(&SnowBallNrvBlow::sInstance);
        mStopCount = 0;
        return true;
    }
    return false;
}

bool SnowBall::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pSender)) {
        if (MR::isEqualSensor(pReceiver, this, cPushSensorName) && isHideBreakOrThawNerve()) {
            TVec3f front;
            MR::getPlayerFrontVec(&front);
            front.killElement(front, mGravity);
            MR::normalizeOrZero(&front);

            TVec3f dir;
            MR::calcSensorHorizonNormalize(&dir, mGravity, pSender, pReceiver);
            if (dir.dot(front) > 0.0f && mVelocity.dot(dir) < 20.0f) {
                if (dir.dot(front) > 0.0f) {
                    TVec3f side(front);
                    side -= dir;
                    MR::addVelocity(this, side * 0.5f);
                }

                TVec3f push(dir);
                MR::addVelocity(this, push * 1.0f);
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

void SnowBall::setRadius(f32 radius) {
    f32 value = radius;
    if (value < 1.0f) {
        value = 1.0f;
    }
    MR::setScale(this, value / 100.0f);
    MR::setBinderRadius(this, value);
    MR::setSensorRadius(this, "Push", calcSensorRadius(mRadius));
    MR::setSensorRadius(this, "Body", calcSensorRadius(mRadius));
    MR::setClippingTypeSphere(this, 1.2f * mRadius);
    MR::setStarPointerTargetRadius3d(this, mRadius);
}

void SnowBall::tryDropSnowStep(const TVec3f& rPos) {
    mStepCount++;
    if (mStepCount > 30 || PSVECDistance(&rPos, &mLastStepPos) > 150.0f) {
        bool isLong = mStepCount > 30;
        f32 speed = isLong ? 400.0f : 4.0f + mSpeedTarget;
        s32 size = isLong ? 40 : 25;
        TVec3f down(-mGravity);
        if (MR::tryAppearSnowStep(rPos, down, size, 240, 120, speed)) {
            MR::startActionSound(this, cMeltSoundName, -1, -1, -1);
            mLastStepPos.set(rPos);
            mStepCount = 0;
            mSpeedTarget -= 4.0f;
        }
    }
}

void SnowBall::launch(const TVec3f& rPos, const TVec3f& rVel, bool canRespawn) {
    mBasePos.set(rPos);
    mCanRespawn = canRespawn;
    resetRoll();
    appear();
    MR::invalidateClipping(this);
    setNerve(&SnowBallNrvBreak::sInstance);
    MR::setVelocity(this, rVel);
}

// fn_803021B0: give up after the break; respawn is only possible when mCanRespawn
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
    setNerve(&SnowBallNrvWait::sInstance);
}

// fn_803022B0: put the snowball back at its base position in the thaw state
void SnowBall::resetRoll() {
    MR::onCalcGravity(this);
    MR::onBind(this);
    MR::validateHitSensors(this);
    MR::showModel(this);
    MR::resetPosition(this, mBasePos);
    mPrevPos.set(mPosition);
    setNerve(&SnowBallNrvThaw::sInstance);
    MR::zeroVelocity(this);
    mRadius = 100.0f * mBaseRadius;
    mSpeedTarget = mRadius;
    setRadius(mRadius);
    mStepCount = 0;
}

// fn_80302350: rolling / binded ground update, returns true when the ball is too small to keep going
bool SnowBall::updateBindedGroundRoll(bool bindHit) {
    if (MR::isBindedGroundSnow(this)) {
        const TVec3f* pNormal = &MR::getGroundNormal(this);
        TVec3f proj;
        JMAVECScaleAdd(pNormal, &mVelocity, &proj, -pNormal->dot(mVelocity));
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
        const TVec3f* pNormal = &MR::getGroundNormal(this);
        TVec3f negGravity(-mGravity);
        f32 deg = 57.29578f * JMAAcosRadian(pNormal->dot(negGravity));
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

    f32 damp = (mGroundCount < 5) ? 0.96f : 0.99f;
    MR::attenuateVelocity(this, damp);
    mRadius = MR::lerp(mRadius, mSpeedTarget, 0.1f);

    if (MR::isBindedGround(this)) {
        MR::rotateQuatRollBall(&mQuat, mVelocity, MR::getGroundNormal(this), mRadius);
    } else {
        TVec3f up(-mGravity);
        MR::rotateQuatRollBall(&mQuat, mVelocity, up, mRadius);
    }
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

void SnowBall::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::zeroVelocity(this);
    }
    if (MR::isJudgedToClipFrustum50m(mBasePos, 100.0f * mBaseRadius)) {
        resetRoll();
    }
}

void SnowBall::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::tryHitReactionBreak(this);
        MR::zeroVelocity(this);
        MR::startActionSound(this, cBreakSoundName, -1, -1, -1);
    }
    if (MR::isGreaterStep(this, 5)) {
        breakAndWait();
    }
}

void SnowBall::exeThaw() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, cThawActionName);
        mSpeedTarget = mRadius;
    }
    mRadius = MR::calcNerveEaseInOutValue(this, 0, 40, 50.0f, 0.05f);
    if (MR::isGreaterStep(this, 40)) {
        breakAndWait();
    }
}

void SnowBall::exeBlow() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, cMoveActionName);
        fn_80034820(this);
    }
    bool bindHit = MR::sendMsgEnemyAttackToBindedSensor(this, getSensor(cBodySensorName));
    if (MR::sendMsgEnemyAttackRollToBindedSensor(this, getSensor(cBodySensorName)) && !bindHit) {
        setNerve(&SnowBallNrvFall::sInstance);
        return;
    }
    if (updateBindedGroundRoll(bindHit)) {
        setNerve(&SnowBallNrvRoll::sInstance);
        return;
    }
    if (MR::sendMsgToBindedSensor(0x12, this, getSensor(cBodySensorName))) {
        setNerve(&SnowBallNrvThaw::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 30)) {
        setNerve(&SnowBallNrvThaw::sInstance);
    }
}

void SnowBall::exeRoll() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::tryStartActionNotPlaying(this, cMoveActionName);
    }
    bool bindHit = MR::sendMsgEnemyAttackToBindedSensor(this, getSensor(cBodySensorName));
    if (updateBindedGroundRoll(bindHit)) {
        setNerve(&SnowBallNrvRoll::sInstance);
        return;
    }
    if (mStopCount > 60) {
        if (MR::isBindedGround(this) && !MR::isBindedGroundDamageFire(this)) {
            setNerve(&SnowBallNrvHide::sInstance);
            return;
        }
    }
    f32 velLen = MR::calcVelocityLength(this);
    if (mGroundCount < 5 && velLen > 4.0f) {
        MR::startActionSound(this, cRollSoundName, (s8) (s32) (3.0f * (velLen * velLen)), -1, -1);
    }
}

void SnowBall::exeFall() {
    MR::addVelocityToGravity(this, 1.2f);
    MR::reboundVelocityFromEachCollision(this, -1.0f, 0.0f, 0.0f, 0.0f);
    MR::attenuateVelocity(this, 0.99f);
    TVec3f up(-mGravity);
    MR::rotateQuatRollBall(&mQuat, mVelocity, up, mRadius);
    if (MR::isGreaterStep(this, 10) && MR::isBindedGround(this)) {
        setNerve(&SnowBallNrvThaw::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 300)) {
        makeActorDead();
    }
}

void SnowBall::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::validateClipping(this);
        MR::tryStartActionNotPlaying(this, "Stable");
    }
    MR::killVelocityOnGroundH(this);
    if (mStopCount <= 0) {
        setNerve(&SnowBallNrvThaw::sInstance);
    }
    MR::attachSupportTicoToTarget(this);
}

