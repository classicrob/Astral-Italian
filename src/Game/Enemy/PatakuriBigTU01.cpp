#include "Game/Enemy/PatakuriBig.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/YoshiLockOnUtil.hpp"

namespace {
    class PatakuriBigParam : public AnimScaleParam {
    public:
        PatakuriBigParam();
    };

    PatakuriBigParam::PatakuriBigParam() : AnimScaleParam() {
        mSpringStrength = 0.1f;
        mDpdVibrationTime = 30.0f;
        mDpdVibrationBase = 0.6f;
        mDpdVibrationAmplitude = 0.06f;
        mDpdVibrationFrequency = 3.0f;
        mHitReactionSpringStrength = 0.1f;
    }

    PatakuriBigParam sBigScaleParam;

    // Nerve objects of PatakuriBig (TU01). Declared in the sinit order of the target.
#ifdef GALAXY_NATIVE
#define PATAKURI_BIG_NERVE(name, cls, fn)                                                                                                         \
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
#define PATAKURI_BIG_NERVE(name, cls, fn)                                                                                                         \
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

    PATAKURI_BIG_NERVE(PatakuriBigNrvRail, PatakuriBig, exeBigRail);
    PATAKURI_BIG_NERVE(PatakuriBigNrvBlow, PatakuriBig, exeBigBlow);
    PATAKURI_BIG_NERVE(PatakuriBigNrvSwoonLand, PatakuriBig, exeBigSwoonLand);
    PATAKURI_BIG_NERVE(PatakuriBigNrvStaggerEnd, PatakuriBig, exeBigStaggerEnd);
    PATAKURI_BIG_NERVE(PatakuriBigNrvHipDropDown, PatakuriBig, exeBigHipDropDown);
    PATAKURI_BIG_NERVE(PatakuriBigNrvWingHide, PatakuriBig, exeBigWingHide);
    PATAKURI_BIG_NERVE(PatakuriBigNrvBind, PatakuriBig, exeBigBind);
    PATAKURI_BIG_NERVE(PatakuriBigNrvDead, PatakuriBig, exeBigDead);

    class PatakuriBigWingNrvWait : public Nerve {
    public:
        virtual void execute(Spine*) const {};
        static PatakuriBigWingNrvWait sInstance;
    };
#ifdef GALAXY_NATIVE
    PatakuriBigWingNrvWait PatakuriBigWingNrvWait::sInstance;
#else
    PatakuriBigWingNrvWait PatakuriBigWingNrvWait::sInstance ATTRIBUTE_WEAK;
#endif

    class PatakuriBigWingNrvKill : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            if (MR::isActionEnd(actor)) {
                actor->kill();
            }
        };
        static PatakuriBigWingNrvKill sInstance;
    };
#ifdef GALAXY_NATIVE
    PatakuriBigWingNrvKill PatakuriBigWingNrvKill::sInstance;
#else
    PatakuriBigWingNrvKill PatakuriBigWingNrvKill::sInstance ATTRIBUTE_WEAK;
#endif
}  // namespace

// ---------------------------------------------------------------------------------------------
// PatakuriBigWing
// ---------------------------------------------------------------------------------------------

// NONMATCHING: 99.1% (ctor store order / register allocation)
PatakuriBigWing::PatakuriBigWing(const char* pHost)
    : PartsModel(reinterpret_cast< LiveActor* >(const_cast< char* >(pHost)), "\x89\x48", "PatakuriWingBig", nullptr, -1, false) {
    initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), "Head");
    initWithoutIter();
    initNerve(&PatakuriBigWingNrvWait::sInstance, 0);
}

PatakuriBigWing::~PatakuriBigWing() {}

void PatakuriBigWing::calcAndSetBaseMtx() {
    MR::updateBaseScale(this, reinterpret_cast< PatakuriBig* >(mHost)->mScaleCtrl);
    PartsModel::calcAndSetBaseMtx();
}

void PatakuriBigWing::startBigWingDamage() {
    MR::startAction(this, "Damage");
    mCheckHiddenHost = true;
    setNerve(&PatakuriBigWingNrvKill::sInstance);
}

// ---------------------------------------------------------------------------------------------
// PatakuriBig
// ---------------------------------------------------------------------------------------------

PatakuriBig::PatakuriBig(const char* pName) : LiveActor(pName) {
    mBody = nullptr;
    mLockOnTarget = nullptr;
    mScaleCtrl = nullptr;
    mBindStarPointer = nullptr;
}

PatakuriBig::~PatakuriBig() {}

void PatakuriBig::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "KuriboChief", "PatakuriBig", false);
    MR::moveCoordAndTransToNearestRailPos(this);
    MR::makeQuatAndFrontFromRotate(&mQuat, &mFront, this);

    mScaleCtrl = new AnimScaleController(&sBigScaleParam);

    mBody = new KuriboChief("\x83\x4e\x83\x8a\x83\x7b\x81\x5b\x83\x60\x81\x5b\x83\x74");
    MR::setPosition(mBody, mPosition);
    mBody->initWithoutIter();
    mBody->makeActorDead();

    if (MR::isValidInfo(rIter)) {
        MR::setGroupClipping(this, rIter, 32);
        MR::setGroupClipping(mBody, rIter, 32);
    }

    initHitSensor(8);
    MR::addHitSensorAtJointEnemyAttack(this, "Body", "Body", 8, 150.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointEnemyAttack(this, "Head1", "Head", 8, 300.0f, TVec3f(120.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointEnemyAttack(this, "Head2", "Head", 8, 150.0f, TVec3f(360.0f, 0.0f, 0.0f));
    MR::addHitSensorAtJointEnemyAttack(this, "LegL", "LegL", 8, 130.0f, TVec3f(60.0f, -50.0f, 0.0f));
    MR::addHitSensorAtJointEnemyAttack(this, "LegR", "LegR", 8, 130.0f, TVec3f(60.0f, -50.0f, 0.0f));
    MR::addHitSensorEnemy(this, "Punch", 8, 400.0f, TVec3f(0.0f, 0.0f, 0.0f));

    mWing = new PatakuriBigWing(reinterpret_cast< const char* >(this));
    mWing->initWithoutIter();
    mWing->offFixedPosNormalizeScale();
    MR::startAction(mWing, "Wait");
    MR::startAction(this, "PatakuriWait");

    initNerve(&PatakuriBigNrvRail::sInstance, 1);

    mBindStarPointer = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBindStarPointer, &PatakuriBigNrvBind::sInstance, "SupportFreeze");

    mLockOnTarget = MR::createYoshiLockOnTarget(this, static_cast< MtxPtr >(nullptr), TVec3f(0.0f));
    makeActorAppeared();
}

void PatakuriBig::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    MR::calcGravityVector(this, mPosition, &mGravity, nullptr, 0);
    MR::makeQuatAndFrontFromRotate(&mQuat, &mFront, this);
}

// NONMATCHING: epilogue restore order (lwz r31 before lwz r0)
void PatakuriBig::control() {
    mScaleCtrl->updateNerve();
    MR::blendQuatFromGravityAndFront(&mQuat, this, &mFront, 0.05f, 0.2f);
    if (tryStartBigBindStar()) {
        return;
    }
}

void PatakuriBig::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
    MR::updateBaseScale(this, mScaleCtrl);
}

void PatakuriBig::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayer(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        if (isNerve(&PatakuriBigNrvBind::sInstance)) {
            MR::sendMsgPush(pReceiver, pSender);
        } else if (isNerve(&PatakuriBigNrvRail::sInstance)) {
            if (!MR::isPlayerHipDropFalling() && MR::sendMsgEnemyAttack(pReceiver, pSender)) {
                MR::startAction(mWing, "Hit");
                MR::startAction(this, "PatakuriHit");
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        }
    } else if (!MR::isSensorEnemy(pSender)) {
        MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender);
    }
}

bool PatakuriBig::tryStartBigBindStar() {
    if (isNerve(&PatakuriBigNrvRail::sInstance) && mBindStarPointer->tryStartPointBind()) {
        setNerve(&PatakuriBigNrvBind::sInstance);
        return true;
    }
    return false;
}

bool PatakuriBig::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorEnemy(pReceiver)) {
        return false;
    }
    if (MR::isMsgLockOnStarPieceShoot(msg)) {
        return false;
    }
    if (MR::isMsgStarPieceReflect(msg)) {
        if (pReceiver != getSensor("Punch")) {
            mScaleCtrl->startHitReaction();
            return true;
        }
        return false;
    }
    if (MR::isMsgPlayerHitAll(msg)) {
        PatakuriBigWing* pWing = mWing;
        pWing->startBigWingDamage();
        setNerve(&PatakuriBigNrvDead::sInstance);

        TVec3f pos = calcBigBodyOffsetPos(this);
        mBody->appearAndStagger(pos, mQuat, mVelocity, pSender, pReceiver);
        return true;
    }
    return false;
}

bool PatakuriBig::receiveOtherMsg(u32, HitSensor*, HitSensor*) {
    return false;
}

namespace {
    inline void applyBodyOffset(TVec3f* const& result, const TVec3f& offset) {
        result->add(offset);
    }

    inline TVec3f addBodyOffset(const TVec3f& base, const TVec3f& offset) {
        TVec3f result(base);
        applyBodyOffset(&result, offset);
        return result;
    }
}

TVec3f PatakuriBig::calcBigBodyOffsetPos(PatakuriBig* pPatakuri) {
    TVec3f bodyRoot;
    TVec3f bodyBody;
    TVec3f actorRoot;
    TVec3f actorBody;

    MR::extractMtxTrans(MR::getJointMtx(pPatakuri->mBody, "Root"), &bodyRoot);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri->mBody, "Body"), &bodyBody);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri, "Root"), &actorRoot);
    MR::extractMtxTrans(MR::getJointMtx(pPatakuri, "Body"), &actorBody);

    return addBodyOffset(pPatakuri->mPosition - (bodyBody - bodyRoot), actorBody - actorRoot);
}

void PatakuriBig::exeBigRail() {
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

void PatakuriBig::exeBigBlow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Blow");
    }
    MR::applyVelocityDampAndGravity(this, 0.3f, 0.8f, 0.96f, 0.93f, 1.0f);
    if (MR::isOnGround(this)) {
        setNerve(&PatakuriBigNrvSwoonLand::sInstance);
    }
}

void PatakuriBig::exeBigSwoonLand() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "SwoonLand");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriBigNrvStaggerEnd::sInstance);
    }
}

void PatakuriBig::exeBigStaggerEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "StaggerEnd");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriBigNrvDead::sInstance);
        mBody->appearAndWander(calcBigBodyOffsetPos(this), mQuat, TVec3f(0.0f, 0.0f, 0.0f));
    }
}

void PatakuriBig::exeBigHipDropDown() {
    if (MR::isActionEnd(this)) {
        setNerve(&PatakuriBigNrvDead::sInstance);
        TVec3f pos = calcBigBodyOffsetPos(this);
        MR::setPosition(mBody, pos);
        mBody->calcAndSetBaseMtx();
        mBody->kill();
    }
}

void PatakuriBig::exeBigWingHide() {
    if (MR::isStep(this, 1)) {
        mWing->mCheckHiddenHost = true;
        MR::hideModelAndOnCalcAnim(this);
    }
}

void PatakuriBig::exeBigBind() {
    if (MR::isFirstStep(this)) {
        MR::setBckRate(mWing, 0.0f);
    }
    if (MR::updateActorStateAndNextNerve(this, &PatakuriBigNrvRail::sInstance)) {
        MR::setBckRate(mWing, 1.0f);
    }
}

void PatakuriBig::exeBigDead() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
    }
    if (MR::isStep(this, 1)) {
        MR::hideModel(this);
    }
    if (MR::isDead(mWing)) {
        kill();
    }
}
