#include "Game/LiveActor/ActorItemGenerator.hpp"
#include "Game/Util/BellyDragon.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

extern "C" const char lbl_8066D0B0[];
extern "C" const char lbl_8066D0F4[];
extern "C" const char lbl_8066D120[];
extern "C" const char lbl_8066D128[];
extern "C" const char lbl_8066D130[];
extern "C" const char lbl_8066D138[];
extern "C" const char lbl_8066D140[];
extern "C" const char lbl_8066D14C[];
extern "C" const char lbl_8066D158[];
extern "C" const char lbl_8066D184[];
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
extern "C" const f32 lbl_807D7870;
extern "C" const f32 lbl_807D7874;
extern "C" const f32 lbl_807D7878;
extern "C" const f32 lbl_807D787C;
extern "C" const f32 lbl_807D7880;
extern "C" const f32 lbl_807E2FE0;

extern "C" s32 fn_80038870(TVec3f*, TVec3f*, TVec3f*, s32, s32, s32, s32, f32);
extern "C" int snprintf(char*, unsigned long, const char*, ...);
extern "C" void* __nw__FUl(u32);

// Pod helpers (TU05). Pod group members are BellyDragonFireGround (0xC4, TPos3f at 0x90) and group B actors.
extern "C" NameObj* fn_80092E70(void*);
extern "C" void* fn_80095970(void*, const char*, const char*, TPos3f*, s32);
extern "C" void fn_80095B00(void*, const char*);
extern "C" void fn_80095C30(void*);
extern "C" void fn_80095C80(void*);
extern "C" void fn_80095D60(void*);
extern "C" void fn_80095CD0(void*, const char*, TPos3f*);
extern "C" void fn_80094C90(BellyDragonPod*);

// Host helpers (TU04)
extern "C" u8 fn_80093310(BellyDragon*);
extern "C" s32 fn_80093320(NerveExecutor*, BellyDragon*);
extern "C" void fn_80093370(BellyDragon*, LiveActor*);
extern "C" void fn_80093590(TPos3f*, NameObj*, const f32 (*)[4], f32);
extern "C" void fn_800936C0(TPos3f*, NameObj*, const TVec3f*);
extern "C" void fn_800937C0(BellyDragon*);
extern "C" void fn_80093450(void*, const f32 (*)[4]);
extern "C" s32 fn_80093970(HitSensor*);
extern "C" s32 fn_80093980(BellyDragon*, s32);
extern "C" s32 fn_80093A30(BellyDragon*, s32);
extern "C" void fn_800934A0(BellyDragon*, f32);

// Arm states (other TUs)
extern "C" ActorStateBaseInterface* fn_80097520(void*, BellyDragon*, u8);
extern "C" ActorStateBaseInterface* fn_80096060(void*, BellyDragon*, u8);
extern "C" ActorStateBaseInterface* fn_80096960(void*, BellyDragon*, u8);
extern "C" void fn_80096220(ActorStateBaseInterface*, HitSensor*, HitSensor*);
extern "C" void fn_80096B60(ActorStateBaseInterface*, HitSensor*, HitSensor*);
extern "C" void fn_80097690(ActorStateBaseInterface*);
extern "C" void fn_800976A0(ActorStateBaseInterface*);
extern "C" bool fn_80097A50(ActorStateBaseInterface*);
extern "C" void fn_8008EDB0(void*, f32);


namespace NrvBellyDragonArm {
    NEW_NERVE(BellyDragonArmNrvWait, BellyDragonArm, Wait);
    NEW_NERVE(BellyDragonArmNrvFire, BellyDragonArm, Fire);
    NEW_NERVE(BellyDragonArmNrvShot, BellyDragonArm, Shot);
    NEW_NERVE(BellyDragonArmNrvAttack, BellyDragonArm, Attack);
};  // namespace NrvBellyDragonArm

// Pod: 0x80 bytes, plain class (no vtable).
BellyDragonPod::BellyDragonPod(BellyDragon* pHost, const JMapInfoIter* pIter) {
    mHost = pHost;
    mCameraInfo = NULL;
    mFlameCtrl = NULL;
    mDecoration = NULL;
    mCameraTargetMtx = NULL;
    mEffectCount = 0;
    mGroupA = NULL;
    mGroupB = NULL;
    mBaseMtx.identity();
    mHostMtx.set(pHost->getBaseMtx());
    mCameraInfo = new ActorCameraInfo(*pIter);
    MR::declareCameraRegisterVec(pHost, 0, &pHost->mPosition);
    mCameraTargetMtx = new CameraTargetMtx(lbl_8066D0B0);
    void* pMem = __nw__FUl(0x34);
    if (pMem != NULL) {
        pMem = fn_80095970(pMem, lbl_8066D0B0 + 0x1C, lbl_8066D0B0 + 0x30, &mBaseMtx, 0x20);
    }
    mFlameCtrl = pMem;
    for (s32 i = 0; i < 0x20; i++) {
        char name[0x20];
        snprintf(name, 0x20, lbl_8066D0B0 + 0x44, i);
        fn_80095B00(mFlameCtrl, name);
    }
    mDecoration = MR::createModelObjMapObjDecoration(lbl_8066D0B0 + 0x50, lbl_8066D0B0 + 0x5C, NULL);
    mDecoration->makeActorDead();
    if (fn_80093310(mHost)) {
        fn_80094C90(this);
    }
}

extern "C" void fn_80094460(BellyDragonPod* pPod, BellyDragon* pHost) {
    pPod->mEffectCount += 1;
    if (pPod->mEffectCount == 0x14) {
        MR::emitEffect(pHost, lbl_8066D120);
        MR::startActionSound(pHost, lbl_8066D120, -1, -1, -1);
    }
    if (pPod->mEffectCount == 0x28) {
        MR::emitEffect(pHost, lbl_8066D128);
        MR::startActionSound(pHost, lbl_8066D128, -1, -1, -1);
    }
    if (pPod->mEffectCount >= 0x28) {
        pPod->mEffectCount = 0;
    }
}

extern "C" void fn_80094520(BellyDragonPod* pPod, s32 isShort, const TVec3f* pVec) {
    if (fn_80093310(pPod->mHost)) {
        LiveActor* pActor = pPod->mGroupA->getDeadActor();
        if (pActor != NULL) {
            if (pVec != NULL) {
                fn_800936C0((TPos3f*)((u8*)pActor + 0x90), pPod->mHost, pVec);
            } else {
                f32 scale = lbl_807D784C;
                if (isShort != 0) {
                    scale = lbl_807D7850;
                }
                fn_80093590((TPos3f*)((u8*)pActor + 0x90), pPod->mHost, pPod->mHost->getBaseMtx(), scale);
            }
            pActor->appear();
        }
    }
}

extern "C" void fn_80094600(BellyDragonPod* pPod) {
    if (fn_80093310(pPod->mHost)) {
        pPod->mGroupA->makeAllDead();
        for (s32 i = 0; i < pPod->mGroupA->getObjNum(); i++) {
            MR::tryForceDeleteEffectAll(pPod->mGroupA->getActor(i));
        }
    }
}

extern "C" void fn_80094670(BellyDragonPod* pPod, s32 count) {
    if (fn_80093310(pPod->mHost) == 0) {
        return;
    }
    f32 step = lbl_807D7870 / (f32)count;
    f32 baseDeg = MR::getRandomDegree();
    for (s32 i = 0; i < count; i++) {
        LiveActor* pActor = pPod->mGroupB->getDeadActor();
        if (pActor == NULL) {
            continue;
        }
        const f32 (*pMtxA)[4] = MR::getJointMtx(pPod->mHost, lbl_8066D130);
        const f32 (*pMtxB)[4] = MR::getJointMtx(pPod->mHost, lbl_8066D138);
        TVec3f pos(pMtxA[0][3], pMtxA[1][3], pMtxA[2][3]);
        TVec3f front(pMtxA[0][0], pMtxA[1][0], pMtxA[2][0]);
        TVec3f up(pMtxA[0][1], pMtxA[1][1], pMtxA[2][1]);
        MR::normalize(&front);
        MR::normalize(&up);
        TVec3f dir(pMtxB[0][0], pMtxB[1][0], pMtxB[2][0]);
        MR::normalize(&dir);
        dir.scale(lbl_807D7848);
        if (count == 1) {
            MR::setPosition(pActor, pos);
            TVec3f axis(0, 1, 0);
            MR::turnVecToVecDegree(&dir, dir, up, lbl_807D785C, axis);
            dir.scale(lbl_807D7858);
        } else {
            TVec3f sp14(up);
            sp14.scale(lbl_807E2FE0);
            TVec3f sp20(front);
            sp20.scale(lbl_807D7864);
            TVec3f sp2C(pos);
            sp2C.add(sp20);
            TVec3f sp38(sp2C);
            sp38.add(sp14);
            MR::setPosition(pActor, sp38);
            TVec3f axis(0, 1, 0);
            MR::turnVecToVecDegree(&dir, front, up, lbl_807D7860, axis);
            MR::rotateVecDegree(&dir, front, baseDeg + step * (f32)i);
            dir.scale(MR::getRandom(lbl_807D7854, lbl_807D7858));
        }
        pActor->mVelocity = dir;
        pActor->appear();
    }
}

extern "C" void fn_80094930(BellyDragonPod* pPod) {
    if (fn_80093310(pPod->mHost)) {
        pPod->mGroupB->makeAllDead();
        pPod->mGroupA->makeAllDead();
        for (s32 i = 0; i < pPod->mGroupB->getObjNum(); i++) {
            MR::tryForceDeleteEffectAll(pPod->mGroupB->getActor(i));
        }
        for (s32 i = 0; i < pPod->mGroupA->getObjNum(); i++) {
            MR::tryForceDeleteEffectAll(pPod->mGroupA->getActor(i));
        }
    }
}

extern "C" void fn_800949D0(BellyDragonPod* pPod) {
    fn_80095C30(pPod->mFlameCtrl);
}

extern "C" void fn_800949E0(BellyDragonPod* pPod) {
    fn_80095C80(pPod->mFlameCtrl);
}

extern "C" void fn_800949F0(BellyDragon* pHost, TPos3f* pOut, const char* pJoint, s32 flag) {
    TPos3f sp78;
    sp78.set(MR::getJointMtx(pHost, pJoint));
    TVec3f sp68(lbl_807D7844, lbl_807D7844, lbl_807D7840);
    TVec3f sp5C(lbl_807D7844, lbl_807D7840, lbl_807D7844);
    TVec3f sp50(lbl_807D7840, lbl_807D7844, lbl_807D7844);
    TVec3f sp44(lbl_807D7844);
    if (flag != 0) {
        sp78.getXDir(sp50);
        sp78.getYDir(sp5C);
        sp78.getZDir(sp68);
    } else {
        sp78.getXDir(sp68);
        sp78.getYDir(sp5C);
        sp78.getZDir(sp50);
        sp68.scale(lbl_807D7848);
    }
    sp44.set(sp78.mMtx[0][3], sp78.mMtx[1][3], sp78.mMtx[2][3]);
    TVec3f sp38(lbl_807D7844, lbl_807D7848, lbl_807D7844);
    MR::calcGravityVector(pHost, sp44, &sp38, NULL, 0);
    TVec3f sp2C(lbl_807D7844);
    if (fn_80038870(&sp44, &sp2C, &sp38, 0, 0, 0, 0, lbl_807D7868) != 0 && MR::isSameDirection(sp68, sp38, lbl_807D7874) == 0) {
        TVec3f sp20 = -sp38;
        if (MR::isSameDirection(sp68, sp20, lbl_807D7874) == 0) {
            TVec3f sp14(sp38);
            f32 k = lbl_807D786C;
            sp14.scale(-k);
            sp2C.add(sp14);
            TVec3f sp8 = -sp38;
            MR::makeMtxUpFrontPos(pOut, sp8, sp68, sp2C);
        }
    }
}

extern "C" void fn_80094BE0(BellyDragonPod* pPod) {
    TPos3f sp28;
    char name[0x20];
    fn_800949F0(pPod->mHost, &pPod->mBaseMtx, lbl_8066D138, 0);
    sp28.identity();
    for (s32 i = 0; i < 0x20; i++) {
        snprintf(name, 0x20, lbl_8066D0F4, i);
        fn_800949F0(pPod->mHost, &sp28, name, 1);
        fn_80095CD0(pPod->mFlameCtrl, name, &sp28);
    }
}

extern "C" void fn_80094C80(BellyDragonPod* pPod) {
    fn_80095D60(pPod->mFlameCtrl);
}

extern "C" void fn_80094C90(BellyDragonPod* pPod) {
    pPod->mGroupA = new LiveActorGroup(lbl_8066D140, 0x20);
    for (s32 i = 0; i < 0x20; i++) {
        void* pActorMem = __nw__FUl(0xC4);
        NameObj* pActor = (NameObj*)pActorMem;
        if (pActorMem != NULL) {
            pActor = fn_80092E70(pActorMem);
        }
        pActor->initWithoutIter();
        pPod->mGroupA->registerActor((LiveActor*)pActor);
    }
    pPod->mGroupB = new LiveActorGroup(lbl_8066D14C, 8);
    for (s32 i = 0; i < 8; i++) {
        NameObj* pActor = new BellyDragonPodActor(pPod->mHost);
        pActor->initWithoutIter();
        pPod->mGroupB->registerActor((LiveActor*)pActor);
    }
}

BellyDragonArm::BellyDragonArm(BellyDragon* pHost, s32 side) : ActorStateBaseInterface(lbl_8066D158), mHost(pHost), mSide(side), mStateA(NULL), mStateB(NULL), mStateC(NULL), mCounter(0) {
}

void BellyDragonArm::init() {
    initNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance, 0x10);
    void* pMem = __nw__FUl(0x28);
    if (pMem != NULL) {
        pMem = fn_80097520(pMem, mHost, mSide);
    }
    mStateA = (ActorStateBaseInterface*)pMem;
    pMem = __nw__FUl(0x48);
    if (pMem != NULL) {
        pMem = fn_80096060(pMem, mHost, mSide);
    }
    mStateB = (ActorStateBaseInterface*)pMem;
    pMem = __nw__FUl(0x58);
    if (pMem != NULL) {
        pMem = fn_80096960(pMem, mHost, mSide);
    }
    mStateC = (ActorStateBaseInterface*)pMem;
    MR::initActorState(this, mStateA, &NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance, lbl_8066D158 + 0xC);
    MR::initActorState(this, mStateB, &NrvBellyDragonArm::BellyDragonArmNrvFire::sInstance, lbl_8066D158 + 0x14);
    MR::initActorState(this, mStateC, &NrvBellyDragonArm::BellyDragonArmNrvShot::sInstance, lbl_8066D158 + 0x20);
    kill();
}

void BellyDragonArm::appear() {
    mIsDead = false;
    mCounter = 0;
    setNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance);
}

void BellyDragonArm::exeWait() {
    if (MR::isFirstStep(this) && mCounter >= 3) {
        fn_80097690(mStateA);
        mCounter = 0;
    }
    fn_800937C0(mHost);
    if (fn_80093320(this, mHost) != 0) {
        setNerve(&NrvBellyDragonArm::BellyDragonArmNrvFire::sInstance);
    }
}

void BellyDragonArm::exeFire() {
    if (fn_80093320(this, mHost) != 0) {
        if (*(u8*)((u8*)mStateB + 0x24) != 0) {
            mCounter = 0;
            setNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance);
            return;
        }
        setNerve(&NrvBellyDragonArm::BellyDragonArmNrvShot::sInstance);
    }
}

void BellyDragonArm::exeShot() {
    if (MR::isFirstStep(this) && mSide != 0) {
        mCounter += 1;
    }
    if (fn_80093320(this, mHost) != 0) {
        setNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance);
    }
}

void BellyDragonArm::exeAttack() {
    if (MR::isFirstStep(this)) {
        if (fn_80093A30(mHost, 0) != 0) {
            MR::stopStageBGM(5);
        }
    }
    fn_800934A0(mHost, lbl_807D787C);
    if (MR::isGreaterStep(this, 0x14) && fn_80093980(mHost, 0) != 0) {
        kill();
        return;
    }
    if (MR::isGreaterStep(this, 0x14) && fn_80093A30(mHost, 0) != 0) {
        kill();
    }
}

extern "C" void fn_800951D0(BellyDragonArm* pArm, HitSensor* pSensorA, HitSensor* pSensorB) {
    if (pArm->isNerve(&NrvBellyDragonArm::BellyDragonArmNrvFire::sInstance)) {
        fn_80096220(pArm->mStateB, pSensorA, pSensorB);
    }
    if (pArm->isNerve(&NrvBellyDragonArm::BellyDragonArmNrvShot::sInstance)) {
        fn_80096B60(pArm->mStateC, pSensorA, pSensorB);
    }
    MR::sendMsgPush(pSensorB, pSensorA);
}

extern "C" s32 fn_80095260(BellyDragonArm* pArm, u32 msg, HitSensor* pSensorA, HitSensor* pSensorB) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return 1;
    }
    if (!pArm->isNerve(&NrvBellyDragonArm::BellyDragonArmNrvAttack::sInstance) && MR::isMsgPlayerSpinAttackOrSupportTico(msg) && fn_80093970(pSensorA)) {
        MR::startAction(pArm->mHost, lbl_8066D184);
        TVec3f sp2C(MR::getSensorHost(pSensorA)->mPosition);
        TVec3f sp20(lbl_807D7878, lbl_807D787C, lbl_807D7878);
        MR::calcGravityVector(pArm->mHost, sp2C, &sp20, NULL, 0);
        TVec3f sp8(sp20);
        sp8.scale(lbl_807D7880);
        TVec3f sp14(sp2C);
        sp14 -= sp8;
        MR::emitItemBossDamageAtPos(pArm->mHost, &sp14);
        if (fn_80093980(pArm->mHost, 1) != 0 || fn_80093A30(pArm->mHost, 1) != 0) {
            pArm->setNerve(&NrvBellyDragonArm::BellyDragonArmNrvAttack::sInstance);
            return 1;
        }
        if (pArm->isNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance)) {
            fn_800976A0(pArm->mStateA);
        }
        return 1;
    }
    return 0;
}

BellyDragonArm::~BellyDragonArm() {
}
