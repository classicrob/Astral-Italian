#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util.hpp"
#include "Game/Util/JointController.hpp"
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphBase/J3DPacket.hpp>
#include <JSystem/J3DGraphBase/J3DShape.hpp>
#include <JSystem/J3DGraphBase/J3DSys.hpp>

extern "C" int snprintf(char*, unsigned long, const char*, ...);

namespace JMath {
    void gekko_ps_copy12(void*, const void*);
}

namespace NrvBellyDragonState {
    NEW_NERVE(BellyDragonStateNrvWait, BellyDragonState, Wait);
    NEW_NERVE(BellyDragonStateNrvPlay, BellyDragonState, Play);
};  // namespace NrvBellyDragonState

void BellyDragonState::setOverlay() {
    mIsOverlay = true;
}

void BellyDragonState::clearActive() {
    mIsActive = false;
    mIsDeleteEffect = true;
}

BellyDragonState::BellyDragonState(BellyDragon* pHost, const char* pName, const f32* pTime)
    : ActorStateBaseInterface(pName), mHost(pHost), mStateName(pName), mIsActive(true), mIsOverlay(false), mIsDeleteEffect(false), mpTime(pTime) {
}

void BellyDragonState::init() {
    BellyDragonUtil::initPodAnimCamera(mHost, mStateName);
    initNerve(&NrvBellyDragonState::BellyDragonStateNrvWait::sInstance, 0);
    kill();
}

void BellyDragonState::appear() {
    mIsDead = false;
    setNerve(&NrvBellyDragonState::BellyDragonStateNrvWait::sInstance);
}

BellyDragonState::~BellyDragonState() {
}

void BellyDragonState::exeWait() {
    s32 isReady = BellyDragonUtil::tryStartDemo(mHost, mStateName, false);
    if (isReady) {
        MR::overlayWithPreviousScreen(2);
        if (mIsDeleteEffect && BellyDragonUtil::isLv2(mHost)) {
            BellyDragonUtil::setWeakPointsWaitC(mHost);
        }
        if (BellyDragonUtil::isLv2(mHost)) {
            BellyDragonUtil::killPodObjects(mHost);
        }
        setNerve(&NrvBellyDragonState::BellyDragonStateNrvPlay::sInstance);
    }
}

void BellyDragonState::exePlay() {
    if (mIsOverlay && MR::isStep(this, 0xCD)) {
        BellyDragonUtil::appearJawHill(mHost);
        BellyDragonUtil::startPodFlame(mHost);
    }
    if (BellyDragonUtil::isLv2(mHost) && mIsOverlay && MR::isStep(this, 0xD7)) {
        BellyDragonUtil::appearFireGround(mHost);
    }
    if (mIsDeleteEffect && MR::isStep(this, 0x21C)) {
        if (BellyDragonUtil::isLv2(mHost)) {
            for (s32 i = 0; i < 0x20; i++) {
                char name[0x40];
                snprintf(name, 0x40, "FireBody%02d", i);
                MR::tryDeleteEffect(mHost, name);
            }
        }
        BellyDragonUtil::stopPodFlame(mHost);
    }
    if (BellyDragonUtil::updateDemo(mHost, mStateName, mIsActive, mpTime, *mpTime)) {
        BellyDragonUtil::startPodFlame(mHost);
        BellyDragonUtil::killFireGrounds(mHost);
        kill();
    }
}

BellyDragonFire::BellyDragonFire(const char* pName, const char* pModelName, MtxPtr pMtx, s32 count)
    : ShadowVolumeDrawer(pName) {
    mCount = count;
    mCurrent = 0;
    mModelObj = NULL;
    mIds = NULL;
    mModelObj = MR::createModelObjMapObjDecoration(pName, pModelName, pMtx);
    mModelObj->makeActorDead();
    mIds = new BellyDragonMtxBlock*[mCount];
    for (s32 i = 0; i < mCount; i++) {
        mIds[i] = 0;
    }
}

BellyDragonFire::~BellyDragonFire() {
}

// Joint name, local matrix and the callback that installs it during animation (0x38 bytes).
class BellyDragonMtxBlock {
public:
    BellyDragonMtxBlock(const char* pName);
    bool copyMtx(TPos3f*, const JointControllerInfo&);

    /* 0x00 */ const char* mJointName;
    /* 0x04 */ TPos3f mMtx;
    /* 0x34 */ JointController* mController;
};

BellyDragonMtxBlock::BellyDragonMtxBlock(const char* pName) {  // was fn_80095900
    mJointName = pName;
    mController = NULL;
    mMtx.identity();
}

#pragma dont_inline on
bool BellyDragonMtxBlock::copyMtx(TPos3f* pDst, const JointControllerInfo&) {  // was fn_80095940
    JMath::gekko_ps_copy12(pDst, &mMtx);
    return 1;
}
#pragma dont_inline reset

void BellyDragonFire::addJointMtx(const char* pJointName) {
    u16 jointIndex = MR::getJointIndex(mModelObj, pJointName);
    const char* name = MR::getJointName(mModelObj, jointIndex);
    BellyDragonMtxBlock* block = new BellyDragonMtxBlock(name);
    block->mController = MR::createJointController<BellyDragonMtxBlock>(block, mModelObj, jointIndex, &BellyDragonMtxBlock::copyMtx, NULL);
    mIds[mCurrent] = block;
    ++mCurrent;
}

void BellyDragonFire::appearModel() {
    if (MR::isDead(mModelObj))
        mModelObj->makeActorAppeared();
}

void BellyDragonFire::killModel() {
    if (!MR::isDead(mModelObj))
        mModelObj->makeActorDead();
}

void BellyDragonFire::setJointMtx(const char* pJointName, const TPos3f* pMtx) {
    for (s32 i = 0; i < mCurrent; ++i) {
        if (MR::isEqualString(pJointName, mIds[i]->mJointName)) {
            JMath::gekko_ps_copy12(&mIds[i]->mMtx, pMtx);
            break;
        }
    }
}

void BellyDragonFire::registerCallBacks() {
    for (s32 i = 0; i < mCurrent; ++i)
        mIds[i]->mController->registerCallBack();
    PSMTXCopy(MR::getCameraViewMtx(), j3dSys.mViewMtx);
    mModelObj->calcAnim();
    mModelObj->calcViewAndEntry();
}

void BellyDragonFire::loadModelDrawMtx() const {
    MR::getJ3DModel(mModelObj)->getShapePacket(0)->getShape()->loadPreDrawSetting();
}

// NONMATCHING: redundant zero extension of the u16 material count returned by the callee.
void BellyDragonFire::drawShape() const {
    s32 materialNum = MR::getMaterialNum(mModelObj);
    for (u16 i = 0; i < materialNum; ++i) {
        J3DShapePacket* packet = MR::getJ3DModel(mModelObj)->getShapePacket(i);
        if (!packet->getShape()->checkFlag(1))
            packet->drawFast();
    }
}

bool BellyDragonFire::isDraw() const {
    return !MR::isDead(mModelObj);
}
