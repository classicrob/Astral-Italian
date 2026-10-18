#include "Game/Enemy/Unbaba.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/UnbabaSlim.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"

// PLACEHOLDER(enemies): unnamed mutable spline slope in sbss2; resolve when its data symbol is named.
extern f32 lbl_807E3080;

namespace NrvUnbaba {
    NEW_NERVE(UnbabaNrvWait, Unbaba, Wait);
    NEW_NERVE(UnbabaNrvAppear, Unbaba, Appear);
    NEW_NERVE(UnbabaNrvAppearStep, Unbaba, AppearStep);
    NEW_NERVE(UnbabaNrvAttackSign, Unbaba, AttackSign);
    NEW_NERVE(UnbabaNrvAttack, Unbaba, Attack);
    NEW_NERVE(UnbabaNrvLaunch, Unbaba, Launch);
    NEW_NERVE(UnbabaNrvRestart, Unbaba, Restart);
};  // namespace NrvUnbaba

namespace {
    extern const f32 sOne;
    extern const f32 sRiseTime;
    extern const f32 sRiseTime1;
    extern const f32 sRiseSlope1;
    TVec3f sParam0(1.2f, 1.0f, 1.4f);
    TVec3f sParam1(400.0f, 0.0f, -300.0f);
    TVec3f sParam2(0.09f, 0.09f, 0.09f);
};  // namespace

#define UNBABA_NERVE(n) (&NrvUnbaba::UnbabaNrv##n::sInstance)

Unbaba::Unbaba(const char* pName) : LiveActor(pName), mVolumeDrawer(nullptr) {
    mPos.set(0.0f, 0.0f, 0.0f);
    mRotate.set(0.0f, 0.0f, 0.0f);
    mUpVec.set(0.0f, 0.0f, 0.0f);
    mHeight = 0.0f;
    mDistance = 0.0f;
    mT0 = 0.0f;
    mTimer = 0.0f;
    mStepAttackSign = 30;
    mStepAttack = 20;
    mIsScaleUp = false;
    mIsAttackBlocked = false;
    mMtx.identity();
    mLaunchMtx.identity();
}

void Unbaba::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Unbaba", false);
    mVolumeDrawer = MR::createVolumeDrawer("マグマ範囲描画", "VolumeSphere", mMtx.toMtxPtr());
    mVolumeDrawer->setHost(this);
    f32 dist = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &dist);
    if (-1.0f == dist) dist = 500.0f;
    mDistance = dist;
    fn_80215FB0();
    MR::getJMapInfoArg1NoInit(rIter, &mStepAttack);
    MR::getJMapInfoArg2NoInit(rIter, &mStepAttackSign);
    MR::getJMapInfoArg3NoInit(rIter, &mIsAttackBlocked);
    MR::getJMapInfoTrans(rIter, &mPos);
    MR::getJMapInfoRotate(rIter, &mRotate);
    MR::calcUpVec(&mUpVec, this);
    initNerve(UNBABA_NERVE(Wait), 0);
    makeActorAppeared();
}

void Unbaba::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(UNBABA_NERVE(Appear));
    MR::startBckNoInterpole(this, "Wait");
    MR::setPosition(this, mPos);
    MR::setRotation(this, mRotate);
    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR(mtx.toMtxPtr(), TVec3f(0.0f), mRotate);
    mtx.getYDir(mUpVec);
    mHeight = -2500.0f * mScale.x;
    mVolumeDrawer->makeActorAppeared();
    mVolumeDrawer->setScaleDownRate(0);
}

void Unbaba::makeActorDead() {
    LiveActor::makeActorDead();
    mVolumeDrawer->makeActorDead();
}

void Unbaba::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Wait");
        MR::startBckNoInterpole(this, "Wait");
        MR::validateClipping(this);
    }
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            setNerve(UNBABA_NERVE(AppearStep));
            return;
        }
    } else if (MR::isGreaterStep(this, 0x78)) {
        setNerve(UNBABA_NERVE(AppearStep));
        return;
    }
    MR::startActionSound(this, "MagmaWait", -1, -1, -1);
    if (MR::isFirstStep(this)) {
        setNerve(UNBABA_NERVE(Wait));
    }
}

void Unbaba::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartActionNotPlaying(this, "Wait");
        MR::validateClipping(this);
    }
    if (MR::isValidSwitchA(this)) {
        if (MR::isOnSwitchA(this)) {
            setNerve(UNBABA_NERVE(AppearStep));
        }
    } else if (MR::isGreaterStep(this, 0x78)) {
        setNerve(UNBABA_NERVE(AppearStep));
    }
    MR::startActionSound(this, "MagmaWait", -1, -1, -1);
}

void Unbaba::exeAppearStep() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
        MR::invalidateClipping(this);
    }
    if (MR::isGreaterStep(this, mStepAttackSign)) {
        setNerve(UNBABA_NERVE(AttackSign));
    }
    fn_80215E50();
    MR::startActionSound(this, "MagmaAttack", -1, -1, -1);
}

void Unbaba::exeAttackSign() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, "AttackSign");
    }
    if (MR::isActionEnd(this)) {
        setNerve(UNBABA_NERVE(Attack));
    }
    fn_80215E50();
    MR::startActionSound(this, "MagmaAttack", -1, -1, -1);
}

void Unbaba::exeAttack() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, "Attack");
    }
    if (MR::isGreaterStep(this, 0x58)) mIsScaleUp = false;
    else mIsScaleUp = true;
    if (MR::isActionEnd(this)) {
        setNerve(UNBABA_NERVE(Launch));
    }
    MR::startActionSound(this, "MagmaAttack", -1, -1, -1);
}

void Unbaba::exeLaunch() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }
    if (MR::isGreaterStep(this, mStepAttack)) {
        mT0 = 0.0f;
        mHeight = -2500.0f * mScale.x;
        setNerve(UNBABA_NERVE(Restart));
    }
    fn_80215EF0();
}

void Unbaba::exeRestart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Restart");
    }
    if (MR::isActionEnd(this)) {
        setNerve(UNBABA_NERVE(Wait));
    }
    MR::startActionSound(this, "MagmaWait", -1, -1, -1);
}

// NONMATCHING: TVec3 sub/assign, matrix copy and Color8 init stay out-of-line in this TU while the target inlines them (paired-single code)
void Unbaba::control() {
    mMtx.setInline(MR::getJointMtx(this, "Head"));
    MR::addTransMtxLocal(mMtx.toMtxPtr(), sParam1);
    MR::preScaleMtx(mMtx.toMtxPtr(), sParam0 * 10.0f);
    const GXColor color = {0x8C, 0x28, 0x00, 0x87};
    Color8 drawColor;
    drawColor.r = color.r;
    drawColor.g = color.g;
    drawColor.b = color.b;
    drawColor.a = color.a;
    mVolumeDrawer->setColor(drawColor);
    mVolumeDrawer->setDrawPasses(4);
    mVolumeDrawer->setScale(sParam2);
    bool scaleUp = false;
    if (mIsScaleUp && !mIsAttackBlocked) scaleUp = true;
    if (scaleUp) {
        TVec3f diff(0.0f);
        TVec3f pos(0.0f);
        MR::extractMtxTrans(mMtx.toMtxPtr(), &pos);
        diff = *MR::getPlayerPos() - pos;
        s32 distance = 3500;
        if (diff.length() > f32(distance)) scaleUp = false;
    }
    if (scaleUp) mVolumeDrawer->setScaleUpRate(60);
    else mVolumeDrawer->setScaleDownRate(60);
    mIsScaleUp = false;
}

// NONMATCHING: temporary allocation, register assignment and vector-add scheduling differ.
void Unbaba::calcAndSetBaseMtx() {
    MR::setPosition(this, mPos + mUpVec * mHeight);
    LiveActor::calcAndSetBaseMtx();
}

// NONMATCHING: the target keeps one extra unreachable `b` after the attack-result test (dead jump the compiler folds here)
void Unbaba::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            if (MR::sendMsgEnemyAttackFireStrong(pReceiver, pSender)) {
                return;
            }
        }
    } else if (MR::isSensorEnemy(pSender)) {
        if (MR::isSensorEnemy(pReceiver) || MR::isSensorMapObj(pReceiver)) {
            if (MR::sendMsgPushAndKillVelocityToTarget(this, pReceiver, pSender)) {
                return;
            }
        }
    }
}

bool Unbaba::receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*) {
    return false;
}

// NONMATCHING: scalar expression scheduling and float-register assignment differ.
f32 fn_80215D50(f32 t, f32 start, f32 value, f32 slope, f32 end, f32 endValue, f32 endSlope) {
    f32 dt = t - start;
    f32 rate = dt / (end - start);
    f32 rate2 = rate * rate;
    f32 curve = rate2 - rate;
    return (value + ((((rate + rate) * curve) - rate2) * (value - endValue))) - (dt * ((rate * slope) - (slope + (slope * curve) + (endSlope * curve))));
}

f32 fn_80215DA0(f32 t, f32 slope0, f32 time1, f32 value1, f32 slope1, f32 time2, f32 slope2) {
    if (t >= 1.0f) return 1.0f;
    if (t <= 0.0f) return 0.0f;
    if (t <= time1) return fn_80215D50(t, 0.0f, 0.0f, slope0, time1, value1, slope1);
    if (t <= time2) return fn_80215D50(t, time1, value1, slope1, time2, 1.0f, slope2);
    return fn_80215D50(t, time2, 1.0f, slope2, 1.0f, 1.0f, 0.0f);
}

// NONMATCHING: float-load scheduling and product register assignment differ.
bool Unbaba::fn_80215E50() {
    this->mTimer += sOne / sRiseTime;
    MR::clamp01(&this->mTimer);
    f32 t = fn_80215DA0(this->mTimer, lbl_807E3080, sRiseTime1, 1.1f, sRiseSlope1, 0.85f, 0.3f);
    this->mHeight = this->mDistance * t + -2500.0f * this->mScale.x;
    return 1.0f == this->mTimer;
}

// NONMATCHING: float-load scheduling and product register assignment differ.
bool Unbaba::fn_80215EF0() {
    s32 frames = 20;
    this->mTimer -= sOne / f32(frames);
    MR::clamp01(&this->mTimer);
    f32 t = fn_80215DA0(this->mTimer, 3.0f, 0.75f, 1.0f, -0.9f, 0.92f, 0.55f);
    this->mHeight = this->mDistance * t + -2500.0f * this->mScale.x;
    return 0.0f == this->mTimer;
}

// NONMATCHING: constant and field load order differ.
void Unbaba::fn_80215FB0() {
    PSMTXCopy(getBaseMtx(), mLaunchMtx.toMtxPtr());
    MR::setMtxTrans(mLaunchMtx.toMtxPtr(), mPos.x, mPos.y, mPos.z);
    MR::addTransMtxLocalX(mLaunchMtx.toMtxPtr(), mDistance + -1000.0f);
}

Unbaba::~Unbaba() {}

namespace {
    const f32 sOne = 1.0f;
    const f32 sRiseTime = 130.0f;
    const f32 sRiseTime1 = 0.6f;
    const f32 sRiseSlope1 = 0.6f;
}

