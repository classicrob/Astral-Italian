#include "Game/Enemy/BoomerangBros.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/GeometryBindUtil.hpp"


// Tangents of the flight curve (.bss statics of the target, constructed in __sinit).
static TVec3f sCurveTangentA(0.0f, 0.0f, 0.0f);
static TVec3f sCurveTangentB(0.0f, 0.0f, 0.0f);
static TVec3f sCurveTangentC(0.0f, 0.0f, 2000.0f);

BoomerangBrosBoomerang::BoomerangBrosBoomerang(const char* pName) : BrosProjectileBase(pName), mPathDistance(2000.0f), mStep(0), mBind(nullptr) {
}

namespace NrvBoomerangBrosBoomerang {
    NEW_NERVE(BoomerangBrosBoomerangNrvFly, BoomerangBrosBoomerang, Fly);
    NEW_NERVE(BoomerangBrosBoomerangNrvOut, BoomerangBrosBoomerang, Out);
    NEW_NERVE(BoomerangBrosBoomerangNrvBack, BoomerangBrosBoomerang, Back);
    NEW_NERVE(BoomerangBrosBoomerangNrvTongue, BoomerangBrosBoomerang, Tongue);
    NEW_NERVE(BoomerangBrosBoomerangNrvEaten, BoomerangBrosBoomerang, Eaten);
    NEW_NERVE(BoomerangBrosBoomerangNrvThrown, BoomerangBrosBoomerang, Thrown);
    NEW_NERVE(BoomerangBrosBoomerangNrvThrownBack, BoomerangBrosBoomerang, ThrownBack);

    class BoomerangBrosBoomerangNrvFreeze : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            BoomerangBrosBoomerang* pActor = reinterpret_cast< BoomerangBrosBoomerang* >(pSpine->mExecutor);
            pActor->mIsPointBound = true;
            MR::updateActorStateAndNextNerve(pActor, &BoomerangBrosBoomerangNrvOut::sInstance);
        }
        static BoomerangBrosBoomerangNrvFreeze sInstance;
    };
    BoomerangBrosBoomerangNrvFreeze BoomerangBrosBoomerangNrvFreeze::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvBoomerangBrosBoomerang

#define BBNRV(n) (&NrvBoomerangBrosBoomerang::BoomerangBrosBoomerangNrv##n::sInstance)

void BoomerangBrosBoomerang::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BoomerangBrosBoomerang", false);
    BrosProjectileBase::init(rIter);
    initNerve(BBNRV(Fly), 1);
    mBind = new WalkerStateBindStarPointer(this, mScaleCtrl);
    MR::initActorState(this, mBind, BBNRV(Freeze), "SupportFreeze");
    makeActorDead();
}

void BoomerangBrosBoomerang::appear() {
    BrosProjectileBase::appear();
    mStep = 0;
    MR::startAction(this, "Normal");
    setNerve(BBNRV(Fly));
    MR::showModel(this);
}

void BoomerangBrosBoomerang::kill() {
    LiveActor::kill();
    if (!isNerve(BBNRV(Tongue))) {
        MR::tryHitReactionBreak(this);
    }
}

void BoomerangBrosBoomerang::control() {
    BrosProjectileBase::control();
    if (isNerve(BBNRV(Out)) && mBind->tryStartPointBind()) {
        setNerve(BBNRV(Freeze));
    }
}

void BoomerangBrosBoomerang::attachToBros(BrosBase* pBros) {
    BrosProjectileBase::attachToBros(pBros);
    setNerve(BBNRV(Fly));
}

void BoomerangBrosBoomerang::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body")) {
        if (!MR::isSensorPlayerOrRide(pReceiver)) {
            MR::sendMsgEnemyAttack(pReceiver, pSender);
        }
    } else {
        if (MR::isSensorPlayerOrRide(pReceiver) && MR::isPlayerSquat()) {
            return;
        }
        if (v84() && MR::isSensorPlayerOrYoshiOrRide(pReceiver)) {
            if (!isNerve(BBNRV(Thrown)) || !MR::isLessStep(this, 30)) {
                MR::tryHitReaction(this, "\203\210\203b\203V\201[\222e\203v\203\214\203C\203\204\201[\220\332\220G");
                killWithState3();
            }
            return;
        }
        BrosProjectileBase::attackSensor(pSender, pReceiver);
    }
}

bool BoomerangBrosBoomerang::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Body")) {
        if (MR::isMsgPlayerTrample(msg) || MR::isMsgPlayerHipDrop(msg)) {
            killWithState2();
            kill();
            return true;
        }
        return false;
    }
    return BrosProjectileBase::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

// NONMATCHING: the target pools the two clamp bounds as two separate 2000.0f literals; ours merges them (fmr).
namespace {
    extern const f32 sPathDistanceMin;
    extern const f32 sPathDistanceMax;
}

void BoomerangBrosBoomerang::v70(const TVec3f& rPos, const TVec3f& rTarget) {
    mOwner = nullptr;
    MR::setPosition(this, rPos);
    if (MR::isDead(this)) {
        appear();
    }
    setNerve(BBNRV(Out));
    MR::offBind(this);
    TVec3f diff = *MR::getPlayerPos() - rPos;
    MR::vecKillElement(diff, mGravity, &diff);
    mPathDistance = PSVECMag(&diff);
    mPathDistance = MR::clamp(mPathDistance, sPathDistanceMin, sPathDistanceMax);
    MR::normalizeOrZero(&mFront, diff);
    mMtx.identity();
    MR::calcMtxFromGravityAndZAxis(&mMtx, this, mFront);
    MR::setBaseTRMtx(this, mMtx);
}

void BoomerangBrosBoomerang::exeFly() {
    if (MR::isFirstStep(this)) {
        MR::tryStartAllAnim(this, "Normal");
        MR::offBind(this);
    }
    mScale.x = MR::clamp(0.2f + mScale.x, 0.0f, 1.0f);
    TVec3f scale(mScale.x);
    MR::setScale(this, scale);
}

void BoomerangBrosBoomerang::exeOut() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Spin");
        mIsPointBound = false;
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    s32 duration = 240;
    f32 t0 = (f32)mStep / (f32)duration;
    f32 t1 = (f32)(mStep + 1) / (f32)duration;
    TVec3f p0(0.0f);
    TVec3f p1(0.0f);
    calcPath(&p0, t0);
    calcPath(&p1, t1);
    mLocalVel = p1 - p0;
    mScale.x = MR::clamp(0.2f + mScale.x, 0.0f, 1.0f);
    TVec3f scale(mScale.x);
    MR::setScale(this, scale);
    if (MR::isBinded(this)) {
        killWithState1();
        kill();
        return;
    }
    if (mStep > 228) {
        setNerve(BBNRV(Back));
        return;
    }
    mStep++;
}

void BoomerangBrosBoomerang::exeBack() {
    mLocalVel *= 0.99f;
    mLocalVel.y += -0.1f;
    if (MR::isBinded(this) || MR::isGreaterStep(this, 300)) {
        killWithState1();
        kill();
    }
}

void BoomerangBrosBoomerang::exeTongue() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Normal");
        MR::deleteEffectAll(this);
    }
    exeEaten();
}

void BoomerangBrosBoomerang::exeThrown() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Spin");
        mIsPointBound = false;
        mStep = 0;
        mPathDistance = 2000.0f;
        MR::getPlayerThrowVec(&mFront);
        mMtx.identity();
        MR::calcMtxFromGravityAndZAxis(&mMtx, this, mFront);
        MR::setBaseTRMtx(this, mMtx);
    }
    if (MR::isGreaterStep(this, 10) && MR::isNoBind(this)) {
        MR::onBind(this);
    }
    s32 duration = 240;
    f32 t0 = (f32)mStep / (f32)duration;
    f32 t1 = (f32)(mStep + 1) / (f32)duration;
    TVec3f p0(0.0f);
    TVec3f p1(0.0f);
    calcPath(&p0, t0);
    calcPath(&p1, t1);
    mLocalVel = p1 - p0;
    mScale.x = MR::clamp(0.2f + mScale.x, 0.0f, 1.0f);
    TVec3f scale(mScale.x);
    MR::setScale(this, scale);
    if (MR::isBinded(this)) {
        MR::sendMsgToBindedSensor(8, getSensor("Attack"));
        killWithState1();
        kill();
        return;
    }
    if (mStep > 180) {
        setNerve(BBNRV(ThrownBack));
        return;
    }
    mStep++;
}

void BoomerangBrosBoomerang::exeThrownBack() {
    mLocalVel *= 0.99f;
    mLocalVel.y += -0.1f;
    if (MR::isBinded(this) || MR::isGreaterStep(this, 300)) {
        if (MR::isBinded(this)) {
            MR::sendMsgToBindedSensor(8, getSensor("Attack"));
        }
        killWithState1();
        kill();
    }
}

// NONMATCHING: instruction scheduling of the p1 stores and one register choice.
void BoomerangBrosBoomerang::calcPath(TVec3f* pOut, f32 t) {
    f32 rate;
    f32 split = 0.45f;
    TVec3f p0(0.0f, 0.0f, 0.0f);
    TVec3f m0(0.0f, 0.0f, 0.0f);
    TVec3f p1(0.0f, 0.0f, mPathDistance);
    TVec3f m1(0.0f, 0.0f, 0.0f);
    if (t < split) {
        rate = t / split;
        m0 = sCurveTangentA;
        m1 = sCurveTangentB;
    } else {
        if (split >= 1.0f) {
            rate = 1.0f;
        } else {
            rate = 1.0f - (t - split) / (1.0f - split);
        }
        m0 = sCurveTangentC;
        m1 = -sCurveTangentB;
    }
    HermiteSpline curve(p0, m0, p1, m1);
    curve.calcPosition(pOut, rate);
}

bool BoomerangBrosBoomerang::isReturning() const {
    s32 duration = 240;
    if (isNerve(BBNRV(Back)) || (isNerve(BBNRV(Out)) && 0.45f * (f32)duration < (f32)getNerveStep())) {
        return true;
    }
    return false;
}

bool BoomerangBrosBoomerang::v78() {
    return isNerve(BBNRV(Fly));
}

bool BoomerangBrosBoomerang::v7C() {
    return isNerve(BBNRV(Tongue));
}

bool BoomerangBrosBoomerang::v80() {
    return isNerve(BBNRV(Eaten));
}

bool BoomerangBrosBoomerang::v84() {
    bool ret = false;
    if (isNerve(BBNRV(Thrown)) || isNerve(BBNRV(ThrownBack))) {
        ret = true;
    }
    return ret;
}

bool BoomerangBrosBoomerang::v88() {
    if (!v7C() && !v80()) {
        if (!isNerve(BBNRV(Fly))) {
            MR::zeroVelocity(this);
            setNerve(BBNRV(Tongue));
            MR::invalidateHitSensors(this);
            return true;
        }
    }
    return false;
}

bool BoomerangBrosBoomerang::v8C() {
    if (v7C()) {
        setNerve(BBNRV(Eaten));
        MR::invalidateHitSensors(this);
        MR::hideModel(this);
        return true;
    }
    return false;
}

bool BoomerangBrosBoomerang::v90() {
    if (v80()) {
        setNerve(BBNRV(Thrown));
        MR::validateHitSensors(this);
        MR::getPlayerThrowVec(&mFront);
        MR::showModel(this);
        return true;
    }
    return false;
}

BoomerangBrosBoomerang::~BoomerangBrosBoomerang() {
}

namespace {
    const f32 sPathDistanceMin = 2000.0f;
    const f32 sPathDistanceMax = 2000.0f;
}
