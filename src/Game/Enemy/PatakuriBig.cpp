#include "Game/Enemy/Patakuri.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

namespace {
    // Nerve objects of Patakuri (TU00). Declared in the sinit order of the target.
#ifdef GALAXY_NATIVE
#define PATAKURI_NERVE(name, cls, fn)                                                                                                             \
    class name : public Nerve {                                                                                                                  \
    public:                                                                                                                                      \
        virtual void execute(Spine* pSpine) const {                                                                                              \
            cls* actor = reinterpret_cast< cls* >(pSpine->mExecutor);                                                                            \
            actor->fn();                                                                                                                         \
        };                                                                                                                                       \
        static name sInstance;                                                                                                                   \
    };                                                                                                                                           \
    name name::sInstance;
#else
#define PATAKURI_NERVE(name, cls, fn)                                                                                                             \
    class name : public Nerve {                                                                                                                  \
    public:                                                                                                                                      \
        virtual void execute(Spine* pSpine) const {                                                                                              \
            cls* actor = reinterpret_cast< cls* >(pSpine->mExecutor);                                                                            \
            actor->fn();                                                                                                                         \
        };                                                                                                                                       \
        static name sInstance;                                                                                                                   \
    };                                                                                                                                           \
    name name::sInstance ATTRIBUTE_WEAK;
#endif

    PATAKURI_NERVE(PatakuriNrvRail, Patakuri, exeRail);
    PATAKURI_NERVE(PatakuriNrvBlow, Patakuri, exePatakuriBlow);
    PATAKURI_NERVE(PatakuriNrvSwoonLand, Patakuri, exeLandSwoon);
    PATAKURI_NERVE(PatakuriNrvStaggerEnd, Patakuri, exePatakuriStaggerEnd);
    PATAKURI_NERVE(PatakuriNrvHipDropDown, Patakuri, exePatakuriHipDropDown);
    PATAKURI_NERVE(PatakuriNrvCoinPop, Patakuri, exeCoinPop);
    PATAKURI_NERVE(PatakuriNrvHitStep, Patakuri, exeHitStep);

    class PatakuriNrvBind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &PatakuriNrvRail::sInstance);
        };
        static PatakuriNrvBind sInstance;
    };
#ifdef GALAXY_NATIVE
    PatakuriNrvBind PatakuriNrvBind::sInstance;
#else
    PatakuriNrvBind PatakuriNrvBind::sInstance ATTRIBUTE_WEAK;
#endif

    PATAKURI_NERVE(PatakuriNrvDead, Patakuri, exePatakuriDead);

    class PatakuriWingNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {};
        static PatakuriWingNrvWait sInstance;
    };
#ifdef GALAXY_NATIVE
    PatakuriWingNrvWait PatakuriWingNrvWait::sInstance;
#else
    PatakuriWingNrvWait PatakuriWingNrvWait::sInstance ATTRIBUTE_WEAK;
#endif

    class PatakuriNrvKill : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            if (MR::isActionEnd(actor)) {
                actor->kill();
            }
        };
        static PatakuriNrvKill sInstance;
    };
#ifdef GALAXY_NATIVE
    PatakuriNrvKill PatakuriNrvKill::sInstance;
#else
    PatakuriNrvKill PatakuriNrvKill::sInstance ATTRIBUTE_WEAK;
#endif
}  // namespace

// ---------------------------------------------------------------------------------------------
// PatakuriWing
// ---------------------------------------------------------------------------------------------

PatakuriWing::PatakuriWing(LiveActor* pHost)
    : PartsModel(pHost, "\x89\x48", "PatakuriWing", nullptr, -1, false) {
    initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 90.0f, 0.0f), "Head");
    initWithoutIter();
    initNerve(&PatakuriWingNrvWait::sInstance, 0);
}

PatakuriWing::~PatakuriWing() {}

void PatakuriWing::startWingDamage() {
    MR::startAction(this, "Damage");
    mCalcOwnMatrix = false;
    mCheckHiddenHost = true;
    setNerve(&PatakuriNrvKill::sInstance);
}

// ---------------------------------------------------------------------------------------------
// Patakuri
// ---------------------------------------------------------------------------------------------

Patakuri::Patakuri(const char* pName) : LiveActor(pName) {
    mBody = nullptr;
    mFront.set(0.0f, 0.0f, 0.0f);
    mWing = nullptr;
    mScaleCtrl = nullptr;
    mBindStarPointer = nullptr;
    mLockOnTarget = nullptr;
}

Patakuri::~Patakuri() {}

void Patakuri::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Kuribo", "Patakuri", false);
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::makeQuatAndFrontFromRotate(&mQuat, &mFront, this);
    MR::onCalcGravity(this);

    mBody = new Kuribo("\x83\x4e\x83\x8a\x83\x7b\x81\x5b");
    MR::setPosition(mBody, mPosition);
    mBody->initWithoutIter();
    MR::setClippingTypeSphere(mBody, 1000.0f);
    mBody->makeActorDead();

    if (MR::isValidInfo(rIter)) {
        MR::setGroupClipping(this, rIter, 32);
        MR::setGroupClipping(mBody, rIter, 32);
    }

    mWing = new PatakuriWing(this);
    MR::startAction(mWing, "Wait");
    MR::startAction(this, "PatakuriWait");

    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));
    initNerve(&PatakuriNrvRail::sInstance, 1);

    mScaleCtrl = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, &PatakuriNrvBind::sInstance, "SupportFreeze");

    makeActorAppeared();
}

void Patakuri::control() {
    mScaleCtrl->updateNerve();
    MR::blendQuatFromGravityAndFront(&mQuat, this, &mFront, 0.05f, 0.2f);

    if (isNerve(&PatakuriNrvHitStep::sInstance) == false) {
        if (isNerve(&PatakuriNrvDead::sInstance) == false) {
            YoshiFunction::tryYoshiEat(mLockOnTarget);
        }
    }

    if (isNerve(&PatakuriNrvRail::sInstance) && mBindStarPointer->tryStartPointBind()) {
        setNerve(&PatakuriNrvBind::sInstance);
    }
}

void Patakuri::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
    MR::updateBaseScale(this, mScaleCtrl);
}

void Patakuri::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isSensorEnemyAttack(pSender)) {
            if (isNerve(&PatakuriNrvRail::sInstance)) {
                if (!MR::isPlayerHipDropFalling() && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                    MR::startAction(this, "PatakuriHit");
                    MR::startAction(mWing, "Hit");
                    return;
                }
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

bool Patakuri::isRailOrBind() const {
    if (isNerve(&PatakuriNrvRail::sInstance) || isNerve(&PatakuriNrvBind::sInstance)) {
        return true;
    }
    return false;
}

bool Patakuri::tryTrampleDefeat() {
    if (!isRailOrBind()) {
        return false;
    }

    mWing->startWingDamage();
    setNerve(&PatakuriNrvDead::sInstance);

    TVec3f pos = calcBodyDropPos(this);
    mBody->appearHipDropped(pos, mQuat);
    ScoreAttackFunction::scoreTrample(this);
    return true;
}

bool Patakuri::tryStartHipDropDown() {
    if (!isRailOrBind()) {
        return false;
    }

    MR::offBind(this);
    MR::zeroVelocity(this);
    MR::invalidateHitSensors(this);
    mWing->startWingDamage();
    setNerve(&PatakuriNrvHipDropDown::sInstance);
    return true;
}

TVec3f Patakuri::calcBodyDropPos(Patakuri* pPatakuri) {
    TVec3f bodyRoot;
    TVec3f bodyBody;
    TVec3f actorRoot;
    TVec3f actorBody;

    MR::extractMtxTrans(MR::getJointMtx(pPatakuri->mBody, "Root"), &bodyRoot);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri->mBody, "Body"), &bodyBody);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri, "Root"), &actorRoot);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri, "Body"), &actorBody);

    return (pPatakuri->mPosition - (bodyBody - bodyRoot)) + (actorBody - actorRoot);
}

bool Patakuri::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgPlayerTrample(msg)) {
        return tryTrampleDefeat();
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        return tryStartHipDropDown();
    }
    if (MR::isMsgPlayerHitAll(msg) || MR::isMsgStarPieceAttack(msg)) {
        mWing->startWingDamage();
        setNerve(&PatakuriNrvDead::sInstance);

        TVec3f pos = calcBodyDropPos(this);
        mBody->appearKuriboWandering(pos, mQuat, mVelocity);
        mBody->receiveMsgPlayerAttack(msg, pSender, pReceiver);
        MR::invalidateClipping(mBody);
        MR::invalidateClipping(this);
        return true;
    }
    return false;
}

bool Patakuri::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        TQuat4f quat;
        TVec3f toPlayer;
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        TVec3f up = -mGravity;
        MR::makeQuatUpFront(&quat, up, toPlayer);

        mBody->appearKuriboWandering(calcBodyDropPos(this), quat, mGravity * -30.0f);
        setNerve(&PatakuriNrvHitStep::sInstance);
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return true;
    }
    if (MR::isMsgYoshiReleaseEaten(msg) || MR::isMsgYoshiCancelEaten(msg)) {
        MR::invalidateClipping(this);
        mWing->kill();
        MR::hideModelAndOnCalcAnim(this);
        setNerve(&PatakuriNrvDead::sInstance);
        return true;
    }
    return false;
}

void Patakuri::exeRail() {
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(mWing, "Wait");
        MR::startAction(this, "PatakuriWait");
    }

    f32 speed = 1.0f;
    f32 coord = MR::getRailRemainingLength(this) - 30.0f;
    f32 railLen = MR::getRailTotalLength(this);
    if (railLen - coord < 200.0f) {
        coord = (MR::getRailTotalLength(this) - 30.0f) - coord;
    }
    if (coord < 200.0f) {
        f32 t = MR::normalize(coord, 0.0f, 200.0f);
        speed = MR::getInterpolateValue(t, 0.2f, 1.0f);
    }

    if (!MR::isSameDirection(MR::getRailDirection(this), mGravity, 0.1f)) {
        if (MR::getRailRemainingLength(this) - 30.0f < 100.0f) {
            MR::turnVecToVecDegree(&mFront, mFront, -MR::getRailDirection(this), 3.0f, TVec3f(0, 1, 0));
        } else {
            MR::turnVecToVecDegree(&mFront, mFront, MR::getRailDirection(this), 3.0f, TVec3f(0, 1, 0));
        }
    } else {
        TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
        MR::normalize(&toPlayer);
        MR::turnVecToVecDegree(&mFront, mFront, toPlayer, 2.0f, TVec3f(0, 1, 0));
    }

    MR::moveCoordAndFollowTrans(this, 4.0f * speed);
    if (MR::isRailReachedNearGoal(this, 30.0f)) {
        MR::reverseRailDirection(this);
    }
}

void Patakuri::exePatakuriBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
    }
    MR::applyVelocityDampAndGravity(this, 0.3f, 0.8f, 0.96f, 0.93f, 1.0f);
    if (MR::isOnGround(this)) {
        setNerve(&PatakuriNrvSwoonLand::sInstance);
    }
}

void Patakuri::exeLandSwoon() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonLand");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvStaggerEnd::sInstance);
    }
}

void Patakuri::exePatakuriStaggerEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StaggerEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvDead::sInstance);
        mBody->appearKuriboWandering(calcBodyDropPos(this), mQuat, TVec3f(0.0f, 0.0f, 0.0f));
    }
}

void Patakuri::exePatakuriHipDropDown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropDown");
    }
    MR::setPosition(this, *MR::getPlayerPos());
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvDead::sInstance);
        TVec3f pos = calcBodyDropPos(this);
        MR::setPosition(mBody, pos);
        MR::appearCoinPop(mBody, mBody->mPosition, 1);
        mBody->calcAndSetBaseMtx();
        mBody->kill();
        ScoreAttackFunction::scoreTrample(this);
        return;
    }
    if (MR::isPlayerHipDropLand()) {
        setNerve(&PatakuriNrvCoinPop::sInstance);
    }
}

void Patakuri::exeCoinPop() {
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvDead::sInstance);
        TVec3f pos = calcBodyDropPos(this);
        MR::setPosition(mBody, pos);
        MR::appearCoinPop(mBody, mBody->mPosition, 1);
        mBody->calcAndSetBaseMtx();
        mBody->kill();
        ScoreAttackFunction::scoreTrample(this);
    }
}

void Patakuri::exeHitStep() {
    if (MR::isStep(this, 1)) {
        mWing->mCheckHiddenHost = true;
        MR::hideModelAndOnCalcAnim(this);
        MR::tryHitReaction(this, "\x89\x48\x90\x48\x82\xd7\x82\xe7\x82\xea");
    }
}

void Patakuri::exePatakuriDead() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
    }
    if (MR::isStep(this, 1)) {
        MR::hideModel(this);
    }
    if (MR::isDead(mWing) && MR::isDead(mBody)) {
        kill();
    }
}
