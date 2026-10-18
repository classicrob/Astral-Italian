#include "Game/Enemy/Peto.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/LiveActor/ActorStateParamScale.hpp"

class YoshiLockOnTarget;

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};  // namespace MR

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
};  // namespace YoshiFunction

// Unnamed SMG2 helpers.
extern "C" void fn_800345E0(LiveActor*, const char*);
extern "C" void fn_80012B90(LiveActor*, HitSensor*, HitSensor*, s32, f32, f32);
extern "C" void fn_80034910(LiveActor*);
extern "C" void fn_8022BA10(LiveActor*);
extern "C" void fn_8022BAA0(LiveActor*);
extern "C" void fn_800347C0(LiveActor*, TVec3f*);
extern "C" f32 fn_8022D440(ActorStateParamScale*, s32);
extern "C" f32 fn_8022D3E0(ActorStateParamScale*, s32);
extern "C" f32 lbl_807E3030;
extern "C" u32 lbl_807E3060;
extern "C" f32 lbl_807E3068;

// Nerve instances (sbss 0x807D2F28..0x807D2F64). Names are provisional.
namespace NrvPeto {
    NEW_NERVE(PetoNrvHidden, Peto, Hidden);
    NEW_NERVE(PetoNrvFall, Peto, Fall);
    NEW_NERVE(PetoNrvLand, Peto, Land);
    NEW_NERVE(PetoNrvExtendsSign, Peto, ExtendsSign);
    NEW_NERVE(PetoNrvExtends, Peto, Extends);
    NEW_NERVE(PetoNrvExtendsKeep, Peto, ExtendsKeep);
    NEW_NERVE(PetoNrvShrink, Peto, Shrink);
    NEW_NERVE(PetoNrvShrinkEnd, Peto, ShrinkEnd);
    NEW_NERVE(PetoNrvJumpSign, Peto, JumpSign);
    NEW_NERVE(PetoNrvJump, Peto, Jump);
    class PetoNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static PetoNrvNone sInstance;
    };
    PetoNrvNone PetoNrvNone::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PetoNrvWait, Peto, Wait);
    NEW_NERVE(PetoNrvSwoon, Peto, Swoon);
    NEW_NERVE(PetoNrvYoshiEaten, Peto, YoshiEaten);
    NEW_NERVE(PetoNrvDown, Peto, Down);
    NEW_NERVE(PetoNrvPress, Peto, Press);
};  // namespace NrvPeto

Peto::Peto(const char* pName) : LiveActor(pName) {
    mAnimScale = nullptr;
    mBindStar = nullptr;
    mVolumeDrawer = nullptr;
    mYoshiLockOn = nullptr;
    mStateParam = nullptr;
    mScale = 1.0f;
    mIsOnceFlag = 0;
    mIsDustEmit = 0;
    mMtx.identity();
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mJumpPos.set(0.0f, 0.0f, 0.0f);
    mJumpStart.set(0.0f, 0.0f, 0.0f);
    mRailPos.set(0.0f, 0.0f, 0.0f);
    mWalkPos.set(0.0f, 0.0f, 0.0f);
    mDustPos.set(0.0f, 0.0f, 0.0f);
}

void Peto::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Peto", false);
    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordAndTransToNearestRailPoint(this);
    }
    mJumpPos.set(mPosition);
    mJumpStart.set(mPosition);
    mVolumeDrawer = MR::createVolumeDrawer("\x83\x78\x83\x67\x83\x5f\x83\x81", "VolumeSphere", mMtx.toMtxPtr());
    MR::calcGravity(this);
    mVolumeDrawer->setHost(this);
    MR::setGroupClipping(this, rIter, 16);
    initNerve(GET_NERVE(Peto, PetoNrvHidden), 1);
    mAnimScale = new AnimScaleController(nullptr);
    mBindStar = new WalkerStateBindStarPointer(this, mAnimScale);
    MR::initActorState(this, mBindStar, GET_NERVE(Peto, PetoNrvWait), "SupportFreeze");
    mStateParam = new ActorStateParamScale(this);
    mStateParam->init(rIter);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, nullptr, TVec3f(1.0f, 1.0f, 1.0f));
    MR::invalidateClipping(this);
    makeActorAppeared();
}

void Peto::control() {
    mAnimScale->updateNerve();
    calcAndSetBaseMtx();
    MR::preScaleMtx(mMtx.toMtxPtr(), TVec3f(mScale, mScale, mScale));
    MR::setSensorRadius(this, "Attack", mScale);
    if (isHitNerve() && mBindStar->tryStartPointBind()) {
        setNerve(GET_NERVE(Peto, PetoNrvPress));
    }
    MR::blendQuatFromGroundAndFront(&mQuat, this, mFront, 0.0f, 0.0f);
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
    if (isHitNerve()) {
        if (MR::isEqualSensor(pSender, this, "Attack")) {
            if (MR::isSensorPlayer(pReceiver) && MR::isOnGroundPlayer()) {
                TVec3f dir;
                MR::calcVecToPlayer(&dir, this);
                TVec3f up;
                MR::calcUpVec(&up, this);
                TVec3f off(up);
                off.scale(dir.dot(up));
                dir.add(off);
                f32 dist = dir.length();
                if (dist == mScale + MR::getSensorRadius(pReceiver)) {
                    MR::sendMsgEnemyAttackElectric(pReceiver, pSender);
                }
            }
        }
    }
    if (MR::isEqualSensor(pSender, this, "Body") && isLandNerve()) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool Peto::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        if (isPressNerve()) {
            setNerve(GET_NERVE(Peto, PetoNrvSwoon));
            fn_800345E0(this, "\x83\x58\x83\x5e\x81\x5b\x83\x73\x81\x5b");
        }
    }
    if (MR::isMsgPlayerHitProjectiles(msg)) {
        if (recvDamage(pSender, pReceiver)) {
            return true;
        }
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        if (recvDamage(pSender, pReceiver)) {
            return true;
        }
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        return recvTrample();
    }
    return false;
}

bool Peto::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
    }
    if (MR::isMsgExplosionAttack(msg) && isLandNerve()) {
        return recvDamage(pSender, pReceiver);
    }
    return false;
}

bool Peto::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
    }
    if (MR::isSensorEnemy(pSender) || MR::isSensorRide(pSender)) {
        return true;
    }
    if (!recvDamageCheck() && MR::isSensorPlayer(pSender)) {
        return true;
    }
    return false;
}

bool Peto::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isPressNerve()) {
            MR::sendMsgEnemyAttackElectric(MR::getPlayerBodySensor(), pReceiver);
        }
        if (!isNerve(GET_NERVE(Peto, PetoNrvYoshiEaten))) {
            MR::invalidateClipping(this);
            MR::zeroVelocity(this);
            setNerve(GET_NERVE(Peto, PetoNrvYoshiEaten));
        }
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
    }
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        resetState(true);
    }
    if (MR::isMsgPlayerKick(msg) && isNerve(GET_NERVE(Peto, PetoNrvHidden))) {
        recvDamage(pSender, pReceiver);
    }
    return false;
}

// fn_801EB4F0: damage reaction (shared by attack paths).
bool Peto::recvDamage(HitSensor* pSender, HitSensor* pReceiver) {
    MR::invalidateClipping(this);
    MR::onBind(this);
    fn_80012B90(this, pSender, pReceiver, 4, 30.0f, 30.0f);
    TVec3f normal;
    MR::calcSensorHorizonNormalize(&mFront, mGravity, pReceiver, pSender);
    TVec3f up(mGravity);
    up.negate();
    MR::makeQuatUpFront(&mQuat, up, mFront);
    setNerve(GET_NERVE(Peto, PetoNrvDown));
    return true;
}

// fn_801EB5A0
bool Peto::recvTrample() {
    if (MR::isExistRail(this) || isLandNerve()) {
        MR::invalidateClipping(this);
        MR::onBind(this);
        setNerve(GET_NERVE(Peto, PetoNrvPress));
        return true;
    }
    return false;
}

bool Peto::recvExplosion() {
    return false;
}

// fn_801EC3B0: reset to hidden.
void Peto::resetState(bool bShowSwitch) {
    MR::hideModel(this);
    MR::zeroVelocity(this);
    MR::setRotation(this, TVec3f(0.0f, 0.0f, 0.0f));
    MR::setScale(this, 1.0f);
    MR::setPosition(this, mJumpStart);
    setNerve(GET_NERVE(Peto, PetoNrvHidden));
    if (bShowSwitch) {
        fn_80034910(this);
    }
    if (MR::isValidSwitchDead(this)) {
        MR::onSwitchDead(this);
    }
}

// fn_801EB600
void Peto::exeHidden() {
    if (mStateParam->isFirstStep()) {
        MR::hideModel(this);
    }
    MR::setRotation(this, TVec3f(0.0f, 0.0f, 0.0f));
    updateScale(0.0f);
    if (mStateParam->isGreaterStep(0xF0)) {
        setNerve(GET_NERVE(Peto, PetoNrvFall));
    }
}

// fn_801EB690
void Peto::exeFall() {
    if (mStateParam->isFirstStep()) {
        MR::showModel(this);
        MR::startAction(this, "Fall");
    }
    MR::attenuateVelocity(this, 0.9f);
    MR::addVelocityToGravity(this, 1.0f);
    if (MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
        mFront.set(mPosition);
        setNerve(GET_NERVE(Peto, PetoNrvLand));
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
}

// fn_801EB740
void Peto::exeLand() {
    if (mStateParam->isFirstStep()) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Land");
        MR::validateClipping(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Peto, PetoNrvExtendsSign));
    }
}

// fn_801EB7C0
void Peto::exeExtendsSign() {
    if (mStateParam->isFirstStep()) {
        MR::invalidateClipping(this);
        MR::startAction(this, "ExtendsSign");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Peto, PetoNrvExtends));
    }
}

// fn_801EB850
void Peto::exeExtends() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Extends");
        mScale = 0.0f;
        mIsOnceFlag = 0;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    f32 speed = 0.1f * mStateParam->getSpeed();
    mScale += speed;
    updateScale(speed);
    if (mScale == 1.0f) {
        mScale = 1.0f;
        setNerve(GET_NERVE(Peto, PetoNrvExtendsKeep));
    }
}

// fn_801EB900
void Peto::exeExtendsKeep() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "ExtendsKeep");
        mIsOnceFlag = 0;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    updateJumpVel();
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
    if (updateScale(-0.02f)) {
        setNerve(GET_NERVE(Peto, PetoNrvShrinkEnd));
    }
}

// fn_801EB9F0
void Peto::exeShrinkEnd() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "ShurinkEnd");
        mScale = 0.5f;
    }
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Peto, PetoNrvJumpSign));
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
}

// fn_801EBA70
void Peto::exeJumpSign() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "JumpSign");
        mIsOnceFlag = 0;
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isActionEnd(this)) {
        if (mIsOnceFlag == 0) {
            mWalkPos.set(mJumpStart);
        }
        mIsOnceFlag = 0;
        mJumpPos.set(mPosition);
        setNerve(GET_NERVE(Peto, PetoNrvJump));
    }
}

// fn_801EBB30
void Peto::exeJump() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Jump");
    }
    TVec3f toJump = mWalkPos - mJumpPos;
    f32 dot = -mGravity.dot(toJump);
    f32 a = 0.0f;
    f32 b = 0.0f;
    MR::calcParabolicFunctionParam(&a, &b, 0.0f, dot);
    f32 t = fn_8022D440(mStateParam, 45);
    f32 s = fn_8022D3E0(mStateParam, 45);
    f32 h = s * (b + a * s);
    TVec3f pos(mJumpPos);
    pos.add(toJump);
    TVec3f blend;
    MR::vecBlend(&blend, mJumpPos, pos, t);
    TVec3f lift(mGravity);
    lift.scale(-h);
    blend.add(lift);
    MR::setPosition(this, blend);
    MR::turnDirectionToPlayerDegreeHorizon(this, &mFront, 180.0f);
    if (mStateParam->isGreaterStep(45) || (mStateParam->isGreaterStep(22) && MR::isBindedGround(this))) {
        MR::zeroVelocity(this);
        setNerve(GET_NERVE(Peto, PetoNrvLand));
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
}

// fn_801EBCE0
void Peto::exeWait() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Wait");
    }
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    updateJumpVel();
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Peto, PetoNrvHidden));
}

// fn_801EBD50
void Peto::exeSwoon() {
    if (mStateParam->isFirstStep()) {
        mAnimScale->startHitReaction();
        MR::startAction(this, "Swoon");
    }
    MR::attenuateVelocity(this, 0.9f);
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    if (MR::isBindedGround(this)) {
        MR::zeroVelocity(this);
        mFront.set(mPosition);
        setNerve(GET_NERVE(Peto, PetoNrvLand));
    }
}

// fn_801EBE00
void Peto::exeYoshiEaten() {
    YoshiFunction::tryYoshiEat(mYoshiLockOn);
    updateJumpVel();
    if (mStateParam->isGreaterStep(180)) {
        setNerve(GET_NERVE(Peto, PetoNrvJump));
    }
}

// fn_801EBE60
void Peto::exeDown() {
    if (mStateParam->isFirstStep()) {
        MR::startAction(this, "Down");
        fn_800347C0(this, &mPosition);
    }
    MR::addVelocityToGravity(this, 0.1f);
    MR::attenuateVelocity(this, 0.98f);
    if (mStateParam->isGreaterStep(40) || MR::isBinded(this)) {
        fn_8022BA10(this);
        resetState(true);
    }
}

// fn_801EBF10
void Peto::exePress() {
    if (mStateParam->isFirstStep()) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Press");
    }
    if (mStateParam->isGreaterStep(40)) {
        fn_8022BAA0(this);
        resetState(true);
    }
}

// fn_801EBF90
bool Peto::isHitNerve() const {
    return isNerve(GET_NERVE(Peto, PetoNrvExtends)) || isNerve(GET_NERVE(Peto, PetoNrvExtendsKeep)) ||
           isNerve(GET_NERVE(Peto, PetoNrvShrink));
}

// fn_801EC000
bool Peto::isLandNerve() const {
    return isNerve(GET_NERVE(Peto, PetoNrvFall)) || isNerve(GET_NERVE(Peto, PetoNrvLand)) ||
           isNerve(GET_NERVE(Peto, PetoNrvJump));
}

// fn_801EC0D0
bool Peto::recvDamageCheck() const {
    return isNerve(GET_NERVE(Peto, PetoNrvHidden)) || isNerve(GET_NERVE(Peto, PetoNrvPress)) ||
           isNerve(GET_NERVE(Peto, PetoNrvDown));
}

// fn_801EC1D0
bool Peto::isPressNerve() const {
    if (isNerve(GET_NERVE(Peto, PetoNrvHidden)) || isNerve(GET_NERVE(Peto, PetoNrvPress)) ||
        isNerve(GET_NERVE(Peto, PetoNrvDown))) {
        return true;
    }
    if (isNerve(GET_NERVE(Peto, PetoNrvSwoon))) {
        return MR::isGreaterStep(this, 20);
    }
    return false;
}

// fn_801EC340: shrink the balloon scale by the given rate.
bool Peto::updateScale(f32 rate) {
    mScale -= rate * mStateParam->getSpeed();
    if (mScale < 0.5f) {
        mScale = 0.5f;
        return true;
    }
    return false;
}

// fn_801EC790: jump/extend dust.
void Peto::updateJumpVel() {
    if (mIsDustEmit == 0 && isNearPlayerSensor()) {
        mIsDustEmit = 1;
    }
    if (MR::isIntervalStep(this, 5)) {
        if (mIsDustEmit != 0) {
            MR::emitEffectHit(this, mDustPos, "GroundSpark");
        }
        mIsDustEmit = 0;
    }
}

bool Peto::isNearPlayerSensor() {
    return false;
}

void Peto::spawnDust() {}

// Peto::~Peto
Peto::~Peto() {}
