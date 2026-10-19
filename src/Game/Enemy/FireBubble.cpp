#include "Game/Enemy/FireBubble.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const f32 cSensorRadius = 30.0f;
    const f32 cBinderRadius = 40.0f;
    const s32 cCoinMaxNum = 2;
    const s32 cRandomActFrame = 120;
    const f32 cStarWandRadius = 60.0f;
    const f32 cAppearGravity = 0.75f;
    const s32 cChaseInvalidFrame = 300;
    const s32 cForceKillFrame = 600;
    const f32 cChaseStartDistance = 2000.0f;
    const f32 cChaseSpeed = 4.0f;
    const f32 cChaseGravity = 2.0f;
    const f32 cChaseRotateSpeed = 1.0f;
    const s32 cChaseFrame = 30;
    const f32 cReflectGravity = 1.5f;
    const s32 cReflectWaitFrame = 60;
    const f32 cReflectCursorSpeed = 10.0f;
    const s32 cAppearInterval = 120;
    const Vec cAppearVelocity = {0.0f, 35.0f, 6.0f};
    const Vec cReflectVelocity = {0.0f, 40.0f, -15.0f};
};  // namespace

namespace NrvFireBubble {
    NEW_NERVE(FireBubbleNrvAppear, FireBubble, Appear);
    NEW_NERVE(FireBubbleNrvWait, FireBubble, Wait);
    NEW_NERVE(FireBubbleNrvChase, FireBubble, Chase);
    NEW_NERVE(FireBubbleNrvReflect, FireBubble, Reflect);
    NEW_NERVE(FireBubbleNrvReflectLand, FireBubble, ReflectLand);
    NEW_NERVE(FireBubbleNrvReflectWait, FireBubble, ReflectWait);
    NEW_NERVE(FireBubbleNrvDown, FireBubble, Down);
    NEW_NERVE(FireBubbleNrvWaitToAppear, FireBubble, WaitToAppear);
};  // namespace NrvFireBubble

FireBubble::FireBubble(const char* pName)
    : LiveActor(pName), mIsValidInfo(), mInitPos(gZeroVec), mInitFront(0.0f, 0.0f, 1.0f), mInitVelocity(::cAppearVelocity), mFront(0.0f, 0.0f, 1.0f), mActFrame(),
      mChaseCounter() {
}

inline void FireBubble::initHitSensors(const char* const hitSensor1, const char* const hitSensor2) {
    initHitSensor(1);
    MR::addHitSensorAtJointEnemy(this, hitSensor1, hitSensor2, 8, ::cSensorRadius, TVec3f(0.0f, 0.0f, 0.0f));
}

// NONMATCHING: Vector temporary stack slots and register allocation differ.
void FireBubble::init(const JMapInfoIter& rIter) {
    if (MR::isValidInfo(rIter)) {
        initMapToolInfo(rIter);
    }

    MR::initActor(this, "FireBubble", false);
    MR::connectToSceneEnemy(this);
    initHitSensors("Body", "Body");
    initBinder(::cBinderRadius, 40.0f, 0);
    initEffectKeeper(1, nullptr, false);
    MR::initStarPointerTargetAtJoint(this, "Body", ::cStarWandRadius, TVec3f(0.0f, 0.0f, 0.0f));
    initSound(4, "FireBubble", nullptr, TVec3f(0.0f));
    MR::initShadowVolumeCylinder(this, 40.0f);
    MR::invalidateClipping(this);
    MR::declareCoin(this, ::cCoinMaxNum);
    initNerve(GET_NERVE(FireBubble, FireBubbleNrvAppear));


    makeActorDead();
}

void FireBubble::appear() {
    mPosition.set(mInitPos);
    mFront.set(mInitFront);
    MR::calcGravity(this);
    LiveActor::appear();
    setNerve(GET_NERVE(FireBubble, FireBubbleNrvAppear));
}

void FireBubble::appear(const TVec3f& rVec1, const TVec3f& rVec2, const TVec3f& rVec3) {
    mInitPos.set(rVec1);
    mInitFront.set(rVec2);
    mInitVelocity.set(rVec3);
    appear();
}

void FireBubble::kill() {
    LiveActor::kill();
    if (!mIsValidInfo) {
        MR::emitEffect(this, "Down");
    }
}

void FireBubble::control() {
}

void FireBubble::calcAndSetBaseMtx() {
    TVec3f minusGravity;
    TPos3f baseMtx;
    minusGravity.negate(mGravity);

    if (MR::isSameDirection(mFront, minusGravity)) {
        baseMtx.set(getBaseMtx());
        baseMtx.setTrans(mPosition);
    } else {
        MR::makeMtxUpFrontPos(&baseMtx, minusGravity, mFront, mPosition);
    }

    MR::setBaseTRMtx(this, baseMtx);
}

void FireBubble::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(GET_NERVE(FireBubble, FireBubbleNrvAppear)) && !isNerve(GET_NERVE(FireBubble, FireBubbleNrvDown)) &&
        !isNerve(GET_NERVE(FireBubble, FireBubbleNrvWaitToAppear))) {
        if (MR::isSensorPlayer(pReceiver) && MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
            killOrWaitToApppear();
        }
    }
}

bool FireBubble::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        MR::appearCoinPop(this, mPosition, 1);
        killOrWaitToApppear();
        return true;
    }

    if (MR::isMsgSupportTicoSpin(msg) && !isNerve(GET_NERVE(FireBubble, FireBubbleNrvReflect))) {
        MR::onCalcGravity(this);
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvReflect));
    }

    return false;
}

void FireBubble::initMapToolInfo(const JMapInfoIter& rIter) {
    mIsValidInfo = true;

    MR::initDefaultPos(this, rIter);
    MR::syncStageSwitchAppearAndSync(this, rIter);
    mInitPos.set(mPosition);
    MR::calcGravity(this);

    TPos3f Mtx;
    TVec3f minusGravity;
    TVec3f rotateZDir;
    minusGravity.negate(mGravity);

    MR::makeMtxRotate(Mtx, mRotation);
    Mtx.getZDir(rotateZDir);

    MR::makeMtxUpFront(&Mtx, minusGravity, rotateZDir);
    Mtx.getZDir(mInitFront);
}

#pragma push
#pragma opt_propagation off
void FireBubble::updateChaseFrontVec(f32 flt) {
    TVec3f vec2;
    TVec3f vec3;
    TVec3f pos;
    TVec3f playerUpVec;

    MR::getPlayerUpVec(&playerUpVec);
    MR::normalize(&playerUpVec);
    playerUpVec.scale(40.0f);

    pos.add(*MR::getPlayerPos(), playerUpVec);
    vec2.sub(pos, *getPosition());

    if (!MR::normalizeOrZero(&vec2)) {
        if (MR::isBindedGround(this)) {
            vec3.negate(MR::getGroundNormal(this));
        } else {
            vec3.set(mGravity);
        }

        if (!MR::isSameDirection(vec2, vec3)) {
            MR::vecKillElement(vec2, vec3, &vec2);
            MR::normalize(&vec2);

            TVec3f copy(mFront);

            MR::turnVecToVecCos(&mFront, copy, vec2, MR::cosDegree(flt), vec3);
        }
    }
}
#pragma pop

void FireBubble::updateChaseFrontVecAndVelocity(f32 flt) {
    updateChaseFrontVec(flt);
    mVelocity.set(mFront);
    mVelocity.mult(::cChaseSpeed);
}

void FireBubble::updateGravity(f32 strength) {
    mVelocity.add(mGravity * strength);
}

void FireBubble::calcReflectVelocity(TVec3f* pOut) const {
    TVec3f toPlayer;

    toPlayer.sub(*MR::getPlayerPos(), mPosition);

    if (MR::normalizeOrZero(&toPlayer)) {
        MR::calcFrontVec(&toPlayer, this);
    }

    TVec3f upVec;
    TPos3f rotateMatrix;
    MR::calcUpVec(&upVec, this);

    MR::makeMtxUpFront(&rotateMatrix, upVec, toPlayer);

    rotateMatrix.mult33(TVec3f(::cReflectVelocity), *pOut);
}

bool FireBubble::tryChaseEnd() {
    mChaseCounter++;

    if (mActFrame < mChaseCounter) {
        MR::onCalcGravity(this);
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvDown));
        return true;
    }

    return false;
}

void FireBubble::killOrWaitToApppear() {
    if (mIsValidInfo) {
        MR::emitEffect(this, "Down");
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvWaitToAppear));
    } else {
        kill();
    }
}

bool FireBubble::isMovingDown() {
    TVec3f normalisedVel;
    return !MR::normalizeOrZero(mVelocity, &normalisedVel) && normalisedVel.dot(mGravity) < 0.0f;
}

void FireBubble::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::emitEffect(this, "Wait");
        MR::startActionSound(this, "EmFireBubbleLaunch");
        MR::showModel(this);
        MR::onCalcGravity(this);
        MR::calcGravity(this);
        MR::offBind(this);
        MR::validateHitSensors(this);

        calcAndSetBaseMtx();
        TPos3f baseMtx;
        baseMtx.set(getBaseMtx());
        baseMtx.mult33(mInitVelocity, mVelocity);

        mChaseCounter = 0;
    }

    if (!MR::isNoBind(this) && MR::isOnGround(this)) {
        MR::startActionSound(this, "EmFireBubbleJump");
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvWait));
    } else {
        updateGravity(::cAppearGravity);

        if (MR::isNoBind(this) && !isMovingDown()) {
            MR::onBind(this);
        }
    }
}

void FireBubble::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        mVelocity.zero();
        mActFrame = MR::getRandom((s32)0, ::cRandomActFrame) + ::cForceKillFrame;
    }

    MR::isStarPointerPointing2PAttachTarget(this);

    if (MR::isGreaterStep(this, ::cChaseFrame) && MR::isNearPlayer(this, ::cChaseStartDistance)) {
        MR::startActionSound(this, "EmFireBubbleJump");
        mActFrame = MR::getRandom((s32)0, ::cRandomActFrame) + ::cChaseInvalidFrame;
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvChase));
    } else if (MR::isStep(this, mActFrame)) {
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvDown));
    }
}

void FireBubble::exeChase() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::emitEffect(this, "Wait");
    }

    updateChaseFrontVecAndVelocity(::cChaseRotateSpeed);
    updateGravity(::cChaseGravity);

    if (MR::checkPassBckFrame(this, 0.0f)) {
        MR::startActionSound(this, "EmFireBubbleJump");
    }

    MR::isStarPointerPointing2PAttachTarget(this);
    if (!tryChaseEnd()) {
        // return needed to generate comparison with unused result
        return;
    }
}

void FireBubble::exeReflect() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DPDHit");
        calcReflectVelocity(&mVelocity);
        MR::deleteEffect(this, "Wait");
        MR::emitEffect(this, "WaitS");
        MR::startActionSound(this, "EmFireBubbleReflect");
        MR::start2PAttackAssistSound();
    }

    updateChaseFrontVec(::cChaseRotateSpeed);
    mChaseCounter++;

    if (MR::isOnGround(this)) {
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvReflectLand));
    } else {
        updateGravity(::cReflectGravity);
    }
}

void FireBubble::exeReflectLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DPDHitDown");
        mVelocity.zero();
        MR::offCalcGravity(this);
    }

    MR::isStarPointerPointing2PAttachTarget(this);
    if (!tryChaseEnd()) {
        MR::setNerveAtBckStopped(this, GET_NERVE(FireBubble, FireBubbleNrvReflectWait));
    }
}

void FireBubble::exeReflectWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DPDHitDownWait");
    }

    MR::isStarPointerPointing2PAttachTarget(this);
    if (!tryChaseEnd() && MR::isStep(this, ::cReflectWaitFrame)) {
        MR::deleteEffect(this, "WaitS");
        MR::onCalcGravity(this);
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvChase));
    }
}

void FireBubble::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::deleteEffect(this, "Wait");
        MR::deleteEffect(this, "WaitS");
        MR::startActionSound(this, "EmFireBubbleDie");
        mVelocity.zero();
        MR::invalidateHitSensors(this);
    }

    updateChaseFrontVecAndVelocity(::cChaseRotateSpeed);

    updateGravity(::cChaseGravity);
    if (MR::isActionEnd(this)) {
        killOrWaitToApppear();
    }
}

void FireBubble::exeWaitToAppear() {
    if (MR::isFirstStep(this)) {
        MR::hideModel(this);
        MR::offCalcGravity(this);
        MR::offBind(this);
        MR::invalidateHitSensors(this);
    }

    if (MR::isStep(this, ::cAppearInterval)) {
        mPosition.set(mInitPos);
        mFront.set(mInitFront);
        mInitVelocity.set(::cAppearVelocity);
        MR::calcGravity(this);
        setNerve(GET_NERVE(FireBubble, FireBubbleNrvAppear));
    }
}
