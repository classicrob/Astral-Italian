#pragma auto_inline off
#include "Game/Boss/TwoLegs.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/MapObj/DummyDisplayModel.hpp"


#define UPDATE_NERVE(name)                                                      \
    class name : public Nerve {                                                 \
    public:                                                                     \
        virtual void execute(Spine* pSpine) const {                             \
            reinterpret_cast< TwoLegs* >(pSpine->mExecutor)->updateStateAndNext(); \
        }                                                                       \
        static name sInstance;                                                  \
    };

namespace NrvTwoLegs {
    class TwoLegsNrvStart : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TwoLegs* pActor = reinterpret_cast< TwoLegs* >(pSpine->mExecutor);
            if (MR::isFirstStep(pActor)) {
                MR::startAction(pActor, "WaitFar");
            }
        }
        static TwoLegsNrvStart sInstance;
    };
    class TwoLegsNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TwoLegs* pActor = reinterpret_cast< TwoLegs* >(pSpine->mExecutor);
            if (MR::updateActorState(pActor)) {
                pActor->selectNextState();
            }
        }
        static TwoLegsNrvWait sInstance;
    };
    UPDATE_NERVE(TwoLegsNrvWalkNoDamage);
    UPDATE_NERVE(TwoLegsNrvWalkDamage);
    UPDATE_NERVE(TwoLegsNrvJump);
    class TwoLegsNrvShot : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TwoLegs* pActor = reinterpret_cast< TwoLegs* >(pSpine->mExecutor);
            if (pActor->updateStateAndNext()) {
                pActor->mFlagC0 = 1;
            }
        }
        static TwoLegsNrvShot sInstance;
    };
    UPDATE_NERVE(TwoLegsNrvDamage);
    UPDATE_NERVE(TwoLegsNrvChangePos);
    class TwoLegsNrvOpening : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TwoLegs* pActor = reinterpret_cast< TwoLegs* >(pSpine->mExecutor);
            if (pActor->updateStateAndNext()) {
                pActor->mCamera->start();
            }
        }
        static TwoLegsNrvOpening sInstance;
    };
    class TwoLegsNrvExplosion : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            TwoLegs* pActor = reinterpret_cast< TwoLegs* >(pSpine->mExecutor);
            if (MR::updateActorState(pActor)) {
                pActor->kill();
            }
        }
        static TwoLegsNrvExplosion sInstance;
    };
    TwoLegsNrvStart TwoLegsNrvStart::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvWait TwoLegsNrvWait::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvWalkNoDamage TwoLegsNrvWalkNoDamage::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvWalkDamage TwoLegsNrvWalkDamage::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvJump TwoLegsNrvJump::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvShot TwoLegsNrvShot::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvDamage TwoLegsNrvDamage::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvChangePos TwoLegsNrvChangePos::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvOpening TwoLegsNrvOpening::sInstance ATTRIBUTE_WEAK;
    TwoLegsNrvExplosion TwoLegsNrvExplosion::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvTwoLegs

TwoLegs::TwoLegs(const char* pName)
    : LiveActor(pName), mPhase(0), mInitPos(0.0f, 0.0f, 0.0f), mDummyModel(nullptr), mBodyDir(nullptr), mCameraInfo(nullptr), mCamera(nullptr),
      mJointCtrl(nullptr), mReflectCtrl(nullptr), mCoin(nullptr), mIsLeft(false), mIsUp(false), mHitFlag(0), mAlternate(0), mFlagC0(0),
      mStateShot(nullptr), mStateChangePos(nullptr), mStateDamage(nullptr), mStateJump(nullptr), mStateWalkNoDamage(nullptr), mStateWalkDamage(nullptr) {}

void TwoLegs::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mInitPos.set(mPosition);
    mBodyDir = new TwoLegsBodyDir();
    mReflectCtrl = new TwoLegsReflectCtrl(this);
    startBva();
    TVec3f front;
    MR::calcFrontVec(&front, this);
    mBodyDir->mFront.set(front);
    initNerve(&NrvTwoLegs::TwoLegsNrvStart::sInstance, 16);
    MR::initActorState(this, new TwoLegsStateWait(this), &NrvTwoLegs::TwoLegsNrvWait::sInstance, "Wait");
    mStateWalkNoDamage = new TwoLegsStateWalkNoDamage(this);
    MR::initActorState(this, mStateWalkNoDamage, &NrvTwoLegs::TwoLegsNrvWalkNoDamage::sInstance, "Walk");
    mStateWalkDamage = new TwoLegsStateWalkDamage(this);
    MR::initActorState(this, mStateWalkDamage, &NrvTwoLegs::TwoLegsNrvWalkDamage::sInstance, "WalkChance");
    mStateJump = new TwoLegsStateJump(this);
    MR::initActorState(this, mStateJump, &NrvTwoLegs::TwoLegsNrvJump::sInstance, "Jump");
    mStateShot = new TwoLegsStateShot(this, rIter);
    MR::initActorState(this, mStateShot, &NrvTwoLegs::TwoLegsNrvShot::sInstance, "Bullet");
    mStateDamage = new TwoLegsStateDamage(this);
    MR::initActorState(this, mStateDamage, &NrvTwoLegs::TwoLegsNrvDamage::sInstance, "Damage");
    mStateChangePos = new TwoLegsStateChangePos(this);
    MR::initActorState(this, mStateChangePos, &NrvTwoLegs::TwoLegsNrvChangePos::sInstance, "ChangeSide");
    MR::initActorState(this, new TwoLegsStateOpening(this), &NrvTwoLegs::TwoLegsNrvOpening::sInstance, "DemoOpening");
    MR::initActorState(this, new TwoLegsStateExplosion(this), &NrvTwoLegs::TwoLegsNrvExplosion::sInstance, "DemoExplosion");
    mDummyModel = MR::createDummyDisplayModel(this, 7, "Head", TVec3f(0.0f, -200.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    MR::registerDemoSimpleCastAll(mDummyModel);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, "Opening");
    MR::initAnimCamera(this, mCameraInfo, "Explosion");
    mCamera = new TwoLegsCamera(this);
    mJointCtrl = new TwoLegsJointCtrl(this);
    mCoin = new TwoLegsCoin(this);
    MR::declarePowerStar(this);
    makeActorAppeared();
    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::FunctorV0M< TwoLegs*, void (TwoLegs::*)() >(this, &TwoLegs::startFirstNerve));
        return;
    }
    startFirstNerve();
}

void TwoLegs::kill() {
    LiveActor::kill();
    mCamera->end(false);
}

void TwoLegs::startFirstNerve() {
    MR::endStartPosCamera();
    TVec3f offset;
    offset.scale(mBodyDir->getRadius(), mGravity);
    TVec3f pos;
    pos.add(mPosition, offset);
    mBodyDir->mPos.set(pos);
    setNerve(&NrvTwoLegs::TwoLegsNrvOpening::sInstance);
}

void TwoLegs::placeAtAllRoot() {
    calcAllRootPosOnPlane(&mPosition);
    MR::calcGravity(this);
}

void TwoLegs::placeOnBodyPlane(const TVec3f& rVec) {
    calcPosOnPlaneImpl(&mPosition, rVec);
    MR::calcGravity(this);
}

void TwoLegs::resetToInitPos() {
    MR::setPosition(this, mInitPos);
    MR::resetPosition(this);
}

void TwoLegs::hideAndKillDummy() {
    MR::hideModel(this);
    mDummyModel->kill();
    MR::forceDeleteEffectAll(mDummyModel);
}

bool TwoLegs::calcJointCallback(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    return mJointCtrl->calcJoint(pMtx, rInfo);
}

s32 TwoLegs::getPhaseNumInner() {
    return 3;
}

bool TwoLegs::tryAttackSensorState(HitSensor* pOther, HitSensor* pMine) {
    if (!MR::isSensorPlayerOrRide(pOther)) {
        return false;
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvWalkNoDamage::sInstance) && mStateWalkNoDamage->tryAttackStrong(pMine, pOther)) {
        mHitFlag = 1;
        return true;
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvWalkDamage::sInstance) && mStateWalkDamage->tryAttackStrong(pMine, pOther)) {
        mHitFlag = mStateWalkDamage->getDamageSide();
        return true;
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvJump::sInstance) && mStateJump->tryAttackStrong(pMine, pOther)) {
        mHitFlag = mStateJump->getAttackFlag();
        return true;
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvShot::sInstance) && mStateShot->tryAttackStrong(pMine, pOther)) {
        return true;
    }
    return false;
}

bool TwoLegs::tryAttackSensorStateEnemy(HitSensor* pOther, HitSensor* pMine) {
    if (!MR::isSensorEnemy(pOther)) {
        return false;
    }
    if (MR::isEqualSensor(pMine, this, "Body")) {
        return MR::sendMsgEnemyAttack(pOther, pMine);
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvWalkNoDamage::sInstance)) {
        return TwoLegsStateWalkNoDamage::tryStamp(mStateWalkNoDamage, this, pMine, pOther);
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvWalkDamage::sInstance)) {
        return mStateWalkDamage->tryAttack(pMine, pOther);
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvJump::sInstance)) {
        return mStateJump->tryAttack(pMine, pOther);
    }
    if (isNerve(&NrvTwoLegs::TwoLegsNrvShot::sInstance) && mStateShot->tryAttack(pMine, pOther)) {
        return true;
    }
    return false;
}

bool TwoLegs::tryPush(HitSensor* pOther, HitSensor* pMine) {
    if (!MR::isSensorPlayerOrRide(pOther)) {
        return false;
    }
    MR::sendMsgPush(pOther, pMine);
    return true;
}

bool TwoLegs::tryReflectEnemy(HitSensor* pOther, HitSensor* pMine) {
    if (!MR::isSensorEnemy(pOther)) {
        return false;
    }
    if (!MR::isEqualSensor(pMine, this, "Body") && !MR::isEqualSensor(pMine, this, "StampLeft") && !MR::isEqualSensor(pMine, this, "StampRight")) {
        return false;
    }
    MR::sendMsgReflect(pOther, pMine);
    return true;
}

bool TwoLegs::tryDrillDamage(HitSensor* pMine, HitSensor* pOther) {
    if (!MR::isEqualSensor(pMine, this, "Body")) {
        return false;
    }
    if (isPhase0()) {
        if (!isNerve(&NrvTwoLegs::TwoLegsNrvWalkNoDamage::sInstance) && !isNerve(&NrvTwoLegs::TwoLegsNrvWait::sInstance)) {
            return false;
        }
    } else if (!isNerve(&NrvTwoLegs::TwoLegsNrvWalkDamage::sInstance) || !mStateWalkDamage->isDamageable()) {
        return false;
    }
    damage();
    return true;
}

bool TwoLegs::tryReflectDrill(HitSensor* pOther, HitSensor* pMine, HitSensor*) {
    if (!MR::isEqualSensor(pMine, this, "Body") && !MR::isEqualSensor(pMine, this, "StampLeft") && !MR::isEqualSensor(pMine, this, "StampRight")) {
        return false;
    }
    if (!MR::sendMsgReflectV(pOther, pMine)) {
        return false;
    }
    MR::startActionSound(this, "Reflect", -1, -1, -1);
    if (!MR::isEqualSensor(pMine, this, "Body")) {
        mReflectCtrl->start(MR::isEqualSensor(pMine, this, "StampLeft"));
    }
    MR::tryRumblePadMiddle(this, 0);
    MR::stopSceneForDefaultHit(6);
    return true;
}

void TwoLegs::startBva() {
    const char* names[4] = {"Damage0", "Damage1", "Damage2", "Damage3"};
    u32 idx = mPhase;
    if (idx >= 4) {
        idx = 3;
    }
    MR::startBva(this, names[idx]);
}

void TwoLegs::damage() {
    mStateShot->killBullets();
    mPhase++;
    if (mPhase < 3) {
        MR::stopSceneForDefaultHit(10);
        setNerve(&NrvTwoLegs::TwoLegsNrvDamage::sInstance);
    } else {
        MR::stopSceneForDefaultHit(20);
        setNerve(&NrvTwoLegs::TwoLegsNrvExplosion::sInstance);
    }
    if (mPhase > 0) {
        MR::emitEffect(this, "Weaken1st");
    }
    if (mPhase > 1) {
        MR::emitEffect(this, "Weaken2nd");
    }
    startBva();
    mFlagC0 = 0;
}

bool TwoLegs::isInvincible() {
    bool ret = false;
    if (isNerve(&NrvTwoLegs::TwoLegsNrvStart::sInstance) ||
        (isNerve(&NrvTwoLegs::TwoLegsNrvDamage::sInstance) && !mStateDamage->isHide()) ||
        isNerve(&NrvTwoLegs::TwoLegsNrvOpening::sInstance) || isNerve(&NrvTwoLegs::TwoLegsNrvExplosion::sInstance) ||
        (isNerve(&NrvTwoLegs::TwoLegsNrvChangePos::sInstance) && mStateChangePos->isRunningAway())) {
        ret = true;
    }
    return ret;
}

void TwoLegs::selectNextState() {
    if (isPlayerBetweenFeet()) {
        mFlagC0 = 0;
        mHitFlag = 0;
        setNerve(&NrvTwoLegs::TwoLegsNrvChangePos::sInstance);
        return;
    }
    if (mFlagC0 == 0 && mStateShot->isAllBulletDead()) {
        setNerve(&NrvTwoLegs::TwoLegsNrvShot::sInstance);
        return;
    }
    mFlagC0 = 0;
    if (mHitFlag != 0) {
        toggleSide();
        mHitFlag = 0;
    } else if (MR::isHalfProbability()) {
        mIsLeft = true;
    } else {
        mIsLeft = false;
    }
    if (isPhase0()) {
        setNerve(&NrvTwoLegs::TwoLegsNrvWalkNoDamage::sInstance);
        return;
    }
    if (mAlternate != 0) {
        setNerve(&NrvTwoLegs::TwoLegsNrvWalkDamage::sInstance);
    } else {
        setNerve(&NrvTwoLegs::TwoLegsNrvJump::sInstance);
    }
    mAlternate = !mAlternate;
}

bool TwoLegs::updateStateAndNext() {
    if (MR::updateActorState(this)) {
        if (isPlayerBetweenFeet()) {
            setNerve(&NrvTwoLegs::TwoLegsNrvChangePos::sInstance);
        } else {
            setNerve(&NrvTwoLegs::TwoLegsNrvWait::sInstance);
        }
        return true;
    }
    return false;
}

void TwoLegs::updateCamera() {
    if (!isNerve(&NrvTwoLegs::TwoLegsNrvStart::sInstance) && !isNerve(&NrvTwoLegs::TwoLegsNrvOpening::sInstance) &&
        !isNerve(&NrvTwoLegs::TwoLegsNrvExplosion::sInstance)) {
        mCamera->keepAlive();
    }
}

void TwoLegs::control() {
    updateCamera();
    mReflectCtrl->update();
    mCamera->update();
    mJointCtrl->update();
}

void TwoLegs::calcAndSetBaseMtx() {
    mReflectCtrl->setCallBack();
    mJointCtrl->registerCallBack();
    TVec3f up;
    up.negate(mGravity);
    if (!isNerve(&NrvTwoLegs::TwoLegsNrvStart::sInstance)) {
        TVec3f dir;
        dir.sub(mPosition, mBodyDir->mPos);
        if (!MR::normalizeOrZero(&dir)) {
            up.set(dir);
        }
    }
    MR::calcAndSetBaseMtxFromFrontAndUp(this, &mBodyDir->mFront, &up);
}

void TwoLegs::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!tryAttackSensorStateEnemy(pReceiver, pSender)) {
        tryReflectEnemy(pReceiver, pSender);
    }
    if (!isInvincible() && !tryAttackSensorState(pReceiver, pSender)) {
        tryPush(pReceiver, pSender);
    }
}

bool TwoLegs::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }
    if (isInvincible()) {
        return false;
    }
    if (MR::isMsgDrillAttack(msg)) {
        if (tryDrillDamage(pReceiver, pSender)) {
            return true;
        }
        return tryReflectDrill(pSender, pReceiver, pSender);
    }
    return false;
}

bool TwoLegs::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgRushEnd(msg)) {
        return !isInvincible();
    }
    return false;
}

TwoLegs::~TwoLegs() {}
