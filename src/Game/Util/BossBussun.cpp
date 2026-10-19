#include "Game/Util/BossBussun.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/MaterialCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Screen/ScreenAlphaCapture.hpp"
#include "Game/Util/ScreenUtil.hpp"

namespace {
    // Stage positions the boss walks between (SMG2 BossBussun map).
    static TVec3f sHomePos[5] = {
        TVec3f(-15736.0f, -5820.0f, -1472.0f),
        TVec3f(-16646.0f, -5820.0f, -3552.0f),
        TVec3f(-15486.0f, -5820.0f, -5657.0f),
        TVec3f(-13246.0f, -5820.0f, -4690.0f),
        TVec3f(-13219.0f, -5820.0f, -2267.0f),
    };
};  // namespace

namespace NrvBossBussun {
    class BossBussunNrvStart : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvStart sInstance;
    };
    class BossBussunNrvSleep : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvSleep sInstance;
    };
    class BossBussunNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvWait sInstance;
    };
    class BossBussunNrvTurn : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvTurn sInstance;
    };
    class BossBussunNrvMove : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvMove sInstance;
    };
    class BossBussunNrvChangeSide : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvChangeSide sInstance;
    };
    class BossBussunNrvFind : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvFind sInstance;
    };
    class BossBussunNrvShot : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvShot sInstance;
    };
    class BossBussunNrvDash : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvDash sInstance;
    };
    class BossBussunNrvDamage : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvDamage sInstance;
    };
    class BossBussunNrvRecover : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvRecover sInstance;
    };
    class BossBussunNrvIdle : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvIdle sInstance;
    };
    class BossBussunNrvDemoOpening : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvDemoOpening sInstance;
    };
    class BossBussunNrvDemoDown : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvDemoDown sInstance;
    };
    class BossBussunNrvFinish : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossBussunNrvFinish sInstance;
    };

    BossBussunNrvStart BossBussunNrvStart::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvSleep BossBussunNrvSleep::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvWait BossBussunNrvWait::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvTurn BossBussunNrvTurn::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvMove BossBussunNrvMove::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvChangeSide BossBussunNrvChangeSide::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvFind BossBussunNrvFind::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvShot BossBussunNrvShot::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvDash BossBussunNrvDash::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvDamage BossBussunNrvDamage::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvRecover BossBussunNrvRecover::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvIdle BossBussunNrvIdle::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvDemoOpening BossBussunNrvDemoOpening::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvDemoDown BossBussunNrvDemoDown::sInstance ATTRIBUTE_WEAK;
    BossBussunNrvFinish BossBussunNrvFinish::sInstance ATTRIBUTE_WEAK;

    inline void BossBussunNrvStart::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeStart();
    }
    inline void BossBussunNrvSleep::execute(Spine* pSpine) const {
        MR::updateActorState(reinterpret_cast< BossBussun* >(pSpine->mExecutor));
    }
    inline void BossBussunNrvWait::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeWait();
    }
    inline void BossBussunNrvTurn::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeTurn();
    }
    inline void BossBussunNrvMove::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeMove();
    }
    inline void BossBussunNrvChangeSide::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< BossBussun* >(pSpine->mExecutor), &BossBussunNrvWait::sInstance);
    }
    inline void BossBussunNrvFind::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeFind();
    }
    inline void BossBussunNrvShot::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< BossBussun* >(pSpine->mExecutor), &BossBussunNrvDash::sInstance);
    }
    inline void BossBussunNrvDash::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeDash();
    }
    inline void BossBussunNrvDamage::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeDamage();
    }
    inline void BossBussunNrvRecover::execute(Spine* pSpine) const {
        MR::updateActorStateAndNextNerve(reinterpret_cast< BossBussun* >(pSpine->mExecutor), &BossBussunNrvFind::sInstance);
    }
    inline void BossBussunNrvIdle::execute(Spine*) const {
    }
    inline void BossBussunNrvDemoOpening::execute(Spine* pSpine) const {
        BossBussun* pActor = reinterpret_cast< BossBussun* >(pSpine->mExecutor);
        MR::isFirstStep(pActor);
        if (MR::updateActorStateAndNextNerve(pActor, &BossBussunNrvFind::sInstance)) {
            MR::emitItemBossDamage(pActor);
        }
    }
    inline void BossBussunNrvDemoDown::execute(Spine* pSpine) const {
        reinterpret_cast< BossBussun* >(pSpine->mExecutor)->exeDemoDown();
    }
    inline void BossBussunNrvFinish::execute(Spine*) const {
    }
};  // namespace NrvBossBussun

BossBussun::BossBussun(const char* pName)
    : LiveActor(pName), mProjmapEffect(nullptr), mSequencer(nullptr), mCameraInfo(nullptr), mSleepState(nullptr), mShotState(nullptr),
      mDashState(nullptr), mAnimScale(nullptr), mBodyUp(0.0f, 1.0f, 0.0f), mGroundPos(0.0f, 0.0f, 0.0f), mTargetPos(0.0f, 0.0f, 0.0f), mFrontDir(0.0f, 0.0f, 1.0f),
      mHomePos(0.0f, 0.0f, 0.0f), mUpVec(0.0f, 1.0f, 0.0f), mCenterPos(0.0f, 0.0f, 0.0f), mStepCount(0), mBodyOffset(0.0f), mHitCount(4), mSeVersion(0), mSeVersionNext(0) {
    MR::createScreenAlphaSceneObj(1, 1.0f);
}

void BossBussun::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, true);
    MR::addEffectHitNormal(this, nullptr);
    MR::setEffectBaseScale(this, "HitMarkNormal", 2.5f);
    mGroundPos.set(mPosition);
    mProjmapEffect = MR::initDLMakerProjmapEffectMtxSetter(this);
    MR::newDifferedDLBuffer(this);
    mProjmapEffect->updateMtxUseBaseMtx();
    mHomePos.set(mPosition);
    MR::calcUpVec(&mUpVec, this);
    MR::calcUpVec(&mBodyUp, this);
    TPos3f zoneMtx;
    zoneMtx.setInline(MR::getZonePlacementMtx(rIter));
    zoneMtx.mult(TVec3f(0.0f), mCenterPos);
    MR::tryRegisterNamePosLinkObj(this, rIter, "ボスブッスン位置");
    MR::tryRegisterNamePosLinkObj(this, rIter, "マリオ位置");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "Damage");
    mSequencer = new BossBussunSequencer(this, rIter);
    MR::setShadowProjectionPtr(this, nullptr, &mPosition, &mBodyUp);
    initNerve(&NrvBossBussun::BossBussunNrvStart::sInstance, 0x10);
    mSleepState = new BossBussunStateSleep(this);
    MR::initActorState(this, mSleepState, &NrvBossBussun::BossBussunNrvSleep::sInstance, "Sleep");
    MR::initActorState(this, new BossBussunStateChangeSide(this), &NrvBossBussun::BossBussunNrvChangeSide::sInstance, "ChangeSide");
    mShotState = new BossBussunStateShot(this);
    MR::initActorState(this, mShotState, &NrvBossBussun::BossBussunNrvShot::sInstance, "Shot");
    mDashState = new BossBussunStateDash(this);
    MR::initActorState(this, mDashState, &NrvBossBussun::BossBussunNrvDash::sInstance, "Dash");
    MR::initActorState(this, new BossBussunStateDamage(this), &NrvBossBussun::BossBussunNrvDamage::sInstance, "Damage");
    MR::initActorState(this, new BossBussunStateRecover(this), &NrvBossBussun::BossBussunNrvRecover::sInstance, "Recover");
    MR::initActorState(this, new BossBussunStateDemo(this, "DemoOpening", false), &NrvBossBussun::BossBussunNrvDemoOpening::sInstance, "DemoOpening");
    MR::initActorState(this, new BossBussunStateDemo(this, "DemoDown", true), &NrvBossBussun::BossBussunNrvDemoDown::sInstance, "DemoDown");
    MR::invalidateClipping(this);
    MR::declarePowerStar(this);
    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::FunctorV0M< BossBussun*, void (BossBussun::*)() >(this, &BossBussun::startSleep));
    } else {
        startSleep();
    }
    mAnimScale = new AnimScaleController(nullptr);
    BossBussunFunction::setBinderExceptPlayer(this);
    makeActorAppeared();
}

void BossBussun::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void BossBussun::startSleep() {
    mBodyOffset = 0.0f;
    mSequencer->startCamera();
    setNerve(&NrvBossBussun::BossBussunNrvSleep::sInstance);
    MR::invalidateClipping(this);
}

f32 BossBussun::getDashDistanceLimit() const {
    return 1800.0f;
}

f32 BossBussun::getAttackDistanceLimit() const {
    return 2350.0f;
}

void BossBussun::calcNearestHomePos(TVec3f* pOut) {
    f32 minDist = 3.4028235e38f;
    s32 nearest = 0;
    for (s32 i = 0; i < 5; i++) {
        f32 dist = MR::calcDistanceHorizontal(this, ::sHomePos[i], mUpVec);
        if (dist < minDist) {
            minDist = dist;
            nearest = i;
        }
    }

    TVec3f diff = ::sHomePos[nearest] - mCenterPos;
    TVec3f dir;
    JMAVECScaleAdd(&mUpVec, &diff, &dir, -mUpVec.dot(diff));
    MR::normalizeOrZero(&dir);
    TVec3f offset = dir * 2350.0f;
    TVec3f result(mCenterPos);
    result.add(offset);
    pOut->set(result);
}

void BossBussun::kill() {
    LiveActor::kill();
    mSequencer->endCamera();
}

void BossBussun::control() {
    mAnimScale->updateNerve();
    MR::calcGravityOrZero(this);
    MR::calcUpVec(&mBodyUp, this);
    mFrontDir.orthogonalize(mGravity);
    MR::normalize(&mFrontDir);
    mSequencer->updateCamera();
    if (MR::isBindedGround(this)) {
        mGroundPos.set(MR::getGroundHitPos(this));
    }
    MR::setSeVersion(this, mSeVersion);
    mSeVersion = mSeVersionNext;
    mSeVersionNext = 0;
    if (isAngry()) {
        MR::tryEmitEffect(this, "Angry");
        MR::startActionSound(this, "Angry", -1, -1, -1);
    } else {
        MR::tryDeleteEffect(this, "Angry");
    }
}

void BossBussun::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontDir);
    TPos3f mtx;
    mtx.setInline(getBaseMtx());
    mtx.mMtx[0][3] = mGroundPos.x;
    mtx.mMtx[1][3] = mGroundPos.y;
    mtx.mMtx[2][3] = mGroundPos.z;
    MR::addTransMtxLocalY(mtx.toMtxPtr(), mBodyOffset);
    mtx.invert(mtx);
    mProjmapEffect->mEffectMtx.setInline(mtx);
    MR::updateBaseScale(this, mAnimScale);
}

void BossBussun::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body")) {
        if (MR::sendArbitraryMsg(0xD1, pReceiver, pSender)) {
            mSeVersionNext = 1;
        }
        if (!checkSensorPlayerInFront(pSender, pReceiver)) {
            return;
        }
    }

    if (!MR::isEqualSensor(pSender, this, "BodyUpper") || checkSensorPlayerInFront(pSender, pReceiver)) {
        if (!(mGravity.dot(*MR::getPlayerGravity()) < 0.0f) && MR::isSensorPlayer(pReceiver)) {
            if (isEnableAttack() && MR::sendMsgEnemyAttackStrong(pReceiver, pSender)) {
                if (isNerve(&NrvBossBussun::BossBussunNrvSleep::sInstance)) {
                    mSleepState->tryWake();
                }
            } else if (isSleeping()) {
                if (MR::sendMsgPush(pReceiver, pSender)) {
                }
            }
        }
    }
}

bool BossBussun::checkSensorPlayerInFront(HitSensor*, HitSensor* pSensor) {
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    f32 x = 0.0f;
    f32 y = 0.0f;
    MR::func_8003DEF0(&x, &y, &mPosition, MR::getSensorPos(pSensor), &up);
    return !(y < -50.0f);
}

bool BossBussun::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Body") || MR::isEqualSensor(pReceiver, this, "BodyUpper")) {
        if (!checkSensorPlayerInFront(pReceiver, pSender)) {
            return false;
        }
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        if (isEnableDamage() && mGravity.dot(*MR::getPlayerGravity()) < 0.0f && checkSensorPlayerInFront(pReceiver, pSender)) {
            mHitCount--;
            MR::tryHitReactionBlowBetweenSensors(this, pSender, pReceiver);
            setNerve(&NrvBossBussun::BossBussunNrvDamage::sInstance);
            return true;
        }
    }

    if (isNerve(&NrvBossBussun::BossBussunNrvSleep::sInstance)) {
        if (mSleepState->receiveOtherMsg(msg)) {
            return true;
        }
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        if (isNerve(&NrvBossBussun::BossBussunNrvDash::sInstance)) {
            if (mDashState->tryStartDash()) {
                mAnimScale->startHitReaction();
            }
        } else if (isAttackingNerve()) {
            mAnimScale->startHitReaction();
        }
        return true;
    }

    return false;
}

void BossBussun::updateVelocityOnGround() {
    if (!MR::isBindedGround(this)) {
        MR::addVelocityToGravity(this, 1.0f);
    }
    MR::attenuateVelocity(this, 0.85f);
}

bool BossBussun::isPlayerAttackable() {
    TVec3f up(0.0f);
    MR::calcUpVec(&up, this);
    TVec3f offset = up * 250.0f;
    TVec3f from(mPosition);
    from.add(offset);
    TVec3f dir = *MR::getPlayerCenterPos() - from;
    if (MR::isExistMapCollision(from, dir)) {
        return false;
    }
    if (MR::isExistMoveLimitCollision(from, dir)) {
        return false;
    }
    if (mHitCount > 2) {
        f32 x;
        f32 y;
        MR::func_8003DEF0(&x, &y, &mCenterPos, MR::getPlayerCenterPos(), &mUpVec);
        if (x > 2350.0f) {
            return false;
        }
    }
    if (MR::isFaceToPlayerHorizontalDegree(this, mFrontDir, 180.0f)) {
        return true;
    }
    return false;
}

bool BossBussun::tryStartAttack() {
    if (isPlayerAttackable()) {
        storePlayerPos();
        setNerve(&NrvBossBussun::BossBussunNrvFind::sInstance);
        mStepCount = 0;
        return true;
    }
    return false;
}

void BossBussun::storePlayerPos() {
    mTargetPos = *MR::getPlayerPos();
}

void BossBussun::resetBodyOffset() {
    mBodyOffset = 0.0f;
}

void BossBussun::resetAnimScale() {
    mAnimScale->resetScale();
}

bool BossBussun::isEnableAttack() const {
    if (isNerve(&NrvBossBussun::BossBussunNrvDamage::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvRecover::sInstance)) {
        return false;
    }
    return true;
}

bool BossBussun::isSleeping() const {
    return isNerve(&NrvBossBussun::BossBussunNrvSleep::sInstance);
}

bool BossBussun::isEnableDamage() const {
    if (isNerve(&NrvBossBussun::BossBussunNrvDamage::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvRecover::sInstance)) {
        return false;
    }
    return true;
}

bool BossBussun::isAngry() const {
    if (mHitCount == 1 &&
        (isNerve(&NrvBossBussun::BossBussunNrvWait::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvTurn::sInstance) ||
         isNerve(&NrvBossBussun::BossBussunNrvMove::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvChangeSide::sInstance) ||
         isNerve(&NrvBossBussun::BossBussunNrvFind::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvShot::sInstance) ||
         isNerve(&NrvBossBussun::BossBussunNrvDash::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvIdle::sInstance))) {
        return true;
    }
    return false;
}

bool BossBussun::isAttackingNerve() const {
    if (isNerve(&NrvBossBussun::BossBussunNrvWait::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvTurn::sInstance) ||
        isNerve(&NrvBossBussun::BossBussunNrvMove::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvChangeSide::sInstance) ||
        isNerve(&NrvBossBussun::BossBussunNrvFind::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvShot::sInstance) ||
        isNerve(&NrvBossBussun::BossBussunNrvDash::sInstance) || isNerve(&NrvBossBussun::BossBussunNrvIdle::sInstance)) {
        return true;
    }
    return false;
}

void BossBussun::resetToHomePos() {
    TPos3f mtx;
    mtx.identity();
    MR::tryFindLinkNamePos(this, "ボスブッスン位置", mtx.toMtxPtr());
    mFrontDir.set< f32 >(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);
    TVec3f up;
    up.set< f32 >(mtx.mMtx[0][1], mtx.mMtx[1][1], mtx.mMtx[2][1]);
    mGravity.set(-up);
    TVec3f pos;
    pos.set< f32 >(mtx.mMtx[0][3], mtx.mMtx[1][3], mtx.mMtx[2][3]);
    MR::resetPosition(this, pos);
}

void BossBussun::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Sleep");
        mBodyOffset = -1500.0f;
    }
}

void BossBussun::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "LookAround");
    }
    updateVelocityOnGround();
    if (MR::isGreaterStep(this, 20)) {
        if (mStepCount >= 0) {
            mStepCount = 0;
            setNerve(&NrvBossBussun::BossBussunNrvChangeSide::sInstance);
            return;
        }
        if (MR::isExistRail(this)) {
            MR::calcRandomRailPointPos(&mTargetPos, this);
        } else {
            MR::getRandomVector(&mTargetPos, 2000.0f);
            JMAVECScaleAdd(&mUpVec, &mTargetPos, &mTargetPos, -mUpVec.dot(mTargetPos));
            mTargetPos.add(mHomePos);
        }
        setNerve(&NrvBossBussun::BossBussunNrvTurn::sInstance);
        return;
    }
    if (tryStartAttack()) {
        return;
    }
}

void BossBussun::exeTurn() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Turn");
    }
    updateVelocityOnGround();
    if (MR::turnDirectionToTargetDegreeHorizon(this, &mFrontDir, mTargetPos, 2.0f)) {
        setNerve(&NrvBossBussun::BossBussunNrvMove::sInstance);
        return;
    }
    if (tryStartAttack()) {
        return;
    }
}

void BossBussun::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Walk");
    }
    MR::turnDirectionToTargetDegreeHorizon(this, &mFrontDir, mTargetPos, 2.0f);
    MR::addVelocityMoveToDirection(this, mFrontDir, 4.0f);
    updateVelocityOnGround();
    if (MR::calcDistanceHorizontal(this, mTargetPos, mUpVec) < 100.0f) {
        setNerve(&NrvBossBussun::BossBussunNrvWait::sInstance);
        mStepCount++;
        return;
    }
    if (tryStartAttack()) {
        return;
    }
}

void BossBussun::exeFind() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Find");
    }
    if (isPlayerAttackable()) {
        storePlayerPos();
    }
    bool turned = MR::turnDirectionToTargetDegreeHorizon(this, &mFrontDir, mTargetPos, 2.0f);
    updateVelocityOnGround();
    if (MR::isGreaterStep(this, 40) && turned) {
        if (mHitCount <= 2) {
            setNerve(&NrvBossBussun::BossBussunNrvShot::sInstance);
            return;
        }
        setNerve(&NrvBossBussun::BossBussunNrvDash::sInstance);
    }
}

void BossBussun::exeDash() {
    if (MR::updateActorState(this)) {
        if (isPlayerAttackable()) {
            setNerve(&NrvBossBussun::BossBussunNrvFind::sInstance);
            return;
        }
        setNerve(&NrvBossBussun::BossBussunNrvChangeSide::sInstance);
    }
}

void BossBussun::exeDamage() {
    if (MR::isFirstStep(this)) {
        if (mHitCount == 3) {
            MR::stopStageBgmForDemo();
        } else {
            MR::startActionSound(this, "PgDamageVo", -1, -1, -1);
        }
        if (static_cast< u32 >(mHitCount - 1) <= 1) {
            MR::emitItemBossDamage(this);
        }
        if (mHitCount == 0) {
            MR::stopStageBGM(10);
            MR::startActionSound(this, "PgDown", -1, -1, -1);
        } else {
            MR::startActionSound(this, "PgDamage", -1, -1, -1);
        }
    }
    if (MR::updateActorState(this)) {
        switch (mHitCount) {
        case 0:
            mShotState->killAll();
            setNerve(&NrvBossBussun::BossBussunNrvDemoDown::sInstance);
            return;
        case 1:
            mShotState->setShotCountMax(6);
            setNerve(&NrvBossBussun::BossBussunNrvRecover::sInstance);
            return;
        case 2:
            mShotState->setShotCountMax(3);
            setNerve(&NrvBossBussun::BossBussunNrvRecover::sInstance);
            return;
        case 3:
            setNerve(&NrvBossBussun::BossBussunNrvDemoOpening::sInstance);
            return;
        default:
            setNerve(&NrvBossBussun::BossBussunNrvRecover::sInstance);
            break;
        }
    }
}

void BossBussun::exeDemoDown() {
    if (MR::updateActorState(this)) {
        setNerve(&NrvBossBussun::BossBussunNrvFinish::sInstance);
        MR::startActionSound(this, "Explosion", -1, -1, -1);
        kill();
    }
}

BossBussun::~BossBussun() {
}
