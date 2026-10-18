#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/BgmUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util.hpp"

extern "C" const char lbl_8066CF90[];
extern "C" const char lbl_8066CF98[];
extern "C" const char lbl_8066CFC0[];
extern "C" const char lbl_8066CFF0[];
extern "C" const char lbl_8066D004[];
extern "C" const char lbl_8066D018[];
extern "C" const char lbl_8066D024[];
extern "C" const char lbl_807CF4E8[];
extern "C" const f32 lbl_807D7828;
extern "C" const f32 lbl_807D782C;
extern "C" const f32 lbl_807D7830;
extern "C" const f32 lbl_807D7834;
extern "C" const f32 lbl_807D7838;
extern "C" const f32 lbl_807D783C;
extern "C" const f32 lbl_807D7840;
extern "C" const f32 lbl_807D7844;
extern "C" const f32 lbl_807D7848;
extern "C" const f32 lbl_807D784C;
extern "C" const f32 lbl_807D7850;
extern "C" const f32 lbl_807D7854;
extern "C" const f32 lbl_807D7858;
extern "C" const f32 lbl_807D785C;
extern "C" const f32 lbl_807D7860;
extern "C" const f32 lbl_807D7864;
extern "C" const f32 lbl_807D7868;
extern "C" const f32 lbl_807D786C;
extern "C" const f32 lbl_807D7874;
extern "C" const f32 lbl_807D787C;

extern "C" int snprintf(char*, unsigned long, const char*, ...);
extern "C" void __dl__FPv(void*);
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" s32 fn_80038870(TVec3f*, TVec3f*, TVec3f*, s32, s32, s32, s32, f32);

// Pod helpers (TU05)
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_80094460(BellyDragonPod*, BellyDragon*);
extern "C" void fn_80094520(BellyDragonPod*, s32, const TVec3f*);
extern "C" void fn_80094600(BellyDragonPod*);
extern "C" void fn_80094670(BellyDragonPod*, s32);
extern "C" void fn_80094930(BellyDragonPod*);
extern "C" void fn_800949D0(BellyDragonPod*);
extern "C" void fn_800949E0(BellyDragonPod*);
extern "C" void fn_80094BE0(BellyDragonPod*);
extern "C" void fn_80094C80(BellyDragonPod*);

// Body (TU02) helpers
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them

// Misc helpers (other units)
// PLACEHOLDER(core): fn_8008EDA0 is a C++ (non-extern "C") function in SMG2 (symbol fn_8008EDA0__FP9LiveActor); resolve when named
bool fn_8008EDA0(LiveActor*);
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_8008EDB0(void*, f32);
extern "C" void fn_80013880(LiveActor*, TVec3f*, s32);
extern "C" s32 fn_80008F00(LiveActor*, const char*);

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them
extern "C" void fn_80093450(void*, const f32 (*)[4]);
extern "C" void fn_80093370(BellyDragon*, LiveActor*);
extern "C" s32 fn_800938F0(BellyDragon*);

// BODY_PART indexes the weak points of BellyDragonBody (BellyDragon.hpp).
#define BODY_COUNT(b) ((b)->mWeakPointNum)
#define BODY_PART(b, i) ((b)->mWeakPoints[i])

namespace NrvBellyDragonPodActor {
    NEW_NERVE(BellyDragonPodActorNrvWait, BellyDragonPodActor, Wait);
};  // namespace NrvBellyDragonPodActor

// Group-B actor (0xC4 bytes). Host at 0x90, base matrix at 0x94.
BellyDragonPodActor::BellyDragonPodActor(BellyDragon* pHost) : LiveActor(lbl_8066CFF0), mHost(pHost) {
    mBaseMtx.identity();
}

BellyDragonPodActor::~BellyDragonPodActor() {
}

// ---------------------------------------------------------------------------------------------

extern "C" u8 fn_80093310(BellyDragon* pHost) {
    return pHost->mIsLv2;
}

extern "C" s32 fn_80093320(NerveExecutor* pArm, BellyDragon* pHost) {
    s32 result = MR::updateActorState(pArm);
    fn_8008EDB0(pHost, lbl_807D782C);
    return result;
}

extern "C" void fn_80093370(BellyDragon* pHost, LiveActor*) {
    MR::onBind(pHost);
    LiveActor* pBody = pHost->mBody->mHead;
    MR::setPosition(pHost, pBody->mPosition);
    MR::calcGravity(pHost);
    TPos3f sp20;
    sp20.set(pBody->getBaseMtx());
    TVec3f sp14(sp20.mMtx[0][2], sp20.mMtx[1][2], sp20.mMtx[2][2]);
    TVec3f sp8(sp20.mMtx[0][1], sp20.mMtx[1][1], sp20.mMtx[2][1]);
    MR::makeMtxFrontUpPos(&sp20, sp14, sp8, pBody->mPosition);
    pHost->mBaseMtx.set(sp20.mMtx);
    pHost->mFront.set(sp14);
    fn_80093450(pBody, sp20.mMtx);
    pHost->mBody->unfreeze();
}

extern "C" void fn_80093450(void* pObj, const f32 (*mtx)[4]) {
    ((TPos3f*)((u8*)pObj + 0xA8))->set(mtx);
    ((TVec3f*)((u8*)pObj + 0x14))->set< f32 >(*(f32*)((u8*)pObj + 0xB4), *(f32*)((u8*)pObj + 0xC4), *(f32*)((u8*)pObj + 0xD4));
}

extern "C" void fn_800934A0(BellyDragon* pHost, f32 rate) {
    fn_8008EDB0(pHost, rate);
}

extern "C" void fn_800934B0(BellyDragon* pHost, s32 arg1) {
    fn_80013880(pHost, &pHost->mFront, arg1);
    MR::sendMsgEnemyAttackToBindedSensor(pHost, pHost->getSensor(lbl_8066CF90));
}

extern "C" void fn_80093500(BellyDragon* pHost, f32 farg0, f32 farg1) {
    TVec3f sp8 = pHost->mFront * farg0;
    MR::addVelocity(pHost, sp8);
    MR::attenuateVelocity(pHost, farg1);
    MR::sendMsgEnemyAttackToBindedSensor(pHost, pHost->getSensor(lbl_8066CF90));
}

extern "C" void fn_80093580(BellyDragon* pHost) {
    pHost->mBody->startBindB();
}

extern "C" void fn_80093590(TPos3f* pOut, NameObj* pObj, const f32 (*mtx)[4], f32 farg0) {
    TVec3f sp5C(mtx[0][0], mtx[1][0], mtx[2][0]);
    TVec3f sp50(mtx[0][2], mtx[1][2], mtx[2][2]);
    TVec3f sp44(mtx[0][3], mtx[1][3], mtx[2][3]);
    TVec3f sp14 = sp50 * farg0;
    sp44.add(sp14);
    TVec3f sp38(sp44);
    TVec3f sp2C;
    MR::calcGravityVector(pObj, sp44, &sp2C, NULL, 0);
    TVec3f sp20 = -sp2C;
    TVec3f sp8 = sp20 * lbl_807D7830;
    sp44.add(sp8);
    fn_80038870(&sp44, &sp38, &sp2C, 0, 0, 0, 0, lbl_807D7834);
    MR::makeMtxUpSidePos(pOut, sp20, sp5C, sp38);
}

extern "C" void fn_800936C0(TPos3f* pOut, NameObj* pObj, const TVec3f* pPos) {
    TVec3f sp2C(*pPos);
    TVec3f sp20;
    MR::calcGravityVector(pObj, sp2C, &sp20, NULL, 0);
    TVec3f sp14 = -sp20;
    TVec3f sp8 = sp14 * lbl_807D7830;
    sp2C.add(sp8);
    fn_80038870(&sp2C, &sp2C, &sp20, 0, 0, 0, 0, lbl_807D7834);
    MR::makeMtxUpNoSupportPos(pOut, sp14, sp2C);
}

extern "C" void fn_80093780(BellyDragon* pHost) {
    pHost->mBody->appearJawHill();
}

extern "C" void fn_80093790(BellyDragon* pHost) {
    pHost->mBody->startHeadAttackHill();
}

extern "C" void fn_800937A0(BellyDragon* pHost) {
    pHost->mBody->appearHeadHill();
}

extern "C" void fn_800937B0(BellyDragon* pHost) {
    pHost->mBody->breakHeadHill();
}

extern "C" void fn_800937C0(BellyDragon* pHost) {
    if (fn_8008EDA0(pHost)) {
        fn_80094460(pHost->mPod, pHost);
    }
}

extern "C" void fn_80093800(BellyDragon* pHost) {
    fn_80094930(pHost->mPod);
}

extern "C" void fn_80093810(BellyDragon* pHost) {
    s32 count = BODY_COUNT(pHost->mBody);
    for (s32 i = 0; i < count; i++) {
        LiveActor* pPart = BODY_PART(pHost->mBody, i);
        if (!MR::isDead(pPart)) {
            pPart->kill();
        }
    }
}

extern "C" void fn_800938A0(BellyDragon* pHost) {
    fn_80094520(pHost->mPod, 0, NULL);
}

extern "C" void fn_800938B0(BellyDragon* pHost) {
    fn_80094520(pHost->mPod, 1, NULL);
}

extern "C" void fn_800938C0(BellyDragon* pHost, const TVec3f* pVec) {
    fn_80094520(pHost->mPod, 0, pVec);
}

extern "C" void fn_800938D0(BellyDragon* pHost) {
    fn_80094600(pHost->mPod);
}

extern "C" void fn_800938E0(BellyDragon* pHost, s32 count) {
    fn_80094670(pHost->mPod, count);
}

extern "C" s32 fn_800938F0(BellyDragon* pHost) {
    s32 count = BODY_COUNT(pHost->mBody);
    s32 alive = 0;
    for (s32 i = 0; i < count; i++) {
        if (BODY_PART(pHost->mBody, i)->isAttackable()) {
            alive += 1;
        }
    }
    return alive;
}

extern "C" bool fn_80093970(HitSensor* pSensor) {
    return MR::isSensorType(pSensor, 0x39);
}

extern "C" s32 fn_80093980(BellyDragon* pHost, s32 flag) {
    if (fn_8008EDA0(pHost)) {
        return 0;
    }
    s32 need = 3;
    if (flag != 0) {
        need = 2;
    }
    s32 ok = 0;
    s32 count = BODY_COUNT(pHost->mBody);
    for (s32 i = 0; i < count; i++) {
        if (!BODY_PART(pHost->mBody, i)->isAttackable()) {
            ok += 1;
            if (ok >= need) {
                return 1;
            }
        }
    }
    return 0;
}

extern "C" s32 fn_80093A30(BellyDragon* pHost, s32 flag) {
    s32 n = fn_800938F0(pHost);
    if (flag != 0) {
        n -= 1;
    }
    if (n <= 0) {
        return 1;
    }
    return 0;
}

extern "C" void fn_80093A80(BellyDragon* pHost) {
    s32 count = BODY_COUNT(pHost->mBody);
    for (s32 i = 0; i < count; i++) {
        BODY_PART(pHost->mBody, i)->setNerveWaitC();
    }
}

extern "C" void fn_80093AF0(BellyDragon* pHost) {
    fn_800949D0(pHost->mPod);
}

extern "C" void fn_80093B00(BellyDragon* pHost) {
    fn_800949E0(pHost->mPod);
}

extern "C" void fn_80093B10(BellyDragon* pHost) {
    fn_80094BE0(pHost->mPod);
}

extern "C" void fn_80093B20(BellyDragon* pHost) {
    fn_80094C80(pHost->mPod);
}

extern "C" HitSensor* fn_80093B30(LiveActor* pActor) {
    return pActor->getSensor(lbl_8066CF90);
}

extern "C" void fn_80093B40(BellyDragon* pHost, const char* pName) {
    BellyDragonPod* pPod = pHost->mPod;
    MR::initAnimCamera(pHost, pPod->mCameraInfo, pName);
}

extern "C" void fn_80093B50(BellyDragon* pHost, const char* pName) {
    BellyDragonPod* pPod = pHost->mPod;
    pHost->mBaseMtx.set(pPod->mHostMtx.mMtx);
    TVec3f sp8(pPod->mHostMtx.mMtx[0][3], pPod->mHostMtx.mMtx[1][3], pPod->mHostMtx.mMtx[2][3]);
    MR::setPosition(pHost, sp8);
    MR::zeroVelocity(pHost);
    pHost->mBody->freeze();
    if (fn_80008F00(pHost, pName)) {
        MR::startAction(pHost, pName);
    }
    MR::calcAnimDirect(pHost);
}

extern "C" s32 fn_80093BF0(BellyDragon* pHost, const char* pName, bool) {
    if (!MR::tryStartDemoMarioPuppetable(pHost, pName)) {
        return 0;
    }
    MR::setPlayerPos(lbl_8066CF98);
    MR::startBckPlayer(lbl_8066CFC0, NULL);
    fn_80093B50(pHost, pName);
    BellyDragonPod* pPod = pHost->mPod;
    TVec3f sp28(pPod->mHostMtx.mMtx[0][3], pPod->mHostMtx.mMtx[1][3], pPod->mHostMtx.mMtx[2][3]);
    MR::requestMovementOn(pPod->mDecoration);
    MR::setPosition(pPod->mDecoration, sp28);
    pPod->mDecoration->makeActorAppeared();
    MR::startAction(pPod->mDecoration, pName);
    MR::calcAnimDirect(pPod->mDecoration);
    MR::offEntryDrawBuffer(pPod->mDecoration);
    pHost->mBaseMtx.set(pPod->mHostMtx.mMtx);
    TVec3f sp8(pPod->mHostMtx.mMtx[0][3], pPod->mHostMtx.mMtx[1][3], pPod->mHostMtx.mMtx[2][3]);
    MR::setPosition(pHost, sp8);
    MR::zeroVelocity(pHost);
    pHost->mBody->end();
    LiveActor* pBody = pHost->mBody->mHead;
    fn_80093450(pBody, MR::getJointMtx(pPod->mDecoration, (s32)0));
    pHost->mBaseMtx.set(MR::getJointMtx(pPod->mDecoration, (s32)0));
    MR::setPosition(pHost, pBody->mPosition);
    ((TPos3f*)((u8*)pPod->mCameraTargetMtx + 0x14))->set(pPod->mHostMtx.mMtx);
    CameraTargetArg targetArg;
    MR::startAnimCameraTargetOther(pHost, pPod->mCameraInfo, pName, targetArg, 0, false, lbl_807D7828);
    return 1;
}

extern "C" s32 fn_80093DC0(BellyDragon* pHost, s32 idx, bool useFirst, f32 farg0) {
    BellyDragonPod* pPod = pHost->mPod;
    LiveActor* pDeco = pPod->mDecoration;
    ((void (*)(void*, s32))(*(void***)pDeco)[5])(pDeco, idx);
    ((void (*)(void*))(*(void***)pDeco)[7])(pDeco);
    const f32 (*mtx)[4] = MR::getJointMtx(pDeco, (s32)0);
    TVec3f sp44(mtx[0][2], mtx[1][2], mtx[2][2]);
    TVec3f sp38(mtx[0][3], mtx[1][3], mtx[2][3]);
    pHost->mFront.set(sp44);
    MR::setPosition(pHost, sp38);
    fn_8008EDB0(pHost, farg0);
    s32 found;
    if (useFirst) {
        found = MR::tryEndDemoAndAnimCamera(pHost, pPod->mCameraInfo, reinterpret_cast<const char*>(idx), 0);
    } else {
        found = MR::endDemoAndAnimCameraIfActionEnd(pHost, pPod->mCameraInfo, reinterpret_cast<const char*>(idx), 0);
    }
    if (found != 0) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            pHost->mBody->forceDeleteEffects();
        }
        char name[0x100];
        snprintf(name, 0x100, lbl_8066CF90 + 0x3C, idx);
        fn_80093B50(pHost, name);
        const f32 (*mtxB)[4] = MR::getJointMtx(pHost, lbl_8066CF90 + 0x44);
        TVec3f sp2C(-mtxB[0][0], -mtxB[1][0], -mtxB[2][0]);
        TVec3f sp20(mtxB[0][1], mtxB[1][1], mtxB[2][1]);
        TVec3f sp14(mtxB[0][2], mtxB[1][2], mtxB[2][2]);
        TVec3f sp8(mtxB[0][3], mtxB[1][3], mtxB[2][3]);
        TPos3f mm;
        mm.mMtx[0][0] = sp14.x;
        mm.mMtx[1][0] = sp14.y;
        mm.mMtx[2][0] = sp14.z;
        mm.mMtx[0][1] = sp20.x;
        mm.mMtx[1][1] = sp20.y;
        mm.mMtx[2][1] = sp20.z;
        mm.mMtx[0][2] = sp2C.x;
        mm.mMtx[1][2] = sp2C.y;
        mm.mMtx[2][2] = sp2C.z;
        mm.mMtx[0][3] = sp8.x;
        mm.mMtx[1][3] = sp8.y;
        mm.mMtx[2][3] = sp8.z;
        fn_80093450(pHost->mBody->mHead, mm.mMtx);
        fn_80093370(pHost, NULL);
        MR::startAction(pHost, lbl_8066CF90 + 0x4C);
        return 1;
    }
    return 0;
}

extern "C" void fn_80094080(LiveActor* pActor, const JMapInfoIter& rIter, s32 unusedA, s32 unusedB, bool flag) {
    MR::initActor(pActor, rIter, lbl_8066D004, false);
    pActor->initNerve(&NrvBellyDragonPodActor::BellyDragonPodActorNrvWait::sInstance, 0);
    pActor->makeActorDead();
}

extern "C" void fn_800940E0(LiveActor* pActor) {
    pActor->LiveActor::appear();
    pActor->setNerve(&NrvBellyDragonPodActor::BellyDragonPodActorNrvWait::sInstance);
}

void BellyDragonPodActor::exeWait() {
    MR::isFirstStep(this);
    MR::addVelocityToGravity(this, lbl_807D7838);
    MR::attenuateVelocity(this, lbl_807D783C);
    if (MR::isBinded(this)) {
        fn_800938C0(mHost, &mPosition);
        MR::tryHitReaction(this, lbl_8066D018);
        MR::startActionSound(this, lbl_807CF4E8, -1, -1, -1);
        kill();
        return;
    }
    if (MR::isGreaterStep(this, 0x5A)) {
        MR::startActionSound(this, lbl_8066D024, -1, -1, -1);
    }
    if (MR::isGreaterEqualStep(this, 0x384)) {
        MR::tryHitReactionBreak(this);
        kill();
    }
}

extern "C" void fn_80094220(BellyDragonPodActor* pActor) {
    pActor->mBaseMtx.mMtx[0][3] = pActor->mPosition.x;
    pActor->mBaseMtx.mMtx[1][3] = pActor->mPosition.y;
    pActor->mBaseMtx.mMtx[2][3] = pActor->mPosition.z;
    MR::setBaseTRMtx(pActor, pActor->mBaseMtx);
}

extern "C" void fn_80094240(void*, HitSensor* pSender, HitSensor* pReceiver) {
    MR::sendMsgEnemyAttackFire(pReceiver, pSender);
}

extern "C" NameObj* fn_80094250(NameObj* pObj, s32 flag) {
    if (pObj != NULL) {
        pObj->~NameObj();
        if (flag > 0) {
            __dl__FPv(pObj);
        }
    }
    return pObj;
}
