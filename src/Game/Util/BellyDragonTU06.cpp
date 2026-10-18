#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util.hpp"

extern "C" void* __nwa__FUl(u32);
extern "C" void fn_80093B40(BellyDragon*, const char*);
extern "C" bool fn_80093310(BellyDragon*);
extern "C" void fn_80093780(BellyDragon*);
extern "C" void fn_80093800(BellyDragon*);
extern "C" void fn_800938A0(BellyDragon*);
extern "C" void fn_800938D0(BellyDragon*);
extern "C" void fn_80093A80(BellyDragon*);
extern "C" void fn_80093AF0(BellyDragon*);
extern "C" void fn_80093B00(BellyDragon*);
extern "C" bool fn_80093BF0(BellyDragon*, const char*, bool);
extern "C" bool fn_80093DC0(BellyDragon*, const char*, bool, const f32*, f32);
extern "C" void fn_80094E10(s32);
extern "C" int snprintf(char*, unsigned long, const char*, ...);

namespace JMath {
    void gekko_ps_copy12(void*, const void*);
}

namespace NrvBellyDragonState {
    NEW_NERVE(BellyDragonStateNrvWait, BellyDragonState, Wait);
    NEW_NERVE(BellyDragonStateNrvPlay, BellyDragonState, Play);
};  // namespace NrvBellyDragonState

// Decoration drawer (vtable D280): owns a map-object decoration model and an s32 id table.
class BellyDragonFire : public ShadowVolumeDrawer {
public:
    BellyDragonFire(const char*, const char*, MtxPtr, s32);

    virtual ~BellyDragonFire();

    /* 0x24 */ ModelObj* mModelObj;
    /* 0x28 */ s32 mCount;
    /* 0x2C */ s32 mCurrent;
    /* 0x30 */ s32* mIds;
};

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
    fn_80093B40(mHost, mStateName);
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
    bool isReady = fn_80093BF0(mHost, mStateName, false);
    if (isReady) {
        MR::overlayWithPreviousScreen(2);
        if (mIsDeleteEffect && fn_80093310(mHost)) {
            fn_80093A80(mHost);
        }
        if (fn_80093310(mHost)) {
            fn_80093800(mHost);
        }
        setNerve(&NrvBellyDragonState::BellyDragonStateNrvPlay::sInstance);
    }
}

void BellyDragonState::exePlay() {
    if (mIsOverlay && MR::isStep(this, 0xCD)) {
        fn_80093780(mHost);
        fn_80093AF0(mHost);
    }
    if (fn_80093310(mHost) && mIsOverlay && MR::isStep(this, 0xD7)) {
        fn_800938A0(mHost);
    }
    if (mIsDeleteEffect && MR::isStep(this, 0x21C)) {
        if (fn_80093310(mHost)) {
            for (s32 i = 0; i < 0x20; i++) {
                char name[0x40];
                snprintf(name, 0x40, "FireBody%02d", i);
                MR::tryDeleteEffect(mHost, name);
            }
        }
        fn_80093B00(mHost);
    }
    if (fn_80093DC0(mHost, mStateName, mIsActive, mpTime, *mpTime)) {
        fn_80093AF0(mHost);
        fn_800938D0(mHost);
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
    /* 0x00 */ s32 mType;
    /* 0x04 */ TPos3f mMtx;
    /* 0x34 */ s32 mFlag;
};

BellyDragonMtxBlock* fn_80095900(BellyDragonMtxBlock* pBlock, s32 type) {
    pBlock->mType = type;
    pBlock->mFlag = 0;
    pBlock->mMtx.identity();
    return pBlock;
}

s32 fn_80095940(BellyDragonMtxBlock* pBlock, void* pDst) {
    JMath::gekko_ps_copy12(pDst, &pBlock->mMtx);
    return 1;
}
