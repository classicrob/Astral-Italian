#include "Game/Util/PlayerUtil.hpp"
#include "Game/Demo/DemoFunction.hpp"
#pragma auto_inline off

#include "Game/NPC/KinopioPostman.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActorItem.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NameObj/NameObjFactory.hpp"
#include "Game/Screen/LayoutActor.hpp"
#include "Game/Util.hpp"

// Data of this unit (strings and nerve instances in the target's .data/.sbss).
extern "C" u8 lbl_806B97E0[];
extern "C" u8 lbl_806B8EE0[];
extern "C" u8 lbl_806B8EE8[];
extern "C" u8 lbl_806B7AEC[];
extern "C" u8 lbl_806B7AE0[];
extern "C" u8 lbl_806B7AD4[];
extern "C" u8 lbl_806B7AC8[];
extern "C" u8 lbl_806B99AC[];
extern "C" u8 lbl_806B99C0[];
extern "C" u8 lbl_806B9918[];
extern "C" u8 lbl_806B9A18[];
extern "C" u8 lbl_807CFE90[];
extern "C" u8 lbl_807CFE94[];
extern "C" u8 lbl_807CFE98[];
extern "C" u8 lbl_807CFE9C[];
extern "C" const char* const lbl_8064B9C0[9];
extern "C" u32 lbl_807D497C;
extern "C" u32 lbl_807D4980;
extern "C" u32 lbl_807D4984;
extern "C" u32 lbl_807D4988;
extern "C" u32 lbl_807D498C;
extern "C" s32 lbl_807E3100;

// Helpers of other units (not yet named in this tree).
extern "C" bool fn_8035AEA0(); // PLACEHOLDER(astra): inherited fn_8035AEA0 ABI; resolve when its target signature and canonical owner are verified.
extern "C" NameObj* fn_8035AAA0(KinopioPostman*, s32); // PLACEHOLDER(astra): inherited fn_8035AAA0 ABI; resolve when its target signature and canonical owner are verified.
extern "C" LayoutActor* fn_8035ABB0(); // PLACEHOLDER(astra): inherited fn_8035ABB0 ABI; resolve when its target signature and canonical owner are verified.
extern "C" s32 fn_8035ACB0(); // PLACEHOLDER(astra): inherited fn_8035ACB0 ABI; resolve when its target signature and canonical owner are verified.
extern "C" bool fn_8035AF00(); // PLACEHOLDER(astra): inherited fn_8035AF00 ABI; resolve when its target signature and canonical owner are verified.
extern "C" LiveActor* fn_8035AC20(const JMapInfoIter&); // PLACEHOLDER(astra): inherited fn_8035AC20 ABI; resolve when its target signature and canonical owner are verified.
extern "C" void fn_8035AD60(LiveActor*, s32); // PLACEHOLDER(astra): inherited fn_8035AD60 ABI; resolve when its target signature and canonical owner are verified.
extern "C" bool fn_804C24B0(); // PLACEHOLDER(astra): inherited fn_804C24B0 ABI; resolve when its target signature and canonical owner are verified.
extern "C" s32 fn_8048AE10(LayoutActor*); // PLACEHOLDER(astra): inherited fn_8048AE10 ABI; resolve when its target signature and canonical owner are verified.
extern "C" bool fn_8048AE00(LayoutActor*); // PLACEHOLDER(astra): inherited fn_8048AE00 ABI; resolve when its target signature and canonical owner are verified.

#define KP_STR(ofs) (reinterpret_cast< const char* >(lbl_806B97E0) + (ofs))
#define KP_NERVE(sym) (reinterpret_cast< const Nerve* >(&sym))
#define KP_RAW(T, ofs) (*reinterpret_cast< T* >(reinterpret_cast< u8* >(this) + (ofs)))
#define KP_RAW_OF(obj, T, ofs) (*reinterpret_cast< T* >(reinterpret_cast< u8* >(obj) + (ofs)))

namespace {
    // Plain-function talk callback: the target stores {0, -1, fn} (a non-virtual function pointer)
    // in a TalkMessageFuncM. NONMATCHING: vtable and layout differ from the target template instance.
    class KinopioPostmanTalkFunc : public TalkMessageFuncBase {
    public:
        typedef bool (*Fn)(KinopioPostman*, u32);

        KinopioPostmanTalkFunc(KinopioPostman* pCaller, Fn pFn) : mCaller(pCaller), mFn(pFn) {
        }

        virtual bool operator()(u32 arg) const {
            return mFn(mCaller, arg);
        }

        virtual KinopioPostmanTalkFunc* clone() const {
            return new KinopioPostmanTalkFunc(*this);
        }

        virtual KinopioPostmanTalkFunc* clone(JKRHeap* pHeap) const {
            return new (pHeap, 0) KinopioPostmanTalkFunc(*this);
        }

        KinopioPostman* mCaller;
        Fn mFn;
    };
}  // namespace

extern "C" {
// Talk/event callbacks and helpers (plain C functions in the target).
bool fn_8034FAB0(KinopioPostman*, u32);
bool fn_8034FB10(KinopioPostman*, u32);
bool fn_8034FBB0(KinopioPostman*, u32);
bool fn_8034FC60(KinopioPostman*, u32);
bool fn_8034FCC0(KinopioPostman*, u32);
void fn_8034FA60(KinopioPostman*);
bool fn_8034FDE0(LiveActor*);
bool fn_8034FE40(LiveActor*);
bool fn_8034FEB0(KinopioPostman*);
void fn_8034FF50(KinopioPostman*);
void fn_8034FFD0(TQuat4f*, const TVec3f&, const TVec3f&, f32);
void fn_80350120(KinopioPostman*, LiveActor*);
void fn_803501C0(KinopioPostman*, s32);
void fn_80350290(KinopioPostman*, LiveActor*);
void fn_80350440(KinopioPostman*, LiveActor*);
void fn_803504C0(KinopioPostman*, LiveActor*);
void fn_803505A0(KinopioPostman*, LiveActor*);
bool fn_8034F600(KinopioPostman*);
bool fn_8034F900(KinopioPostman*, const char*, const JMapInfoIter&);
void fn_8034F9B0(KinopioPostman*, const JMapInfoIter&);

void fn_8034F5C0(void*, KinopioPostman*, const u32*);
void fn_8034F8C0(void*, KinopioPostman*, const u32*);
}

KinopioPostman::KinopioPostman(const char* pName) : NPCActor(pName) {
    mLayout = nullptr;
    mIsEventFlag = false;
    mIsReplyFlag = false;
    mStepNum = 7;
    mChildObj = nullptr;
    mPostmanEvent = nullptr;
    mWaitTimer = -1;
}

void KinopioPostman::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps(KP_STR(0x128));
    caps.setDefault();
    initialize(rIter, caps, KP_STR(0x128), nullptr, nullptr);
    MR::invalidateHitSensor(this, KP_STR(0x130));
    setNerve(KP_NERVE(lbl_807D498C));

    TVec3f rot(mRotation);
    rot.x *= 0.017453292f;
    rot.y *= 0.017453292f;
    rot.z *= 0.017453292f;
    reinterpret_cast< TQuat4f* >(reinterpret_cast< u8* >(this) + 0xA4)->setEuler(rot);
    MR::setScale(this, 1.2f);
    bool isTalkReady = fn_8034F600(this);

    KP_RAW(f32, 0x130) = 450.0f;
    KP_RAW(const u8*, 0x134) = lbl_806B7AEC;
    KP_RAW(const u8*, 0x138) = lbl_806B7AE0;
    KP_RAW(const u8*, 0x13C) = lbl_806B7AD4;
    KP_RAW(const u8*, 0x140) = lbl_806B7AC8;
    KP_RAW(const u8*, 0x100) = lbl_806B8EE8;
    KP_RAW(const u8*, 0x104) = lbl_806B8EE0;
    KP_RAW(const u8*, 0x108) = lbl_806B97E0 + 0x138;
    KP_RAW(const u8*, 0x10C) = lbl_806B97E0 + 0x138;
    KP_RAW(const u8*, 0x120) = lbl_806B97E0 + 0x140;
    KP_RAW(const u8*, 0x124) = lbl_806B97E0 + 0x140;
    KP_RAW(u8, 0x128) = 1;
    MR::startBrk(this, KP_STR(0x148));
    MR::setBrkFrameAndStop(this, 2.0f);

    NPCActorItem item("Kinopio");
    MR::getNPCItemData(&item, 3);
    equipment(item, false);
    MR::onCalcGravity(this);
    MR::initUseStageSwitchReadA(this, rIter);
    fn_8034F9B0(this, rIter);

    // NONMATCHING: the stage-switch FunctorV0M (fn_8034F5C0 with the 0x118 data) is not built here.

    const char* const* pNames = lbl_8064B9C0;
    for (s32 i = 0; i < 9; ++i) {
        fn_8034F900(this, pNames[i], rIter);
    }
    mIsReplyFlag = fn_8035AF00();
    mPostmanEvent = fn_8035AC20(rIter);

    if (isTalkReady) {
        makeActorAppeared();
        return;
    }
    makeActorDead();
}

void KinopioPostman::initAfterPlacement() {
    MR::moveCoordAndTransToRailStartPoint(this);
}

void KinopioPostman::control() {
    NPCActor::control();
    if (mLayout != nullptr) {
        mLayout->movement();  // NONMATCHING: target calls vtable slot 0x14 of LayoutActor
    }
}

// fn_8034FAB0
bool fn_8034FAB0(KinopioPostman* pThis, u32 step) {
    if (step == 0) {
        if (fn_8034FEB0(pThis)) {
            return true;
        }
        fn_8034FF50(pThis);
        return false;
    }
    return true;
}

// fn_8034FB10
bool fn_8034FB10(KinopioPostman* pThis, u32 step) {
    switch (step) {
    case 0:
        if (fn_8034FEB0(pThis)) {
            return true;
        }
        fn_8034FF50(pThis);
        return false;
    case 2:
        fn_8035AD60(pThis, fn_8048AE10(pThis->mLayout));
        pThis->mStepNum = 2;
        KP_RAW_OF(pThis, void*, 0x94) = pThis->mTalkCtrls[2];
        pThis->setNerve(KP_NERVE(lbl_807D4984));
        return true;
    default:
        return true;
    }
}

// fn_8034FBB0
bool fn_8034FBB0(KinopioPostman* pThis, u32 step) {
    switch (step) {
    case 0:
        if (MR::isValidPostmanImportantEvent(fn_8048AE10(pThis->mLayout))) {
            s32 count = MR::getPostmanImportantEventReward(fn_8048AE10(pThis->mLayout));
            if (count > 0) {
                MR::setMessageArg(pThis->mTalkCtrls[3], count);
                return true;
            }
        }
        return false;
    case 1:
        if (!MR::isPlayerLuigi()) {
            return false;
        }
        return (fn_8048AE10(pThis->mLayout) - 1) == 0;
    default:
        return false;
    }
}

// fn_8034FC60
bool fn_8034FC60(KinopioPostman* pThis, u32 step) {
    if (step == 0) {
        if (!MR::isPlayerLuigi()) {
            return false;
        }
        return (fn_8048AE10(pThis->mLayout) - 2) == 0;
    }
    return false;
}

// fn_8034FCC0
bool fn_8034FCC0(KinopioPostman* pThis, u32 step) {
    switch (step) {
    case 0:
        if (fn_8034FEB0(pThis)) {
            return true;
        }
        fn_8034FF50(pThis);
        return false;
    case 2:
        fn_8035AD60(pThis, fn_8048AE10(pThis->mLayout));
        MR::startActionSound(pThis, reinterpret_cast< const char* >(lbl_806B99AC), -1, -1, -1);
        pThis->mStepNum = 2;
        KP_RAW_OF(pThis, void*, 0x94) = pThis->mTalkCtrls[2];
        pThis->setNerve(KP_NERVE(lbl_807D4984));
        return true;
    default:
        return true;
    }
}

// fn_8034F5C0 / fn_8034F8C0 build the functor/talk-func objects; the adapter above replaces them.

// fn_8034F600
bool fn_8034F600(KinopioPostman* pThis) {
    if (!fn_8035AEA0()) {
        return false;
    }
    for (s32 i = 0; i < 7; ++i) {
        pThis->mTalkCtrls[i] = reinterpret_cast< TalkMessageCtrl* >(fn_8035AAA0(pThis, i));
    }

    MR::registerEventFunc(pThis->mTalkCtrls[3], KinopioPostmanTalkFunc(pThis, fn_8034FB10));
    MR::registerBranchFunc(pThis->mTalkCtrls[3], KinopioPostmanTalkFunc(pThis, fn_8034FBB0));
    MR::registerEventFunc(pThis->mTalkCtrls[2], KinopioPostmanTalkFunc(pThis, fn_8034FAB0));
    MR::registerEventFunc(pThis->mTalkCtrls[4], KinopioPostmanTalkFunc(pThis, fn_8034FB10));
    MR::registerBranchFunc(pThis->mTalkCtrls[4], KinopioPostmanTalkFunc(pThis, fn_8034FC60));
    MR::registerEventFunc(pThis->mTalkCtrls[5], KinopioPostmanTalkFunc(pThis, fn_8034FB10));
    MR::registerEventFunc(pThis->mTalkCtrls[6], KinopioPostmanTalkFunc(pThis, fn_8034FCC0));
    MR::registerEventFunc(pThis->mTalkCtrls[1], KinopioPostmanTalkFunc(pThis, fn_8034FB10));
    MR::registerEventFunc(pThis->mTalkCtrls[0], KinopioPostmanTalkFunc(pThis, fn_8034FAB0));

    KP_RAW_OF(pThis, void*, 0x94) = pThis->mTalkCtrls[0];
    pThis->mLayout = fn_8035ABB0();
    pThis->mStepNum = fn_8035ACB0();
    pThis->mIsEventFlag = (pThis->mStepNum - 2) != 0;
    KP_RAW_OF(pThis, void*, 0x94) = pThis->mTalkCtrls[pThis->mStepNum];
    return true;
}

// fn_8034F900
bool fn_8034F900(KinopioPostman* pThis, const char* pName, const JMapInfoIter& rIter) {
    if (MR::tryRegisterDemoCast(pThis, pName, rIter)) {
        if (pThis->mLayout != nullptr) {
            DemoFunction::registerDemoTalkMessageCtrlDirect(pThis, KP_RAW_OF(pThis, TalkMessageCtrl*, 0x94), pName);
        }
        // NONMATCHING: the demo-action functor (fn_8034F5C0 with the 0x... data) is not registered here.
        return true;
    }
    return false;
}

// fn_8034F9B0
void fn_8034F9B0(KinopioPostman* pThis, const JMapInfoIter& rIter) {
    s32 num = MR::getChildObjNum(rIter);
    for (s32 i = 0; i < num; ++i) {
        const char* pChildName = nullptr;
        MR::getChildObjName(&pChildName, rIter, i);
        if (MR::isEqualString(pChildName, reinterpret_cast< const char* >(lbl_807CFE90))) {
            pThis->mChildObj = NameObjFactory::initChildObj(rIter, i);
            break;
        }
    }
    reinterpret_cast< LiveActor* >(pThis->mChildObj)->makeActorDead();
}

// fn_8034FA60
void fn_8034FA60(KinopioPostman* pThis) {
    if (!fn_8035AEA0()) {
        reinterpret_cast< LiveActor* >(pThis->mChildObj)->makeActorAppeared();
    }
}

// fn_8034FDE0
bool fn_8034FDE0(LiveActor* pActor) {
    KinopioPostman* pThis = static_cast< KinopioPostman* >(pActor);
    if (!pActor->isNerve(KP_NERVE(lbl_807D4988))) {
        return false;
    }
    if (!pThis->mIsEventFlag) {
        return false;
    }
    KP_RAW_OF(pThis, void*, 0x94) = pThis->mTalkCtrls[pThis->mStepNum];
    return true;
}

// fn_8034FE40
bool fn_8034FE40(LiveActor* pActor) {
    KinopioPostman* pThis = static_cast< KinopioPostman* >(pActor);
    if (!pActor->isNerve(KP_NERVE(lbl_807D4988))) {
        return false;
    }
    if (!pThis->mIsEventFlag) {
        return false;
    }
    KP_RAW_OF(pThis, void*, 0x94) = pThis->mTalkCtrls[pThis->mStepNum];
    return true;
}

// fn_8034FEB0
bool fn_8034FEB0(KinopioPostman* pThis) {
    if (!MR::isDead(pThis->mLayout) && fn_8048AE00(pThis->mLayout)) {
        if (pThis->mWaitTimer < 0) {
            pThis->mWaitTimer = 0x23;
        }
        pThis->mWaitTimer -= 1;
        if (pThis->mWaitTimer < 0) {
            pThis->mIsEventFlag = false;
            pThis->mLayout->kill();  // NONMATCHING: target calls vtable slot 0x30 of LayoutActor
            return true;
        }
    }
    return false;
}

// fn_8034FF50
void fn_8034FF50(KinopioPostman* pThis) {
    if (MR::isDead(pThis->mLayout)) {
        if (pThis->mWaitTimer < 0) {
            pThis->mWaitTimer = lbl_807E3100;
        }
        pThis->mWaitTimer -= 1;
        if (pThis->mWaitTimer < 0) {
            pThis->mLayout->appear();  // NONMATCHING: target calls vtable slot 0x2C of LayoutActor
        }
    }
}

// fn_8034FFD0: turns the quaternion's facing toward rTo around its up axis, then blends up/front.
void fn_8034FFD0(TQuat4f* pQuat, const TVec3f& rUp, const TVec3f& rTo, f32 rate) {
    TVec3f sp2C(rTo);
    TVec3f sp20(rUp);
    f32 x = pQuat->x;
    f32 y = pQuat->y;
    f32 z = pQuat->z;
    f32 w = pQuat->w;
    TVec3f sp14;
    sp14.set<f32>(2.0f * (x * y) - 2.0f * (w * z), (1.0f - 2.0f * (x * x)) - 2.0f * (z * z), 2.0f * (y * z) + 2.0f * (w * x));
    TVec3f sp8;
    sp8.set<f32>(2.0f * (x * z) + 2.0f * (w * y), 2.0f * (y * z) - 2.0f * (w * x), (1.0f - 2.0f * (x * x)) - 2.0f * (y * y));
    MR::turnVecToVecCosOnPlane(&sp8, sp2C, sp14, rate);
    MR::blendQuatUpFront(pQuat, sp20, sp8, 1.0f, 1.0f);
}

// fn_8034F5C0 / fn_8034F8C0: target-side construction of the functor records. NONMATCHING stubs:
// the adapter above builds the equivalent talk callbacks directly.
void fn_8034F5C0(void*, KinopioPostman*, const u32*) {
}

void fn_8034F8C0(void*, KinopioPostman*, const u32*) {
}

// fn_80350120
void fn_80350120(KinopioPostman* pThis, LiveActor* pActor) {
    (void)pActor;
    if (MR::isFirstStep(pThis)) {
        MR::startAction(pThis, reinterpret_cast< const char* >(lbl_806B99C0));
    }
    // NONMATCHING: the target's player-direction math (operator- with an unset arg in the m2c draft) is not recovered.
    TVec3f sp14 = *MR::getPlayerPos() - pThis->mPosition;
    TVec3f sp8 = -pThis->mPosition;
    fn_8034FFD0(reinterpret_cast< TQuat4f* >(reinterpret_cast< u8* >(pThis) + 0xA4), sp8, sp14, 0.997f);
    if (!MR::tryStartReactionAndPushNerve(pThis, KP_NERVE(lbl_807D497C))) {
        MR::tryTalkNearPlayerAndStartMoveTalkAction(pThis);
    }
}

// fn_803501C0
void fn_803501C0(KinopioPostman* pThis, s32 soundStep) {
    (void)soundStep;
    if (KP_RAW_OF(pThis, u8, 0xDC) != 0) {
        MR::startActionSound(pThis, reinterpret_cast< const char* >(lbl_807CFE9C), -1, -1, -1);
        MR::startActionSound(pThis, reinterpret_cast< const char* >(lbl_807CFE98), -1, -1, -1);
    }
    if (pThis->isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(pThis, reinterpret_cast< const char* >(lbl_807CFE94), -1, -1, -1);
    }
    if (KP_RAW_OF(pThis, u8, 0xDF) != 0) {
        MR::startActionSound(pThis, reinterpret_cast< const char* >(lbl_807CFE90), -1, -1, -1);
    }
    MR::tryStartReactionAndPopNerve(pThis);
}

// fn_80350290
void fn_80350290(KinopioPostman* pThis, LiveActor* pActor) {
    if (MR::isFirstStep(pActor)) {
        if (!MR::isExistRail(pActor)) {
            MR::onCalcShadowOneTime(pActor, nullptr);
        }
        fn_8034FDE0(pThis);
    }
    if (!MR::tryStartReactionAndPushNerve(pThis, KP_NERVE(lbl_807D497C))) {
        if (fn_804C24B0()) {
            // NONMATCHING: the talk-end branch of the target is only partly recovered.
            TalkMessageCtrl* pCtrl = KP_RAW_OF(pThis, TalkMessageCtrl*, 0x94);
            if (MR::tryTalkNearPlayerAtEnd(pCtrl)) {
                MR::startAction(pThis, reinterpret_cast< const char* >(lbl_806B9A18));
            }
        }
        if (MR::isActionContinuous(pThis)) {
            if (pThis->isNerve(KP_NERVE(lbl_807D498C))) {
                if (MR::isAnyAnimPlaying(pThis, reinterpret_cast< const char* >(lbl_806B9A18))) {
                    TVec3f sp14 = *MR::getPlayerPos() - pThis->mPosition;
                    TVec3f sp8 = -pThis->mPosition;
                    fn_8034FFD0(reinterpret_cast< TQuat4f* >(reinterpret_cast< u8* >(pThis) + 0xA4), sp8, sp14, 0.997f);
                }
            }
        } else {
            if (pThis->isNerve(KP_NERVE(lbl_807D4988))) {
                if (MR::isNearPlayer(pThis, 200.0f)) {
                    MR::startAction(pThis, reinterpret_cast< const char* >(lbl_806B9A18));
                    fn_8034FE40(pThis);
                    pThis->setNerve(KP_NERVE(lbl_807D498C));
                    return;
                }
                MR::tryStartMoveTalkAction(pThis);
                return;
            }
            if (pThis->isNerve(KP_NERVE(lbl_807D498C))) {
                if (!MR::isNearPlayer(pThis, 300.0f)) {
                    pThis->setNerve(KP_NERVE(lbl_807D4988));
                }
                MR::tryStartTalkAction(pThis);
            }
        }
    }
}

// fn_80350440
void fn_80350440(KinopioPostman* pThis, LiveActor* pActor) {
    if (MR::isFirstStep(pActor)) {
        MR::startAction(pThis, reinterpret_cast< const char* >(lbl_806B9A18));
    }
    if (MR::isActionEnd(pThis)) {
        if (pThis->mIsReplyFlag) {
            pThis->makeActorDead();
        }
        pThis->setNerve(KP_NERVE(lbl_807D498C));
    }
}

// fn_803504C0
void fn_803504C0(KinopioPostman* pThis, LiveActor* pActor) {
    if (MR::isFirstStep(pActor)) {
        if (!MR::isExistRail(pActor)) {
            MR::onCalcShadowOneTime(pActor, nullptr);
        }
        fn_8034FDE0(pThis);
        TVec3f sp8;
        MR::findNamePos(reinterpret_cast< const char* >(lbl_807CFE90), &sp8, nullptr);
        TVec3f sp20 = -sp8;
        TVec3f sp14 = -sp20;
        fn_8034FFD0(reinterpret_cast< TQuat4f* >(reinterpret_cast< u8* >(pThis) + 0xA4), sp14, sp20, -1.0f);
        MR::startAction(pThis, reinterpret_cast< const char* >(lbl_806B9918));
    }
    if (!MR::isTimeKeepDemoActive()) {
        MR::startAction(pThis, reinterpret_cast< const char* >(lbl_806B9A18));
        if (pThis->mIsReplyFlag) {
            MR::startPlayerNormalBva();
        }
        pThis->setNerve(KP_NERVE(lbl_807D498C));
    }
}

// fn_803505A0
void fn_803505A0(KinopioPostman* pThis, LiveActor* pActor) {
    MR::moveCoordAndTransToRailStartPoint(pActor);
    MR::followRailPoseOnGround(pThis, 1.0f);
    if (pThis->mIsReplyFlag) {
        pThis->mPostmanEvent->makeActorDead();
    }
    pThis->setNerve(KP_NERVE(lbl_807D4980));
}
