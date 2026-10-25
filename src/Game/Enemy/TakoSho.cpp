#include "Game/Enemy/TakoSho.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Enemy/TakoboInkHolder.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"

#include "Game/Util/YoshiLockOnUtil.hpp"

static const volatile s32 sShotStep = 0;

namespace NrvTakoSho {
    NEW_NERVE(TakoShoNrvWaitAfter, TakoSho, Wait);
    NEW_NERVE(TakoShoNrvWait, TakoSho, Wait);
    NEW_NERVE(TakoShoNrvMove, TakoSho, Move);
    NEW_NERVE(TakoShoNrvAttackSign, TakoSho, AttackSign);
    NEW_NERVE(TakoShoNrvAttack1st, TakoSho, Attack1st);
    NEW_NERVE(TakoShoNrvAttack2nd, TakoSho, Attack2nd);
    NEW_NERVE(TakoShoNrvRecover, TakoSho, Recover);
    NEW_NERVE(TakoShoNrvHitReaction, TakoSho, HitReaction);
    NEW_NERVE(TakoShoNrvSpinHit, TakoSho, SpinHit);
    NEW_NERVE(TakoShoNrvPress, TakoSho, Press);
    NEW_NERVE(TakoShoNrvStun, TakoSho, Stun);
    NEW_NERVE(TakoShoNrvStunEnd, TakoSho, StunEnd);

    // Nerve with an empty execute (Tongue; only set on Yoshi start-tongue).
    class TakoShoNrvTongue : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static TakoShoNrvTongue sInstance;
    };
    TakoShoNrvTongue TakoShoNrvTongue::sInstance ATTRIBUTE_WEAK;

    class TakoShoNrvSupportFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TakoSho* actor = reinterpret_cast<TakoSho*>(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &TakoShoNrvWait::sInstance);
        };
        static TakoShoNrvSupportFreeze sInstance;
    };
    TakoShoNrvSupportFreeze TakoShoNrvSupportFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvTakoSho

void TakoSho::turnFrontToPlayerNearby(TVec3f* pFront, LiveActor* pActor, f32 angle) {
    TVec3f direction(*MR::getPlayerPos());
    direction -= pActor->mPosition;
    if (PSVECMag(&direction) < 800.0f) {
        MR::vecKillElement(direction, pActor->mGravity, &direction);
        MR::normalizeOrZero(&direction);
        MR::turnVecToVecDegree(pFront, *pFront, direction, angle, pActor->mGravity);
    }
}

TakoSho::TakoSho(const char* pName)
    : LiveActor(pName), mFrontVec(0, 0, 1), mSideVec(1, 0, 0), mIsReverse(false), mIsRight(false), mMoveRange(5.0f),
      mSpeed(500.0f), mTargetPos(0.0f, 0.0f, 0.0f), mShotStep(0), mScaleController(nullptr), mBindStarPointer(nullptr),
      mSpinHitController(nullptr), mYoshiLockOn(nullptr), mConnector(nullptr) {}

TakoSho::~TakoSho() {}

// NONMATCHING: Argument sentinel register lifetime and constructor-result address selection differ.
void TakoSho::init(const JMapInfoIter& rIter) {
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
    }
    if (mConnector != nullptr) {
        MR::setSensorFollowMtx(this, "Body", getBaseMtx());
    }

    mInkHolder = new TakoboInkHolder(2);
    mScaleController = new AnimScaleController(nullptr);
    initNerve(&NrvTakoSho::TakoShoNrvWait::sInstance, 1);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, &NrvTakoSho::TakoShoNrvSupportFreeze::sInstance, "SupportFreeze");
    mSpinHitController = new SpinHitController(this, 0x16, 0, 1.5f, 20.0f, 35.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Center", TVec3f(0.0f));
    MR::useStageSwitchSyncAppear(this, rIter);
}

// NONMATCHING: Vector temporary stack slots differ.
void TakoSho::initAfterPlacement() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f start = mPosition + up * 500.0f;
    MR::getFirstPolyOnLineToMap(&mPosition, nullptr, start, -up * 500.0f);
    if (mConnector != nullptr) {
        mConnector->attachToUnder();
    }
}

void TakoSho::kill() {
    MR::tryHitReactionDead(this);
    LiveActor::kill();
}

void TakoSho::control() {
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

// NONMATCHING: Vector temporary stack slots differ.
void TakoSho::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, mScaleController);
    if (mConnector != nullptr) {
        if (!isDamaging() || isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance)) {
            mConnector->connect();
            TVec3f up;
            MR::calcUpVec(&up, this);
            TPos3f mtx;
            MR::makeMtxUpFrontPos(&mtx, up, mFrontVec, mPosition);
            MR::setBaseTRMtx(this, mtx);
            MR::calcFrontVec(&mFrontVec, this);
            mGravity.set((-up));
            return;
        }
        TVec3f up;
        MR::calcUpVec(&up, this);
        TPos3f mtx;
        MR::makeMtxUpFrontPos(&mtx, up, mFrontVec, mPosition);
        MR::setBaseTRMtx(this, mtx);
        MR::calcFrontVec(&mFrontVec, this);
        return;
    }
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, mFrontVec);
    MR::setBaseTRMtx(this, mtx);
}


void TakoSho::exeWait() {
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
    turnFrontToPlayerNearby(&mFrontVec, this, 2.865f);
    if (MR::calcDistanceToPlayer(this) < 750.0f) {
        setNerve(&NrvTakoSho::TakoShoNrvAttackSign::sInstance);
        return;
    }
    if (MR::isGreaterStep(this, 60)) {
        if (isMoving()) {
            setNerve(&NrvTakoSho::TakoShoNrvMove::sInstance);
            return;
        }
        setNerve(&NrvTakoSho::TakoShoNrvWaitAfter::sInstance);
    }
}

// NONMATCHING: TVec3f stack slots and finite integer-derived multiply operand order differ.
inline f32 calcTakoShotFrames(f32 frames, f32 factor) {
    return frames * factor;
}

void TakoSho::exeMove() {
    if (MR::isFirstStep(this)) {
        f32 sign = mIsRight ? 1.0f : -1.0f;
        mTargetPos = mStartPos - (mSideVec * mSpeed) * sign;
        mShotStep = static_cast<s32>(calcTakoShotFrames(static_cast<s32>((mTargetPos - mPosition).length() / mMoveRange), 1.3f));
    }
    f32 gravitySpeed = MR::calcGravitySpeedAbs(this);
    if (MR::isBindedGround(this)) {
        gravitySpeed = 0.0f;
    } else {
        gravitySpeed -= 7.0f;
    }
    TVec3f dir(mTargetPos);
    dir -= mPosition;
    MR::vecKillElement(dir, mGravity, &dir);
    MR::normalizeOrZero(&dir);
    f32 moveSpeed = mMoveRange / 1.3f;
    MR::setVelocity(this, dir * moveSpeed - mGravity * gravitySpeed);
    if (MR::isGreaterStep(this, mShotStep)) {
        mIsRight = !mIsRight;
        MR::zeroVelocity(this);
        if (MR::calcDistanceToPlayer(this) < 750.0f) {
            setNerve(&NrvTakoSho::TakoShoNrvAttackSign::sInstance);
        } else {
            setNerve(&NrvTakoSho::TakoShoNrvWaitAfter::sInstance);
        }
    } else {
        turnFrontToPlayerNearby(&mFrontVec, this, 2.865f);
    }
}

void TakoSho::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "AttackSign");
        MR::startBrk(this, "GreenToRed");
    }
    if (MR::isLessStep(this, 20)) {
        turnFrontToPlayerNearby(&mFrontVec, this, 2.865f);
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(&NrvTakoSho::TakoShoNrvAttack1st::sInstance);
    }
}

// NONMATCHING: Vector temporary stack slots differ.
void TakoSho::exeAttack1st() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack1st");
    }
    s32 shotStep = sShotStep;
    if (MR::isStep(this, shotStep)) {
        TVec3f pos(mPosition);
        TVec3f up;
        MR::calcUpVec(&up, this);
        pos += mFrontVec * 150.0f;
        pos += up * 120.0f;
        mInkHolder->spurtTakoboInk(pos, mFrontVec * 17.0f + up * 4.0f);
    }
    updateFallVelocity();
    if (MR::isGreaterStep(this, shotStep + 20)) {
        turnFrontToPlayerNearby(&mFrontVec, this, 4.8f);
    }
    if (MR::isGreaterStep(this, 35)) {
        setNerve(&NrvTakoSho::TakoShoNrvAttack2nd::sInstance);
    }
}

// NONMATCHING: Vector temporary stack slots differ.
void TakoSho::exeAttack2nd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack2nd");
    }
    if (MR::isStep(this, sShotStep)) {
        TVec3f pos(mPosition);
        TVec3f up;
        MR::calcUpVec(&up, this);
        pos += mFrontVec * 150.0f;
        pos += up * 120.0f;
        mInkHolder->spurtTakoboInk(pos, mFrontVec * 17.0f + up * 4.0f);
    }
    updateFallVelocity();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakoSho::TakoShoNrvRecover::sInstance);
    }
}

void TakoSho::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBrk(this, "RedToGreen");
    }
    updateFallVelocity();
    turnFrontToPlayerNearby(&mFrontVec, this, 2.865f);
    if (MR::isGreaterStep(this, 60)) {
        if (isMoving()) {
            setNerve(&NrvTakoSho::TakoShoNrvMove::sInstance);
            return;
        }
        setNerve(&NrvTakoSho::TakoShoNrvWait::sInstance);
    }
}

void TakoSho::exeSpinHit() {
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

void TakoSho::exePress() {
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

void TakoSho::exeHitReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HitReaction");
        MR::zeroVelocity(this);
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakoSho::TakoShoNrvWaitAfter::sInstance);
    }
}

void TakoSho::exeStun() {
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
        setNerve(&NrvTakoSho::TakoShoNrvStunEnd::sInstance);
    }
}

void TakoSho::exeStunEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StunEnd");
    }
    updateFallVelocity();
    if (MR::isActionEnd(this)) {
        setNerve(&NrvTakoSho::TakoShoNrvWaitAfter::sInstance);
    }
}

bool TakoSho::tryStartSpinHit(u32 msg, HitSensor* pSensor2, HitSensor* pSensor3) {
    if (!isDamaging()) {
        startBrkDamage();
        MR::zeroVelocity(this);
        MR::startBlowHitReaction(this);
        setNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance);
        const TVec3f& pos3 = *MR::getSensorPos(pSensor3);
        mSpinHitController->startWithoutStopScene(this, *MR::getSensorPos(pSensor2), pos3);
        ScoreAttackFunction::scoreByAttack(this, msg, 0);
        return true;
    }
    return false;
}

bool TakoSho::tryStartPress(u32 msg) {
    if (!isDamaging()) {
        startBrkDamage();
        MR::zeroVelocity(this);
        MR::tryHitReaction(this, "\x93\xA5\x82\xDD");
        setNerve(&NrvTakoSho::TakoShoNrvPress::sInstance);
        ScoreAttackFunction::scoreByAttack(this, msg, 0);
        return true;
    }
    return false;
}

bool TakoSho::tryStartStun() {
    if (isEnableStun()) {
        startBrkDamage();
        setNerve(&NrvTakoSho::TakoShoNrvStun::sInstance);
        return true;
    }
    return false;
}

bool TakoSho::tryStartSupportFreeze() {
    if (isEnableSupportFreeze() && mBindStarPointer->tryStartPointBind()) {
        startBrkDamage();
        setNerve(&NrvTakoSho::TakoShoNrvSupportFreeze::sInstance);
        return true;
    }
    return false;
}

bool TakoSho::isYoshiEatEnable() const {
    if (!isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) && !isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance)) {
        return true;
    }
    return false;
}

bool TakoSho::isEnableStun() const {
    if (!isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) && !isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance)) {
        return true;
    }
    return false;
}

bool TakoSho::isEnableSupportFreeze() const {
    if (!isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) && !isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance) &&
           (!isNerve(&NrvTakoSho::TakoShoNrvStun::sInstance) || !MR::isLessStep(this, 60)) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSupportFreeze::sInstance)) {
        return true;
    }
    return false;
}

bool TakoSho::isDamaging() const {
    if (isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance) ||
        isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance)) {
        return true;
    }
    return false;
}

void TakoSho::startBrkDamage() {
    if (isNerve(&NrvTakoSho::TakoShoNrvAttackSign::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvAttack1st::sInstance) ||
        isNerve(&NrvTakoSho::TakoShoNrvAttack2nd::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvRecover::sInstance)) {
        MR::startBrk(this, "RedToGreenShort");
    }
}

bool TakoSho::isMoving() const {
    return mSpeed > 0.0f;
}

void TakoSho::updateFallVelocity() {
    if (isMoving()) {
        if (MR::isBindedGround(this)) {
            MR::zeroVelocity(this);
            return;
        }
        MR::addVelocityToGravity(this, 7.0f);
    }
}

// NONMATCHING: Equivalent hip-drop-land early-return branch is folded.
void TakoSho::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isDamaging()) {
        return;
    }
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isPlayerHipDropFalling() || MR::isPlayerHipDropLand()) {
            return;
        }
        if (!MR::isEqualSensor(pSender, this, "Body")) {
            if (isNerve(&NrvTakoSho::TakoShoNrvMove::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvWaitAfter::sInstance) ||
                isNerve(&NrvTakoSho::TakoShoNrvWait::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvAttackSign::sInstance) ||
                isNerve(&NrvTakoSho::TakoShoNrvAttack1st::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvAttack2nd::sInstance) ||
                isNerve(&NrvTakoSho::TakoShoNrvRecover::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvHitReaction::sInstance)) {
                if (MR::isEqualSensor(pSender, this, "Attack") && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    mScaleController->startHitReaction();
                    setNerve(&NrvTakoSho::TakoShoNrvHitReaction::sInstance);
                    return;
                }
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (isNerve(&NrvTakoSho::TakoShoNrvStunEnd::sInstance)) {
                setNerve(&NrvTakoSho::TakoShoNrvHitReaction::sInstance);
                MR::sendMsgPush(pReceiver, pSender);
                return;
            }
            if (isNerve(&NrvTakoSho::TakoShoNrvSupportFreeze::sInstance)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else {
        if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver) || MR::isSensorYoshi(pReceiver)) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

bool TakoSho::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerKick(msg)) {
        if (isDamaging()) {
            return false;
        }
        if (isNerve(&NrvTakoSho::TakoShoNrvStun::sInstance)) {
            return tryStartSpinHit(msg, pSender, pReceiver);
        }
        return false;
    }
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isYoshiEatEnable()) {
            setNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance);
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

bool TakoSho::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
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

bool TakoSho::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
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
