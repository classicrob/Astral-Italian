#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"

namespace JMath {
    void gekko_ps_copy12(void*, const void*);
}

namespace {
    // PLACEHOLDER(core): the target interpolates literal pairs at run time (distinct .sdata2 slots, no folding); resolve when the real helper is identified
    static inline f32 interpolate(const f32& rStart, const f32& rEnd, f32 t) {
        return rStart + t * (rEnd - rStart);
    }

    static inline f32 scaledValue(const f32& value, const f32& factor) { return value * factor; }
    static inline f32 offsetValue(const f32& value, const f32& offset) {
        f32 radius = value;
        return offset + radius;
    }

    // Addresses of these constants are stored in the part (mpTurnSpeed / mpBindStep).
    static const f32 sTurnSpeedA = 1.5f;
    static const f32 sTurnSpeedB = 5.0f;
    static const s32 sBindStepA = 60;
    static const s32 sBindStepB = 45;
    // zero-initialised constants (.sbss2 in the target)
    static const f32 sBindBGravity = 0.0f;
    static const f32 sNextPull = 0.0f;
};  // namespace

namespace NrvBellyDragonBodyPart {
    class BellyDragonBodyPartNrvHead : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }
        static BellyDragonBodyPartNrvHead sInstance;
    };
    BellyDragonBodyPartNrvHead BellyDragonBodyPartNrvHead::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(BellyDragonBodyPartNrvWait, BellyDragonBodyPart, Wait);
    NEW_NERVE(BellyDragonBodyPartNrvFollow, BellyDragonBodyPart, Follow);
    NEW_NERVE(BellyDragonBodyPartNrvBindA, BellyDragonBodyPart, BindA);
    NEW_NERVE(BellyDragonBodyPartNrvBindAWeak, BellyDragonBodyPart, BindAWeak);
    NEW_NERVE(BellyDragonBodyPartNrvExitA, BellyDragonBodyPart, ExitA);
    NEW_NERVE(BellyDragonBodyPartNrvBindB, BellyDragonBodyPart, BindB);
    NEW_NERVE(BellyDragonBodyPartNrvExitB, BellyDragonBodyPart, ExitB);
};  // namespace NrvBellyDragonBodyPart

BellyDragonBodyPart::BellyDragonBodyPart(s32 index, BellyDragonBodyPart* pPrev, bool isLv2, f32 radius, f32 scale, f32 rate)
    : LiveActor("体（ドラゴン）"), mIsLv2(isLv2), mIsAttackable(false), mIndex(index), mRadius(radius), mRate(rate), mPrev(pPrev), mNext(nullptr),
      mWeakPoint(nullptr), mIsFrozen(false), mpTurnSpeed(&sTurnSpeedA), mpBindStep(&sBindStepA), mJointCtrl(nullptr), mScale(scale), mBreakA(nullptr),
      mBreakB(nullptr), mHillA(nullptr), mHillB(nullptr) {
    mMtx.identity();
    mBindMtxA.identity();
    mBindMtxB.identity();
    mHillMtxA.identity();
    mHillMtxB.identity();
    if (mPrev != nullptr) {
        JMath::gekko_ps_copy12(&mMtx, mPrev->getBaseMtx());
        mMtx.getTrans(mPosition);
    }
}

BellyDragonBodyPart::~BellyDragonBodyPart() {
}

MtxPtr BellyDragonBodyPart::getBaseMtx() const {
    return const_cast< MtxPtr >(mMtx.mMtx);
}

void BellyDragonBodyPart::init(const JMapInfoIter& rIter) {
    if (mPrev == nullptr) {
        MR::initSensors(this, "BellyDragonSensor", "Head");
    } else {
        MR::initSensors(this, "BellyDragonSensor", "Body");
        MR::setSensorRadius(this, "Body", mRadius);
        MR::setSensorRadius(this, "EnemyAttack", scaledValue(mRadius, 0.9f));
        initBinder(mRadius, 0.0f, 0);
    }
    MR::onCalcGravity(this);
    MR::invalidateClipping(this);
    if (mPrev == nullptr) {
        initNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvHead::sInstance, 0);
    } else {
        initNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvFollow::sInstance, 0);
    }
    s32 lastIndex = 0x1F;
    if ((f32)mIndex < (f32)lastIndex - 1.0f) {
        mHillB = new BellyDragonBodyHill("体の塚（飛び出し）", mHillMtxB.mMtx, false, isTailSide());
        mHillB->initWithoutIter();
        MR::setScale(mHillB, mScale);
        if (mIndex >= 0) {
            mHillA = new BellyDragonBodyHill("体の塚（飛び込み）", mHillMtxA.mMtx, true, isTailSide());
            mHillA->initWithoutIter();
            MR::setScale(mHillA, mScale);
            mBreakA = new BreakModel(this, "飛び散り岩（飛び込み）", "BellyDragonHoleBreak", mHillMtxA.mMtx, "Break");
            mBreakA->initWithoutIter();
            mBreakA->makeActorDead();
            mBreakB = new BreakModel(this, "飛び散り岩（飛び出し）", "BellyDragonHoleBreak", mHillMtxB.mMtx, "Break");
            mBreakB->initWithoutIter();
            mBreakB->makeActorDead();
        }
    }
    makeActorDead();
}

void BellyDragonBodyPart::pullToMtx(MtxPtr pMtx, f32 rate) {
    TVec3f dir(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    dir.sub(mPosition);
    dir.scale(rate);
    MR::addVelocity(this, dir);
}

void BellyDragonBodyPart::pullToMtxSide(MtxPtr pMtx, bool isForward, f32 rate, f32 sideRate, f32 sideDist) {
    pullToMtx(pMtx, rate);
    if (sideRate > 0.0f) {
        TVec3f front(pMtx[0][2], pMtx[1][2], pMtx[2][2]);
        TVec3f target(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
        if (isForward) {
            TVec3f offset(front);
            offset.scale(sideDist);
            target.add(offset);
        } else {
            TVec3f offset(front);
            offset.scale(sideDist);
            target.sub(offset);
        }
        target.sub(mPosition);
        target.scale(sideRate);
        MR::addVelocity(this, target);
    }
}

void BellyDragonBodyPart::exeWait() {
    mVelocity.scale(0.6f);
    MR::addVelocityToGravity(this, -1.0f);
    f32 prevRadius = mPrev->mRadius;
    f32 radius = mRadius;
    pullToMtxSide(mPrev->getBaseMtx(), false, 0.025f, 0.02f, prevRadius + radius);
    if (mNext != nullptr) {
        pullToMtx(mNext->getBaseMtx(), 0.012f);
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void BellyDragonBodyPart::exeFollow() {
    mVelocity.scale(0.75f);
    MR::addVelocityToGravity(this, -1.0f);
    if (isHead()) {
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, 0.03f, 0.05f, prevRadius + radius);
    } else if (isSecond()) {
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, 0.02f, 0.01f, prevRadius + radius);
    } else if (isTailSide()) {
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, 0.03f, 0.015f, prevRadius + radius);
    } else {
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        f32 pullRate = interpolate(0.015f, 0.025f, mRate);
        f32 sideRate = interpolate(0.01f, 0.015f, mRate);
        pullToMtxSide(mPrev->getBaseMtx(), false, pullRate, sideRate, prevRadius + radius);
    }
    if (mNext != nullptr) {
        pullToMtx(mNext->getBaseMtx(), 0.012f);
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void BellyDragonBodyPart::exeBindA() {
    mVelocity.scale(0.75f);
    if (isHead()) {
        pullToMtx(mBindMtxA.mMtx, 0.05f);
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, 0.0f, 0.15f, prevRadius + radius);
    } else if (isSecond()) {
        pullToMtx(mBindMtxA.mMtx, 0.04f);
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, 0.0f, 0.05f, prevRadius + radius);
    } else {
        pullToMtx(mBindMtxA.mMtx, 0.02f);
    }
    if (tryBindA()) {
        return;
    }
}

void BellyDragonBodyPart::exeBindAWeak() {
    mVelocity.zero();
    if (!hasWeakPoint() || MR::isGreaterEqualStep(this, *mpBindStep)) {
        MR::offBind(this);
        if (hasWeakPoint()) {
            mWeakPoint->setNerveChance();
        }
        BellyDragonBodyHill* pHill = mHillA;
        if (pHill != nullptr) {
            pHill->mIsCheckAppearEnd = true;
            pHill->mIsLongDisappear = true;
        }
        if (mNext != nullptr) {
            mNext->startBindA(mBindMtxA);
        }
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvExitA::sInstance);
    }
}

bool BellyDragonBodyPart::hasWeakPoint() const {
    return mWeakPoint != nullptr;
}

void BellyDragonBodyPart::exeExitA() {
    TVec3f oldVelocity(mVelocity);
    mVelocity.scale(0.75f);
    MR::addVelocityToGravity(this, 5.0f);
    if (isHead()) {
        pullToMtx(mPrev->getBaseMtx(), 0.03f);
    } else if (isSecond()) {
        pullToMtx(mPrev->getBaseMtx(), 0.02f);
    } else if (isTailSide()) {
        pullToMtx(mPrev->getBaseMtx(), 0.03f);
    } else {
        pullToMtx(mPrev->getBaseMtx(), 0.015f);
    }
    if (mNext != nullptr) {
        pullToMtx(mNext->getBaseMtx(), sNextPull);
    }
}

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void BellyDragonBodyPart::exeBindB() {
    f32 base;
    f32 ratio;
    TVec3f oldVelocity(mVelocity);
    mVelocity.scale(0.75f);
    MR::addVelocityToGravity(this, sBindBGravity);
    f32 dist = PSVECDistance(mPosition, mPrev->mPosition);
    TVec3f target(mBindMtxB.mMtx[0][3], mBindMtxB.mMtx[1][3], mBindMtxB.mMtx[2][3]);
    TVec3f up(mBindMtxB.mMtx[0][1], mBindMtxB.mMtx[1][1], mBindMtxB.mMtx[2][1]);
    f32 radius = mRadius;
    f32 offsetDist = offsetValue(radius, 100.0f) + 200.0f;
    TVec3f offset(up);
    offset.scale(offsetDist);
    target.add(offset);
    target.sub(mPosition);
    MR::normalizeOrZero(&target);
    if (isHead()) {
        f32 speed = dist;
        speed *= 0.05f;
        target.scale(speed);
    } else if (isSecond()) {
        f32 speed = dist;
        speed *= 0.015f;
        target.scale(speed);
    } else if (isTailSide()) {
        f32 speed = dist;
        speed *= 0.03f;
        target.scale(speed);
    } else {
        s32 lastIndex = 0x1F;
        ratio = (f32)mIndex / (f32)lastIndex;
        if (mIsAttackable) {
            base = 0.012f;
        } else {
            base = 0.008f;
        }
        target.scale(dist * (base + ratio * (0.015f - base)));
    }
    mVelocity.add(target);
    if (mNext != nullptr) {
        pullToMtx(mNext->getBaseMtx(), sNextPull);
    }
    if (tryBindB()) {
        return;
    }
}

// NONMATCHING: FP operand commutation only (grouping checked); register/stack/branch lowering.
void BellyDragonBodyPart::exeExitB() {
    TVec3f dir(mVelocity);
    mVelocity.scale(0.75f);
    f32 gravity = MR::calcNerveEaseInValue(this, 90, -6.5f, -1.0f);
    MR::addVelocityToGravity(this, gravity);
    MR::calcNerveEaseInRate(this, 90);
    if (isSecond()) {
        f32 prevRadius = mPrev->mRadius;
        f32 radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, 0.02f, 0.01f, prevRadius + radius);
    } else {
        f32 prevRadius;
        f32 radius;
        f32 pullTarget = interpolate(0.015f, 0.025f, mRate);
        f32 sideTarget = interpolate(0.01f, 0.015f, mRate);
        f32 pullRate = MR::calcNerveEaseInValue(this, 90, 0.01f, pullTarget);
        f32 sideRate = MR::calcNerveEaseInValue(this, 90, 0.0f, sideTarget);
        prevRadius = mPrev->mRadius;
        radius = mRadius;
        pullToMtxSide(mPrev->getBaseMtx(), false, pullRate, sideRate, prevRadius + radius);
    }
    if (mNext != nullptr) {
        pullToMtx(mNext->getBaseMtx(), 0.012f);
    }
    if ((f32)mIndex <= 2.0f) {
        MtxPtr pMtx = getBaseMtx();
        TVec3f front(pMtx[0][2], pMtx[1][2], pMtx[2][2]);
        const TVec3f* pNextPos = &mNext->mPosition;
        TVec3f toNext(mPosition);
        toNext.sub(*pNextPos);
        MR::normalizeOrZero(&toNext);
        if (!MR::isNearZero(toNext, 0.001f)) {
            f32 d = toNext.dot(front);
            pullToMtx(mNext->getBaseMtx(), 0.02f * (1.0f - d));
        }
    }
    if (MR::isGreaterEqualStep(this, 90)) {
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvFollow::sInstance);
    }
}

void BellyDragonBodyPart::updateDirection() {
    MtxPtr pMtx = mPrev->getBaseMtx();
    TVec3f prevPos(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    PSVECDistance(prevPos, mPosition);
    TVec3f front(mMtx.mMtx[0][2], mMtx.mMtx[1][2], mMtx.mMtx[2][2]);
    TVec3f toPrev(prevPos);
    toPrev.sub(mPosition);
    if (MR::isNearZero(toPrev, 0.001f)) {
        mMtx.getZDir(toPrev);
    } else {
        MR::normalize(&toPrev);
    }
    TVec3f prevUp(pMtx[0][1], pMtx[1][1], pMtx[2][1]);
    MR::normalize(&prevUp);
    TVec3f up(mMtx.mMtx[0][1], mMtx.mMtx[1][1], mMtx.mMtx[2][1]);
    MR::normalize(&up);
    if (isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindB::sInstance)) {
        MR::turnVecToVecDegree(&up, up, prevUp, 8.0f, TVec3f(0, 1, 0));
    } else {
        MR::turnVecToVecDegree(&up, up, prevUp, *mpTurnSpeed, TVec3f(0, 1, 0));
    }
    TVec3f prevFront(pMtx[0][0], pMtx[1][0], pMtx[2][0]);
    MR::normalize(&prevFront);
    TVec3f cross(up);
    PSVECCrossProduct(toPrev, prevFront, cross);
    MR::normalizeOrZero(&cross);
    if (!MR::isNearZero(cross, 0.001f)) {
        if (isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindB::sInstance)) {
            MR::turnVecToVecDegree(&up, up, cross, 3.0f, TVec3f(0, 1, 0));
        } else {
            MR::turnVecToVecDegree(&up, up, cross, 0.5f, TVec3f(0, 1, 0));
        }
    }
    if (MR::isSameDirection(toPrev, up, 0.01f)) {
        up.set< f32 >(mMtx.mMtx[0][1], mMtx.mMtx[1][1], mMtx.mMtx[2][1]);
    }
    MR::makeMtxFrontUpPos(&mMtx, toPrev, up, mPosition);
}

void BellyDragonBodyPart::pullTailToMtx(MtxPtr pMtx, f32 rate) {
    TVec3f dir(pMtx[0][3], pMtx[1][3], pMtx[2][3]);
    dir.sub(mPosition);
    mVelocity.add((dir * 0.015f) * rate);
}

void BellyDragonBodyPart::startBindA(const TPos3f& rMtx) {
    JMath::gekko_ps_copy12(&mBindMtxA, &rMtx);
    if (MR::isNoBind(this)) {
        MR::onBind(this);
    }
    setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindA::sInstance);
}

void BellyDragonBodyPart::startBindB(const TPos3f& rMtx) {
    JMath::gekko_ps_copy12(&mBindMtxB, &rMtx);
    if (!MR::isNoBind(this)) {
        MR::offBind(this);
    }
    if (hasWeakPoint()) {
        mWeakPoint->setNerveWaitA();
    }
    setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindB::sInstance);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
bool BellyDragonBodyPart::tryBindA() {
    TVec3f bindPos(mBindMtxA.mMtx[0][3], mBindMtxA.mMtx[1][3], mBindMtxA.mMtx[2][3]);
    if (!MR::isBinded(this) || !MR::isNear(this, bindPos, offsetValue(mRadius, 100.0f))) {
        return false;
    }
    BellyDragonBodyHill* pHill = mPrev->mHillA;
    if (pHill != nullptr) {
        pHill->mIsCheckAppearEnd = true;
        pHill->mIsLongDisappear = false;
    }
    if (hasWeakPoint()) {
        MR::zeroVelocity(this);
        mWeakPoint->setNerveBurn();
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindAWeak::sInstance);
    } else {
        MR::offBind(this);
        if (mNext != nullptr) {
            mNext->startBindA(mBindMtxA);
        }
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvExitA::sInstance);
    }
    TVec3f up = -mGravity;
    TVec3f side;
    mMtx.getXDir(side);
    TVec3f pos(mPosition);
    MR::calcMapHitAlongDirection(mPosition, &pos, mGravity, nullptr, nullptr, nullptr, nullptr, 500.0f);
    MR::makeMtxUpSidePos(&mHillMtxA, up, side, pos);
    MR::rotateMtxLocalYDegree(mHillMtxA.mMtx, 15.0f * (f32)mIndex);
    if (mBreakA != nullptr) {
        mBreakA->appear();
    }
    if (mHillA != nullptr) {
        mHillA->appear();
    }
    return true;
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
bool BellyDragonBodyPart::tryBindB() {
    if (mNext != nullptr && mNext->isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindA::sInstance)) {
        return false;
    }
    TVec3f target(mBindMtxB.mMtx[0][3], mBindMtxB.mMtx[1][3], mBindMtxB.mMtx[2][3]);
    TVec3f up(mBindMtxB.mMtx[0][1], mBindMtxB.mMtx[1][1], mBindMtxB.mMtx[2][1]);
    f32 radius = mRadius;
    f32 offsetDist = offsetValue(radius, 200.0f) + 100.0f;
    target.add(up.scaleInline(offsetDist));
    f32 nearRadius = mRadius;
    if (!MR::isNear(this, target, offsetValue(nearRadius, 100.0f))) {
        return false;
    }
    bool wasNoBind = MR::isNoBind(this);
    BellyDragonBodyHill* pHill = mPrev->mHillB;
    if (pHill != nullptr) {
        pHill->mIsCheckAppearEnd = true;
        pHill->mIsLongDisappear = false;
    }
    if (mNext != nullptr) {
        mNext->startBindB(mBindMtxB);
    }
    MR::onBind(this);
    TVec3f hillUp = -mGravity;
    TVec3f side;
    mMtx.getXDir(side);
    TVec3f pos(mPosition);
    MR::calcMapHitAlongDirection(mPosition, &pos, mGravity, nullptr, nullptr, nullptr, nullptr, 500.0f);
    MR::makeMtxUpSidePos(&mHillMtxB, hillUp, side, pos);
    MR::rotateMtxLocalYDegree(mHillMtxB.mMtx, 15.0f * (f32)mIndex);
    if (mBreakB != nullptr) {
        mBreakB->appear();
    }
    if (mHillB != nullptr) {
        mHillB->appear();
    }
    if (isHead() || isTailSide()) {
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvFollow::sInstance);
    } else {
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvExitB::sInstance);
    }
    return true;
}

bool BellyDragonBodyPart::isHead() const {
    return mIndex == 0;
}

bool BellyDragonBodyPart::isSecond() const {
    return mIndex == 1;
}

bool BellyDragonBodyPart::isTailSide() const {
    return mIndex >= 24;
}

bool BellyDragonBodyPart::isBindAWeak() const {
    return isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindAWeak::sInstance);
}

bool BellyDragonBodyPart::isBindNerve() const {
    if (isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindA::sInstance) ||
        isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindAWeak::sInstance) ||
        isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvExitA::sInstance) ||
        isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvBindB::sInstance)) {
        return true;
    }
    return false;
}

void BellyDragonBodyPart::stop(bool isHeadPart) {
    if (mIndex > 0) {
        mpTurnSpeed = &sTurnSpeedB;
    }
    MR::zeroVelocity(this);
    MR::clearHitSensors(this);
    MR::requestMovementOn(this);
    if (hasWeakPoint()) {
        mWeakPoint->setNerveWaitB();
    }
    if (mBreakA != nullptr) {
        mBreakA->makeActorDead();
    }
    if (mBreakB != nullptr) {
        mBreakB->makeActorDead();
    }
    if (mHillA != nullptr) {
        mHillA->makeActorDead();
    }
    if (mHillB != nullptr) {
        mHillB->makeActorDead();
    }
    if (!isHeadPart) {
        setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvWait::sInstance);
    }
}

void BellyDragonBodyPart::resume() {
    if (mIndex > 0) {
        mpTurnSpeed = &sTurnSpeedA;
    }
    mIsFrozen = false;
    MR::zeroVelocity(this);
    MR::onBind(this);
    MR::onCalcGravity(this);
    MR::clearHitSensors(this);
    MR::validateHitSensors(this);
    setNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvFollow::sInstance);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
bool BellyDragonBodyPart::calcJoint(TPos3f* pMtx, const JointControllerInfo& rInfo) {
    if (mIsFrozen) {
        JMath::gekko_ps_copy12(&mMtx, pMtx);
        if (mPrev != nullptr) {
            TVec3f negX(-mMtx.mMtx[0][0], -mMtx.mMtx[1][0], -mMtx.mMtx[2][0]);
            TVec3f yDir(mMtx.mMtx[0][1], mMtx.mMtx[1][1], mMtx.mMtx[2][1]);
            TVec3f zDir(mMtx.mMtx[0][2], mMtx.mMtx[1][2], mMtx.mMtx[2][2]);
            mMtx.setXDir(zDir);
            mMtx.setYDir(yDir);
            mMtx.setZDir(negX);
            mMtx.getTrans(mPosition);
        }
        return false;
    }
    TVec3f jointX;
    pMtx->getXDir(jointX);
    if (MR::isNearZero(jointX, 0.001f)) {
        return false;
    }
    TPos3f newMtx;
    newMtx.identity();
    if (mNext != nullptr) {
        TVec3f up;
        mMtx.getYDir(up);
        MtxPtr pNextMtx = mNext->getBaseMtx();
        TVec3f nextPos(pNextMtx[0][3], pNextMtx[1][3], pNextMtx[2][3]);
        TVec3f side(nextPos);
        side.sub(mPosition);
        if (MR::isNearZero(side, 0.001f)) {
            side.set< f32 >(-pNextMtx[0][2], -pNextMtx[1][2], -pNextMtx[2][2]);
        } else {
            MR::normalize(&side);
        }
        if (MR::isSameDirection(side, up, 0.01f)) {
            up.set< f32 >(pNextMtx[0][1], pNextMtx[1][1], pNextMtx[2][1]);
        }
        MR::makeMtxSideUpPos(&newMtx, side, up, mPosition);
    } else {
        TVec3f front;
        TVec3f up;
        mMtx.getYDir(up);
        mMtx.getZDir(front);
        if (mPrev == nullptr) {
            MR::makeMtxFrontUpPos(&newMtx, front, up, mPosition);
        } else {
            MR::rotateVecDegree(&front, up, 90.0f);
            MR::makeMtxFrontUpPos(&newMtx, front, up, mPosition);
        }
    }
    TVec3f scale(1.0f);
    {
        f32 y = pMtx->mMtx[1][0];
        f32 x = pMtx->mMtx[0][0];
        f32 z = pMtx->mMtx[2][0];
        scale.x = JGeometry::TUtil< f32 >::sqrt(x * x + y * y + z * z);
    }
    {
        f32 y = pMtx->mMtx[1][1];
        f32 x = pMtx->mMtx[0][1];
        f32 z = pMtx->mMtx[2][1];
        scale.y = JGeometry::TUtil< f32 >::sqrt(x * x + y * y + z * z);
    }
    {
        f32 y = pMtx->mMtx[1][2];
        f32 x = pMtx->mMtx[0][2];
        f32 z = pMtx->mMtx[2][2];
        scale.z = JGeometry::TUtil< f32 >::sqrt(x * x + y * y + z * z);
    }
    TPos3f orig = *pMtx;
    MR::blendMtx(orig.mMtx, newMtx.mMtx, 1.0f, pMtx->mMtx);
    MR::preScaleMtx(pMtx->mMtx, scale);
    return true;
}

void BellyDragonBodyPart::pushFromSensor(HitSensor* pSender, HitSensor* pReceiver, f32 rate) {
    TVec3f dir(0.0f);
    MR::calcSensorDirection(&dir, pSender, pReceiver);
    f32 mag = PSVECMag(dir);
    f32 receiverRadius = MR::getSensorRadius(pReceiver);
    f32 senderRadius = MR::getSensorRadius(pSender);
    f32 overlap = __fabsf((senderRadius + receiverRadius) - mag);
    MR::normalizeOrZero(&dir);
    dir.scale(overlap);
    TVec3f push(dir);
    push.scale(rate);
    MR::addVelocity(this, push);
}

bool BellyDragonBodyPart::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    return MR::isMsgStarPieceReflect(msg);
}

bool BellyDragonBodyPart::receiveMsgPush(HitSensor* pSender, HitSensor* pReceiver) {
    if (mIsFrozen) {
        return false;
    }
    if (mPrev == nullptr) {
        return false;
    }
    if (MR::isSensorType(pSender, 0x38)) {  // BellyDragon body sensor type (not named yet)
        BellyDragonBodyPart* pOther = static_cast< BellyDragonBodyPart* >(MR::getSensorHost(pSender));
        s32 otherIndex = pOther->mIndex;
        s32 index = mIndex;
        if (index != otherIndex - 1) {
            if (index == otherIndex + 1) {
                pushFromSensor(pSender, pReceiver, 0.05f);
            } else if (!isBindNerve()) {
                if (mIndex < pOther->mIndex) {
                    pushFromSensor(pSender, pReceiver, 0.01f);
                } else {
                    pushFromSensor(pSender, pReceiver, 0.2f);
                }
            }
        }
        return true;
    }
    if (MR::isSensorType(pSender, 0x37)) {  // not named yet
        if (!isHead() && isBindNerve()) {
            return false;
        }
        pushFromSensor(pSender, pReceiver, 0.5f);
        return true;
    }
    if (MR::isSensorType(pSender, 0x39)) {  // not named yet
        BellyDragonWeakPoint* pWeakPoint = static_cast< BellyDragonWeakPoint* >(MR::getSensorHost(pSender));
        s32 diff = pWeakPoint->mPart->mIndex - mIndex;
        if (__abs(diff) <= 2) {
            return false;
        }
        if (isBindNerve()) {
            return false;
        }
        pushFromSensor(pSender, pReceiver, 0.2f);
        return true;
    }
    return false;
}

void BellyDragonBodyPart::control() {
    if (MR::isExistBinder(this)) {
        if (!isNerve(&NrvBellyDragonBodyPart::BellyDragonBodyPartNrvWait::sInstance)) {
            MR::sendMsgEnemyAttackToBindedSensor(this, getSensor("Body"));
        }
    }
    MR::restrictVelocity(this, 100.0f);
}

void BellyDragonBodyPart::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorType(pReceiver, 0x38)) {  // not named yet
        MR::sendMsgPush(pReceiver, pSender);
        return;
    }
    if (MR::isSensorPlayer(pReceiver)) {
        if (MR::isSensorType(pSender, ATYPE_ENEMY_SIMPLE)) {
            if (mIsLv2) {
                if (!MR::sendMsgEnemyAttackFire(pReceiver, pSender)) {
                    MR::sendMsgPush(pReceiver, pSender);
                }
            } else {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else if (!mIsLv2) {
            MR::sendMsgPush(pReceiver, pSender);
        }
    }
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
BellyDragonBody::BellyDragonBody(BellyDragon* pHost, TPos3f* pBaseMtx, bool isLv2) {
    mIsLv2 = isLv2;
    mBaseMtx = pBaseMtx;
    mHead = nullptr;
    mPartNum = 0x20;
    mParts = nullptr;
    mWeakPointNum = 6;
    mWeakPoints = nullptr;
    mBreakHeadHillAppear = nullptr;
    mBreakA = nullptr;
    mBreakB = nullptr;
    mHeadHill = nullptr;
    mBreakHeadHill = nullptr;
    mHillA = nullptr;
    mHillB = nullptr;
    mBreakJawHill = nullptr;
    mBreakJawAppear = nullptr;
    mMtxA.identity();
    mMtxB.identity();
    mMtxC.identity();
    mMtxD.identity();
    BellyDragonBodyPart* pHead = new BellyDragonBodyPart(-1, nullptr, mIsLv2, 350.0f, 1.0f, 1.0f);
    mHead = pHead;
    JMath::gekko_ps_copy12(&pHead->mMtx, mBaseMtx);
    pHead->mMtx.getTrans(pHead->mPosition);
    MR::setPosition(mHead, mBaseMtx->mMtx[0][3], mBaseMtx->mMtx[1][3], mBaseMtx->mMtx[2][3]);
    mHead->mJointCtrl = MR::createJointController< BellyDragonBodyPart >(mHead, pHost, MR::getJointIndex(pHost, "Root"), &BellyDragonBodyPart::calcJoint, nullptr);
    mHead->initWithoutIter();
    mParts = new BellyDragonBodyPart*[mPartNum];
    BellyDragonBodyPart* pPrev = mHead;
    s32 jointIndex = MR::getJointIndex(pHost, "Body00");
    f32 maxRadius = 200.0f;
    f32 minRadius = 100.0f;
    f32 maxScale = 1.0f;
    f32 minScale = 0.6f;
    f32 tailLen = 10.0f;
    s32 easeLen = 0x10;
    for (s32 i = 0; i < mPartNum; i++) {
        f32 rate;
        f32 radius = 200.0f;
        f32 scale = 1.0f;
        if ((f32)(mPartNum - 1 - i) <= tailLen) {
            f32 t = MR::getEaseInValue((f32)(mPartNum - 1 - i) / tailLen, 0.0f, 1.0f, 1.0f);
            radius = minRadius + t * (maxRadius - minRadius);
            scale = minScale + t * (maxScale - minScale);
        }
        rate = 1.0f;
        if ((u32)(i - 7) <= 0x10) {
            rate = MR::getEaseInValue((f32)(i - 7) / (f32)easeLen, 0.0f, 1.0f, 1.0f);
        }
        mParts[i] = new BellyDragonBodyPart(i, pPrev, mIsLv2, radius, scale, rate);
        mParts[i]->mJointCtrl = MR::createJointController< BellyDragonBodyPart >(mParts[i], pHost, (u16)(jointIndex + i), &BellyDragonBodyPart::calcJoint, nullptr);
        mParts[i]->initWithoutIter();
        pPrev = mParts[i];
    }
    for (s32 i = 0; i < mPartNum - 1; i++) {
        mParts[i]->mNext = mParts[i + 1];
    }
    HitSensor* pMessage = pHost->getSensor("Message");
    s32 weakIndices[7] = {4, 8, 12, 16, 20, 24, -1};
    mWeakPoints = new BellyDragonWeakPoint*[mWeakPointNum];
    BellyDragonBodyPart* pPart;
    for (s32 i = 0; i < mWeakPointNum; i++) {
        pPart = mParts[weakIndices[i]];
        mWeakPoints[i] = new BellyDragonWeakPoint(pMessage, pPart);
        mWeakPoints[i]->initWithoutIter();
        pPart->mHillA->mIsDiveSound = true;
        pPart->mHillB->mIsDiveSound = true;
    }
    mBreakA = new BreakModel(mHead, "飛び散り岩（頭の飛び込み）", "BellyDragonHoleBreak", mMtxC.mMtx, "Break");
    mBreakA->initWithoutIter();
    MR::setScale(mBreakA, 1.5f);
    mBreakB = new BreakModel(mHead, "頭の塚の破壊（頭の飛び込み）", "BellyDragonHeadHillBreak", mMtxC.mMtx, "Break");
    mBreakB->initWithoutIter();
    mHeadHill = MR::createModelObjMapObjDecoration("頭の塚", "BellyDragonHeadHill", mMtxB.mMtx);
    mHeadHill->makeActorDead();
    mBreakHeadHill = new BreakModel(mHead, "頭の塚の破壊", "BellyDragonHeadHillBreak", mMtxB.mMtx, "Break");
    mBreakHeadHill->initWithoutIter();
    mBreakHeadHillAppear = new BreakModel(mHead, "飛び散り岩（頭の塚の出現時）", "BellyDragonHoleBreak", mMtxB.mMtx, "Break");
    mBreakHeadHillAppear->initWithoutIter();
    mHillA = new BellyDragonBodyHill("首の塚（飛び込み）", mMtxA.mMtx, true, false);
    mHillA->initWithoutIter();
    MR::setScale(mHillA, 1.2f);
    mHillB = new BellyDragonBodyHill("首の塚（飛び出し）", mMtxB.mMtx, false, false);
    mHillB->initWithoutIter();
    MR::setScale(mHillB, 1.2f);
    mBreakJawHill = new BreakModel(mHead, "顎の塚", "BellyDragonJawHill", mMtxD.mMtx, "Appear");
    mBreakJawHill->initWithoutIter();
    mBreakJawAppear = new BreakModel(mHead, "飛び散り岩（顎の塚の出現時）", "BellyDragonHoleBreak", mMtxA.mMtx, "Break");
    mBreakJawAppear->initWithoutIter();
    MR::setScale(mBreakJawAppear, 1.2f);
}

void BellyDragonBody::appear() {
    mHead->appear();
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->appear();
    }
    for (s32 i = 0; i < mWeakPointNum; i++) {
        mWeakPoints[i]->appear();
    }
}

void BellyDragonBody::kill() {
    mHead->kill();
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->kill();
    }
}

void BellyDragonBody::end() {
    unfreeze();
    mHead->stop(true);
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->stop(false);
    }
    mBreakHeadHillAppear->makeActorDead();
    mHeadHill->makeActorDead();
    mBreakHeadHill->makeActorDead();
    mHillA->makeActorDead();
    mHillB->makeActorDead();
    mBreakJawHill->makeActorDead();
    mBreakJawAppear->makeActorDead();
    mBreakA->makeActorDead();
    mBreakB->makeActorDead();
}

void BellyDragonBody::freeze() {
    mHead->mIsFrozen = true;
    MR::zeroVelocity(mHead);
    MR::invalidateHitSensors(mHead);
    MR::clearHitSensors(mHead);
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->mIsFrozen = true;
        MR::zeroVelocity(mParts[i]);
        MR::invalidateHitSensors(mParts[i]);
    }
}

void BellyDragonBody::unfreeze() {
    mHead->mIsFrozen = false;
    MR::validateHitSensors(mHead);
    MR::clearHitSensors(mHead);
    MR::zeroVelocity(mHead);
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->resume();
    }
}

void BellyDragonBody::setAttackable() {
    for (s32 i = 0; i < mPartNum; i++) {
        BellyDragonBodyPart* pPart = mParts[i];
        pPart->mIsAttackable = true;
        pPart->mpBindStep = &sBindStepB;
    }
}

void BellyDragonBody::update() {
    updateTailSpray();
    mHead->movement();
    TVec3f pos(mBaseMtx->mMtx[0][3], mBaseMtx->mMtx[1][3], mBaseMtx->mMtx[2][3]);
    MR::setPosition(mHead, pos);
    BellyDragonBodyPart* pHead = mHead;
    JMath::gekko_ps_copy12(&pHead->mMtx, mBaseMtx);
    pHead->mMtx.getTrans(pHead->mPosition);
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->movement();
        mParts[i]->updateDirection();
    }
    for (s32 i = 0; i < mWeakPointNum; i++) {
        if (!MR::isDead(mWeakPoints[i])) {
            mWeakPoints[i]->movement();
            mWeakPoints[i]->updateMtx();
        }
    }
}

void BellyDragonBody::registerJointCallBacks() {
    mHead->mJointCtrl->registerCallBack();
    for (s32 i = 0; i < mPartNum; i++) {
        mParts[i]->mJointCtrl->registerCallBack();
    }
}

void BellyDragonBody::appearJawHill() {
    BellyDragonUtil::calcGroundHillMtx(&mMtxD, mHead, mBaseMtx->mMtx, 1000.0f);
    BellyDragonUtil::calcGroundHillMtx(&mMtxA, mHead, mBaseMtx->mMtx, 700.0f);
    mBreakJawHill->appear();
    mBreakJawAppear->appear();
    MR::requestMovementOn(mBreakJawHill);
    MR::requestMovementOn(mBreakJawAppear);
}

void BellyDragonBody::startHeadAttackHill() {
    BellyDragonUtil::calcGroundHillMtx(&mMtxA, mHead, mBaseMtx->mMtx, 300.0f);
    BellyDragonUtil::calcGroundHillMtx(&mMtxC, mHead, mBaseMtx->mMtx, 300.0f);
    mHillA->appear();
    MR::startActionSound(mHillA, "HeadAttack", -1, -1, -1);
    BellyDragonBodyHill* pHill = mHillA;
    pHill->mIsCheckAppearEnd = true;
    pHill->mIsLongDisappear = false;
    mBreakA->appear();
    mBreakB->appear();
    JMath::gekko_ps_copy12(&mHead->mBindMtxA, &mMtxA);
    mParts[0]->startBindA(mMtxA);
}

void BellyDragonBody::appearHeadHill() {
    BellyDragonUtil::calcGroundHillMtx(&mMtxB, mHead, mBaseMtx->mMtx, 1000.0f);
    mBreakHeadHillAppear->appear();
    mHeadHill->appear();
    MR::startAction(mHeadHill, "Appear");
    JMath::gekko_ps_copy12(&mHead->mBindMtxB, &mMtxB);
}

void BellyDragonBody::breakHeadHill() {
    mBreakHeadHillAppear->appear();
    mHeadHill->kill();
    mBreakHeadHill->appear();
    mHead->mHillB->appear();
    mHillB->appear();
    BellyDragonBodyHill* pHill = mHillB;
    pHill->mIsCheckAppearEnd = true;
    pHill->mIsLongDisappear = true;
    MR::validateHitSensors(mHead);
}

void BellyDragonBody::startBindB() {
    mParts[0]->startBindB(mMtxB);
}

void BellyDragonBody::forceDeleteEffects() {
    MR::tryForceDeleteEffectAll(mHead);
    for (s32 i = 0; i < mPartNum; i++) {
        MR::tryForceDeleteEffectAll(mParts[i]);
    }
    MR::tryKillAndForceDeleteEffectAll(mBreakHeadHillAppear);
    MR::tryKillAndForceDeleteEffectAll(mHeadHill);
    MR::tryKillAndForceDeleteEffectAll(mBreakHeadHill);
    MR::tryKillAndForceDeleteEffectAll(mHillA);
    MR::tryKillAndForceDeleteEffectAll(mHillB);
    MR::tryKillAndForceDeleteEffectAll(mBreakJawHill);
    MR::tryKillAndForceDeleteEffectAll(mBreakJawAppear);
    MR::tryKillAndForceDeleteEffectAll(mBreakA);
    MR::tryKillAndForceDeleteEffectAll(mBreakB);
}

void BellyDragonBody::updateTailSpray() {
    BellyDragonBodyPart* pPart;
    for (s32 i = mPartNum - 1; i >= 0; i--) {
        if (mParts[i]->isBindAWeak()) {
            for (s32 k = 0; (f32)k < 5.0f; k++) {
                s32 j = (i - 1) - k;
                if (j >= 0) {
                    pPart = mParts[j];
                    f32 rate = 1.0f - (f32)k / 5.0f;
                    pPart->pullTailToMtx(mParts[j + 1]->getBaseMtx(), rate);
                }
            }
        }
    }
}
