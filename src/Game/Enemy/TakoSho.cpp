#include "Game/Enemy/TakoSho.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/MapObj/MapObjConnector.hpp"
#include "Game/Enemy/SpinHitController.hpp"
#include "Game/Enemy/TakoboInkHolder.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, const char*, const TVec3f&);
};  // namespace MR

namespace YoshiFunction {
    void tryYoshiEat(YoshiLockOnTarget*);
};  // namespace YoshiFunction

namespace MR {
    void makeMtxUpFrontPos(TPos3f*, const TVec3f&, const TVec3f&, const TVec3f&);
};  // namespace MR

// Unnamed SMG2 helpers outside this unit.
extern "C" void fn_80208970(TVec3f*, LiveActor*, f32);
extern "C" void fn_80034910();
extern "C" void fn_80034820(LiveActor*);
extern "C" void fn_800345E0(LiveActor*, const char*);
extern "C" void fn_80376C30(LiveActor*, u32, s32);
extern "C" void fn_8022BA10(LiveActor*);
extern "C" void fn_8022BAA0(LiveActor*);
extern "C" s32 lbl_807E3078;

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

TakoSho::TakoSho(const char* pName)
    : LiveActor(pName), mFrontVec(0, 0, 1), mSideVec(1, 0, 0), mIsReverse(false), mIsRight(false), mMoveRange(5.0f),
      mSpeed(500.0f), mTargetPos(0.0f, 0.0f, 0.0f), mShotStep(0), mScaleController(nullptr), mBindStarPointer(nullptr),
      mSpinHitController(nullptr), mYoshiLockOn(nullptr), mConnector(nullptr) {}

TakoSho::~TakoSho() {}

void TakoSho::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mStartPos.set(mPosition);
    MR::calcFrontVec(&mFrontVec, this);
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f negUp = -up;
    mGravity.set(negUp);

    s32 arg = -1;
    MR::getJMapInfoArg1NoInit(rIter, &arg);
    switch (arg) {
    case 0:
        MR::calcSideVec(&mSideVec, this);
        mIsReverse = false;
        mIsRight = false;
        break;
    case 1:
        MR::calcSideVec(&mSideVec, this);
        mIsReverse = false;
        mIsRight = true;
        break;
    case 3:
        MR::calcFrontVec(&mSideVec, this);
        mIsReverse = true;
        mIsRight = true;
        break;
    default:
        MR::calcFrontVec(&mSideVec, this);
        mIsReverse = true;
        mIsRight = false;
        break;
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

    mInkHolder = new TakoboInkHolder();
    mScaleController = new AnimScaleController(nullptr);
    initNerve(&NrvTakoSho::TakoShoNrvWait::sInstance, 1);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleController);
    MR::initActorState(this, mBindStarPointer, &NrvTakoSho::TakoShoNrvSupportFreeze::sInstance, "SupportFreeze");
    mSpinHitController = new SpinHitController(this, 0x16, 0, 1.5f, 20.0f, 35.0f);
    TVec3f zero(0.0f, 0.0f, 0.0f);
    mYoshiLockOn = MR::createYoshiLockOnTarget(this, "Center", zero);
    MR::useStageSwitchSyncAppear(this, rIter);
}

void TakoSho::initAfterPlacement() {
    TVec3f up;
    MR::calcUpVec(&up, this);
    TVec3f offset = up * 500.0f;
    TVec3f start(mPosition);
    start += offset;
    TVec3f negUp = -up;
    TVec3f end = negUp * 500.0f;
    MR::getFirstPolyOnLineToMap(&mPosition, nullptr, start, end);
    if (mConnector != nullptr) {
        mConnector->attachToUnder();
    }
}

void TakoSho::kill() {
    fn_80034910();
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
            TVec3f diff = mPosition - mGravity;
            mGravity.set(diff);
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

// NOTE: the exe/helper bodies below are first drafts from m2c; their fn_ names are listed in the report.

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
    fn_80208970(&mFrontVec, this, 2.865f);
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

void TakoSho::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::zeroVelocity(this);
        MR::startAction(this, "AttackSign");
        MR::startBrk(this, "GreenToRed");
    }
    if (MR::isLessStep(this, 20)) {
        fn_80208970(&mFrontVec, this, 2.865f);
    }
    if (MR::isGreaterStep(this, 60)) {
        setNerve(&NrvTakoSho::TakoShoNrvAttack1st::sInstance);
    }
}

void TakoSho::exeAttack1st() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack1st");
    }
    if (MR::isStep(this, lbl_807E3078)) {
        TVec3f pos(mPosition);
        TVec3f up;
        MR::calcUpVec(&up, this);
        TVec3f a = mFrontVec * 150.0f;
        pos += a;
        TVec3f b = up * 120.0f;
        pos += b;
        TVec3f c = up * 4.0f;
        TVec3f d = mFrontVec * 17.0f;
        TVec3f dir(d);
        dir += c;
        mInkHolder->spurtTakoboInk(pos, dir);
    }
    updateFallVelocity();
    if (MR::isGreaterStep(this, lbl_807E3078 + 20)) {
        fn_80208970(&mFrontVec, this, 4.8f);
    }
    if (MR::isGreaterStep(this, 35)) {
        setNerve(&NrvTakoSho::TakoShoNrvAttack2nd::sInstance);
    }
}

void TakoSho::exeAttack2nd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack2nd");
    }
    if (MR::isStep(this, lbl_807E3078)) {
        TVec3f pos(mPosition);
        TVec3f up;
        MR::calcUpVec(&up, this);
        TVec3f a = mFrontVec * 150.0f;
        pos += a;
        TVec3f b = up * 120.0f;
        pos += b;
        TVec3f c = up * 4.0f;
        TVec3f d = mFrontVec * 17.0f;
        TVec3f dir(d);
        dir += c;
        mInkHolder->spurtTakoboInk(pos, dir);
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
    fn_80208970(&mFrontVec, this, 2.865f);
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
        fn_8022BA10(this);
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
        fn_8022BAA0(this);
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
        fn_80034820(this);
        setNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance);
        TVec3f pos3(*MR::getSensorPos(pSensor3));
        mSpinHitController->startWithoutStopScene(this, *MR::getSensorPos(pSensor2), pos3);
        fn_80376C30(this, msg, 0);
        return true;
    }
    return false;
}

bool TakoSho::tryStartPress(u32 msg) {
    if (!isDamaging()) {
        startBrkDamage();
        MR::zeroVelocity(this);
        fn_800345E0(this, "\x93\xA5\x82\xDD");
        setNerve(&NrvTakoSho::TakoShoNrvPress::sInstance);
        fn_80376C30(this, msg, 0);
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
    return !isNerve(&NrvTakoSho::TakoShoNrvStun::sInstance) && !isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance);
}

bool TakoSho::isEnableStun() const {
    return !isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) && !isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance);
}

bool TakoSho::isEnableSupportFreeze() const {
    return !isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) && !isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance) &&
           (!isNerve(&NrvTakoSho::TakoShoNrvStun::sInstance) || !MR::isLessStep(this, 60)) &&
           !isNerve(&NrvTakoSho::TakoShoNrvSupportFreeze::sInstance);
}

bool TakoSho::isDamaging() const {
    return isNerve(&NrvTakoSho::TakoShoNrvPress::sInstance) || isNerve(&NrvTakoSho::TakoShoNrvSpinHit::sInstance) ||
           isNerve(&NrvTakoSho::TakoShoNrvTongue::sInstance);
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

void TakoSho::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isDamaging()) {
        return;
    }
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isPlayerHipDropFalling()) {
            return;
        }
        if (MR::isPlayerHipDropLand()) {
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
