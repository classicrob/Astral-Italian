#define JGEOMETRY_VEC3_WMR_OUTLINE
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

extern "C" int snprintf(char*, unsigned long, const char*, ...);
extern "C" void* __nw__FUl(u32);

// Pod helpers (TU05). Pod group members are BellyDragonFireGround (0xC4, TPos3f at 0x90) and group B actors.

// Host helpers (TU04)

// Arm states (other TUs)


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
    MR::declareCameraRegisterVec(mHost, 0, &mHost->mPosition);
    mCameraTargetMtx = new CameraTargetMtx("ベリードラゴンデモ注視用");
    mFlameCtrl = new BellyDragonFire("ベリードラゴン影", "BellyDragonShadow", mBaseMtx.mMtx, 0x20);
    for (s32 i = 0; i < 0x20; i++) {
        char name[0x20];
        snprintf(name, 0x20, "Body%02d", i);
        mFlameCtrl->addJointMtx(name);
    }
    mDecoration = MR::createModelObjMapObjDecoration("デモ軌跡", "BellyDragonDemoPath", NULL);
    mDecoration->makeActorDead();
    if (BellyDragonUtil::isLv2(mHost)) {
        this->createGroups();
    }
}

void BellyDragonPod::updateFireEffect(BellyDragon* pHost) {
    mEffectCount += 1;
    if (mEffectCount == 0x14) {
        MR::emitEffect(pHost, "BlowL");
        MR::startActionSound(pHost, "BlowL", -1, -1, -1);
    }
    if (mEffectCount == 0x28) {
        MR::emitEffect(pHost, "BlowR");
        MR::startActionSound(pHost, "BlowR", -1, -1, -1);
    }
    if (mEffectCount >= 0x28) {
        mEffectCount = 0;
    }
}

void BellyDragonPod::appearFireGround(s32 isShort, const TVec3f* pVec) {
    BellyDragon* host;
    LiveActor* pActor;
    if (BellyDragonUtil::isLv2(mHost)) {
        pActor = mGroupA->getDeadActor();
        if (pActor != NULL) {
            if (pVec != NULL) {
#ifdef GALAXY_NATIVE
                BellyDragonUtil::calcGroundUpMtx(&static_cast<BellyDragonFireGround*>(pActor)->mBaseMtx, mHost, pVec);
#else
                BellyDragonUtil::calcGroundUpMtx((TPos3f*)((u8*)pActor + 0x90), mHost, pVec);
#endif
            } else {
                f32 scale = lbl_807D784C;
                if (isShort != 0) {
                    scale = lbl_807D7850;
                }
                host = mHost;
#ifdef GALAXY_NATIVE
                BellyDragonUtil::calcGroundHillMtx(&static_cast<BellyDragonFireGround*>(pActor)->mBaseMtx, host, host->getBaseMtx(), scale);
#else
                BellyDragonUtil::calcGroundHillMtx((TPos3f*)((u8*)pActor + 0x90), host, host->getBaseMtx(), scale);
#endif
            }
            pActor->appear();
        }
    }
}

void BellyDragonPod::killFireGrounds() {
    if (BellyDragonUtil::isLv2(mHost)) {
        mGroupA->makeAllDead();
        for (s32 i = 0; i < mGroupA->getObjNum(); i++) {
            MR::tryForceDeleteEffectAll(mGroupA->getActor(i));
        }
    }
}

// NONMATCHING: scalar/paired vector copy lowering and register/stack layout (FP order/grouping checked).
void BellyDragonPod::appearSparks(s32 count) {
    if (BellyDragonUtil::isLv2(mHost) == 0) {
        return;
    }
    f32 step = lbl_807D7870 / (f32)count;
    f32 baseDeg = MR::getRandomDegree();
    for (s32 i = 0; i < count; i++) {
        LiveActor* pActor = mGroupB->getDeadActor();
        if (pActor == NULL) {
            continue;
        }
        const f32 (*pMtxA)[4] = MR::getJointMtx(mHost, "Tongue1");
        const f32 (*pMtxB)[4] = MR::getJointMtx(mHost, "Head");
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

void BellyDragonPod::killAll() {
    if (BellyDragonUtil::isLv2(mHost)) {
        mGroupB->makeAllDead();
        mGroupA->makeAllDead();
        for (s32 i = 0; i < mGroupB->getObjNum(); i++) {
            MR::tryForceDeleteEffectAll(mGroupB->getActor(i));
        }
        for (s32 i = 0; i < mGroupA->getObjNum(); i++) {
            MR::tryForceDeleteEffectAll(mGroupA->getActor(i));
        }
    }
}

void BellyDragonPod::startFlame() {
    mFlameCtrl->appearModel();
}

void BellyDragonPod::stopFlame() {
    mFlameCtrl->killModel();
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
void BellyDragonUtil::calcJointGroundMtx(BellyDragon* pHost, TPos3f* pOut, const char* pJoint, s32 flag) {
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
    f32 z = sp78.mMtx[2][3];
    f32 y = sp78.mMtx[1][3];
    f32 x = sp78.mMtx[0][3];
    sp44.set< f32 >(x, y, z);
    TVec3f sp38(lbl_807D7844, lbl_807D7848, lbl_807D7844);
    MR::calcGravityVector(pHost, sp44, &sp38, NULL, 0);
    TVec3f sp2C(lbl_807D7844);
    if (MR::calcMapHitAlongDirection(sp44, &sp2C, sp38, NULL, NULL, NULL, NULL, lbl_807D7868) != 0 && MR::isSameDirection(sp68, sp38, lbl_807D7874) == 0) {
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

void BellyDragonPod::updateFlameMtx() {
    TPos3f sp28;
    char name[0x20];
    BellyDragonUtil::calcJointGroundMtx(mHost, &mBaseMtx, "Head", 0);
    sp28.identity();
    for (s32 i = 0; i < 0x20; i++) {
        snprintf(name, 0x20, "Body%02d", i);
        BellyDragonUtil::calcJointGroundMtx(mHost, &sp28, name, 1);
        mFlameCtrl->setJointMtx(name, &sp28);
    }
}

void BellyDragonPod::resetFlame() {
    mFlameCtrl->registerCallBacks();
}

void BellyDragonPod::createGroups() {
    mGroupA = new LiveActorGroup("地面炎保持", 0x20);
    for (s32 i = 0; i < 0x20; i++) {
        NameObj* pActor = new BellyDragonFireGround();
        pActor->initWithoutIter();
        mGroupA->registerActor((LiveActor*)pActor);
    }
    mGroupB = new LiveActorGroup("メテオ保持", 8);
    for (s32 i = 0; i < 8; i++) {
        NameObj* pActor = new BellyDragonPodActor(mHost);
        pActor->initWithoutIter();
        mGroupB->registerActor((LiveActor*)pActor);
    }
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout) (FP order/grouping checked: no FP instructions).
BellyDragonArm::BellyDragonArm(BellyDragon* pHost, s32 side) : ActorStateBase<BellyDragon>("Phase[戦闘]", pHost), mSide(side), mStateA(NULL), mStateB(NULL), mStateC(NULL), mCounter(0) {
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout) (FP order/grouping checked: no FP instructions).
void BellyDragonArm::init() {
    initNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance, 0x10);
    mStateA = new BellyDragonArmWait(mHost, mSide);
    mStateB = new BellyDragonArmFire(mHost, mSide);
    mStateC = new BellyDragonArmShot(mHost, mSide);
    MR::initActorState(this, mStateA, &NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance, "Wander");
    MR::initActorState(this, mStateB, &NrvBellyDragonArm::BellyDragonArmNrvFire::sInstance, "AttackBite");
    MR::initActorState(this, mStateC, &NrvBellyDragonArm::BellyDragonArmNrvShot::sInstance, "AttackDive");
    kill();
}

void BellyDragonArm::appear() {
    mIsDead = false;
    mCounter = 0;
    setNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance);
}

void BellyDragonArm::exeWait() {
    if (MR::isFirstStep(this) && mCounter >= 3) {
        mStateA->startAttack();
        mCounter = 0;
    }
    BellyDragonUtil::updatePodFireEffect(mHost);
    if (BellyDragonUtil::updateArmState(this, mHost) != 0) {
        setNerve(&NrvBellyDragonArm::BellyDragonArmNrvFire::sInstance);
    }
}

void BellyDragonArm::exeFire() {
    if (BellyDragonUtil::updateArmState(this, mHost) != 0) {
        if (mStateB->mIsEnd) {
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
    if (BellyDragonUtil::updateArmState(this, mHost) != 0) {
        setNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance);
    }
}

void BellyDragonArm::exeAttack() {
    if (MR::isFirstStep(this)) {
        if (BellyDragonUtil::isNoAttackableWeakPoint(mHost, 0) != 0) {
            MR::stopStageBGM(5);
        }
    }
    BellyDragonUtil::updateFront(mHost, lbl_807D787C);
    if (MR::isGreaterStep(this, 0x14) && BellyDragonUtil::isEnoughWeakPointsBroken(mHost, 0) != 0) {
        kill();
        return;
    }
    if (MR::isGreaterStep(this, 0x14) && BellyDragonUtil::isNoAttackableWeakPoint(mHost, 0) != 0) {
        kill();
    }
}

void BellyDragonArm::attackSensor(HitSensor* pSensorA, HitSensor* pSensorB) {
    if (isNerve(&NrvBellyDragonArm::BellyDragonArmNrvFire::sInstance)) {
        mStateB->attackSensor(pSensorA, pSensorB);
    }
    if (isNerve(&NrvBellyDragonArm::BellyDragonArmNrvShot::sInstance)) {
        mStateC->attackSensor(pSensorA, pSensorB);
    }
    MR::sendMsgPush(pSensorB, pSensorA);
}

// NONMATCHING: register/stack or instruction/branch lowering (FP operand order/grouping checked).
bool BellyDragonArm::receiveMessage(u32 msg, HitSensor* pSensorA, HitSensor* pSensorB) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return 1;
    }
    if (!isNerve(&NrvBellyDragonArm::BellyDragonArmNrvAttack::sInstance) && MR::isMsgPlayerSpinAttackOrSupportTico(msg) && BellyDragonUtil::isSensorWeakPoint(pSensorA)) {
        MR::startAction(mHost, "Damage");
        TVec3f sp2C(MR::getSensorHost(pSensorA)->mPosition);
        TVec3f sp20;
        sp20.set(lbl_807D7878, lbl_807D787C, lbl_807D7878);
        MR::calcGravityVector(mHost, sp2C, &sp20, NULL, 0);
        TVec3f sp8(sp20);
        sp8.x *= lbl_807D7880;
        sp8.y *= lbl_807D7880;
        sp8.z *= lbl_807D7880;
        TVec3f sp14(sp2C);
        sp14 -= sp8;
        MR::emitItemBossDamageAtPos(mHost, &sp14);
        if (BellyDragonUtil::isEnoughWeakPointsBroken(mHost, 1) != 0 || BellyDragonUtil::isNoAttackableWeakPoint(mHost, 1) != 0) {
            setNerve(&NrvBellyDragonArm::BellyDragonArmNrvAttack::sInstance);
            return 1;
        }
        if (isNerve(&NrvBellyDragonArm::BellyDragonArmNrvWait::sInstance)) {
            mStateA->startDamage();
        }
        return 1;
    }
    return 0;
}

#pragma dont_inline on
BellyDragonArm::~BellyDragonArm() {
}
#pragma dont_inline reset
