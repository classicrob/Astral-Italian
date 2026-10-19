#include "Game/Boss/Sandy.hpp"
// SMG2: MR::appearCoinPop returns bool (ObjUtil.hpp declares it void), declare the real one here.
#define appearCoinPop appearCoinPopUnused
#include "Game/Util.hpp"
#undef appearCoinPop
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
static inline void mulScaleMini(register const f32* a, register const f32* b, register f32* d) {
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

// Single-use copy of TRotation3::getEulerXYZ followed by the radian->degree scale (see mulScaleSandy).
static inline void calcEulerMini(const TPos3f& rMtx, TVec3f& rDest) {
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
        tmp.x = px * k;
        tmp.y = py * k;
        tmp.z = pz * k;
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

namespace NrvSandyMini {
    NEW_NERVE(SandyMiniNrvStart, SandyMini, Start);
    NEW_NERVE(SandyMiniNrvWait, SandyMini, Wait);
    NEW_NERVE(SandyMiniNrvDamage, SandyMini, Damage);
    NEW_NERVE(SandyMiniNrvDown, SandyMini, Down);
    NEW_NERVE(SandyMiniNrvRecover, SandyMini, Recover);
    NEW_NERVE(SandyMiniNrvShot, SandyMini, Shot);
    NEW_NERVE(SandyMiniNrvShotEnd, SandyMini, ShotEnd);

    class SandyMiniNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
        }
        static SandyMiniNrvStop sInstance;
    };
    SandyMiniNrvStop SandyMiniNrvStop::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvSandyMini

static f32 sMiniShotRate = 1.0f;
static TVec3f sMiniMouthOffset(100.0f, 0.0f, 0.0f);

// 800FF0C0
SandyMini::SandyMini(Sandy* pSandy, const char* pName, FixedPosition* pFixedPos)
    : LiveActor(pName), mSandy(pSandy), mFixedPos(pFixedPos), mBall(nullptr), mBallCount(0), mWaitStep(0), mNextNerve(nullptr),
      mScaleController(nullptr), mScaleParam(nullptr) {
    mQuat.set(0.0f, 0.0f, 0.0f, 1.0f);
}

// 800FF150
void SandyMini::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SandyMini", false);
    initNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance, 0);
    MR::registerDemoSimpleCastAll(this);
    mScaleParam = createScaleParam();
    mScaleController = new AnimScaleController(mScaleParam);
    makeActorDead();
}

// 800FF1E0
void SandyMini::appear() {
    LiveActor::appear();
    mBallCount = 0;
    setNerve(&NrvSandyMini::SandyMiniNrvStart::sInstance);
}

// 800FF220
void SandyMini::kill() {
    mBallCount++;
    LiveActor::kill();
}

// 800FF230
void SandyMini::setRecover() {
    mBallCount = 0;
    setNerve(&NrvSandyMini::SandyMiniNrvRecover::sInstance);
}

// 800FF240
void SandyMini::setNextNerve() {
    if (isNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance) || isNerve(&NrvSandyMini::SandyMiniNrvDamage::sInstance)) {
        mNextNerve = &NrvSandyMini::SandyMiniNrvDown::sInstance;
    } else {
        mNextNerve = &NrvSandyMini::SandyMiniNrvWait::sInstance;
    }
    setNerve(&NrvSandyMini::SandyMiniNrvStop::sInstance);
}

// 800FF2B0
void SandyMini::resume() {
    if (mNextNerve != nullptr) {
        setNerve(mNextNerve);
    } else {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FF2D0
bool SandyMini::isNerveStop() const {
    return isNerve(&NrvSandyMini::SandyMiniNrvStop::sInstance);
}

// 800FF2E0
bool SandyMini::isDownOrDead() const {
    if (isNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance)) {
        return true;
    }
    if (isNerve(&NrvSandyMini::SandyMiniNrvStop::sInstance)) {
        return mNextNerve == &NrvSandyMini::SandyMiniNrvDown::sInstance;
    }
    return MR::isDead(this);
}

// 800FF360
bool SandyMini::isShot() const {
    bool ret = false;
    if (isNerve(&NrvSandyMini::SandyMiniNrvShot::sInstance) || isNerve(&NrvSandyMini::SandyMiniNrvShotEnd::sInstance)) {
        ret = true;
    }
    return ret;
}

// 800FF3C0
void SandyMini::control() {
    mScaleController->updateNerve();
}

// 800FF3D0
void SandyMini::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pSender, this, "Body") && MR::isSensorPlayerOrRide(pReceiver) && MR::sendMsgPush(pReceiver, pSender)) {
        mScaleController->startHitReaction();
        MR::startActionSound(this, "Touch", -1, -1, -1);
    }
}

// 800FF470
// NONMATCHING: register numbers of the two ps_mul operands.
void SandyMini::calcAndSetBaseMtx() {
    TVec3f scale;
    mulScaleMini(&mScaleController->mScale.x, &mScale.x, &scale.x);
    MR::setBaseScale(this, scale);
    MR::setBaseTRMtx(this, mQuat);
}

// 800FF4D0
bool SandyMini::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        mScaleController->startHitReaction();
        return true;
    }
    if (!MR::isEqualSensor(pReceiver, this, "Mouth")) {
        return false;
    }
    if (!MR::isMsgFireBallAttack(msg)) {
        return false;
    }
    if (!canReceive()) {
        return false;
    }
    setNerve(&NrvSandyMini::SandyMiniNrvDamage::sInstance);
    return true;
}

// 800FF590
bool SandyMini::canReceive() const {
    if (isNerve(&NrvSandyMini::SandyMiniNrvDamage::sInstance)) {
        return false;
    }
    if (isNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance)) {
        return false;
    }
    return !isNerve(&NrvSandyMini::SandyMiniNrvRecover::sInstance);
}

// 800FF600
// NONMATCHING: position/euler inline expansion differs (operator* and TVec3 copy are expanded inline in the target).
void SandyMini::calcPosFromFixedPos() {
    mFixedPos->calc();
    mFixedPos->copyTrans(&mPosition);
    calcEulerMini(mFixedPos->mMtx, mRotation);
    mFixedPos->mMtx.getQuat(mQuat);
}

// 800FF770
void SandyMini::startWait() {
    MR::startAction(this, "WaitNoInterpole");
    setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
}

// 800FF7B0
void SandyMini::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "DemoSandyPowerUp");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FF810
void SandyMini::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "WaitNormal");
        MR::startBrkAndSetFrameAndStop(this, "Damage", 0.0f);
    }
    if (MR::isGreaterEqualStep(this, mWaitStep + 90)) {
        mWaitStep = 0;
        setNerve(&NrvSandyMini::SandyMiniNrvShot::sInstance);
    }
}

// 800FF8A0
void SandyMini::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Damage");
        if (mBall != nullptr && !MR::isDead(mBall)) {
            mBall->tryBreak();
        }
    }
    if (MR::isActionEnd(this)) {
        MR::tryHitReactionDead(this);
        setNerve(&NrvSandyMini::SandyMiniNrvDown::sInstance);
    }
}

// 800FF930
void SandyMini::exeDown() {
    if (MR::isFirstStep(this)) {
        mBallCount++;
        MR::startAction(this, "DamageWait");
    }
}

// 800FF980
void SandyMini::exeRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Recover");
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FF9E0
void SandyMini::exeShot() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Shot");
        MR::setBckRate(this, sMiniShotRate);
    }
    s32 shotFrame = 14;
    if (MR::isStep(this, (s32)((f32)shotFrame / sMiniShotRate))) {
        mBallCount++;
        mBall = takeBall();
        mBall->appear();
        MR::zeroVelocity(mBall);
    }
    if (MR::isGreaterEqualStep(this, (s32)((f32)shotFrame / sMiniShotRate))) {
        attachBallToJoint(this, mBall, "Mouth1", sMiniMouthOffset);
    }
    s32 endFrame = 30;
    if (MR::isGreaterEqualStep(this, (s32)((f32)endFrame / sMiniShotRate))) {
        setNerve(&NrvSandyMini::SandyMiniNrvShotEnd::sInstance);
    }
}

// 800FFB60
void SandyMini::exeShotEnd() {
    if (MR::isFirstStep(this)) {
        mBall->tryShoot();
        mBall = nullptr;
    }
    if (MR::isActionEnd(this)) {
        setNerve(&NrvSandyMini::SandyMiniNrvWait::sInstance);
    }
}

// 800FFBC0
SandyMini::~SandyMini() {
}
