#pragma auto_inline off
#include "Game/MapObj/PullTargetBridge.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Map/CollisionParts.hpp"

namespace YoshiFunction {
    void tryYoshiPull(YoshiLockOnTarget*);
};

namespace MR {
    YoshiLockOnTarget* createYoshiLockOnTarget(LiveActor*, MtxPtr, const TVec3f&);
};

extern "C" void fn_804222D0(LiveActor*);

PullTargetAnim::PullTargetAnim(LiveActor* pHost) {
    mHost = pHost;
    mPrevDist = 0.0f;
    mStillCount = 0;
    mIsPulling = false;
}

void PullTargetAnim::reset() {
    MR::startAction(mHost, "Wait");
    mPrevDist = 0.0f;
    mStillCount = 0;
    mIsPulling = false;
}

void PullTargetAnim::update(f32 dist) {
    if (dist - mPrevDist > 1.0f) {
        if (!mIsPulling) {
            MR::startAction(mHost, "PullStart");
            mIsPulling = true;
        }
        mStillCount = 0;
        mPrevDist = dist;
    } else {
        mStillCount++;
        if (mIsPulling && mStillCount > 5) {
            MR::startAction(mHost, "Wait");
            mIsPulling = false;
        }
    }
}

PullTarget::PullTarget(LiveActor* pHost) : PartsModel(pHost, "引っ張りターゲット", "PullTarget", nullptr, -1, false) {
    mIsEnable = true;
    mLockOnTarget = nullptr;
    initWithoutIter();
    initHitSensor(1);
    MR::addHitSensorMapObjSimple(this, "Body", 32, 1.0f, TVec3f(0.0f, 0.0f, 0.0f));
    initFixedPosition("PullTargetPos");
    MR::initStarPointerTarget(this, 300.0f, TVec3f(0.0f, 0.0f, 0.0f));
    mLockOnTarget = MR::createYoshiLockOnTarget(this, nullptr, TVec3f(0.0f));
}

// NONMATCHING: target inlines the TPos3f psq copy (gekko_ps_copy12 stays out of line under auto_inline off)
bool PullTarget::isFacingPlayer() {
    TVec3f toPlayer;
    MR::calcVecToPlayer(&toPlayer, this);
    TPos3f mtx;
    mtx.set(getBaseMtx());
    TVec3f front;
    front.set< f32 >(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);
    return toPlayer.dot(front) > 0.1f;
}

void PullTarget::control() {
    if (mIsEnable && isFacingPlayer()) {
        YoshiFunction::tryYoshiPull(mLockOnTarget);
    }
}

bool PullTarget::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    LiveActor* host = mHost;
    return host->receiveMessage(msg, pSender, host->getSensor("Body"));
}

PullTargetBridge::PullTargetBridge(const char* pName) : LiveActor(pName) {
    mPullTarget = nullptr;
    mPlayerPos.x = 0.0f;
    mPlayerPos.y = 0.0f;
    mPlayerPos.z = 0.0f;
    mPullDist = 0.0f;
    mAnim = nullptr;
}

namespace NrvPullTargetBridge {
    class PullTargetBridgeNrvWait : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            PullTargetBridge* actor = reinterpret_cast< PullTargetBridge* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, "Wait");
            }
        }
        static PullTargetBridgeNrvWait sInstance;
    };
    PullTargetBridgeNrvWait PullTargetBridgeNrvWait::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(PullTargetBridgeNrvPull, PullTargetBridge, Pull);
    class PullTargetBridgeNrvFall : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            PullTargetBridge* actor = reinterpret_cast< PullTargetBridge* >(pSpine->mExecutor);
            if (MR::isFirstStep(actor)) {
                MR::startAction(actor, "Fall");
            }
        }
        static PullTargetBridgeNrvFall sInstance;
    };
    PullTargetBridgeNrvFall PullTargetBridgeNrvFall::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvPullTargetBridge

void PullTargetBridge::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "PullTargetBridge", false);
    initHitSensor(1);
    MR::addHitSensorMapObjSimple(this, "Body", 32, 1.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MtxPtr mtx = MR::getJointMtx(this, "PullTargetBridge");
    MR::initCollisionParts(this, "PullTargetBridge", getSensor("Body"), mtx);
    MR::connectToSceneMapObj(this);
    initNerve(&NrvPullTargetBridge::PullTargetBridgeNrvWait::sInstance, 0);
    mClippingCenter.set< f32 >(mPosition);
    MR::setClippingTypeSphere(this, 1500.0f, &mClippingCenter);
    mPullTarget = new PullTarget(this);
    mAnim = new PullTargetAnim(this);
    makeActorAppeared();
}

void PullTargetBridge::exePull() {
    if (MR::isFirstStep(this)) {
        mAnim->reset();
    }
    mAnim->update(mPullDist);
    bool isPulled = mPullDist > 250.0f;
    if (isPulled) {
        PullTarget* target = mPullTarget;
        target->mIsEnable = false;
        fn_804222D0(target);
        setNerve(&NrvPullTargetBridge::PullTargetBridgeNrvFall::sInstance);
    }
}

// NONMATCHING: register allocation only (sum temp kept in r31 in the target)
void PullTargetBridge::control() {
    TVec3f posA;
    MR::copyJointPos(this, "PullTargetBridge", &posA);
    TVec3f posB;
    MR::copyJointPos(this, "PullTargetPos", &posB);
    mClippingCenter.set< f32 >(posA * 0.5f + posB * 0.5f);
}

bool PullTargetBridge::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        if (isNerve(&NrvPullTargetBridge::PullTargetBridgeNrvWait::sInstance)) {
            mPlayerPos.set< f32 >(*MR::getPlayerPos());
            mPullDist = 0.0f;
            setNerve(&NrvPullTargetBridge::PullTargetBridgeNrvPull::sInstance);
        }
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return tryPull();
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        if (isNerve(&NrvPullTargetBridge::PullTargetBridgeNrvPull::sInstance)) {
            setNerve(&NrvPullTargetBridge::PullTargetBridgeNrvWait::sInstance);
        }
        return true;
    }
    return false;
}

// NONMATCHING: target inlines operator-= (ps_sub); ours calls __ami under auto_inline off
bool PullTargetBridge::tryPull() {
    TVec3f move(*MR::getPlayerPos());
    move -= mPlayerPos;
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir(move);
    MR::normalizeOrZero(&dir);
    MtxPtr mtx = mPullTarget->getBaseMtx();
    TVec3f front(mtx[0][2], mtx[1][2], mtx[2][2]);
    if (front.dot(dir) < 0.0f) {
        return false;
    }
    mPullDist += PSVECMag(&move);
    return true;
}

// PullTargetDrawer: its nerve instances (sbss 807D4138..48) and sinit live in a neighbouring split.
extern "C" s32 lbl_807D4138[1];
extern "C" s32 lbl_807D413C[1];
extern "C" s32 lbl_807D4140[1];
extern "C" s32 lbl_807D4144[1];
extern "C" s32 lbl_807D4148[1];
#define DRAWER_NRV(x) (reinterpret_cast< const Nerve* >(x))

PullTargetDrawer::PullTargetDrawer(const char* pName) : LiveActor(pName) {
    mClippingCenter.x = 0.0f;
    mClippingCenter.y = 0.0f;
    mClippingCenter.z = 0.0f;
    mPullTarget = nullptr;
    mPlayerPos.x = 0.0f;
    mPlayerPos.y = 0.0f;
    mPlayerPos.z = 0.0f;
    mPullDist = 0.0f;
    mJointCtrl = nullptr;
    mMaxLength = 1000.0f;
    mLength = 0.0f;
    mValidIndex = -1;
    mParts = nullptr;
    mPartsNum = 0;
    mPartsMtx = nullptr;
    mShrinkTime = -1;
}

void PullTargetDrawer::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::getJMapInfoArg0NoInit(rIter, &mMaxLength);
    MR::getJMapInfoArg1NoInit(rIter, &mShrinkTime);
    MR::initActor(this, rIter, "PullTargetDrawer", false);
    initHitSensor(1);
    MR::addHitSensorMapObjSimple(this, "Body", 32, 1.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initShadowFromCSVWithInitShadowVolumeSphere(this, "Shadow");
    initEffectKeeper(0, nullptr, false);
    MR::connectToSceneMapObj(this);
    initNerve(DRAWER_NRV(lbl_807D4138), 0);
    updateClipping(mRotation);
    initSound(4, "PullTargetDrawer", nullptr, TVec3f(0.0f));
    mPullTarget = new PullTarget(this);
    mJointCtrl = MR::createJointDelegatorWithNullChildFunc(this, &PullTargetDrawer::calcJointBridge, "Drawer");
    initCollision();
    mAnim = new PullTargetAnim(this);
    MR::initUseStageSwitchWriteA(this, rIter);
    if (mShrinkTime >= 0) {
        MR::startBrk(this, "Timer");
    } else {
        MR::startBrk(this, "PullTargetDrawer");
    }
    makeActorAppeared();
}

void PullTargetDrawer::exeWait() {
    if (MR::isFirstStep(this)) {
        mPullDist = 0.0f;
        mPullTarget->mIsEnable = true;
        MR::startAction(this, "Wait");
    }
}

void PullTargetDrawer::exePull() {
    if (MR::isFirstStep(this)) {
        mAnim->reset();
    }
    mAnim->update(mPullDist);
    if (mPullDist > 250.0f) {
        MR::tryRumblePadStrong(this, 0);
        PullTarget* target = mPullTarget;
        target->mIsEnable = false;
        fn_804222D0(target);
        setNerve(DRAWER_NRV(lbl_807D4140));
        return;
    }
    if (!mPullTarget->isFacingPlayer()) {
        fn_804222D0(mPullTarget);
        setNerve(DRAWER_NRV(lbl_807D4138));
    }
}

void PullTargetDrawer::exeExtend() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "PullEnd");
    }
    MR::startActionSound(this, "PullMoving", -1, -1, -1);
    f32 rate = 0.5f + -0.5f * MR::cos((3.1415927f * getNerveStep()) / 30.0f);
    if (rate < 0.0f) {
        rate = 0.0f;
    }
    if (rate > 1.0f) {
        rate = 1.0f;
    }
    mLength = mMaxLength * rate;
    updateCollisionValid();
    updateCollisionMtx();
    pushPlayerFromBridge();
    if (MR::isStep(this, 27)) {
        MR::tryRumblePadStrong(this, 0);
        MR::shakeCameraNormalWeak();
    }
    if (MR::isStep(this, 28)) {
        MR::startActionSound(this, "PullStop", -1, -1, -1);
    }
    if (MR::isGreaterEqualStep(this, 30)) {
        mLength = mMaxLength;
        setNerve(DRAWER_NRV(lbl_807D4144));
        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }
    }
}

void PullTargetDrawer::exeExtendWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
    }
    if (mShrinkTime >= 0) {
        if (MR::isStep(this, mShrinkTime - 120)) {
            MR::startAction(this, "DrawBackSign");
        }
        if (MR::isGreaterStep(this, mShrinkTime - 120)) {
            MR::startActionSound(this, "TimerCount2", -1, -1, -1);
        } else if (MR::isGreaterStep(this, mShrinkTime % 30)) {
            MR::startActionSound(this, "TimerCount1", -1, -1, -1);
        }
        if (MR::isGreaterEqualStep(this, mShrinkTime)) {
            setNerve(DRAWER_NRV(lbl_807D4148));
            MR::startActionSound(this, "TimerZero", -1, -1, -1);
        }
    }
}

void PullTargetDrawer::exeShrink() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Back");
    }
    f32 rate = 0.5f + -0.5f * MR::cos((3.1415927f * getNerveStep()) / 120.0f);
    if (rate < 0.0f) {
        rate = 0.0f;
    }
    if (rate > 1.0f) {
        rate = 1.0f;
    }
    mLength = mMaxLength * (1.0f - rate);
    MR::startActionSound(this, "ReturnMoving", -1, -1, -1);
    updateCollisionValid();
    updateCollisionMtx();
    if (MR::isStep(this, 117)) {
        MR::tryRumblePadStrong(this, 0);
        MR::shakeCameraNormalWeak();
        MR::startActionSound(this, "ReturnStop", -1, -1, -1);
        MR::emitEffect(this, "BackSmoke");
    }
    if (MR::isGreaterEqualStep(this, 120)) {
        mLength = 0.0f;
        MR::validateClipping(this);
        setNerve(DRAWER_NRV(lbl_807D4138));
    }
}

bool PullTargetDrawer::calcJointBridge(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr base = mPullTarget->getBaseMtx();
    TVec3f front(base[0][2], base[1][2], base[2][2]);
    TVec3f trans;
    trans.set< f32 >(pMtx->mMtx[0][3], pMtx->mMtx[1][3], pMtx->mMtx[2][3]);
    pMtx->setTrans(trans + front * mLength);
    return true;
}

bool PullTargetDrawer::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        mPlayerPos.set< f32 >(*MR::getPlayerPos());
        setNerve(DRAWER_NRV(lbl_807D413C));
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return tryPull();
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        if (isNerve(DRAWER_NRV(lbl_807D413C))) {
            setNerve(DRAWER_NRV(lbl_807D4138));
        }
        return true;
    }
    return false;
}

void PullTargetDrawer::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mJointCtrl->registerCallBack();
}

void PullTargetDrawer::updateClipping(const TVec3f& rRotate) {
    TPos3f mtx;
    MR::makeMtxTR(mtx, mPosition, rRotate);
    TVec3f front;
    front.set< f32 >(mtx.mMtx[0][2], mtx.mMtx[1][2], mtx.mMtx[2][2]);
    mClippingCenter.set< f32 >(mPosition + front * mMaxLength * 0.5f);
    MR::setClippingTypeSphere(this, 0.5f * mMaxLength, &mClippingCenter);
}

void PullTargetDrawer::initCollision() {
    mPartsNum = (s32)(mMaxLength / 200.0f) + 1;
    mParts = new CollisionParts*[mPartsNum];
    mPartsMtx = new TPos3f[mPartsNum];
    for (u32 i = 0; i < mPartsNum; i++) {
        mPartsMtx[i].identity();
        mParts[i] = MR::createCollisionPartsFromLiveActor(this, "PullTargetDrawer", getSensor("Body"), mPartsMtx[i], MR::CollisionScaleType_Unk2);
        MR::invalidateCollisionParts(mParts[i]);
    }
}

bool PullTargetDrawer::tryPull() {
    TVec3f move = *MR::getPlayerPos() - mPlayerPos;
    mPlayerPos.set< f32 >(*MR::getPlayerPos());
    TVec3f dir(move);
    MR::normalizeOrZero(&dir);
    MtxPtr mtx = mPullTarget->getBaseMtx();
    TVec3f front(mtx[0][2], mtx[1][2], mtx[2][2]);
    if (front.dot(dir) < 0.0f) {
        return false;
    }
    mPullDist += PSVECMag(&move);
    return true;
}

void PullTargetDrawer::updateCollisionMtx() {
    TPos3f jointMtx(MR::getJointMtx(this, "Drawer"));
    TVec3f trans(jointMtx.mMtx[0][3], jointMtx.mMtx[1][3], jointMtx.mMtx[2][3]);
    TVec3f front(jointMtx.mMtx[0][2], jointMtx.mMtx[1][2], jointMtx.mMtx[2][2]);
    for (s32 i = 0; i <= mValidIndex; i++) {
        mPartsMtx[i].set(jointMtx);
        mPartsMtx[i].setTrans(trans - front * (-150.0f + 200.0f * i));
        mParts[i]->setMtx();
    }
}

void PullTargetDrawer::updateCollisionValid() {
    s32 index = mLength / 200.0f;
    if (0.0f == mLength) {
        index = -1;
    }
    if (index > mValidIndex) {
        for (s32 i = 0; i < index - mValidIndex; i++) {
            MR::validateCollisionParts(mParts[++mValidIndex]);
        }
    } else if (index < mValidIndex) {
        s32 n = mValidIndex - index;
        for (s32 i = 0; i < n; i++) {
            MR::invalidateCollisionParts(mParts[mValidIndex]);
            mValidIndex--;
        }
    }
}

// NONMATCHING: matrix copy/invert temp layout differs
void PullTargetDrawer::pushPlayerFromBridge() {
    TPos3f jointMtx(MR::getJointMtx(this, "PullTargetPos"));
    TPos3f inv;
    inv.set(jointMtx);
    inv.invert(inv);
    TVec3f local(*MR::getPlayerCenterPos());
    inv.mult(local, local);
    if (local.z <= 0.0f && __fabsf(local.x) <= 370.0f && __fabsf(local.y) <= 215.0f) {
        local.z = 0.0f;
        jointMtx.mult(local, local);
        MR::pushPlayer(local - *MR::getPlayerCenterPos());
    }
}
