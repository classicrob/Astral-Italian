#include "Game/Enemy/Takobo.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/SupportTicoStop.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Enemy/TakoboInkHolder.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

// Takobo is the blue sibling of TakoSho: same layout and almost the same code (see TakoSho.cpp).
// NOTE: the unit's split ends at 0x80205390 but the nerve ctors/executes (0x80205390..0x80205548) and the
// leading strings/.data (0x80691CA8..) belong to it too.

extern const volatile s32 gTakoboInkSpurtStep;

namespace NrvTakobo {
    NEW_NERVE(TakoboNrvWaitAfter, Takobo, Idle);
    NEW_NERVE(TakoboNrvWait, Takobo, Idle);
    NEW_NERVE(TakoboNrvMove, Takobo, Move);
    NEW_NERVE(TakoboNrvAttackSign, Takobo, AttackSign);
    NEW_NERVE(TakoboNrvAttack, Takobo, Attack);
    NEW_NERVE(TakoboNrvRecover, Takobo, Wait);
    NEW_NERVE(TakoboNrvHitReaction, Takobo, HitReaction);
    NEW_NERVE(TakoboNrvSpinHit, Takobo, HitPunch);
    NEW_NERVE(TakoboNrvPress, Takobo, Press);
    NEW_NERVE(TakoboNrvStun, Takobo, Stun);
    NEW_NERVE(TakoboNrvStunEnd, Takobo, StunEnd);

    // Nerve with an empty execute (Tongue; only set on Yoshi start-tongue).
    class TakoboNrvTongue : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static TakoboNrvTongue sInstance;
    };
    TakoboNrvTongue TakoboNrvTongue::sInstance ATTRIBUTE_WEAK;

    class TakoboNrvSupportFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            Takobo* actor = static_cast< Takobo* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &TakoboNrvWaitAfter::sInstance);
        };
        static TakoboNrvSupportFreeze sInstance;
    };
    TakoboNrvSupportFreeze TakoboNrvSupportFreeze::sInstance ATTRIBUTE_WEAK;

    // Dead nerve (set by kill; makeActorAppeared refuses to revive in it).
    class TakoboNrvDead : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static TakoboNrvDead sInstance;
    };
    TakoboNrvDead TakoboNrvDead::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvTakobo

Takobo::Takobo(const char* pName)
    : LiveActor(pName), mFrontVec(0, 0, 1), mSideVec(1, 0, 0), mIsReverse(false), mIsRight(false), mMoveRange(5.0f), mSpeed(500.0f),
      mTargetPos(0.0f, 0.0f, 0.0f), mMoveStep(0), mScaleController(nullptr), mBindStarPointer(nullptr), mSpinHitController(nullptr),
      mInkHolder(nullptr), mYoshiLockOn(nullptr), mConnector(nullptr) {}

// NONMATCHING: register allocation of the ink-gravity loop (i/pInk swap r28/r30).
void Takobo::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mStartPos.set(mPosition);
    s32 arg = -1;
    MR::calcFrontVec(&mFrontVec, this);
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f negUp = -up;
    mGravity.set(negUp);

    arg = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg);
    if (arg == 0) {
        MR::calcSideVec(&mSideVec, this);
        mIsReverse = false;
        mIsRight = false;
    } else if (arg == 1) {
        MR::calcSideVec(&mSideVec, this);
        mIsReverse = false;
        mIsRight = true;
    } else if (arg == 3) {
        MR::calcFrontVec(&mSideVec, this);
        mIsReverse = true;
        mIsRight = true;
    } else {
        MR::calcFrontVec(&mSideVec, this);
        mIsReverse = true;
        mIsRight = false;
    }

    arg = -1;
    MR::getJMapInfoArg2NoInit(rIter, &arg);
    if (arg != -1) {
        mMoveRange = arg;
    }
    arg = -1;
    MR::getJMapInfoArg3NoInit(rIter, &arg);
    if (arg != -1) {
        mSpeed = arg;
    }
    arg = -1;
    MR::getJMapInfoArg5NoInit(rIter, &arg);
    if (arg != -1) {
        mConnector = new MapObjConnector(this);
        mSpeed = 0.0f;
    }
    if (mConnector != nullptr) {
        MR::setSensorFollowMtx(this, "Body", getBaseMtx());
    }

    mInkHolder = new TakoboInkHolder(2);
    arg = -1;
    MR::getJMapInfoArg4NoInit(rIter, &arg);
    if (arg != -1) {
        s32 i;
        TakoboInk* pInk;
        for (i = 0; i < mInkHolder->getObjectCount(); i++) {
            pInk = mInkHolder->getMember(i);
            MR::offCalcGravity(pInk);
            pInk->mGravity = mGravity;
        }
    }
    mScaleController = new AnimScaleController(nullptr);
    initNerve(&NrvTakobo::TakoboNrvWait::sInstance, 1);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, &NrvTakobo::TakoboNrvSupportFreeze::sInstance, "SupportFreeze");
    mSpinHitController = new SpinHitController(this, 0x16, 0, 1.5f, 20.0f, 35.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Center", TVec3f(0.0f));
    makeActorAppeared();
}

void Takobo::initAfterPlacement() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f start = mPosition + up * 500.0f;
    if (MR::getFirstPolyOnLineToMap(&mPosition, nullptr, start, -up * 500.0f)) {
        mStartPos.set(mPosition);
    }
    if (mConnector != nullptr) {
        mConnector->attachToUnder();
    }
}

void Takobo::makeActorAppeared() {
    if (!isNerve(&NrvTakobo::TakoboNrvDead::sInstance)) {
        LiveActor::makeActorAppeared();
    }
}

void Takobo::kill() {
    MR::tryHitReactionDead(this);
    setNerve(&NrvTakobo::TakoboNrvDead::sInstance);
    LiveActor::kill();
}

void Takobo::control() {
    if (isYoshiEatEnable()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
    TVec3f zero;
    zero.x = 0.0f;
    zero.y = 0.0f;
    zero.z = 0.0f;
    if (MR::isInDeath(this, zero)) {
        kill();
        return;
    }
    tryStartSupportFreeze();
    mScaleController->updateNerve();
}

void Takobo::exeIdle() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::zeroVelocity(this);
        if (isMoving()) {
            MR::onBind(this);
        } else {
            MR::offBind(this);
        }
    }
    updateFallVelocity();
    SupportTicoStop::turnDirectionToNearbyPlayer(&mFrontVec, this);
    if (MR::calcDistanceToPlayer(this) < 700.0f) {
        setNerve(&NrvTakobo::TakoboNrvAttackSign::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 60)) {
        if (isMoving()) {
            setNerve(&NrvTakobo::TakoboNrvMove::sInstance);
            return;
        }
        setNerve(&NrvTakobo::TakoboNrvWaitAfter::sInstance);
    }
}

// NONMATCHING: vector temporaries occupy different stack slots; calls, constants and branches match.
namespace {
    inline f32 scaleTakoboMoveDuration(f32 steps, f32 scale) {
        return steps * scale;
    }
}

void Takobo::exeMove() {
    if (MR::isFirstStep(this)) {
        f32 dir;
        if (mIsRight) {
            dir = 1.0f;
        } else {
            dir = -1.0f;
        }
        mTargetPos = mStartPos - (mSideVec * mSpeed) * dir;
        mMoveStep = (s32)scaleTakoboMoveDuration((f32)(s32)((mTargetPos - mPosition).length() / mMoveRange), 1.3f);
    }
    f32 fall = MR::calcGravitySpeedAbs(this);
    if (MR::isBindedGround(this)) {
        fall = 0.0f;
    } else {
        fall -= 1.0f;
    }
    TVec3f dirVec(mTargetPos);
    dirVec -= mPosition;
    MR::vecKillElement(dirVec, mGravity, &dirVec);
    MR::normalizeOrZero(&dirVec);
    f32 speed = mMoveRange / 1.3f;
    MR::setVelocity(this, dirVec * speed - mGravity * fall);
    if (MR::isGreaterStep(this, mMoveStep)) {
        mIsRight = !mIsRight;
        MR::zeroVelocity(this);
        if (isTakoboPlayerInAttackRange()) {
            setNerve(&NrvTakobo::TakoboNrvAttackSign::sInstance);
            return;
        }
        setNerve(&NrvTakobo::TakoboNrvWaitAfter::sInstance);
        return;
    }
    SupportTicoStop::turnDirectionToNearbyPlayer(&mFrontVec, this);
}

void Takobo::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "AttackSign");
        MR::startBrk(this, "BlueToRed");
    }
    if (MR::isLessStep(this, 25)) {
        SupportTicoStop::turnDirectionToNearbyPlayer(&mFrontVec, this);
    }
    updateFallVelocity();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakobo::TakoboNrvAttack::sInstance);
    }
}

// NONMATCHING: stack slot order of the vector temporaries only.
void Takobo::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
    }
    if (MR::isStep(this, gTakoboInkSpurtStep)) {
        TVec3f pos(mPosition);
        TVec3f up;
        MR::calcUpVec(&up, this);
        TVec3f a = mFrontVec * 150.0f;
        pos += a;
        TVec3f b = up * 120.0f;
        pos += b;
        TVec3f dir = mFrontVec * 12.0f + up * 2.0f;
        mInkHolder->spurtTakoboInk(pos, dir);
    }
    updateFallVelocity();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakobo::TakoboNrvRecover::sInstance);
    }
}

void Takobo::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBrk(this, "RedToBlue");
    }
    updateFallVelocity();
    if (MR::isGreaterStep(this, 60)) {
        if (isMoving()) {
            setNerve(&NrvTakobo::TakoboNrvMove::sInstance);
            return;
        }
        setNerve(&NrvTakobo::TakoboNrvWait::sInstance);
    }
}

void Takobo::exeHitPunch() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HitPunch");
        MR::clearHitSensors(this);
        MR::invalidateHitSensors(this);
        MR::onBind(this);
    }
    if (mSpinHitController->execute(this)) {
        MR::emitItemHit(this);
        kill();
    }
}

void Takobo::exePress() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "Press");
        MR::clearHitSensors(this);
        MR::invalidateHitSensors(this);
    }
    if (MR::isActionEnd(this)) {
        MR::emitItemStomp(this);
        kill();
    }
}

void Takobo::exeHitReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HitReaction");
        MR::zeroVelocity(this);
    }
    updateFallVelocity();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakobo::TakoboNrvWaitAfter::sInstance);
    }
}

void Takobo::exeStun() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StunStart");
        MR::zeroVelocity(this);
    }
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Stun");
    }
    updateFallVelocity();
    MR::startActionSound(this, "EmLvSwoonS", -1, -1, -1);
    if (MR::isGreaterStep(this, 120)) {
        setNerve(&NrvTakobo::TakoboNrvStunEnd::sInstance);
    }
}

void Takobo::exeStunEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StunEnd");
    }
    updateFallVelocity();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakobo::TakoboNrvWait::sInstance);
    }
}

bool Takobo::tryStartSpinHit(u32 msg, HitSensor* pSensor2, HitSensor* pSensor3) {
    if (!isDamaging()) {
        startBrkDamage();
        MR::zeroVelocity(this);
        MR::startBlowHitReaction(this);
        setNerve(&NrvTakobo::TakoboNrvSpinHit::sInstance);
        mSpinHitController->startWithoutStopScene(this, *MR::getSensorPos(pSensor2), *MR::getSensorPos(pSensor3));
        ScoreAttackFunction::scoreByAttack(this, msg, 0);
        return true;
    }
    return false;
}

bool Takobo::tryStartPress(u32 msg) {
    if (!isDamaging()) {
        startBrkDamage();
        MR::zeroVelocity(this);
        MR::tryHitReaction(this, "\x93\xA5\x82\xDD");
        setNerve(&NrvTakobo::TakoboNrvPress::sInstance);
        ScoreAttackFunction::scoreByAttack(this, msg, 0);
        return true;
    }
    return false;
}

bool Takobo::tryStartStun() {
    if (isEnableStun()) {
        startBrkDamage();
        setNerve(&NrvTakobo::TakoboNrvStun::sInstance);
        return true;
    }
    return false;
}

bool Takobo::tryStartSupportFreeze() {
    if (isEnableSupportFreeze() && mBindStarPointer->tryStartPointBind()) {
        startBrkDamage();
        setNerve(&NrvTakobo::TakoboNrvSupportFreeze::sInstance);
        return true;
    }
    return false;
}

bool Takobo::isYoshiEatEnable() const {
    if (!isNerve(&NrvTakobo::TakoboNrvPress::sInstance) && !isNerve(&NrvTakobo::TakoboNrvTongue::sInstance) &&
        !isNerve(&NrvTakobo::TakoboNrvSpinHit::sInstance)) {
        return true;
    }
    return false;
}

bool Takobo::isEnableStun() const {
    if (!isNerve(&NrvTakobo::TakoboNrvPress::sInstance) && !isNerve(&NrvTakobo::TakoboNrvTongue::sInstance) &&
        !isNerve(&NrvTakobo::TakoboNrvSpinHit::sInstance)) {
        return true;
    }
    return false;
}

bool Takobo::isEnableSupportFreeze() const {
    if (!isNerve(&NrvTakobo::TakoboNrvPress::sInstance) && !isNerve(&NrvTakobo::TakoboNrvTongue::sInstance) &&
        !isNerve(&NrvTakobo::TakoboNrvSpinHit::sInstance) &&
        (!isNerve(&NrvTakobo::TakoboNrvStun::sInstance) || !MR::isLessStep(this, 60)) &&
        !isNerve(&NrvTakobo::TakoboNrvSupportFreeze::sInstance)) {
        return true;
    }
    return false;
}

bool Takobo::isDamaging() const {
    if (isNerve(&NrvTakobo::TakoboNrvPress::sInstance) || isNerve(&NrvTakobo::TakoboNrvSpinHit::sInstance) ||
        isNerve(&NrvTakobo::TakoboNrvTongue::sInstance)) {
        return true;
    }
    return false;
}

void Takobo::startBrkDamage() {
    if (isNerve(&NrvTakobo::TakoboNrvAttackSign::sInstance) || isNerve(&NrvTakobo::TakoboNrvAttack::sInstance) ||
        isNerve(&NrvTakobo::TakoboNrvRecover::sInstance)) {
        MR::startBrk(this, "RedToBlueShort");
    }
}

bool Takobo::isMoving() const {
    return mSpeed > 0.0f;
}

bool Takobo::isTakoboPlayerInAttackRange() const {
    TVec3f pos = mPosition - mGravity * 75.0f;
    return (*MR::getPlayerCenterPos() - pos).length() < 700.0f;
}

void Takobo::updateFallVelocity() {
    if (isMoving()) {
        if (MR::isBindedGround(this)) {
            MR::zeroVelocity(this);
            return;
        }
        MR::addVelocityToGravity(this, 1.0f);
    }
}

void Takobo::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isDamaging()) {
        return;
    }
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) {
            return;
        }
        if (!MR::isEqualSensor(pSender, this, "Body")) {
            if (isNerve(&NrvTakobo::TakoboNrvMove::sInstance) || isNerve(&NrvTakobo::TakoboNrvWaitAfter::sInstance) ||
                isNerve(&NrvTakobo::TakoboNrvWait::sInstance) || isNerve(&NrvTakobo::TakoboNrvAttackSign::sInstance) ||
                isNerve(&NrvTakobo::TakoboNrvAttack::sInstance) || isNerve(&NrvTakobo::TakoboNrvRecover::sInstance) ||
                isNerve(&NrvTakobo::TakoboNrvHitReaction::sInstance)) {
                if (MR::isEqualSensor(pSender, this, "Attack") && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    mScaleController->startHitReaction();
                    setNerve(&NrvTakobo::TakoboNrvHitReaction::sInstance);
                    return;
                }
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (isNerve(&NrvTakobo::TakoboNrvStunEnd::sInstance)) {
                setNerve(&NrvTakobo::TakoboNrvHitReaction::sInstance);
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (isNerve(&NrvTakobo::TakoboNrvSupportFreeze::sInstance)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else {
        if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver) || MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool Takobo::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg)) {
        if (isDamaging()) {
            return false;
        }
        if (isNerve(&NrvTakobo::TakoboNrvStun::sInstance)) {
            return tryStartSpinHit(msg, pSender, pReceiver);
        }
        return false;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isYoshiEatEnable()) {
            setNerve(&NrvTakobo::TakoboNrvTongue::sInstance);
            return true;
        }
        return false;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        makeActorDead();
        return true;
    }
    return false;
}

bool Takobo::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pReceiver)) {
        return false;
    }
    if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
        return tryStartPress(msg);
    }
    if (isDamaging()) {
        return false;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        tryStartStun();
        return true;
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        return tryStartSpinHit(msg, pSender, pReceiver);
    }
    return false;
}

bool Takobo::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (isDamaging()) {
        return false;
    }
    if (MR::isMsgToEnemyAttackTrample(msg)) {
        return tryStartPress(msg);
    }
    if (MR::isMsgToEnemyAttackBlow(msg) || MR::isMsgExplosionAttack(msg)) {
        return tryStartSpinHit(msg, pSender, pReceiver);
    }
    return false;
}

void Takobo::calcAndSetBaseMtx() {
    if (mConnector != nullptr) {
        if (!isDamaging() || isNerve(&NrvTakobo::TakoboNrvPress::sInstance)) {
            mConnector->connect();
            TVec3f up;
            MR::calcUpVec(&up, this);
            TPos3f mtx;
            MR::makeMtxUpFrontPos(&mtx, up, mFrontVec, mPosition);
            MR::setBaseTRMtx(this, mtx);
            MR::calcFrontVec(&mFrontVec, this);
            mGravity.set((-up));
        } else {
            TVec3f up;
            MR::calcUpVec(&up, this);
            TPos3f mtx;
            MR::makeMtxUpFrontPos(&mtx, up, mFrontVec, mPosition);
            MR::setBaseTRMtx(this, mtx);
            MR::calcFrontVec(&mFrontVec, this);
        }
    } else {
        TPos3f mtx;
        MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFrontVec);
        MR::setBaseTRMtx(this, mtx);
    }
    MR::updateBaseScale(this, mScaleController);
}

Takobo::~Takobo() {}

extern const volatile s32 gTakoboInkSpurtStep = 0;
