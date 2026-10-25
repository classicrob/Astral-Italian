#include "Game/Util/Functor.hpp"
#include "Game/Enemy/Kameck.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/KameckBeam.hpp"
#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Player/YoshiFunction.hpp"

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};  // namespace MR


namespace {
    static const f32 sDefaultActiveDistance = 3000.0f;
    static const f32 sFrontVecBlendRate = 0.2f;
    static const f32 sUpVecBlendRate = 0.04f;
    static const f32 sTurnPlayerLimit = 0.98f;
    // static const s32 sWaitTime =
    // static const s32 mMoveHideTime =
    // static const f32 sMoveSpeed =
    static const s32 sGuardTime = 50;
    // static const f32 sGuardTurnSpeed =
    static const f32 sAttackRange = 2000.0f;
    static const s32 sAttackWaitTime = 60;
    // static const s32 sShootTiming =
    static const f32 sBeamSpeed = 12.0f;
    static const f32 sHitHorizontalPower = 25.0f;
    static const f32 sHitVerticalPower = 25.0f;
    static const f32 sDownGravityAccel = 1.0f;
    static const f32 sDownFreq = 0.99f;
    // static const s32 sPressTime =
};  // namespace

namespace NrvKameck {
    NEW_NERVE(KameckNrvOpeningDemo, Kameck, OpeningDemo);
    NEW_NERVE(KameckNrvDemoAppear, Kameck, DemoAppear);
    NEW_NERVE(KameckNrvNonActive, Kameck, NonActive);
    NEW_NERVE(KameckNrvAppear, Kameck, Appear);
    NEW_NERVE(KameckNrvWait, Kameck, Wait);
    NEW_NERVE(KameckNrvHide, Kameck, Hide);
    class KameckNrvWaitAfterShot : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            static_cast<Kameck*>(pSpine->mExecutor)->exeWaitAfterShot();
        }
        static KameckNrvWaitAfterShot sInstance;
    };
    KameckNrvWaitAfterShot KameckNrvWaitAfterShot::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KameckNrvAttackWait, Kameck, AttackWait);
    NEW_NERVE(KameckNrvAttack, Kameck, Attack);
    NEW_NERVE(KameckNrvGuard, Kameck, Guard);
    NEW_NERVE(KameckNrvHit, Kameck, Hit);
    NEW_NERVE(KameckNrvMoveHide, Kameck, MoveHide);
    NEW_NERVE(KameckNrvMove, Kameck, Move);
    NEW_NERVE(KameckNrvBindStarPointer, Kameck, BindStarPointer);
    NEW_NERVE(KameckNrvDown, Kameck, Down);
    NEW_NERVE(KameckNrvPressDown, Kameck, PressDown);
    class KameckNrvYoshiTongue : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KameckNrvYoshiTongue sInstance;
    };
    KameckNrvYoshiTongue KameckNrvYoshiTongue::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKameck

Kameck::Kameck(const char* pName)
    : LiveActor(pName), mBeam(), mActiveActorList(), mBeamEventListener(), mAnimScaleController(), mStateBindStarPointer(),
      mRotateQuat(0.0f, 0.0f, 0.0f, 1.0f), mFrontVec(0.0f, 0.0f, 1.0f), mMoveStep(240), mRailCoord(), mRailNextPointCoord(),
      mBeamType(1), mBeamCount(1), mShootTiming(30), mActiveDistance(::sDefaultActiveDistance), mYoshiLockOn() {
    mBeamEventListener = new SmallKameckBeamEventListener(this);
}

void Kameck::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Kameck", false);
    MR::makeQuatAndFrontFromRotate(&mRotateQuat, &mFrontVec, this);
    initJMapParam(rIter);
    initBeam();
    MR::initStarPointerTarget(this, 80.0f, TVec3f(0.0f, 0.0f, 0.0f));

    mAnimScaleController = new AnimScaleController(nullptr);
    mAnimScaleController->setParamTight();

    mYoshiLockOn = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));

    MR::addToAttributeGroupSearchTurtle(this);

    if (MR::isConnectedWithRail(rIter)) {
        MR::moveCoordToStartPos(this);
        MR::moveTransToCurrentRailPos(this);
    }

    initNerve(GET_NERVE(Kameck, KameckNrvWait), 1);

    mStateBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleController);
    MR::initActorState(this, mStateBindStarPointer, GET_NERVE(Kameck, KameckNrvBindStarPointer), "SupportFreeze");

    initDemo(rIter);
    setNonActive();
    MR::useStageSwitchSyncAppear(this, rIter);
}

void Kameck::initAfterPlacement() {
    MR::trySetMoveLimitCollision(this);
}

void Kameck::initBeam() {
    mActiveActorList = new KameckBeamHolder(mBeamType, 3, mBeamCount);
}

void Kameck::initJMapParam(const JMapInfoIter& rIter) {
    if (!MR::isValidInfo(rIter)) {
        return;
    }

    MR::getJMapInfoArg3NoInit(rIter, &mActiveDistance);
    mShootTiming = 30;
    MR::getJMapInfoArg1NoInit(rIter, &mShootTiming);

    mBeamType = KameckBeam::BeamType_FireBall1;
    if (MR::isObjectName(rIter, "KameckKuriboMini")) {
        mBeamType = KameckBeam::BeamType_KuriboMini;
    }
    if (MR::isObjectName(rIter, "KameckMeramera")) {
        mBeamType = KameckBeam::BeamType_Meramera;
    }

    MR::getJMapInfoArg0NoInit(rIter, &mBeamCount);
    s32 count;
    if (mBeamCount < 1) {
        count = 1;
    } else if (mBeamCount > 3) {
        count = 3;
    } else {
        count = mBeamCount;
    }
    mBeamCount = count;
}

void Kameck::initDemo(const JMapInfoIter& rIter) {
    if (!MR::tryRegisterDemoCast(this, rIter)) {
        return;
    }

    MR::registerDemoActionFunctor(this, MR::Functor(this, &Kameck::startDemoAppear), "ザコカメック登場");
    MR::registerDemoActionFunctor(this, MR::Functor(this, &Kameck::killForce), "ザコカメック強制死亡");
    makeActorDead();
}

void Kameck::makeActorDead() {
    LiveActor::makeActorDead();
    resetBeam();
}

void Kameck::appear() {
    MR::moveCoordAndTransToRailStartPoint(this);
    LiveActor::appear();
    setNerve(GET_NERVE(Kameck, KameckNrvAppear));
}

void Kameck::kill() {
    LiveActor::kill();
    MR::emitEffect(this, "Death");
}

void SmallKameckBeamEventListener::hitBeam() {
    Kameck* pHost = mHost;
    if (!MR::isDead(pHost)) {
        MR::startActionSound(pHost, "EvKameckAtkSuccess", -1, -1, -1);
    }
}

void Kameck::control() {
    mAnimScaleController->updateNerve();
    MR::blendQuatUpFront(&mRotateQuat, -mGravity, mFrontVec, ::sUpVecBlendRate, ::sFrontVecBlendRate);

    if (isEnableEat()) {
        YoshiFunction::tryYoshiEat(mYoshiLockOn);
    }
}

bool Kameck::isEnableEat() const {
    if (isNerve(GET_NERVE(Kameck, KameckNrvWait)) || isNerve(GET_NERVE(Kameck, KameckNrvAppear)) ||
        isNerve(GET_NERVE(Kameck, KameckNrvAttackWait)) || isNerve(GET_NERVE(Kameck, KameckNrvWaitAfterShot)) ||
        isNerve(GET_NERVE(Kameck, KameckNrvAttack)) || isNerve(GET_NERVE(Kameck, KameckNrvMoveHide)) ||
        isNerve(GET_NERVE(Kameck, KameckNrvBindStarPointer))) {
        return true;
    }

    return false;
}

void Kameck::startClipped() {
    LiveActor::startClipped();
    tryNonActive();
}

void Kameck::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mRotateQuat);
    MR::updateBaseScale(this, mAnimScaleController);
}

void Kameck::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && !MR::isPlayerHipDropFalling() && MR::isSensorEnemyAttack(pSender)) {
        requestAttack(pSender, pReceiver);
    }

    if (MR::isSensorYoshi(pReceiver)) {
        requestAttack(pSender, pReceiver);
    }
}

bool Kameck::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }

    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }

    if (MR::isMsgStarPieceReflect(msg)) {
        return requestGuard(pSender, pReceiver);
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        return isNerve(GET_NERVE(Kameck, KameckNrvBindStarPointer));
    }

    if (MR::isMsgJetTurtleAttack(msg)) {
        return requestDown(pSender, pReceiver);
    }

    if (MR::isMsgPlayerHitAll(msg)) {
        return requestDown(pSender, pReceiver);
    }

    if (MR::isMsgPlayerTrample(msg)) {
        return requestPressDown();
    }

    if (MR::isMsgPlayerHipDrop(msg)) {
        return requestPressDown();
    }

    return false;
}

bool Kameck::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        return requestDown(pSender, pReceiver);
    }

    return false;
}

namespace MR {
    bool isMsgYoshiStartTongue(u32 msg);  // not in shared headers yet (unit-local decl)
    bool isMsgYoshiLoopTongue(u32 msg);
    bool isMsgYoshiReleaseEaten(u32 msg);
    bool isMsgYoshiCancelEaten(u32 msg);
};  // namespace MR

bool Kameck::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStartPowerStarGet(msg)) {
        makeActorDead();
        return true;
    }

    if (MR::isMsgYoshiStartTongue(msg)) {
        setNerve(GET_NERVE(Kameck, KameckNrvYoshiTongue));
        return true;
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

bool Kameck::requestAttack(HitSensor* pSender, HitSensor* pReceiver) {
    if (isEnableAttack()) {
        if (MR::sendMsgEnemyAttack(pReceiver, pSender)) {
            if (isEnableAttack()) {
                resetBeam();
                setNerve(GET_NERVE(Kameck, KameckNrvHit));
                return true;
            }
        } else {
            MR::sendMsgPush(pReceiver, pSender);
        }
    } else {
        if (isNerve(GET_NERVE(Kameck, KameckNrvBindStarPointer))) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }

    return false;
}

bool Kameck::requestDown(HitSensor* pSender, HitSensor* pReceiver) {
    if (isEnableDown()) {
        resetBeam();
        MR::onBind(this);
        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);
        MR::setVelocityBlowAttack(this, pSender, pReceiver, ::sHitHorizontalPower, ::sHitVerticalPower, 4);
        setNerve(GET_NERVE(Kameck, KameckNrvDown));

        return true;
    }

    return false;
}

bool Kameck::requestGuard(HitSensor* pSender, HitSensor* pReceiver) {
    if (isEnableGurad()) {
        TVec3f vec;
        vec.killElement(-MR::getSensorHost(pSender)->mVelocity, mGravity);

        if (!MR::normalizeOrZero(&vec)) {
            mFrontVec.set(vec);
        }

        resetBeam();
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(Kameck, KameckNrvGuard));

        return true;
    }

    return false;
}

bool Kameck::requestPressDown() {
    if (isEnableDown()) {
        resetBeam();
        MR::onBind(this);
        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(Kameck, KameckNrvPressDown));
        return true;
    }

    return false;
}

void Kameck::startDemoAppear() {
    makeActorAppeared();
    MR::invalidateHitSensors(this);
    MR::invalidateClipping(this);
    setNerve(GET_NERVE(Kameck, KameckNrvDemoAppear));
}

void Kameck::killForce() {
    MR::emitEffect(this, "Death");
    resetBeam();
    mActiveActorList->killAll();
    makeActorDead();
}

void Kameck::makeActorDeadForce() {
    resetBeam();
    mActiveActorList->killAll();
    makeActorDead();
}

bool Kameck::tryOpeningDemo() {
    if (MR::isStageStateScenarioOpeningCamera()) {
        setNerve(GET_NERVE(Kameck, KameckNrvOpeningDemo));
        return true;
    }

    return false;
}

inline bool Kameck::tryOpeningDemoEnd() {
    if (!MR::isStageStateScenarioOpeningCamera()) {
        setNonActive();

        return true;
    }

    return false;
}

bool Kameck::tryNonActive() {
    if (isNerve(GET_NERVE(Kameck, KameckNrvWait)) && canNonActive()) {
        setNonActive();

        return true;
    }

    return false;
}

bool Kameck::tryAttackWait() {
    if (MR::isNearPlayer(this, ::sAttackRange)) {
        s32 railPoint = -1;
        if (MR::isExistRail(this)) {
            MR::getCurrentRailPointArg0WithInit(this, &railPoint);
        }

        if (railPoint == 0) {
            setNerve(GET_NERVE(Kameck, KameckNrvWaitAfterShot));
            return true;
        }

        mBeam = mActiveActorList->requestBeamFromWand(0.6f, MR::getJointMtx(this, "Wand"), TVec3f(0.0f, 110.0f, 0.0f), mBeamEventListener);
        if (mBeam != nullptr) {
            setNerve(GET_NERVE(Kameck, KameckNrvAttackWait));
            return true;
        }
    }

    return false;
}

void Kameck::setNonActive() {
    MR::hideModel(this);
    MR::invalidateHitSensors(this);
    MR::offCalcGravity(this);
    MR::offCalcShadow(this, nullptr);
    MR::validateClipping(this);
    MR::deleteEffectAll(this);
    MR::startAction(this, "NonActive");
    setNerve(GET_NERVE(Kameck, KameckNrvNonActive));
}

bool Kameck::tryActive() {
    if (!canNonActive()) {
        setNerve(GET_NERVE(Kameck, KameckNrvAppear));

        return true;
    }

    return false;
}

bool Kameck::tryAppearEnd() {
    if (MR::isActionEnd(this)) {
        if (!tryAttackWait()) {
            setNerve(GET_NERVE(Kameck, KameckNrvWait));
        }

        return true;
    }

    return false;
}

inline bool Kameck::tryAttack() {
    if (MR::isGreaterStep(this, ::sAttackWaitTime)) {
        setNerve(GET_NERVE(Kameck, KameckNrvAttack));
        return true;
    }

    return false;
}

bool Kameck::tryAttackEnd() {
    if (MR::isGreaterStep(this, 30)) {
        setNerve(GET_NERVE(Kameck, KameckNrvMoveHide));
        return true;
    }

    return false;
}

bool Kameck::tryPointBind() {
    if (mStateBindStarPointer->tryStartPointBind()) {
        resetBeam();
        setNerve(GET_NERVE(Kameck, KameckNrvBindStarPointer));

        return true;
    }

    return false;
}

void Kameck::exeOpeningDemo() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::showModel(this);
        MR::validateHitSensors(this);
        MR::validateClipping(this);
    }

    tryOpeningDemoEnd();
}

void Kameck::exeDemoAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::startActionSound(this, "EmKameckSmoke");
        MR::startActionSound(this, "EmKameckAppear");
    }

    MR::turnDirectionToTarget(this, &mFrontVec, *MR::getPlayerPos(), ::sTurnPlayerLimit);
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Wait");
    }
}

void Kameck::endDemoAppear() {
    if (isNerve(GET_NERVE(Kameck, KameckNrvDemoAppear))) {
        setNerve(GET_NERVE(Kameck, KameckNrvWait));
    }
}

void Kameck::exeNonActive() {
    if (tryOpeningDemo() || tryActive()) {
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
    }
}

void Kameck::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::startActionSound(this, "EmKameckSmoke");
        MR::startActionSound(this, "EmKameckAppear");
        MR::showModel(this);
        MR::validateHitSensors(this);
        MR::invalidateClipping(this);
        MR::onCalcShadow(this, nullptr);
        MR::onCalcGravity(this);
    }

    if (MR::isNearPlayer(this, ::sAttackRange)) {
        MR::turnDirectionToTarget(this, &mFrontVec, *MR::getPlayerPos(), ::sTurnPlayerLimit);
    }

    if (!tryPointBind() && tryAppearEnd()) {
        return;
    }
}

void Kameck::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::validateClipping(this);
        MR::validateHitSensors(this);
    }

    if (!tryPointBind() && !tryOpeningDemo() && tryAttackWait()) {
        return;
    }
}

void Kameck::exeHide() {
}

void Kameck::exeWaitAfterShot() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::invalidateClipping(this);
    }
    MR::turnDirectionToTarget(this, &mFrontVec, *MR::getPlayerPos(), ::sTurnPlayerLimit);
    if (!tryPointBind() && MR::isGreaterStep(this, mShootTiming + 60)) {
        setNerve(GET_NERVE(Kameck, KameckNrvMoveHide));
    }
}

void Kameck::exeAttackWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "AttackWait");
        MR::invalidateClipping(this);
    }

    MR::startActionSound(this, "EmLvKameckStaffTurn");
    MR::turnDirectionToTarget(this, &mFrontVec, *MR::getPlayerPos(), ::sTurnPlayerLimit);

    if (!tryPointBind()) {
        tryAttack();
    }
}

void Kameck::exeGuard() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Guard");
    }

    if (!tryPointBind() && MR::isGreaterStep(this, ::sGuardTime)) {
        setNerve(GET_NERVE(Kameck, KameckNrvMoveHide));
    }
}

void Kameck::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Attack");
        MR::startActionSound(this, "EmKameckStaffSwing", -1, -1, -1);
        MR::startActionSound(this, "EvKameckStaffSwing", -1, -1, -1);
    }

    if (MR::isStep(this, 13)) {
        mBeam->requestShootToPlayerCenter(::sBeamSpeed);
        mBeam = nullptr;
    }

    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(this, "Wait");
    }

    if (!tryPointBind() && MR::isGreaterStep(this, mShootTiming)) {
        setNerve(GET_NERVE(Kameck, KameckNrvMoveHide));
    }
}

void Kameck::exeHit() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hit");
    }

    if (!tryPointBind() && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Kameck, KameckNrvMoveHide));
    }
}

void Kameck::exeMoveHide() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Hide");
        MR::startActionSound(this, "EmKameckHide");
    }

    MR::turnDirectionToTarget(this, &mFrontVec, *MR::getPlayerPos(), ::sTurnPlayerLimit);

    if (!tryPointBind() && MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Kameck, KameckNrvMove));
        MR::startActionSound(this, "EmKameckSmoke");
    }
}

void Kameck::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Move");
        MR::hideModelAndOnCalcAnim(this);
        MR::invalidateHitSensors(this);
        MR::invalidateClipping(this);

        if (MR::isExistRail(this)) {
            if (MR::isRailReachedGoal(this)) {
                MR::reverseRailDirection(this);
            }

            mRailCoord = MR::getRailCoord(this);
            MR::calcDistanceToNextRailPoint(this, &mRailNextPointCoord);
            mMoveStep = static_cast<s32>(mRailNextPointCoord / 20.0f);
        }
    }

    MR::startActionSound(this, "EmLvKameckWarp", -1, -1, -1);

    if (MR::isExistRail(this)) {
        MR::setRailCoord(this, MR::calcNerveEaseInOutValue(this, mMoveStep, mRailCoord, mRailCoord + mRailNextPointCoord));
        MR::moveTransToCurrentRailPos(this);
    }

    MR::turnDirectionToTarget(this, &mFrontVec, *MR::getPlayerPos(), ::sTurnPlayerLimit);

    if (MR::isGreaterStep(this, mMoveStep)) {
        setNerve(GET_NERVE(Kameck, KameckNrvAppear));
    }
}

void Kameck::exeBindStarPointer() {
    MR::updateActorStateAndNextNerve(this, GET_NERVE(Kameck, KameckNrvWait));
}

void Kameck::endBindStarPointer() {
    mStateBindStarPointer->kill();
}

void Kameck::exeDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Down");
        MR::startBlowHitSound(this);
        MR::startActionSound(this, "EvKameckDeadBlow");
        MR::tryHitReactionBlow(this, &mPosition);
    }

    MR::addVelocityToGravity(this, ::sDownGravityAccel);
    MR::attenuateVelocity(this, ::sDownFreq);

    if (MR::isActionEnd(this) || MR::isBinded(this)) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitItemHit(this);
        kill();
    }
}

void Kameck::exePressDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Press");
        MR::startActionSound(this, "EmStompedS");
        MR::startActionSound(this, "EvKameckDeadStomped");
        MR::zeroVelocity(this);
    }

    if (MR::isActionEnd(this)) {
        MR::startActionSound(this, "EmExplodeS");
        MR::emitItemStomp(this);
        kill();
    }
}

void Kameck::resetBeam() {
    if (mBeam != nullptr) {
        mBeam->kill();
        mBeam = nullptr;
    }
}

void Kameck::setBeamType(s32 type) {
    mBeamType = type;
}

bool Kameck::canNonActive() const {
    if (MR::isValidSwitchA(this)) {
        return !MR::isOnSwitchA(this);
    }
    return !MR::isNearPlayerAnyTime(this, mActiveDistance);
}

bool Kameck::isEnableAttack() const {
    if (!(!isNerve(GET_NERVE(Kameck, KameckNrvWait)) && !isNerve(GET_NERVE(Kameck, KameckNrvAppear)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvAttackWait)) && !isNerve(GET_NERVE(Kameck, KameckNrvWaitAfterShot)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvAttack)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvMoveHide)))) {
        return true;
    }

    return false;
}

bool Kameck::isEnableGurad() const {
    if (!(!isNerve(GET_NERVE(Kameck, KameckNrvWait)) && !isNerve(GET_NERVE(Kameck, KameckNrvAppear)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvAttackWait)) && !isNerve(GET_NERVE(Kameck, KameckNrvWaitAfterShot)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvAttack)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvGuard)) && !isNerve(GET_NERVE(Kameck, KameckNrvMoveHide)))) {
        return true;
    }

    return false;
}

bool Kameck::isEnableDown() const {
    if (MR::isDead(this)) {
        return false;
    }

    if (!(!isNerve(GET_NERVE(Kameck, KameckNrvNonActive)) && !isNerve(GET_NERVE(Kameck, KameckNrvDown)) &&
          !isNerve(GET_NERVE(Kameck, KameckNrvPressDown)) && !isNerve(GET_NERVE(Kameck, KameckNrvYoshiTongue)))) {
        return false;
    }

    return true;
}

namespace MR {
    NameObj* createFireBallBeamKameck(const char* pName) {
        Kameck* pKameck;

        pKameck = new Kameck(pName);
        pKameck->setBeamType(KameckBeam::BeamType_FireBall1);

        return pKameck;
    }

    NameObj* createTurtleBeamKameck(const char* pName) {
        Kameck* pKameck;

        pKameck = new Kameck(pName);
        pKameck->setBeamType(KameckBeam::BeamType_Turtle);

        return pKameck;
    }
};  // namespace MR

