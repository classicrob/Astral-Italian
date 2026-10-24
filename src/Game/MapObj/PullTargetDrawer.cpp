#pragma auto_inline off
#include "Game/MapObj/PullTargetBridge.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Map/CollisionParts.hpp"

#include "Game/Player/YoshiFunction.hpp"

namespace NrvPullTargetDrawer {
    NEW_NERVE(PullTargetDrawerNrvWait, PullTargetDrawer, Wait);
    NEW_NERVE(PullTargetDrawerNrvPull, PullTargetDrawer, Pull);
    NEW_NERVE(PullTargetDrawerNrvExtend, PullTargetDrawer, Extend);
    NEW_NERVE(PullTargetDrawerNrvExtendWait, PullTargetDrawer, ExtendWait);
    NEW_NERVE(PullTargetDrawerNrvShrink, PullTargetDrawer, Shrink);
};  // namespace NrvPullTargetDrawer

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
    initNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvWait), 0);
    updateClipping();
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
        MR::fn_804222D0();
        setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvExtend));
        return;
    }
    if (!mPullTarget->isFacingPlayer()) {
        MR::fn_804222D0();
        setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvWait));
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
        setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvExtendWait));
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
            setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvShrink));
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
        setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvWait));
    }
}

bool PullTargetDrawer::calcJointBridge(TPos3f* pMtx, const JointControllerInfo&) {
    MtxPtr base = mPullTarget->getBaseMtx();
    TVec3f front(base[0][2], base[1][2], base[2][2]);
    TVec3f trans;
    pMtx->getTrans(trans);
    pMtx->setTrans(trans + front * mLength);
    return true;
}

bool PullTargetDrawer::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgYoshiStartTongue(msg)) {
        mPlayerPos.set< f32 >(*MR::getPlayerPos());
        setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvPull));
        return true;
    }
    if (MR::isMsgYoshiLoopTongue(msg)) {
        return tryPull();
    }
    if (MR::isMsgYoshiReleaseEaten(msg)) {
        if (isNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvPull))) {
            setNerve(GET_NERVE(PullTargetDrawer, PullTargetDrawerNrvWait));
        }
        return true;
    }
    return false;
}

void PullTargetDrawer::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();
    mJointCtrl->registerCallBack();
}

void PullTargetDrawer::updateClipping() {
    TPos3f mtx;
    MR::makeMtxTR(mtx, mPosition, mRotation);
    TVec3f front;
    f32 z = mtx.mMtx[2][2];
    f32 y = mtx.mMtx[1][2];
    f32 x = mtx.mMtx[0][2];
    front.set< f32 >(x, y, z);
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
        s32 n = index - mValidIndex;
        for (s32 i = 0; i < n; i++) {
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

// Target inlines the matrix copy; the current shared helper also changes temporary layout.
void PullTargetDrawer::pushPlayerFromBridge() {
    TPos3f jointMtx(MR::getJointMtx(this, "PullTargetPos"));
    TPos3f inv(jointMtx);
    inv.invert(inv);
    TVec3f local(*MR::getPlayerCenterPos());
    inv.mult(local, local);
    if (local.z > 0.0f) return;
    if (__fabsf(local.x) > 370.0f) return;
    if (__fabsf(local.y) > 215.0f) return;
    local.z = 0.0f;
    jointMtx.mult(local, local);
    MR::pushPlayer(local - *MR::getPlayerCenterPos());
}

template class JointControlDelegator< PullTargetDrawer >;
