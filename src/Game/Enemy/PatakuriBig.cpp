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

    PATAKURI_NERVE(PatakuriNrvRail, Patakuri, fn_801E52A0);
    PATAKURI_NERVE(PatakuriNrvBlow, Patakuri, fn_801E54C0);
    PATAKURI_NERVE(PatakuriNrvSwoonLand, Patakuri, fn_801E5540);
    PATAKURI_NERVE(PatakuriNrvStaggerEnd, Patakuri, fn_801E55A0);
    PATAKURI_NERVE(PatakuriNrvHipDropDown, Patakuri, fn_801E5640);
    PATAKURI_NERVE(PatakuriNrvCoinPop, Patakuri, fn_801E5730);
    PATAKURI_NERVE(PatakuriNrvHitStep, Patakuri, fn_801E57D0);

    class PatakuriNrvBind : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &PatakuriNrvRail::sInstance);
        };
        static PatakuriNrvBind sInstance;
    };
    PatakuriNrvBind PatakuriNrvBind::sInstance ATTRIBUTE_WEAK;

    PATAKURI_NERVE(PatakuriNrvDead, Patakuri, fn_801E5830);

    class PatakuriWingNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {};
        static PatakuriWingNrvWait sInstance;
    };
    PatakuriWingNrvWait PatakuriWingNrvWait::sInstance ATTRIBUTE_WEAK;

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
    PatakuriNrvKill PatakuriNrvKill::sInstance ATTRIBUTE_WEAK;
}  // namespace

// ---------------------------------------------------------------------------------------------
// PatakuriWing
// ---------------------------------------------------------------------------------------------

PatakuriWing::PatakuriWing(const char* pHost)
    : PartsModel(reinterpret_cast< LiveActor* >(const_cast< char* >(pHost)), "\x89\x48", "PatakuriWing", nullptr, -1, false) {
    initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 90.0f, 0.0f), "Head");
    initWithoutIter();
    initNerve(&PatakuriWingNrvWait::sInstance, 0);
}

PatakuriWing::~PatakuriWing() {}

void PatakuriWing::fn_801E4A20() {
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

// NONMATCHING: 99.4% (register allocation / scheduling; see sg.py diff)
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

    mWing = new PatakuriWing(reinterpret_cast< const char* >(this));
    MR::startAction(mWing, "Wait");
    MR::startAction(this, "PatakuriWait");

    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));
    initNerve(&PatakuriNrvRail::sInstance, 1);

    mScaleCtrl = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, &PatakuriNrvBind::sInstance, "SupportFreeze");

    makeActorAppeared();
}

// NONMATCHING: 99.2% (register allocation / scheduling; see sg.py diff)
void Patakuri::control() {
    mScaleCtrl->updateNerve();
    MR::fn_800104F0(&mQuat, this, &mFront, 0.05f, 0.2f);

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

// NONMATCHING: 99.6% (register allocation / scheduling; see sg.py diff)
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

bool Patakuri::fn_801E58C0() const {
    if (isNerve(&PatakuriNrvRail::sInstance) || isNerve(&PatakuriNrvBind::sInstance)) {
        return true;
    }
    return false;
}

bool Patakuri::fn_801E4E50() {
    if (!fn_801E58C0()) {
        return false;
    }

    mWing->fn_801E4A20();
    setNerve(&PatakuriNrvDead::sInstance);

    TVec3f pos;
    fn_801E4F40(&pos, this);
    mBody->appearHipDropped(pos, mQuat);
    ScoreAttackFunction::scoreTrample(this);
    return true;
}

bool Patakuri::fn_801E4ED0() {
    if (!fn_801E58C0()) {
        return false;
    }

    MR::offBind(this);
    MR::zeroVelocity(this);
    MR::invalidateHitSensors(this);
    mWing->fn_801E4A20();
    setNerve(&PatakuriNrvHipDropDown::sInstance);
    return true;
}

void Patakuri::fn_801E4F40(TVec3f* pOut, Patakuri* pPatakuri) {
    TVec3f bodyRoot;
    TVec3f bodyBody;
    TVec3f actorRoot;
    TVec3f actorBody;

    MR::extractMtxTrans(MR::getJointMtx(pPatakuri->mBody, "Root"), &bodyRoot);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri->mBody, "Body"), &bodyBody);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri, "Root"), &actorRoot);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri, "Body"), &actorBody);

    TVec3f diffActor = actorBody - actorRoot;
    TVec3f diffBody = bodyBody - bodyRoot;
    TVec3f base = pPatakuri->mPosition - diffBody;
    *pOut = base + diffActor;
}

// NONMATCHING: 99.6% (register allocation / scheduling; see sg.py diff)
bool Patakuri::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return true;
    }
    if (MR::isMsgPlayerTrample(msg)) {
        return fn_801E4E50();
    }
    if (MR::isMsgPlayerHipDrop(msg)) {
        return fn_801E4ED0();
    }
    if (MR::isMsgPlayerHitAll(msg) || MR::isMsgStarPieceAttack(msg)) {
        mWing->fn_801E4A20();
        setNerve(&PatakuriNrvDead::sInstance);

        TVec3f pos;
        fn_801E4F40(&pos, this);
        mBody->fn_801B9B10(pos, mQuat, mVelocity);
        mBody->receiveMsgPlayerAttack(msg, pSender, pReceiver);
        MR::invalidateClipping(mBody);
        MR::invalidateClipping(this);
        return true;
    }
    return false;
}

// NONMATCHING: 99.5% (register allocation / scheduling; see sg.py diff)
bool Patakuri::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        TVec3f toPlayer;
        MR::calcVecToPlayerH(&toPlayer, this, nullptr);
        TVec3f up = -mGravity;
        TQuat4f quat;
        MR::makeQuatUpFront(&quat, up, toPlayer);

        TVec3f vel(mGravity);
        vel.x *= -30.0f;
        vel.y *= -30.0f;
        vel.z *= -30.0f;

        TVec3f pos;
        fn_801E4F40(&pos, this);
        mBody->fn_801B9B10(pos, quat, vel);
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

void Patakuri::fn_801E52A0() {
    if (MR::isBckOneTimeAndStopped(this)) {
        MR::startAction(mWing, "Wait");
        MR::startAction(this, "PatakuriWait");
    }

    f32 speed = 1.0f;
    f32 coord = MR::getRailCoord(this) - 30.0f;
    f32 railLen = MR::getRailTotalLength(this);
    if (railLen - coord < 200.0f) {
        coord = (MR::getRailTotalLength(this) - 30.0f) - coord;
    }
    if (coord < 200.0f) {
        f32 t = MR::normalize(coord, 0.0f, 200.0f);
        speed = MR::getInterpolateValue(t, 0.2f, 1.0f);
    }

    if (!MR::isSameDirection(MR::getRailDirection(this), mGravity, 0.1f)) {
        TVec3f axis(0.0f, 1.0f, 0.0f);
        if (MR::getRailCoord(this) - 30.0f < 100.0f) {
            TVec3f revDir = -MR::getRailDirection(this);
            MR::turnVecToVecDegree(&mFront, mFront, revDir, 3.0f, axis);
        } else {
            MR::turnVecToVecDegree(&mFront, mFront, MR::getRailDirection(this), 3.0f, axis);
        }
    } else {
        TVec3f toPlayer = *MR::getPlayerPos() - mPosition;
        MR::normalize(&toPlayer);
        TVec3f axis(0.0f, 1.0f, 0.0f);
        MR::turnVecToVecDegree(&mFront, mFront, toPlayer, 2.0f, axis);
    }

    MR::moveCoordAndFollowTrans(this, 4.0f * speed);
    if (MR::isRailReachedNearGoal(this, 30.0f)) {
        MR::reverseRailDirection(this);
    }
}

void Patakuri::fn_801E54C0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
    }
    MR::applyVelocityDampAndGravity(this, 0.3f, 0.8f, 0.96f, 0.93f, 0.0f);
    if (MR::isOnGround(this)) {
        setNerve(&PatakuriNrvSwoonLand::sInstance);
    }
}

void Patakuri::fn_801E5540() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonLand");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvStaggerEnd::sInstance);
    }
}

void Patakuri::fn_801E55A0() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StaggerEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvDead::sInstance);
        TVec3f pos;
        fn_801E4F40(&pos, this);
        TVec3f zero(0.0f);
        mBody->fn_801B9B10(pos, mQuat, zero);
    }
}

void Patakuri::fn_801E5640() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "HipDropDown");
    }
    MR::setPosition(this, *MR::getPlayerPos());
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvDead::sInstance);
        TVec3f pos;
        fn_801E4F40(&pos, this);
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

void Patakuri::fn_801E5730() {
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriNrvDead::sInstance);
        TVec3f pos;
        fn_801E4F40(&pos, this);
        MR::setPosition(mBody, pos);
        MR::appearCoinPop(mBody, mBody->mPosition, 1);
        mBody->calcAndSetBaseMtx();
        mBody->kill();
        ScoreAttackFunction::scoreTrample(this);
    }
}

void Patakuri::fn_801E57D0() {
    if (MR::isStep(this, 1)) {
        mWing->mCheckHiddenHost = true;
        MR::hideModelAndOnCalcAnim(this);
        MR::tryHitReaction(this, "\x89\x48\x90\x48\x82\xd7\x82\xe7\x82\xea");
    }
}

void Patakuri::fn_801E5830() {
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
