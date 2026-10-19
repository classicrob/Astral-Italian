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

// Call a pointer-to-member on every element of a range (std::for_each like).
template < typename It, typename F >
F ForEachPtmf(It first, It last, F f) {
    for (; first != last; ++first) {
        ((*first)->*f)();
    }
    return f;
}

// Single-use inline copy of JGeometry::mulInternal (CW size mode only inlines functions with one call site; the target
// had each of these in its own translation unit).
static inline void mulScaleHand(register const f32* a, register const f32* b, register f32* d) {
    register f32 xy1, xy2, res;
    __asm {
        psq_l xy1, 0(a), 0, 0
        psq_l xy2, 0(b), 0, 0
        ps_mul res, xy1, xy2
        psq_st res, 0(d), 0, 0
    }
    d[2] = a[2] * b[2];
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
        __asm {
            psq_l xy, 0(a), 0, 0
            psq_st xy, 0(b), 0, 0
        }
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
        __asm {
            psq_l xy2, 0(c), 0, 0
            psq_st xy2, 0(d), 0, 0
        }
        rDest.z = tmp.z;
    }
}

// Inline identity (the target stores the 12 elements directly, TMtx34::identity is out of line in our header).


static const bool sCactusTable[20] = {false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false};
static u32 sBallCount = 0;

// 800FD1F0
SandyBallHolder::SandyBallHolder(Sandy* pSandy)
    : NameObj("\203T\203\223\203f\203B\201[\202\314\203T\203{\203e\203\223\203{\201[\203\213\225\333\216\235"), mSandy(pSandy) {
    mBalls0.init(48);
    for (SandyBall** p = mBalls0.begin(); p != mBalls0.end(); p++) {
        *p = new SandyBall(0, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls1.init(32);
    for (SandyBall** p = mBalls1.begin(); p != mBalls1.end(); p++) {
        *p = new SandyBall(2, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls2.init(16);
    for (SandyBall** p = mBalls2.begin(); p != mBalls2.end(); p++) {
        *p = new SandyBall(1, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
        MR::setPosition(*p, mSandy->mPosition);
    }
    mBalls3.init(16);
    for (SandyBall** p = mBalls3.begin(); p != mBalls3.end(); p++) {
        *p = new SandyBall(3, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
    }
    mBalls4.init(2);
    for (SandyBall** p = mBalls4.begin(); p != mBalls4.end(); p++) {
        *p = new SandyBall(4, "\203T\203{\203e\203\223\203{\201[\203\213");
        (*p)->initWithoutIter();
    }
}

// 800FD480
SandyBall* SandyBallHolder::getBall(LiveActor* pOwner, s32 type) {
    if (type == 2) {
        return getFreeBallType2(pOwner);
    }
    if (type == 3) {
        return getFreeBallType3(pOwner);
    }
    if (type == 4) {
        return getFreeBallType4(pOwner);
    }
    u32 idx = sBallCount;
    if (idx >= 20) {
        idx = 0;
    }
    sBallCount = idx + 1;
    if (sCactusTable[idx] && !MR::isGalaxyDarkCometAppearInCurrentStage()) {
        return getFreeBallType1(pOwner);
    }
    return getFreeBall(pOwner);
}

// 800FD540
SandyBall* SandyBallHolder::getFreeBall(LiveActor* pOwner) {
    for (SandyBall** p = mBalls0.begin(); p != mBalls0.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return nullptr;
}

// 800FD5C0
SandyBall* SandyBallHolder::getFreeBallType2(LiveActor* pOwner) {
    for (SandyBall** p = mBalls1.begin(); p != mBalls1.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return nullptr;
}

// 800FD640
SandyBall* SandyBallHolder::getFreeBallType1(LiveActor* pOwner) {
    for (SandyBall** p = mBalls2.begin(); p != mBalls2.end(); p++) {
        if ((*p)->isFree() && !(*p)->mCoinPopped) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD6E0
SandyBall* SandyBallHolder::getFreeBallType3(LiveActor* pOwner) {
    for (SandyBall** p = mBalls3.begin(); p != mBalls3.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD770
SandyBall* SandyBallHolder::getFreeBallType4(LiveActor* pOwner) {
    for (SandyBall** p = mBalls4.begin(); p != mBalls4.end(); p++) {
        if ((*p)->isFree()) {
            (*p)->setOwner(pOwner);
            return *p;
        }
    }
    return getFreeBall(pOwner);
}

// 800FD800
// NONMATCHING: PTMF for_each calls (two stack copies of the member-function pointer in the target).
void SandyBallHolder::killAll() {
    ForEachPtmf(mBalls0.begin(), mBalls0.end(), &SandyBall::tryKill);
    ForEachPtmf(mBalls1.begin(), mBalls1.end(), &SandyBall::tryKill);
    ForEachPtmf(mBalls2.begin(), mBalls2.end(), &SandyBall::tryKill);
    ForEachPtmf(mBalls3.begin(), mBalls3.end(), &SandyBall::tryKill);
    ForEachPtmf(mBalls4.begin(), mBalls4.end(), &SandyBall::tryKill);
}

// 800FDA70
SandyBallHolder::~SandyBallHolder() {
}

// 800FDB10
// NONMATCHING: register allocation (string/new pointer registers swapped).
SandyMini* Sandy::createMini(s32 index, const TVec3f& rPos, const TVec3f& rRot) {
    const char* pName = "\203T\203\223\203f\203B\201[\203~\203M\201[";
    if (index == 0) {
        pName = "\203T\203\223\203f\203B\201[\203q\203_\203\212\201[";
    }
    SandyMini* pMini = new SandyMini(this, pName, new FixedPosition(mMtx, rPos, rRot));
    pMini->initWithoutIter();
    return pMini;
}

// 800FDBC0
SandyHand* Sandy::createHand(s32 index, const TVec3f& rPos, const TVec3f& rRot) {
    bool isLeft = index == 0;
    TVec3f pos(rPos);
    TVec3f rot(rRot);
    if (!isLeft) {
        pos.z *= -1.0f;
    }
    SandyHand* pHand = new SandyHand(this, isLeft, mHolder, new FixedPosition(MR::getJointMtx(this, "Spine2"), pos, rot));
    pHand->initWithoutIter();
    pHand->makeActorDead();
    return pHand;
}

// 800FDCC0
SandyBall* SandyMini::takeBall() {
    return mSandy->mHolder->getBall(this, 0);
}

// 800FDCE0
// NONMATCHING: initializes discarded offset rotation for native safety; matrix-copy outlining and register scheduling differ.
void attachBallToJoint(const LiveActor* pActor, SandyBall* pBall, const char* pJoint, const TVec3f& rOffset) {
    TPos3f joint;
    JMath::gekko_ps_copy12(&joint, MR::getJointMtx(pActor, pJoint));
    TPos3f offset;
    offset.makeTrans(rOffset);
    TPos3f result;
    result.concat(joint, offset);
    TVec3f pos;
    pos.set<f32>(result.mMtx[0][3], result.mMtx[1][3], result.mMtx[2][3]);
    MR::setPosition(pBall, pos);

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

    result.getQuat(pBall->mQuat);
}

// 800FDDE0
bool Sandy::isSensorBack(HitSensor* pSensor) const {
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TVec3f dir = *MR::getSensorPos(pSensor) - mPosition;
    if (MR::normalizeOrZero(&dir)) {
        return false;
    }
    return front.dot(dir) < 0.0f;
}

// 800FDE70
void Sandy::copyQuat() {
    mTargetQuat.x = mQuat.x;
    mTargetQuat.y = mQuat.y;
    mTargetQuat.z = mQuat.z;
    mTargetQuat.w = mQuat.w;
}

// 800FDEA0
void turnToPlayer(TQuat4f* pQuat, LiveActor* pActor, f32 degree) {
    TVec3f dir = *MR::getPlayerPos() - pActor->mPosition;
    MR::vecKillElement(dir, pActor->mGravity, &dir);
    MR::normalize(&dir);
    TVec3f front;
    pQuat->getZDir(front);
    MR::turnDirectionToPlayerDegreeHorizon(pActor, &front, degree);
    TVec3f up = -pActor->mGravity;
    MR::makeQuatUpFront(pQuat, up, front);
}

// 800FDFB0
// NONMATCHING: operand order of the final add (register allocation).
bool Sandy::isTotalCount(s32 count) const {
    s32 rightHand = getRightHand()->mThrowCount;
    s32 leftHand = getLeftHand()->mThrowCount;
    s32 leftMini = getLeftMini()->mBallCount;
    return count == leftMini + leftHand + (rightHand + getRightMini()->mBallCount);
}

// 800FE030
void Sandy::resetCount() {
    SandyHand* pLeftHand = getLeftHand();
    pLeftHand->mThrowCount = 0;
    SandyHand* pRightHand = getRightHand();
    pRightHand->mThrowCount = 0;
    SandyMini* pLeftMini = getLeftMini();
    pLeftMini->mBallCount = 0;
    SandyMini* pRightMini = getRightMini();
    pRightMini->mBallCount = 0;
}

// 800FE090
void Sandy::validateHands() {
    SandyHand* pLeft = getLeftHand();
    pLeft->mIsThrower = true;
    SandyHand* pRight = getRightHand();
    pRight->mIsThrower = true;
}

// 800FE0E0
void Sandy::invalidateHands() {
    SandyHand* pLeft = getLeftHand();
    pLeft->mIsThrower = false;
    SandyHand* pRight = getRightHand();
    pRight->mIsThrower = false;
}

// 800FE130
void Sandy::forceWaitHands() {
    getLeftHand()->forceWait();
    getRightHand()->forceWait();
}

// 800FE170
void Sandy::setWaitHands() {
    getLeftHand()->setWait();
    getRightHand()->setWait();
}

// 800FE1B0
void Sandy::setHandsStep(s32 step) {
    getLeftHand()->mWaitStep = step;
    getRightHand()->mWaitStep = step;
}

// 800FE200
void Sandy::setHandsBallType0() {
    getLeftHand()->setBallType0();
    getRightHand()->setBallType0();
}

// 800FE240
void Sandy::setHandsBallType3() {
    getLeftHand()->setBallType3();
    getRightHand()->setBallType3();
}

// 800FE280
void Sandy::setMinisNext() {
    getLeftMini()->setNextNerve();
    getRightMini()->setNextNerve();
}

// 800FE2C0
bool SandyMini::tryStartNext() {
    if (!isNerveStop()) {
        return false;
    }
    resume();
    return true;
}

// 800FE310
bool Sandy::isMinisDownOrDead() const {
    bool ret = false;
    if (getLeftMini()->isDownOrDead()) {
        if (getRightMini()->isDownOrDead()) {
            ret = true;
        }
    }
    return ret;
}

// 800FE370
void Sandy::showAll() {
    MR::showModel(this);
    MR::showModel(getLeftHand());
    MR::showModel(getRightHand());
}

// 800FE3C0
void Sandy::startActionAll(const char* pName, const char* pHandName) {
    MR::startAction(this, pName);
    getLeftHand()->setDamageAction(pHandName != nullptr ? pHandName : pName);
    getRightHand()->setDamageAction(pHandName != nullptr ? pHandName : pName);
}

// 800FE440
bool Sandy::tryStartBlur() {
    return mBlur->tryStart();
}

// 800FE450
void Sandy::startScaleHitReaction() {
    mScaleController->startHitReaction();
}

// 800FE460
AnimScaleParam* createScaleParam() {
    AnimScaleParam* pParam = new AnimScaleParam();
    pParam->mHitReactionSpringStrength = 0.3f;
    pParam->mHitReactionDamping = 0.7f;
    return pParam;
}

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

static inline s8 getHandType(bool isLeft) {
    s8 type = 1;
    if (isLeft) {
        type = 2;
    }
    return type;
}

// 800FE4A0
// NONMATCHING: register allocation in the ctor init list.
SandyHand::SandyHand(Sandy* pSandy, bool isLeft, SandyBallHolder* pHolder, FixedPosition* pFixedPos)
    : LiveActor(isLeft ? "\203T\203\223\203f\203B\201[\202\314\215\266\216\350" : "\203T\203\223\203f\203B\201[\202\314\211E\216\350"), mSandy(pSandy),
      mHolder(pHolder), mFixedPos(pFixedPos), mBall(nullptr), mIsRight(!isLeft), mIsThrower(isLeft), mHandType(getHandType(isLeft)), mWaitStep(30),
      mThrowCount(0), mBallType(2), mPrevNerve(nullptr), _B4(0), mScaleController(nullptr), mScaleParam(nullptr) {
}

// 800FE570
void SandyHand::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SandyHand", mIsRight);
    if (mIsRight) {
        MR::initDLMakerTexGenNum(this, &mHandType);
        MR::newDifferedDLBuffer(this);
        mScale.x *= -1.0f;
    }
    initNerve(&NrvSandyHand::SandyHandNrvStop::sInstance, 0);
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
// NONMATCHING: register numbers of the two ps_mul operands.
void SandyHand::calcAndSetBaseMtx() {
    TVec3f scale;
    mulScaleHand(&mScaleController->mScale.x, &mScale.x, &scale.x);
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
bool SandyHand::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgExplosionAttack(msg)) {
        return mSandy->tryStartBlur();
    }
    return false;
}

// 800FE940
bool SandyHand::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
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
        JMath::gekko_ps_copy12(&joint, MR::getJointMtx(this, "Hand1"));
        TPos3f offset;
        offset.makeTrans(sHandBallOffset);
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
