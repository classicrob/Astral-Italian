#pragma auto_inline off
#include "Game/Boss/BossJugem.hpp"
#include "Game/Enemy/Togezo.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/DemoObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

// Action sequence tables of the BossJugem (indexed by mActionIdx). 0 Wait, 1 Move, 2/3 throw (one/two), 4 throw all,
// 5 dash, 6 end of sequence.
static const s32 sActionTableLast[] = {1, 2, 1, 2, 1, 2, 1, 2, 6};
static const s32 sActionTableFirst[] = {1, 1, 2, 1, 2, 1, 2, 1, 5, 6, 1, 0, 6, 6, 6};

static const s32* getActionTable(s32 phase) NO_INLINE;
static const s32* getActionTable(s32 phase) {
    if (phase >= 3) {
        return sActionTableLast;
    }
    return sActionTableFirst;
}

namespace NrvBossJugem {
    NEW_NERVE(BossJugemNrvStandBy, BossJugem, StandBy);
    NEW_NERVE(BossJugemNrvDemoOpening, BossJugem, DemoOpening);
    NEW_NERVE(BossJugemNrvWait, BossJugem, Wait);
    NEW_NERVE(BossJugemNrvWaitTogezo, BossJugem, WaitTogezo);
    NEW_NERVE(BossJugemNrvMove, BossJugem, Move);
    NEW_NERVE(BossJugemNrvThrowOne, BossJugem, ThrowOne);
    NEW_NERVE(BossJugemNrvThrowTwo, BossJugem, ThrowTwo);
    NEW_NERVE(BossJugemNrvThrowAll, BossJugem, ThrowAll);
    NEW_NERVE(BossJugemNrvDash, BossJugem, Dash);
    NEW_NERVE(BossJugemNrvDamage, BossJugem, Damage);
    NEW_NERVE(BossJugemNrvAngry, BossJugem, Angry);
    class BossJugemNrvSleep : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BossJugem* pActor = reinterpret_cast< BossJugem* >(pSpine->mExecutor);
            if (MR::updateActorState(pActor)) {
                pActor->selectNextAction();
            }
        }
        static BossJugemNrvSleep sInstance;
    };
    BossJugemNrvSleep BossJugemNrvSleep::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossJugemNrvMoveLeft, BossJugem, MoveLeft);
    NEW_NERVE(BossJugemNrvMoveRight, BossJugem, MoveRight);
    NEW_NERVE(BossJugemNrvStarPieceReaction, BossJugem, StarPieceReaction);
    NEW_NERVE(BossJugemNrvDemoDown, BossJugem, DemoDown);
    NEW_NERVE(BossJugemNrvDemoEnding, BossJugem, DemoEnding);
};  // namespace NrvBossJugem

// Local offsets of the cloud sway, of the wrist joint matrices and of the camera target.
static TVec3f sWristOffsetT(70.0f, -70.0f, 0.0f);
static TVec3f sWristOffsetR(0.0f, 0.0f, 0.0f);
static TVec3f sCameraOffset(0.0f, 0.0f, 2000.0f);
static TVec2f sCloudSwayAmp(50.0f, 60.0f);

BossJugem::BossJugem(const char* pName)
    : LiveActor(pName), mJointRoot(nullptr), mJointWristL(nullptr), mJointWristR(nullptr), mCloud(nullptr), mStateMove(nullptr),
      mStateThrowOne(nullptr), mStateThrowTwo(nullptr), mStateThrowAll(nullptr), mTogezoMgr(nullptr), mTogezoL(nullptr), mTogezoR(nullptr),
      mFrontVec(0.0f, 0.0f, 1.0f), mPhase(3), mActionIdx(0), mIsAngry(false), mIsDashNext(false), mHomePos(0.0f, 0.0f, 0.0f),
      mCameraTarget(0.0f, 0.0f, 0.0f), mCameraRegisterId(-1), mFarCount(0), mCameraInfo(nullptr), mCloudOffset(0.0f, 0.0f, 0.0f), mSwayStepA(0),
      mSwayStepB(0), mSwayRate(0.0f) {
    mHandMtxR.identity();
    mHandMtxL.identity();
}

void BossJugem::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BossJugem", false);
    MR::initFurEnemy(this);
    mCloud = new BossJugemCloud(this, "\211\137\203\202\203\146\203\213", getBaseMtx());
    mCloud->initWithoutIter();
    mCameraInfo = new ActorCameraInfo(rIter);
    MR::initAnimCamera(this, mCameraInfo, "DemoOpening");
    MR::initAnimCamera(this, mCameraInfo, "DemoEndingBossJugem");
    mTogezoMgr = new TogezoMgr(10, mPosition);
    mTogezoMgr->initWithoutIter();
    mPipo = MR::createDemoObj("\203\146\203\202\227\160\203\160\203\103\203\174", "TogezoPaipo", getBaseMtx());
    MR::invalidateClipping(mPipo);
    mPipo->makeActorDead();
    MR::getJMapInfoArg7WithInit(rIter, &mCameraRegisterId);
    if (mCameraRegisterId != -1) {
        MR::declareCameraRegisterVec(this, mCameraRegisterId, &mCameraTarget);
    }
    MR::setRailCoord(this, 0.5f * MR::getRailTotalLength(this));
    MR::moveTransToCurrentRailPos(this);
    mHomePos = mPosition;
    MR::calcFrontVec(&mFrontVec, this);
    mJointRoot = MR::createJointController(this, "AllRoot", &BossJugem::calcRootJoint);
    mJointWristR = MR::createJointController(this, "WristR", &BossJugem::calcWristRJoint);
    mJointWristL = MR::createJointController(this, "WristL", &BossJugem::calcWristLJoint);
    MR::declarePowerStar(this);
    initNerve(&NrvBossJugem::BossJugemNrvWait::sInstance, 6);
    mStateMove = new BossJugemStateMove(this);
    mStateThrowOne = new BossJugemStateThrowOne(this);
    mStateThrowTwo = new BossJugemStateThrowTwo(this);
    mStateThrowAll = new BossJugemStateThrowAll(this);
    mStateDash = new BossJugemStateDash(this);
    mStateSleep = new BossJugemStateSleep(this);
    MR::initActorState(this, mStateMove, &NrvBossJugem::BossJugemNrvMove::sInstance, "Move");
    MR::initActorState(this, mStateThrowOne, &NrvBossJugem::BossJugemNrvThrowOne::sInstance, "ThrowOne");
    MR::initActorState(this, mStateThrowTwo, &NrvBossJugem::BossJugemNrvThrowTwo::sInstance, "ThrowTwo");
    MR::initActorState(this, mStateThrowAll, &NrvBossJugem::BossJugemNrvThrowAll::sInstance, "ThrowAll");
    MR::initActorState(this, mStateDash, &NrvBossJugem::BossJugemNrvDash::sInstance, "Dash");
    MR::initActorState(this, mStateSleep, &NrvBossJugem::BossJugemNrvSleep::sInstance, "Sleep");
    MR::useStageSwitchSyncAppear(this, rIter);
    MR::invalidateClipping(this);
}

void BossJugem::makeActorAppeared() {
    setNerve(&NrvBossJugem::BossJugemNrvStandBy::sInstance);
    LiveActor::makeActorAppeared();
    mCloud->makeActorAppeared();
    resetBattle();
    MR::hideModel(this);
    MR::hideModel(mCloud);
    MR::validateClipping(this);
}

void BossJugem::makeActorDead() {
    LiveActor::makeActorDead();
    mCloud->makeActorDead();
}

void BossJugem::control() {
    updateCameraTarget();
    mTogezoMgr->updateAll();
    if (isAngrySmoke()) {
        MR::tryEmitEffect(this, "AngrySmoke");
    } else if (!isNerve(&NrvBossJugem::BossJugemNrvSleep::sInstance)) {
        MR::tryDeleteEffect(this, "AngrySmoke");
    }
    if (isAngrySmoke() || isNerve(&NrvBossJugem::BossJugemNrvSleep::sInstance)) {
        MR::startActionSound(this, "AngrySmoke", -1, -1, -1);
    }
    updateCloudSway();
    if (!isNerve(&NrvBossJugem::BossJugemNrvStandBy::sInstance) && !isNerve(&NrvBossJugem::BossJugemNrvDemoOpening::sInstance) &&
        !isNerve(&NrvBossJugem::BossJugemNrvDemoDown::sInstance) && !isNerve(&NrvBossJugem::BossJugemNrvDemoEnding::sInstance)) {
        mCloud->updateFloatSound();
    }
}

void BossJugem::startActionAll(const char* pName) {
    MR::startAction(this, pName);
    MR::startAction(mCloud, pName);
}

void BossJugem::killHoldTogezo() {
    if (mTogezoR != nullptr) {
        mTogezoR->makeActorDead();
        mTogezoR = nullptr;
    }
    if (mTogezoL != nullptr) {
        mTogezoL->makeActorDead();
        mTogezoL = nullptr;
    }
}

void BossJugem::appearHoldTogezoR() {
    if (mTogezoR == nullptr) {
        mTogezoR = mTogezoMgr->getDeadMember();
        if (mTogezoR != nullptr) {
            mTogezoR->appearHold(mHandMtxR.toMtxPtr());
        }
    }
}

void BossJugem::appearHoldTogezoL() {
    if (mTogezoL == nullptr) {
        mTogezoL = mTogezoMgr->getDeadMember();
        if (mTogezoL != nullptr) {
            mTogezoL->appearHold(mHandMtxL.toMtxPtr());
        }
    }
}

void BossJugem::releaseTogezoR(f32 angle, f32 speed) {
    if (mTogezoR != nullptr) {
        if (MR::isDead(mTogezoR)) {
            mTogezoR = nullptr;
            return;
        }
        TVec3f dir = *MR::getPlayerPos() - mTogezoR->mPosition;
        MR::vecKillElement(dir, mGravity, &dir);
        MR::normalize(&dir);
        throwTogezo(mTogezoR, dir, angle, speed);
        mTogezoR = nullptr;
    }
}

void BossJugem::releaseTogezoL(f32 angle, f32 speed) {
    if (mTogezoL != nullptr) {
        if (MR::isDead(mTogezoL)) {
            mTogezoL = nullptr;
            return;
        }
        TVec3f dir = *MR::getPlayerPos() - mTogezoL->mPosition;
        MR::vecKillElement(dir, mGravity, &dir);
        MR::normalize(&dir);
        throwTogezo(mTogezoL, dir, angle, speed);
        mTogezoL = nullptr;
    }
}

s32 BossJugem::getMaxTogezoNum() const {
    if (mPhase == 3) {
        return 2;
    }
    return mPhase == 2 ? 3 : 4;
}

s32 BossJugem::getRemainTogezoNum() const {
    s32 active = mTogezoMgr->getActiveNum();
    return getMaxTogezoNum() - active;
}

void BossJugem::updateMoveAnim(s32 step, s32 stepMax) {
    if (step == 0) {
        if (MR::isRailGoingToEnd(this)) {
            startActionAll("MoveSignRight");
        } else {
            startActionAll("MoveSignLeft");
        }
    }
    s32 endStep;
    if (MR::isRailGoingToEnd(this)) {
        endStep = stepMax - MR::getBckFrameMax(this, "MoveEndRight");
    } else {
        endStep = stepMax - MR::getBckFrameMax(this, "MoveEndLeft");
    }
    if (step == endStep) {
        if (MR::isRailGoingToEnd(this)) {
            startActionAll("MoveEndRight");
        } else {
            startActionAll("MoveEndLeft");
        }
    } else if (step < endStep && MR::isBckOneTimeAndStopped(this)) {
        if (MR::isRailGoingToEnd(this)) {
            startActionAll("MoveRight");
        } else {
            startActionAll("MoveLeft");
        }
    }
}

void BossJugem::updateMoveBackAnim(s32 step, s32 stepMax) {
    if (step == 0) {
        startActionAll("MoveBack");
    }
    if (step == stepMax - MR::getBckFrameMax(this, "MoveEndBack")) {
        startActionAll("MoveEndBack");
    }
}

void BossJugem::turnToPlayer(f32 rate) {
    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, this, &mGravity);
    MR::vecBlendSphere(&mFrontVec, mFrontVec, dir, rate);
}

void BossJugem::exeStandBy() {
    if (MR::isFirstStep(this)) {
        startActionAll("Sleep");
        Mtx mtx;
        MR::findNamePos("\203\173\203\130\203\127\203\205\203\121\203\200\203\146\203\202\210\312\222\165", mtx);
        MR::extractMtxZDir(mtx, &mFrontVec);
        MR::extractMtxTrans(mtx, &mPosition);
    }
    if (MR::isOnSwitchA(this)) {
        if (tryStartDemoCamera("DemoOpening")) {
            MR::showModel(this);
            MR::showModel(mCloud);
            setNerve(&NrvBossJugem::BossJugemNrvDemoOpening::sInstance);
        }
    }
}

void BossJugem::exeDemoOpening() {
    if (MR::isFirstStep(this)) {
        MR::hidePlayer();
        MR::invalidateClipping(this);
        MR::stopStageBGM(15);
        startActionAll("DemoOpening");
        MR::stopStageBgmForDemo();
    }
    if (tryEndDemoCamera("DemoOpening")) {
        MR::showPlayer();
        MR::startAction(this, "WaitNoInterpole");
        MR::startBrk(this, "Normal");
        MR::startAction(mCloud, "WaitNoInterpole");
        MR::startBrk(mCloud, "Normal");
        MR::calcVecFromPlayerH(&mFrontVec, this, nullptr);
        MR::moveTransToCurrentRailPos(this);
        selectNextAction();
    }
}

void BossJugem::exeWait() {
    if (MR::isFirstStep(this)) {
        startActionAll("WaitNormal");
        nextActionIndex();
    }
    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, this, &mGravity);
    MR::vecBlendSphere(&mFrontVec, mFrontVec, dir, 0.06f);
    if (MR::isGreaterEqualStep(this, 70)) {
        selectNextAction();
    }
}

void BossJugem::exeMove() {
    if (MR::isFirstStep(this)) {
        nextActionIndex();
    }
    if (MR::updateActorState(this)) {
        selectNextAction();
    }
    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, this, &mGravity);
    MR::vecBlendSphere(&mFrontVec, mFrontVec, dir, 0.06f);
}

void BossJugem::exeWaitTogezo() {
    if (MR::isFirstStep(this)) {
        startActionAll("WaitNormal");
        nextActionIndex();
    }
    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, this, &mGravity);
    MR::vecBlendSphere(&mFrontVec, mFrontVec, dir, 0.06f);
    s32 active = mTogezoMgr->getActiveNum();
    if (active < getMaxTogezoNum()) {
        selectThrowNerve();
    } else if (MR::isGreaterEqualStep(this, 70)) {
        selectNextAction();
    }
}

void BossJugem::exeThrowOne() {
    if (MR::isFirstStep(this)) {
        nextActionIndex();
    }
    if (MR::updateActorState(this)) {
        selectNextAction();
    }
}

void BossJugem::exeThrowTwo() {
    if (MR::isFirstStep(this)) {
        nextActionIndex();
    }
    if (MR::updateActorState(this)) {
        selectNextAction();
    }
}

void BossJugem::exeThrowAll() {
    if (MR::isFirstStep(this)) {
        nextActionIndex();
    }
    if (MR::updateActorState(this)) {
        selectNextAction();
    }
}

void BossJugem::exeDash() {
    if (MR::isFirstStep(this)) {
        nextActionIndex();
    }
    if (MR::updateActorState(this)) {
        selectNextAction();
    }
}

void BossJugem::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");
        MR::startAction(mCloud, "Damage");
    }
    TVec3f dir(0.0f);
    MR::calcVecToPlayerH(&dir, this, &mGravity);
    MR::vecBlendSphere(&mFrontVec, mFrontVec, dir, 0.2f);
    if (MR::isStep(this, 130)) {
        appearStarPieceToPlayer();
    }
    if (MR::isActionEnd(this)) {
        mActionIdx = 0;
        if (mPhase == 1) {
            setNerve(&NrvBossJugem::BossJugemNrvAngry::sInstance);
        } else {
            setNerve(&NrvBossJugem::BossJugemNrvDash::sInstance);
        }
    }
}

void BossJugem::exeAngry() {
    if (MR::isFirstStep(this)) {
        startActionAll("Angry");
    }
    if (MR::isActionEnd(this)) {
        mIsDashNext = true;
        mIsAngry = true;
        setNerve(&NrvBossJugem::BossJugemNrvThrowAll::sInstance);
    }
}

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void BossJugem::exeMoveLeft() {
    if (MR::isFirstStep(this)) {
        mRailCoordStart = MR::getRailCoord(this);
    }
    if (MR::isRailGoingToEnd(this)) {
        MR::reverseRailDirection(this);
    }
    f32 coord = mRailCoordStart + 1200.0f;
    if (!MR::isRailCoordNotBehind(this, coord)) {
        coord = mRailCoordStart - 1200.0f;
    }
    MR::setRailCoord(this, MR::calcNerveEaseOutValue(this, 30, mRailCoordStart, coord));
    MR::moveTransToCurrentRailPos(this);
    updateMoveAnim(getNerveStep(), 30);
    if (MR::isGreaterStep(this, 30)) {
        setNerve(&NrvBossJugem::BossJugemNrvWait::sInstance);
    }
}

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void BossJugem::exeMoveRight() {
    if (MR::isFirstStep(this)) {
        mRailCoordStart = MR::getRailCoord(this);
    }
    if (!MR::isRailGoingToEnd(this)) {
        MR::reverseRailDirection(this);
    }
    f32 coord = mRailCoordStart + 1200.0f;
    if (!MR::isRailCoordNotBehind(this, coord)) {
        coord = mRailCoordStart - 1200.0f;
    }
    MR::setRailCoord(this, MR::calcNerveEaseOutValue(this, 30, mRailCoordStart, coord));
    MR::moveTransToCurrentRailPos(this);
    updateMoveAnim(getNerveStep(), 30);
    if (MR::isGreaterStep(this, 30)) {
        setNerve(&NrvBossJugem::BossJugemNrvWait::sInstance);
    }
}

void BossJugem::exeStarPieceReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StarPieceReaction");
        MR::tryStartAllAnim(mCloud, "StarPieceReaction");
    }
    if (MR::isActionEnd(this)) {
        selectNextAction();
    }
}

void BossJugem::exeDemoDown() {
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(0);
    }
    if (tryStartDemoCamera("DemoEndingBossJugem")) {
        setNerve(&NrvBossJugem::BossJugemNrvDemoEnding::sInstance);
        mPipo->appear();
        MR::requestMovementOn(mPipo);
        killAllTogezo();
        MR::tryForceDeleteEffectAll(this);
        MR::tryForceDeleteEffectAll(mCloud);
    }
}

void BossJugem::exeDemoEnding() {
    if (MR::isFirstStep(this)) {
        MR::showModel(this);
        MR::showModel(mCloud);
        startActionAll("DemoEndingBossJugem");
        MR::startAction(mPipo, "DemoEndingBossJugem");
        Mtx mtx;
        MR::findNamePos("\203\173\203\130\203\127\203\205\203\121\203\200\203\137\203\105\203\223\203\146\203\202\210\312\222\165", mtx);
        MR::extractMtxZDir(mtx, &mFrontVec);
        MR::extractMtxTrans(mtx, &mPosition);
    }
    if (tryEndDemoCameraIfActionEnd("DemoEndingBossJugem")) {
        mPipo->kill();
        MR::requestAppearPowerStar(this, mPosition);
        kill();
    }
}

void BossJugem::calcAndSetBaseMtx() {
    MR::calcAndSetBaseTRMtxFromGravityAndZAxis(this, mFrontVec);
    MR::calcFrontVec(&mFrontVec, this);
    if (!MR::isDead(this)) {
        mJointRoot->registerCallBack();
        mJointWristL->registerCallBack();
        mJointWristR->registerCallBack();
    }
}

void BossJugem::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvBossJugem::BossJugemNrvStandBy::sInstance)) {
        if (isNerve(&NrvBossJugem::BossJugemNrvDash::sInstance)) {
            if (MR::isEqualSensor(pSender, this, "EscapeLeft") || MR::isEqualSensor(pSender, this, "EscapeRight")) {
                return;
            }
            if (MR::isSensorPlayer(pReceiver) || MR::isSensorEnemy(pReceiver)) {
                if (MR::sendMsgEnemyAttackElectric(pReceiver, pSender)) {
                    return;
                }
            }
        } else if (MR::isSensorPlayerOrRide(pReceiver)) {
            if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                return;
            }
        }
    }
}

bool BossJugem::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgJetTurtleAttack(msg) || MR::isMsgYoshiProjectile(msg)) {
        if (isNerve(&NrvBossJugem::BossJugemNrvDash::sInstance) && mStateDash->isInvincible()) {
            return true;
        }
        if (MR::isEqualSensor(pReceiver, this, "Body") || MR::isEqualSensor(pReceiver, this, "Bottom")) {
            tryDamage(pSender, pReceiver);
            return true;
        }
        if (MR::isEqualSensor(pReceiver, this, "EscapeLeft") || MR::isEqualSensor(pReceiver, this, "EscapeRight")) {
            return false;
        }
        return true;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgStarPieceAttack(msg)) {
        tryStarPieceReaction();
        return true;
    }
    return false;
}

bool BossJugem::isDamageable() const {
    bool result = false;
    if (!isNerve(&NrvBossJugem::BossJugemNrvAngry::sInstance) && !isNerve(&NrvBossJugem::BossJugemNrvDash::sInstance) &&
        !isNerve(&NrvBossJugem::BossJugemNrvDamage::sInstance) && !isNerve(&NrvBossJugem::BossJugemNrvStandBy::sInstance) &&
        !isNerve(&NrvBossJugem::BossJugemNrvDemoOpening::sInstance) && !isNerve(&NrvBossJugem::BossJugemNrvDemoDown::sInstance) &&
        !isNerve(&NrvBossJugem::BossJugemNrvDemoEnding::sInstance)) {
        result = true;
    }
    return result;
}

bool BossJugem::tryDamage(HitSensor* pSender, HitSensor* pReceiver) {
    if (isDamageable()) {
        killHoldTogezo();
        mActionIdx = 0;
        mPhase--;
        if (mPhase > 0) {
            setNerve(&NrvBossJugem::BossJugemNrvDamage::sInstance);
            MR::tryHitReactionBetweenSensors(this, "\203\137\203\201\201\133\203\127", pSender, pReceiver);
        } else {
            setNerve(&NrvBossJugem::BossJugemNrvDemoDown::sInstance);
            MR::tryHitReactionBetweenSensors(this, "\215\305\217\111\203\137\203\201\201\133\203\127", pSender, pReceiver);
        }
        breakAllTogezo();
        return true;
    }
    return false;
}

bool BossJugem::tryStarPieceReaction() {
    if (isDamageable() && !isNerve(&NrvBossJugem::BossJugemNrvThrowAll::sInstance)) {
        setNerve(&NrvBossJugem::BossJugemNrvStarPieceReaction::sInstance);
        releaseHoldTogezoAll();
        return true;
    }
    return false;
}

bool BossJugem::isAngrySmoke() const {
    if (mIsAngry && !isNerve(&NrvBossJugem::BossJugemNrvSleep::sInstance) && !isNerve(&NrvBossJugem::BossJugemNrvDemoDown::sInstance) &&
        !isNerve(&NrvBossJugem::BossJugemNrvDemoEnding::sInstance)) {
        return true;
    }
    return false;
}

void BossJugem::resetBattle() {
    mPhase = 3;
    mIsAngry = false;
    mIsDashNext = false;
    mActionIdx = 0;
    MR::invalidateHitSensor(this, "ThunderTop");
    MR::invalidateHitSensor(this, "ThunderMiddle");
    MR::invalidateHitSensor(this, "ThunderBottom");
    mPipo->makeActorDead();
}

void BossJugem::nextActionIndex() {
    s32 tableSize = 60;
    s32 phase = mPhase;
    s32 idx = mActionIdx + 1;
    mActionIdx = idx;
    if (phase >= 3) {
        tableSize = 36;
    }
    if (idx >= tableSize || getActionTable(phase)[mActionIdx] == 6) {
        mActionIdx = 0;
    }
}

void BossJugem::selectNextAction() {
    if (mIsDashNext) {
        setNerve(&NrvBossJugem::BossJugemNrvDash::sInstance);
        mIsDashNext = false;
        return;
    }
    if (isPlayerTooFar()) {
        setNerve(&NrvBossJugem::BossJugemNrvSleep::sInstance);
        return;
    }
    switch (getActionTable(mPhase)[mActionIdx]) {
    case 2:
        if (mPhase == 1) {
            mStateThrowOne->mIsDouble = true;
            mStateThrowTwo->mIsDouble = true;
        } else {
            mStateThrowOne->mIsDouble = false;
            mStateThrowTwo->mIsDouble = false;
        }
        selectThrowNerve();
        break;
    case 3:
        mStateThrowOne->mIsDouble = true;
        mStateThrowTwo->mIsDouble = true;
        selectThrowNerve();
        break;
    case 1:
        setNerve(&NrvBossJugem::BossJugemNrvMove::sInstance);
        break;
    case 4:
        if (getRemainTogezoNum() > 0) {
            setNerve(&NrvBossJugem::BossJugemNrvThrowAll::sInstance);
        } else {
            nextActionIndex();
            selectNextAction();
        }
        break;
    case 5:
        if (mPhase == 1) {
            if (getRemainTogezoNum() > 0) {
                mIsDashNext = true;
                setNerve(&NrvBossJugem::BossJugemNrvThrowAll::sInstance);
            } else {
                setNerve(&NrvBossJugem::BossJugemNrvDash::sInstance);
            }
        } else {
            setNerve(&NrvBossJugem::BossJugemNrvDash::sInstance);
        }
        break;
    case 0:
        setNerve(&NrvBossJugem::BossJugemNrvWait::sInstance);
        break;
    case 6:
        setNerve(&NrvBossJugem::BossJugemNrvMove::sInstance);
        break;
    default:
        setNerve(&NrvBossJugem::BossJugemNrvWait::sInstance);
        break;
    }
}

void BossJugem::selectThrowNerve() {
    if (getRemainTogezoNum() > 0) {
        s32 num = 1;
        if (mPhase == 2) {
            num = 2;
        }
        if (mPhase == 1) {
            num = 2;
        }
        s32 active = mTogezoMgr->getActiveNum();
        if (num + active >= getMaxTogezoNum()) {
            num = getRemainTogezoNum();
        }
        if (num >= 2) {
            num = 2;
        }
        if (num == 0) {
            MR::trySetNerve(this, &NrvBossJugem::BossJugemNrvWaitTogezo::sInstance);
        } else if (num == 1) {
            setNerve(&NrvBossJugem::BossJugemNrvThrowOne::sInstance);
        } else {
            setNerve(&NrvBossJugem::BossJugemNrvThrowTwo::sInstance);
        }
    } else if (mPhase == 1) {
        setNerve(&NrvBossJugem::BossJugemNrvMove::sInstance);
    } else {
        setNerve(&NrvBossJugem::BossJugemNrvWaitTogezo::sInstance);
    }
}

bool BossJugem::isPlayerTooFar() {
    if (MR::calcDistanceToPlayer(mHomePos) > 4100.0f) {
        return ++mFarCount > 4;
    }
    mFarCount = 0;
    return false;
}

void BossJugem::killAllTogezo() {
    s32 num = mTogezoMgr->getObjNum();
    for (s32 i = 0; i < num; i++) {
        Togezo* pTogezo = mTogezoMgr->getMember(i);
        if (!MR::isDead(pTogezo) && (!pTogezo->isYoshiState() || pTogezo->isShot())) {
            pTogezo->makeActorDead();
        }
    }
}

void BossJugem::breakAllTogezo() {
    s32 num = mTogezoMgr->getObjNum();
    for (s32 i = 0; i < num; i++) {
        Togezo* pTogezo = mTogezoMgr->getMember(i);
        if (!MR::isDead(pTogezo) && !pTogezo->isYoshiState()) {
            MR::tryHitReactionDead(pTogezo);
            pTogezo->kill();
        }
    }
}

void BossJugem::releaseHoldTogezoAll() {
    if (mTogezoL != nullptr) {
        mTogezoL->kill();
        mTogezoL = nullptr;
    }
    if (mTogezoR != nullptr) {
        mTogezoR->kill();
        mTogezoR = nullptr;
    }
}

void BossJugem::throwTogezo(Togezo* pTogezo, const TVec3f& rDir, f32 angle, f32 speed) {
    f32 up = speed * -MR::sinDegree(angle);
    f32 forward = speed * MR::cosDegree(angle);
    TVec3f vel;
    TVec3f gravity(mGravity);
    gravity *= up;
    vel.x = gravity.x;
    vel.y = gravity.y;
    vel.z = gravity.z;
    MR::vecScaleAdd(&vel, &rDir, forward);
    pTogezo->releaseHold(vel);
    pTogezo->setTimer(660);
}

void BossJugem::updateCameraTarget() {
    if (isNerve(&NrvBossJugem::BossJugemNrvDamage::sInstance)) {
        Mtx rootMtx;
        PSMTXCopy(MR::getJointMtx(this, "Root"), rootMtx);
        TVec3f rootPos(0.0f);
        MR::extractMtxTrans(rootMtx, &rootPos);
        MR::vecBlend(&mCameraTarget, mCameraTarget, rootPos, 0.05f);
        return;
    }
    Mtx basisMtx;
    MR::findNamePos("\203\112\203\201\203\211\212\356\217\200\223\137", basisMtx);
    MR::addTransMtxLocal(basisMtx, sCameraOffset);
    TVec3f xDir(0.0f);
    TVec3f basisPos(0.0f);
    TVec3f target(0.0f);
    MR::extractMtxXDir(basisMtx, &xDir);
    MR::extractMtxTrans(basisMtx, &basisPos);
    target = basisPos;
    f32 side = xDir.dot(basisPos - mPosition);
    target += xDir * -side;
    f32 rate = 0.5f - MR::normalize(xDir.dot(basisPos - *MR::getPlayerPos()), -1000.0f, 1000.0f);
    target += xDir * -2000.0f * 2.0f * rate;
    if (isNerve(&NrvBossJugem::BossJugemNrvWait::sInstance)) {
        mCameraTarget = target;
    } else {
        MR::vecBlend(&mCameraTarget, mCameraTarget, target, 0.05f);
    }
}

bool BossJugem::tryStartDemoCamera(const char* pName) {
    return MR::tryStartDemoAndAnimCamera(this, mCameraInfo, pName, 0);
}

bool BossJugem::tryEndDemoCamera(const char* pName) {
    if (MR::tryEndDemoAndAnimCamera(this, mCameraInfo, pName, 0)) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::tryForceDeleteEffectAll(this);
            MR::tryForceDeleteEffectAll(mCloud);
        }
        return true;
    }
    return false;
}

bool BossJugem::tryEndDemoCameraIfActionEnd(const char* pName) {
    if (MR::endDemoAndAnimCameraIfActionEnd(this, mCameraInfo, pName, 0)) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            MR::tryForceDeleteEffectAll(this);
            MR::tryForceDeleteEffectAll(mCloud);
        }
        return true;
    }
    return false;
}

void BossJugem::appearStarPieceToPlayer() {
    Mtx rootMtx;
    PSMTXCopy(MR::getJointMtx(this, "Root"), rootMtx);
    TVec3f pos(mPosition);
    MR::extractMtxTrans(rootMtx, &pos);
    TVec3f dir(mFrontVec);
    TVec3f xDir(0.0f);
    MR::extractMtxXDir(rootMtx, &xDir);
    f32 tilt = 35.0f;
    MR::rotateVecDegree(&dir, xDir, -tilt);
    MR::appearStarPieceToDirection(this, pos + dir * 200.0f, dir, 10, 14.0f, 180.0f, false);
    MR::startActionSound(this, "AppearStarPiece", -1, -1, -1);
}

void BossJugem::updateCloudSway() {
    s32 periodA = 360;
    s32 periodB = 180;
    if (++mSwayStepA >= periodA) {
        mSwayStepA = 0;
    }
    if (++mSwayStepB >= periodB) {
        mSwayStepB = 0;
    }
    if (isNerve(&NrvBossJugem::BossJugemNrvWait::sInstance) || isNerve(&NrvBossJugem::BossJugemNrvWaitTogezo::sInstance) ||
        isNerve(&NrvBossJugem::BossJugemNrvMove::sInstance) || isNerve(&NrvBossJugem::BossJugemNrvThrowOne::sInstance) ||
        isNerve(&NrvBossJugem::BossJugemNrvThrowTwo::sInstance) || isNerve(&NrvBossJugem::BossJugemNrvThrowAll::sInstance) ||
        isNerve(&NrvBossJugem::BossJugemNrvDash::sInstance) || isNerve(&NrvBossJugem::BossJugemNrvSleep::sInstance) ||
        isNerve(&NrvBossJugem::BossJugemNrvStarPieceReaction::sInstance)) {
        mSwayRate += 0.1f;
        if (mSwayRate > 1.0f) {
            mSwayRate = 1.0f;
        }
    } else {
        mSwayRate -= 0.1f;
        if (mSwayRate < 0.0f) {
            mSwayRate = 0.0f;
        }
    }
    mCloudOffset = TVec3f(sCloudSwayAmp.x * -MR::cosDegree(360.0f * mSwayStepA / periodA), sCloudSwayAmp.y * MR::sinDegree(360.0f * mSwayStepB / periodB), 0.0f);
    mCloudOffset *= mSwayRate;
}

bool BossJugem::calcRootJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    MR::addTransMtxLocal(pMtx->toMtxPtr(), mCloudOffset);
    return true;
}

bool BossJugem::calcWristRJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    Mtx mtx;
    PSMTXIdentity(mtx);
    MR::makeMtxTR(mtx, sWristOffsetT, sWristOffsetR);
    PSMTXConcat(pMtx->toMtxPtr(), mtx, mHandMtxR.toMtxPtr());
    return false;
}

bool BossJugem::calcWristLJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    Mtx mtx;
    PSMTXIdentity(mtx);
    MR::makeMtxTR(mtx, sWristOffsetT, sWristOffsetR);
    PSMTXConcat(pMtx->toMtxPtr(), mtx, mHandMtxL.toMtxPtr());
    return false;
}

BossJugemCloud::BossJugemCloud(BossJugem* pHost, const char* pName, MtxPtr pBaseMtx)
    : LiveActor(pName), mHost(pHost), mBaseMtx(pBaseMtx), mJointRoot(nullptr), mPrevPos(0.0f, 0.0f, 0.0f) {
}

void BossJugemCloud::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BossJugemCloud", false);
    mJointRoot = MR::createJointController(this, "JointRoot", &BossJugemCloud::calcRootJoint);
    MR::invalidateClipping(this);
    makeActorAppeared();
    mPrevPos = mPosition;
}

void BossJugemCloud::control() {
}

void BossJugemCloud::updateFloatSound() {
    if (!MR::isHiddenModel(this)) {
        TVec3f delta = mPosition - mPrevPos;
        MR::startActionSound(this, "Float", static_cast< s32 >(100.0f * PSVECMag(&delta)), -1, -1);
        mPrevPos = mPosition;
    }
}

void BossJugemCloud::calcAndSetBaseMtx() {
    mPosition.set< f32 >(mBaseMtx[0][3], mBaseMtx[1][3], mBaseMtx[2][3]);
    MR::setBaseTRMtx(this, mBaseMtx);
    mJointRoot->registerCallBack();
}

bool BossJugemCloud::calcRootJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    MR::addTransMtxLocal(pMtx->toMtxPtr(), mHost->mCloudOffset);
    return true;
}
