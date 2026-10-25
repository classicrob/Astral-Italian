#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#else
#include <functional.hpp>
#endif
#include "Game/Boss/Sandy.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/Spine.hpp"
#include "Game/Util/FixedPosition.hpp"
#include <cstdio>

// Single-use inline copy of JGeometry::mulInternal (CW size mode only inlines functions with one call site; the target
// had each of these in its own translation unit).
static inline void mulScaleHand(register const f32* a, register const f32* b, register f32* d) {
#ifdef GALAXY_NATIVE
    const f32 x = a[0] * b[0];
    const f32 y = a[1] * b[1];
    d[0] = x;
    d[1] = y;
#else
    register f32 xy1, xy2, res;
    __asm {
        psq_l xy1, 0(a), 0, 0
        psq_l xy2, 0(b), 0, 0
        ps_mul res, xy1, xy2
        psq_st res, 0(d), 0, 0
    }
#endif
    d[2] = a[2] * b[2];
}

static inline TVec3f calcSandyHandScale(const TVec3f& actorScale, const TVec3f& animationScale) {
    TVec3f scale;
    mulScaleHand(&animationScale.x, &actorScale.x, &scale.x);
    return scale;
}

// Single-use copy of TRotation3::getEulerXYZ followed by the radian->degree scale (see mulScaleSandy).
static inline void calcEulerHand(const TPos3f& rMtx, TVec3f& rDest) {
    const f32 zx = rMtx.mMtx[2][0];
    if (zx - 1.0f >= -(f32)JGeometry::TUtil< f32 >::epsilon()) {
        rDest.set(JMAATan2(-rMtx.mMtx[0][1], rMtx.mMtx[1][1]), -HALF_PI, 0.0f);
    } else {
        f32 f1 = 1.0f;
        if (zx + f1 <= (f32)JGeometry::TUtil< f32 >::epsilon()) {
            rDest.set(JMAATan2(rMtx.mMtx[0][1], rMtx.mMtx[1][1]), HALF_PI, 0.0f);
        } else {
            rDest.x = JMAATan2(rMtx.mMtx[2][1], rMtx.mMtx[2][2]);
            rDest.z = JMAATan2(rMtx.mMtx[1][0], rMtx.mMtx[0][0]);
            rDest.y = JGeometry::TUtil< f32 >::asin(-rMtx.mMtx[2][0]);
        }
    }
    {
        TVec3f tmp;
        const register f32* a = &rDest.x;
        register f32* b = &tmp.x;
        register f32 xy;
#ifdef GALAXY_NATIVE
        tmp.x = rDest.x;
        tmp.y = rDest.y;
#else
        __asm {
            psq_l xy, 0(a), 0, 0
            psq_st xy, 0(b), 0, 0
        }
#endif
        f32 k = 57.29578f;
        f32 px = tmp.x;
        f32 py = tmp.y;
        f32 pz = rDest.z;
        tmp.set(k * px, k * py, k * pz);
        const register f32* c = &tmp.x;
        register f32* d = &rDest.x;
        register f32 xy2;
#ifdef GALAXY_NATIVE
        rDest.x = tmp.x;
        rDest.y = tmp.y;
#else
        __asm {
            psq_l xy2, 0(c), 0, 0
            psq_st xy2, 0(d), 0, 0
        }
#endif
        rDest.z = tmp.z;
    }
}

// Inline identity (the target stores the 12 elements directly, TMtx34::identity is out of line in our header).


namespace NrvSandyHand {
    NEW_NERVE(SandyHandNrvWait, SandyHand, Wait);
    NEW_NERVE(SandyHandNrvThrow, SandyHand, Throw);
    NEW_NERVE(SandyHandNrvThrowBall, SandyHand, ThrowBall);
    NEW_NERVE(SandyHandNrvThrowEnd, SandyHand, ThrowEnd);

    class SandyHandNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyHandNrvStop sInstance;
    };
    SandyHandNrvStop SandyHandNrvStop::sInstance ATTRIBUTE_WEAK;

    class SandyHandNrvDamage : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            SandyHand* pActor = reinterpret_cast< SandyHand* >(pSpine->mExecutor);
            if (MR::isBckOneTimeAndStopped(pActor)) {
                pActor->setNerve(pActor->mPrevNerve);
            }
        }
        static SandyHandNrvDamage sInstance;
    };
    SandyHandNrvDamage SandyHandNrvDamage::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSandyHand

static f32 sThrowRate = 1.0f;
static TVec3f sHandBallOffset(140.0f, -120.0f, 5.0f);

static inline char getCullMode(bool isLeft) {
    char type = 1;
    if (isLeft) {
        type = 2;
    }
    return type;
}

// 800FE4A0
// NONMATCHING: register allocation in the ctor init list.
SandyHand::SandyHand(Sandy* pSandy, bool isLeft, SandyBallHolder* pHolder, FixedPosition* pFixedPos)
    : LiveActor(isLeft ? "\203T\203\223\203f\203B\201[\202\314\215\266\216\350" : "\203T\203\223\203f\203B\201[\202\314\211E\216\350"), mSandy(pSandy),
      mHolder(pHolder), mFixedPos(pFixedPos), mBall(nullptr), mIsRight(!isLeft), mIsThrower(isLeft), mCullMode(getCullMode(isLeft)), mWaitStep(30),
      mThrowCount(0), mBallType(2), mPrevNerve(nullptr), _B4(0), mScaleController(nullptr), mScaleParam(nullptr) {
}

// 800FE570
void SandyHand::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SandyHand", mIsRight);
    if (mIsRight) {
        MR::initDLMakerCullMode(this, &mCullMode);
        MR::newDifferedDLBuffer(this);
        mScale.x *= -1.0f;
    }
    initNerve(&NrvSandyHand::SandyHandNrvWait::sInstance, 0);
    MR::registerDemoSimpleCastAll(this);
    mScaleParam = createScaleParam();
    mScaleController = new AnimScaleController(mScaleParam);
    makeActorDead();
}

// 800FE630
void SandyHand::appear() {
    LiveActor::appear();
    startActionWithBrk("WaitNormal");
    MR::startBrk(this, "Damage");
    MR::setBrkFrameEndAndStop(this);
    setNerve(&NrvSandyHand::SandyHandNrvStop::sInstance);
}

// 800FE690
void SandyHand::forceWait() {
    if (mIsThrower) {
        if (mBall != nullptr && !MR::isDead(mBall)) {
            mBall->kill();
        }
    }
    setNerve(&NrvSandyHand::SandyHandNrvStop::sInstance);
}

// 800FE700
void SandyHand::setWait() {
    setNerve(&NrvSandyHand::SandyHandNrvWait::sInstance);
}

// 800FE710
void SandyHand::setBallType0() {
    mBallType = 0;
}

// 800FE720
void SandyHand::setBallType3() {
    mBallType = 3;
}

// 800FE730
void SandyHand::calcAndSetBaseMtx() {
    TVec3f scale = calcSandyHandScale(mScale, mScaleController->mScale);
    MR::setBaseScale(this, scale);
    LiveActor::calcAndSetBaseMtx();
}

// 800FE790
// NONMATCHING: Euler/vector scale floating-point registers and instruction scheduling differ.
void SandyHand::control() {
    mFixedPos->calc();
    mFixedPos->copyTrans(&mPosition);
    calcEulerHand(mFixedPos->mMtx, mRotation);
    mScaleController->updateNerve();
}

// 800FE8F0
bool SandyHand::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        return mSandy->tryStartBlur();
    }
    return false;
}

// 800FE940
bool SandyHand::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg) || MR::isMsgFireBallAttack(msg)) {
        mScaleController->startHitReaction();
        return true;
    }
    return false;
}

// 800FE9B0
void SandyHand::startActionWithBrk(const char* pName) {
    if (!mIsRight) {
        MR::startAction(this, pName);
        return;
    }
    char buf[256];
    snprintf(buf, 256, "%sR", pName);
    if (!MR::tryStartAllAnim(this, buf)) {
        MR::startAction(this, pName);
    }
}

// 800FEA30
void SandyHand::startWaitAndSetNerve(bool isStop) {
    startActionWithBrk("WaitNoInterpole");
    setNerve(isStop ? static_cast< const Nerve* >(&NrvSandyHand::SandyHandNrvStop::sInstance) : &NrvSandyHand::SandyHandNrvWait::sInstance);
}

// 800FEA90
void SandyHand::setDamageAction(const char* pName) {
    if (!isNerve(&NrvSandyHand::SandyHandNrvDamage::sInstance)) {
        mPrevNerve = mSpine->getCurrentNerve();
    }
    setNerve(&NrvSandyHand::SandyHandNrvDamage::sInstance);
    startActionWithBrk(pName);
}

// 800FEB00
void SandyHand::exeWait() {
    if (MR::isFirstStep(this)) {
        startActionWithBrk("WaitNormal");
    }
    if (mIsThrower && MR::isGreaterStep(this, mWaitStep)) {
        mBall = mHolder->getBall(this, mBallType);
        setNerve(&NrvSandyHand::SandyHandNrvThrow::sInstance);
    }
}

// 800FEB90
void SandyHand::exeThrow() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Throw");
        MR::setBckRate(this, 1.0f);
    }
    s32 frame = 14;
    if (MR::isGreaterEqualStep(this, (s32)((f32)frame / sThrowRate))) {
        mBall->appear();
        MR::zeroVelocity(mBall);
        mThrowCount++;
        setNerve(&NrvSandyHand::SandyHandNrvThrowBall::sInstance);
    }
}

// 800FEC60
// NONMATCHING: initializes discarded offset rotation for native safety; matrix-copy outlining and register scheduling differ.
void SandyHand::calcBallPos() {
    if (mBall != nullptr && !mBall->isFree()) {
        TPos3f joint;
        joint.set(MR::getJointMtx(this, "Hand1"));
        TPos3f offset;
#ifdef GALAXY_NATIVE
        // Initialize the rotation that the Wii routine leaves unspecified.
        offset.makeTrans(sHandBallOffset);
#else
        offset.setTrans(sHandBallOffset);
#endif
        TPos3f result;
        result.concat(joint, offset);
        TVec3f pos;
        result.getTrans(pos);
        MR::setPosition(mBall, pos);

        result.mMtx[0][3] = 0.0f;
        result.mMtx[1][3] = 0.0f;
        result.mMtx[2][3] = 0.0f;
        result.mMtx[0][0] = 1.0f;
        result.mMtx[0][1] = 0.0f;
        result.mMtx[0][2] = 0.0f;
        result.mMtx[1][0] = 0.0f;
        result.mMtx[1][1] = 1.0f;
        result.mMtx[1][2] = 0.0f;
        result.mMtx[2][0] = 0.0f;
        result.mMtx[2][1] = 0.0f;
        result.mMtx[2][2] = 1.0f;

        result.getQuat(mBall->mQuat);
    }
}

// 800FED80
void SandyHand::exeThrowBall() {
    calcBallPos();
    if (MR::checkPassBckFrame(this, 58.0f)) {
        if (!mIsRight) {
            MR::startActionSound(this, "ThrowL", -1, -1, -1);
        } else {
            MR::startActionSound(this, "ThrowR", -1, -1, -1);
        }
    }
    s32 total = 59;
    s32 frame = 14;
    if (MR::isGreaterEqualStep(this, (s32)((f32)total / sThrowRate) - (s32)((f32)frame / sThrowRate))) {
        setNerve(&NrvSandyHand::SandyHandNrvThrowEnd::sInstance);
    }
}

// 800FEE90
void SandyHand::exeThrowEnd() {
    if (MR::isFirstStep(this)) {
        mBall->tryShoot();
        mBall = nullptr;
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyHand::SandyHandNrvWait::sInstance);
    }
}

// 800FEEF0
SandyHand::~SandyHand() {
}
