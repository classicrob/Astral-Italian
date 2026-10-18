#pragma auto_inline off
#include "Game/Enemy/Peto.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/PetoPopoParam.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#include "Game/Util/YoshiLockOnUtil.hpp"

#define PETO_NERVE(Name, Method) \
    class Name : public Nerve { public: virtual void execute(Spine* spine) const { static_cast<Peto*>(spine->mExecutor)->Method(); } static Name sInstance; }; \
    Name Name::sInstance ATTRIBUTE_WEAK;

// Nerve instances (sbss 0x807D2F28..0x807D2F64). Names are provisional.
namespace NrvPeto {
    PETO_NERVE(PetoNrvHidden, fn_801EB600);
    PETO_NERVE(PetoNrvFall, fn_801EB690);
    PETO_NERVE(PetoNrvLand, fn_801EB740);
    PETO_NERVE(PetoNrvExtendsSign, fn_801EB7C0);
    PETO_NERVE(PetoNrvExtends, fn_801EB850);
    PETO_NERVE(PetoNrvExtendsKeep, fn_801EB900);
    NEW_NERVE(PetoNrvShrink, Peto, Shrink);
    PETO_NERVE(PetoNrvShrinkEnd, fn_801EB9F0);
    PETO_NERVE(PetoNrvJumpSign, fn_801EBA70);
    PETO_NERVE(PetoNrvJump, fn_801EBB30);
    class PetoNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const { static_cast<Peto*>(pSpine->mExecutor)->updateScale(40.0f); }
        static PetoNrvNone sInstance;
    };
    PetoNrvNone PetoNrvNone::sInstance ATTRIBUTE_WEAK;
    PETO_NERVE(PetoNrvWait, fn_801EBCE0);
    PETO_NERVE(PetoNrvSwoon, fn_801EBD50);
    PETO_NERVE(PetoNrvYoshiEaten, fn_801EBE00);
    NEW_NERVE(PetoNrvDown, Peto, Down);
    PETO_NERVE(PetoNrvPress, fn_801EBF10);
};  // namespace NrvPeto

Peto::Peto(const char* pName) : LiveActor(pName) {
    mAnimScale = nullptr;
    mBindStar = nullptr;
    mVolumeDrawer = nullptr;
    mYoshiLockOn = nullptr;
    mStateParam = nullptr;
    mFront.set(0.0f, 0.0f, 1.0f);
    mJumpPos.set(0.0f, 0.0f, 0.0f);
    mJumpStart.set(0.0f, 0.0f, 0.0f);
    mScale = 1.0f;
    mRailPos.set(0.0f, 0.0f, 0.0f);
    mWalkPos.set(0.0f, 0.0f, 0.0f);
    mDustPos.set(0.0f, 0.0f, 0.0f);
    mIsOnceFlag = 0;
    mIsDustEmit = 0;
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mMtx.identity();
}

void Peto::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Peto", false);
    MR::makeQuatAndFrontFromRotate(&mQuat, &mFront, this);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToNearestRailPoint(this);
    }
    mJumpPos.set(mPosition);
    mJumpStart.set(mPosition);
    mVolumeDrawer = MR::createVolumeDrawer("ペトダメージ範囲描画", "VolumeSphere", mMtx.toMtxPtr());
    mVolumeDrawer->setHost(this);
    MR::calcGravity(this);
    MR::setGroupClipping(this, rIter, 16);
    initNerve(GET_NERVE(Peto, PetoNrvFall), 1);
    mAnimScale = new AnimScaleController(nullptr);
    mBindStar = new WalkerStateBindStarPointer(this, mAnimScale);
    MR::initActorState(this, mBindStar, GET_NERVE(Peto, PetoNrvWait), "SupportFreeze");
    mStateParam = new ActorStateParamScale(this);
    mStateParam->init(rIter);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast<MtxPtr>(nullptr), TVec3f(0.0f));
    MR::invalidateClipping(this);
    makeActorAppeared();
}

void Peto::control() {
    mAnimScale->updateNerve();
    mMtx.set(getBaseMtx());
    f32 scale = mScale / 100.0f;
    MR::preScaleMtx(mMtx.toMtxPtr(), TVec3f(scale, 0.25f * scale, scale));
    MR::setSensorRadius(this, "Attack", mScale);
    if (fn_801EC1D0() && mBindStar->tryStartPointBind()) {
        setNerve(GET_NERVE(Peto, PetoNrvWait));
        return;
    }
    MR::blendQuatFromGroundAndFront(&mQuat, this, mFront, 0.2f, 0.2f);
    mStateParam->calcHostVelocity();
}

void Peto::movement() {
    mStateParam->update();
    LiveActor::movement();
    mStateParam->resetHostVelocity();
}

void Peto::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
    MR::updateBaseScale(this, mAnimScale);
}

void Peto::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (fn_801EBF90()) {
        if (MR::isEqualSensor(pSender, this, "Attack")) {
            if (MR::isSensorPlayer(pReceiver) && MR::isOnGroundPlayer()) {
                TVec3f dir;
                MR::calcVecToPlayer(&dir, this);
                TVec3f up;
                MR::calcUpVec(&up, this);
                TVec3f off = up * (3.0f * dir.dot(up));
                dir.add(off);
                f32 dist = dir.length();
                if (dist <= mScale + MR::getSensorRadius(pReceiver)) {
                    MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
                    return;
                }
            }
        }
    }
    if (MR::isEqualSensor(pSender, this, "Body") && fn_801EC150()) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Peto::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, this, "Body")) return false;
    if (MR::isMsgLockOnStarPieceShoot(msg)) return true;
    if (MR::isMsgStarPieceAttack(msg) && fn_801EC2A0()) {
        setNerve(GET_NERVE(Peto, PetoNrvSwoon));
        MR::tryHitReaction(this, "スターピースヒット");
        return true;
    }
    if (MR::isMsgPlayerHitProjectiles(msg) && fn_801EC0D0()) return fn_801EB4F0(pSender, pReceiver);
    if (MR::isMsgPlayerHitAll(msg) && fn_801EC000()) return fn_801EB4F0(pSender, pReceiver);
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) return fn_801EB5A0();
    return false;
}

bool Peto::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, this, "Body")) return false;
    if (MR::isMsgExplosionAttack(msg) && fn_801EC0D0()) return fn_801EB4F0(pSender, pReceiver);
    return false;
}

bool Peto::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isEqualSensor(pReceiver, this, "Body")) return false;
    if ((MR::isSensorEnemy(pSender) || MR::isSensorRide(pSender) || (!fn_801EBF90() && MR::isSensorPlayer(pSender))) && fn_801EC150()) return true;
    return false;
}

bool Peto::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (fn_801EBF90()) {
            MR::sendMsgEnemyAttackElectric(MR::getPlayerBodySensor(), pReceiver);
            return false;
        }
        if (!isNerve(GET_NERVE(Peto, PetoNrvNone))) {
            MR::invalidateClipping(this);
            setNerve(GET_NERVE(Peto, PetoNrvNone));
            MR::zeroVelocity(this);
            return true;
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) return true;
    if (MR::isMsgYoshiReleaseOrCancelEaten(msg)) { fn_801EC3B0(false); return true; }
    if (MR::isMsgPlayerKick(msg) && isNerve(GET_NERVE(Peto, PetoNrvYoshiEaten))) {
        fn_801EB4F0(pSender, pReceiver);
        return true;
    }
    return false;
}

// fn_801EB4F0: damage reaction (shared by attack paths).
bool Peto::fn_801EB4F0(HitSensor* pSender, HitSensor* pReceiver) {
    MR::onBind(this);
    MR::invalidateClipping(this);
    MR::setVelocityBlowAttack(this, pSender, pReceiver, 40.0f, 40.0f, 4);
    MR::calcSensorHorizonNormalize(&mFront, mGravity, pReceiver, pSender);
    TVec3f up = -mGravity;
    MR::makeQuatUpFront(&mQuat, up, mFront);
    setNerve(GET_NERVE(Peto, PetoNrvDown));
    return true;
}

// fn_801EB5A0
bool Peto::fn_801EB5A0() {
    if (fn_801EC000()) {
        MR::onBind(this);
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(Peto, PetoNrvPress));
        return true;
    }
    return false;
}

// fn_801EC3B0: reset to hidden.
void Peto::fn_801EC3B0(bool bShowSwitch) {
    MR::hideModel(this);
    MR::setPosition(this, mJumpPos);
    MR::setScale(this, 1.0f);
    MR::setRotation(this, 0.0f);
    MR::zeroVelocity(this);
    setNerve(GET_NERVE(Peto, PetoNrvHidden));
    if (bShowSwitch) MR::tryHitReactionDead(this);
    kill();
    if (MR::isValidSwitchDead(this)) MR::onSwitchDead(this);
}

// fn_801EB600
void Peto::fn_801EB600() {
    if (mStateParam->isFirstStep()) {
        MR::hideModel(this);
    }
    MR::setScale(this, 1.0f);
    MR::setRotation(this, 0.0f);
    updateScale(12.0f);
    if (mStateParam->isGreaterStep(0xF0)) {
        setNerve(GET_NERVE(Peto, PetoNrvFall));
    }
}

// fn_801EB690
void Peto::fn_801EB690() {
    if (mStateParam->isFirstStep()) {
        MR::showModel(this);
        MR::startAction(this, "Fall");
    }
    MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.98f);
    if (MR::isBindedGround(this)) {
        mJumpStart.set(mPosition);
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Peto, PetoNrvLand));
        return;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
}

// fn_801EB740
void Peto::fn_801EB740() {
    if (mStateParam->isFirstStep()) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Land");
        MR::validateClipping(this);
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Peto, PetoNrvExtendsSign));
    }
}

// fn_801EB7C0
void Peto::fn_801EB7C0() {
    if (mStateParam->isFirstStep()) {
        MR::invalidateClipping(this);
        MR::startAction(this, "ExtendsSign");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 5.0f);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Peto, PetoNrvExtends));
    }
}

// fn_801EB850
void Peto::fn_801EB850() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Extends");
        mScale = 100.0f;
        mIsDustEmit = 0;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    f32 speed = 4.0f * mStateParam->getSpeed();
    mScale += speed;
    fn_801EC790();
    if (mScale >= 500.0f) {
        mScale = 500.0f;
        setNerve(GET_NERVE(Peto, PetoNrvExtendsKeep));
    }
}

// fn_801EB900
void Peto::fn_801EB900() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "ExtendsKeep");
        mIsDustEmit = 0;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    fn_801EC790();
    if (mStateParam->isGreaterStep(40)) {
        setNerve(GET_NERVE(Peto, PetoNrvShrink));
    }
}

// fn_801EB980
void Peto::exeShrink() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Shurink");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (updateScale(12.0f)) {
        setNerve(GET_NERVE(Peto, PetoNrvShrinkEnd));
    }
}

// fn_801EB9F0
void Peto::fn_801EB9F0() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "ShurinkEnd");
        mScale = 1.0f;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Peto, PetoNrvJumpSign));
    }
}

// fn_801EBA70
void Peto::fn_801EBA70() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "JumpSign");
        mIsOnceFlag = 0;
    }
    fn_801EC460();
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 4.0f);
    if (MR::isActionEnd(this)) {
        if (mIsOnceFlag == 0) {
            mWalkPos.set(mJumpStart);
        }
        mIsOnceFlag = 0;
        mRailPos.set(mPosition);
        setNerve(GET_NERVE(Peto, PetoNrvJump));
    }
}

// fn_801EBB30
void Peto::fn_801EBB30() {
    if (mStateParam->isFirstStep()) MR::startAction(this, "Jump");
    TVec3f toJump = mWalkPos - mRailPos;
    f32 dot = -mGravity.dot(toJump);
    f32 a, b;
    MR::calcParabolicFunctionParam(&a, &b, 300.0f, dot);
    f32 t = mStateParam->fn_8022D440(45);
    f32 u = mStateParam->fn_8022D3E0(45);
    f32 height = u * (b + a * u);
    TVec3f blend;
    TVec3f pos = mWalkPos + mGravity * dot;
    MR::vecBlend(&blend, mRailPos, pos, t);
    blend.add(mGravity * -height);
    MR::setPosition(this, blend);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 4.0f);
    if (mStateParam->isGreaterStep(45) || (mStateParam->isGreaterStep(22) && MR::isBindedGround(this))) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Peto, PetoNrvLand));
        return;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
}

// fn_801EBCE0
void Peto::fn_801EBCE0() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Wait");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    updateScale(12.0f);
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Peto, PetoNrvFall));
}

// fn_801EBD50
void Peto::fn_801EBD50() {
    if (mStateParam->isFirstStep()) {
        mAnimScale->startHitReaction();
        MR::startAction(this, "Swoon");
    }
    MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.98f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    updateScale(40.0f);
    if (MR::isBindedGround(this)) {
        mJumpStart.set(mPosition);
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Peto, PetoNrvYoshiEaten));
    }
}

// fn_801EBE00
void Peto::fn_801EBE00() {
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    updateScale(40.0f);
    if (mStateParam->isGreaterStep(180)) {
        setNerve(GET_NERVE(Peto, PetoNrvExtendsSign));
    }
}

// fn_801EBE60
void Peto::exeDown() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Down");
        MR::tryHitReactionBlow(this, &mPosition);
    }
    MR::addVelocityToGravity(this, 1.0f);
    MR::attenuateVelocity(this, 0.97f);
    if (mStateParam->isGreaterStep(40) || MR::isBinded(this)) {
        MR::emitItemHit(this);
        fn_801EC3B0(true);
    }
}

// fn_801EBF10
void Peto::fn_801EBF10() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Press");
        MR::zeroVelocity(this);
    }
    if (mStateParam->isGreaterStep(40)) {
        MR::emitItemStomp(this);
        fn_801EC3B0(true);
    }
}

// fn_801EBF90
bool Peto::fn_801EBF90() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvExtends)) || isNerve(GET_NERVE(Peto, PetoNrvExtendsKeep)) ||
           isNerve(GET_NERVE(Peto, PetoNrvShrink))) return true;
    return false;
}

// fn_801EC000
bool Peto::fn_801EC000() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvLand)) || isNerve(GET_NERVE(Peto, PetoNrvExtendsSign)) ||
           isNerve(GET_NERVE(Peto, PetoNrvShrinkEnd)) || isNerve(GET_NERVE(Peto, PetoNrvJumpSign)) ||
           isNerve(GET_NERVE(Peto, PetoNrvJump)) || isNerve(GET_NERVE(Peto, PetoNrvWait)) ||
           isNerve(GET_NERVE(Peto, PetoNrvSwoon)) || isNerve(GET_NERVE(Peto, PetoNrvYoshiEaten))) return true;
    return false;
}

// fn_801EC0D0
bool Peto::fn_801EC0D0() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvHidden)) || isNerve(GET_NERVE(Peto, PetoNrvNone)) ||
             isNerve(GET_NERVE(Peto, PetoNrvDown)) || isNerve(GET_NERVE(Peto, PetoNrvPress))) return false;
    return true;
}

// fn_801EC1D0
bool Peto::fn_801EC1D0() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvHidden)) || isNerve(GET_NERVE(Peto, PetoNrvNone)) ||
        isNerve(GET_NERVE(Peto, PetoNrvDown)) || isNerve(GET_NERVE(Peto, PetoNrvWait)) ||
        isNerve(GET_NERVE(Peto, PetoNrvSwoon)) || isNerve(GET_NERVE(Peto, PetoNrvPress))) return false;
    if (isNerve(GET_NERVE(Peto, PetoNrvYoshiEaten))) return MR::isGreaterStep(this, 20);
    return true;
}

// fn_801EC340: shrink the balloon scale by the given rate.
bool Peto::updateScale(f32 rate) {
    mScale -= rate * mStateParam->getSpeed();
    if (mScale < 1.0f) {
        mScale = 1.0f;
        return true;
    }
    return false;
}

// fn_801EC790: jump/extend dust.
void Peto::fn_801EC790() {
    if (mIsDustEmit == 0 && fn_801EC5E0(&mDustPos)) {
        mIsDustEmit = 1;
    }
    if (MR::isIntervalStep(this, 5)) {
        if (mIsDustEmit != 0) {
            MR::emitEffectHit(this, mDustPos, "GroundSpark");
        }
        mIsDustEmit = 0;
    }
}

bool Peto::fn_801EC150() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvHidden)) || isNerve(GET_NERVE(Peto, PetoNrvNone)) ||
             isNerve(GET_NERVE(Peto, PetoNrvDown)) || isNerve(GET_NERVE(Peto, PetoNrvPress))) return false;
    return true;
}

bool Peto::fn_801EC2A0() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvHidden)) || isNerve(GET_NERVE(Peto, PetoNrvNone)) ||
             isNerve(GET_NERVE(Peto, PetoNrvDown)) || isNerve(GET_NERVE(Peto, PetoNrvSwoon)) ||
             isNerve(GET_NERVE(Peto, PetoNrvPress))) return false;
    return true;
}

void Peto::fn_801EC460() {
    if (mIsOnceFlag == 0) {
        if (MR::isExistRail(this)) {
            if (MR::isRailReachedGoal(this)) MR::reverseRailDirection(this);
            MR::moveRailToNextPoint(this);
            mWalkPos.set(MR::getRailPos(this));
            mIsOnceFlag = 1;
            return;
        }
        fn_801EC4F0();
    }
}

void Peto::fn_801EC4F0() {
    TVec3f random;
    TVec3f hit;
    MR::getRandomVector(&random, 400.0f);
    TVec3f& gravity = mGravity;
    JMAVECScaleAdd(&gravity, &random, &random, -gravity.dot(random));
    random.add(mJumpStart);
    TVec3f offset = (mGravity * 100.0f) * 0.5f;
    if (MR::getFirstPolyOnLineToMap(&hit, nullptr, random - offset, offset * 2.0f)) {
        mWalkPos.set(hit);
        mIsOnceFlag = 1;
        return;
    }
    mWalkPos.set(random);
}

// NONMATCHING: instruction scheduling only (r3/r4 argument setup order around three TVec3 calls).
bool Peto::fn_801EC5E0(TVec3f* out) {
    if (MR::isJudgedToClipFrustum50m(mPosition, mScale)) return false;
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f& gravity = mGravity;
    JMAVECScaleAdd(&gravity, &front, &front, -gravity.dot(front));
    MR::normalizeOrZero(&front);
    MR::rotateVecDegree(&front, gravity, MR::getRandomDegree());
    f32 radius = mScale * MR::cosDegree(20.0f);
    f32 depth = 0.25f * radius * MR::sinDegree(20.0f);
    TVec3f offset = front * radius + mGravity * -depth;
    TVec3f hit;
    if (MR::getFirstPolyOnLineToMap(&hit, nullptr, mPosition + offset, (mGravity * depth) * 2.0f)) {
        out->set(hit);
        return true;
    }
    return false;
}

// Peto::~Peto
Peto::~Peto() {}

PetoPopoParam::PetoPopoParam() : AnimScaleParam() {
    _20 = 0.2f;
    _24 = 0.3f;
}
