#define COLOR8_GX_CTOR_INLINE
#include "Game/Enemy/Unbaba.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/UnbabaSlim.hpp"
#include "Game/MapObj/MapPartsRotator.hpp"
#include "Game/Util/MapPartsUtil.hpp"
#include "Game/LiveActor/Nerve.hpp"

extern const volatile f32 gUnbabaInitialRiseSlope;

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
    mHomePosition.set(0.0f, 0.0f, 0.0f);
    mHomeRotation.set(0.0f, 0.0f, 0.0f);
    mUpVec.set(0.0f, 0.0f, 0.0f);
    mHeight = 0.0f;
    mDistance = 0.0f;
    mUnusedRiseTime = 0.0f;
    mRiseProgress = 0.0f;
    mStepAttackSign = 30;
    mStepAttack = 20;
    mIsScaleUp = false;
    mIsAttackBlocked = false;
    mVolumeMatrix.identity();
    mLaunchMtx.identity();
}

void Unbaba::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Unbaba", false);
    mVolumeDrawer = MR::createVolumeDrawer("マグマ範囲描画", "VolumeSphere", mVolumeMatrix.toMtxPtr());
    mVolumeDrawer->setHost(this);
    f32 dist = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &dist);
    if (-1.0f == dist) dist = 500.0f;
    mDistance = dist;
    updateUnbabaLaunchMatrix();
    MR::getJMapInfoArg1NoInit(rIter, &mStepAttack);
    MR::getJMapInfoArg2NoInit(rIter, &mStepAttackSign);
    MR::getJMapInfoArg3NoInit(rIter, &mIsAttackBlocked);
    MR::getJMapInfoTrans(rIter, &mHomePosition);
    MR::getJMapInfoRotate(rIter, &mHomeRotation);
    MR::calcUpVec(&mUpVec, this);
    initNerve(UNBABA_NERVE(Wait), 0);
    makeActorAppeared();
}

void Unbaba::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(UNBABA_NERVE(Appear));
    MR::startBckNoInterpole(this, "Wait");
    MR::setPosition(this, mHomePosition);
    MR::setRotation(this, mHomeRotation);
    TPos3f mtx;
    mtx.identity();
    MR::makeMtxTR(mtx.toMtxPtr(), TVec3f(0.0f), mHomeRotation);
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
    updateUnbabaRise();
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
    updateUnbabaRise();
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
        mUnusedRiseTime = 0.0f;
        mHeight = -2500.0f * mScale.x;
        setNerve(UNBABA_NERVE(Restart));
    }
    updateUnbabaSink();
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

// NONMATCHING: inlined TVec3 operator- keeps &tmp in a callee-saved reg across the copy ctor (r30/r29 shift in the target); regalloc only
void Unbaba::control() {
    mVolumeMatrix.setInline(MR::getJointMtx(this, "Head"));
    MR::addTransMtxLocal(mVolumeMatrix.toMtxPtr(), sParam1);
    MR::preScaleMtx(mVolumeMatrix.toMtxPtr(), sParam0 * 10.0f);
    static const GXColor color = {0x8C, 0x28, 0x00, 0x87};
    mVolumeDrawer->setColor(color);
    mVolumeDrawer->setDrawPasses(4);
    mVolumeDrawer->setScale(sParam2);
    bool scaleUp = false;
    if (mIsScaleUp && !mIsAttackBlocked) scaleUp = true;
    if (scaleUp) {
        TVec3f diff(0.0f);
        TVec3f pos(0.0f);
        MR::extractMtxTrans(mVolumeMatrix.toMtxPtr(), &pos);
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
    MR::setPosition(this, mHomePosition + mUpVec * mHeight);
    LiveActor::calcAndSetBaseMtx();
}

void Unbaba::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorEnemyAttack(pSender)) {
        if (MR::isSensorPlayerOrRide(pReceiver)) {
            if (MR::sendMsgEnemyAttackFireStrong(pReceiver, pSender)) {
                return;
            }
            return;
        }
        return;
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

// NONMATCHING: one commuted multiply and FPR allocation; grouping and single-precision boundaries match.
f32 calcUnbabaHermiteSegment(f32 t, f32 start, f32 value, f32 slope, f32 end, f32 endValue, f32 endSlope) {
    f32 dt = t - start;
    f32 rate = dt / (end - start);
    f32 rate2 = rate * rate;
    f32 curve = rate2 - rate;
    return (value + ((((rate + rate) * curve) - rate2) * (value - endValue))) - (dt * ((rate * slope) - (slope + (slope * curve) + (endSlope * curve))));
}

f32 calcUnbabaHeightCurve(f32 t, f32 slope0, f32 time1, f32 value1, f32 slope1, f32 time2, f32 slope2) {
    if (t >= 1.0f) return 1.0f;
    if (t <= 0.0f) return 0.0f;
    if (t <= time1) return calcUnbabaHermiteSegment(t, 0.0f, 0.0f, slope0, time1, value1, slope1);
    if (t <= time2) return calcUnbabaHermiteSegment(t, time1, value1, slope1, time2, 1.0f, slope2);
    return calcUnbabaHermiteSegment(t, time2, 1.0f, slope2, 1.0f, 1.0f, 0.0f);
}

// NONMATCHING: float-load scheduling and product register assignment differ.
bool Unbaba::updateUnbabaRise() {
    this->mRiseProgress += sOne / sRiseTime;
    MR::clamp01(&this->mRiseProgress);
    f32 t = calcUnbabaHeightCurve(this->mRiseProgress, gUnbabaInitialRiseSlope, sRiseTime1, 1.1f, sRiseSlope1, 0.85f, 0.3f);
    f32 height = this->mDistance * t;
    height += -2500.0f * this->mScale.x;
    this->mHeight = height;
    return sOne == this->mRiseProgress;
}

// NONMATCHING: float-load scheduling and product register assignment differ.
bool Unbaba::updateUnbabaSink() {
    s32 frames = 20;
    this->mRiseProgress -= sOne / f32(frames);
    MR::clamp01(&this->mRiseProgress);
    f32 t = calcUnbabaHeightCurve(this->mRiseProgress, 3.0f, 0.75f, 1.0f, -0.9f, 0.92f, 0.55f);
    f32 height = this->mDistance * t;
    height += -2500.0f * this->mScale.x;
    this->mHeight = height;
    return 0.0f == this->mRiseProgress;
}

// NONMATCHING: constant and field load order differ.
void Unbaba::updateUnbabaLaunchMatrix() {
    PSMTXCopy(getBaseMtx(), mLaunchMtx.toMtxPtr());
    MR::setMtxTrans(mLaunchMtx.toMtxPtr(), mHomePosition.x, mHomePosition.y, mHomePosition.z);
    MR::addTransMtxLocalX(mLaunchMtx.toMtxPtr(), mDistance + -1000.0f);
}

Unbaba::~Unbaba() {}

namespace {
    const f32 sOne = 1.0f;
    const f32 sRiseTime = 130.0f;
    const f32 sRiseTime1 = 0.6f;
    const f32 sRiseSlope1 = 0.6f;
}


extern const volatile f32 gUnbabaInitialRiseSlope = 0.0f;
