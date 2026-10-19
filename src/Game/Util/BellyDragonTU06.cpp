#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util.hpp"

extern "C" void* __nwa__FUl(u32);
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

// Cannot be expressed as a plain ctor on the shared base yet: the target calls ShadowVolumeDrawer(name)
// first and then stores the decoration model + id table.
BellyDragonFire::BellyDragonFire(const char* pName, const char* pModelName, MtxPtr pMtx, s32 count)
    : ShadowVolumeDrawer(pName) {
    mCount = count;
    mCurrent = 0;
    mModelObj = NULL;
    mIds = NULL;
    mModelObj = MR::createModelObjMapObjDecoration(pName, pModelName, pMtx);
    mModelObj->makeActorDead();
    mIds = (s32*)__nwa__FUl(mCount * 4);
    for (s32 i = 0; i < mCount; i++) {
        mIds[i] = 0;
    }
}

BellyDragonFire::~BellyDragonFire() {
}

// Small POD-like block (0x38 bytes): type id at 0x0, TMtx34 at 0x4, trailing s32 at 0x34.
class BellyDragonMtxBlock {
public:
    BellyDragonMtxBlock(s32 type);
    s32 copyMtx(void* pDst);

    /* 0x00 */ s32 mType;
    /* 0x04 */ TPos3f mMtx;
    /* 0x34 */ s32 mFlag;
};

BellyDragonMtxBlock::BellyDragonMtxBlock(s32 type) {  // was fn_80095900
    mType = type;
    mFlag = 0;
    mMtx.identity();
}

#pragma dont_inline on
s32 BellyDragonMtxBlock::copyMtx(void* pDst) {  // was fn_80095940
    JMath::gekko_ps_copy12(pDst, &mMtx);
    return 1;
}
#pragma dont_inline reset
