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

// Pod helpers (TU05)
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them

// Body (TU02) helpers
// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them

// Misc helpers (other units)

// PLACEHOLDER(core): unnamed SMG2 callees (no symbol name yet); resolve when their owners name them

// BODY_PART indexes the weak points of BellyDragonBody (BellyDragon.hpp).
#define BODY_COUNT(b) ((b)->mWeakPointNum)
#define BODY_PART(b, i) ((b)->mWeakPoints[i])

namespace NrvBellyDragonPodActor {
    NEW_NERVE(BellyDragonPodActorNrvWait, BellyDragonPodActor, Wait);
};  // namespace NrvBellyDragonPodActor

// Group-B actor (0xC4 bytes). Host at 0x90, base matrix at 0x94.
BellyDragonPodActor::BellyDragonPodActor(BellyDragon* pHost) : LiveActor("隕石（ドラゴン）"), mHost(pHost) {
    mBaseMtx.identity();
}

BellyDragonPodActor::~BellyDragonPodActor() {
}

// ---------------------------------------------------------------------------------------------

s32 BellyDragonUtil::isLv2(BellyDragon* pHost) {
    return pHost->mIsLv2;
}

s32 BellyDragonUtil::updateArmState(NerveExecutor* pArm, BellyDragon* pHost) {
    s32 result = MR::updateActorState(pArm);
    pHost->updateFrontVec(lbl_807D782C);
    return result;
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
void BellyDragonUtil::syncToHead(BellyDragon* pHost, LiveActor*) {
    MR::onBind(pHost);
    BellyDragonBodyPart* pBody = pHost->mBody->mHead;
    MR::setPosition(pHost, pBody->mPosition);
    MR::calcGravity(pHost);
    TPos3f sp20;
    sp20.set(pBody->getBaseMtx());
    TVec3f sp14;
    sp14.set< f32 >(sp20.mMtx[0][2], sp20.mMtx[1][2], sp20.mMtx[2][2]);
    TVec3f sp8;
    sp8.set< f32 >(sp20.mMtx[0][1], sp20.mMtx[1][1], sp20.mMtx[2][1]);
    MR::makeMtxFrontUpPos(&sp20, sp14, sp8, pBody->mPosition);
    pHost->mBaseMtx.set(sp20.mMtx);
    pHost->mFront.set(sp14);
    BellyDragonUtil::setBodyPartMtx(pBody, sp20.mMtx);
    pHost->mBody->unfreeze();
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
void BellyDragonUtil::setBodyPartMtx(BellyDragonBodyPart* pObj, const f32 (*mtx)[4]) {
    ((TPos3f*)((u8*)pObj + 0xA8))->set(mtx);
    ((TVec3f*)((u8*)pObj + 0x14))->set< f32 >(*(f32*)((u8*)pObj + 0xB4), *(f32*)((u8*)pObj + 0xC4), *(f32*)((u8*)pObj + 0xD4));
}

void BellyDragonUtil::updateFront(BellyDragon* pHost, f32 rate) {
    pHost->updateFrontVec(rate);
}

void BellyDragonUtil::pullBindedByFront(BellyDragon* pHost, const f32* pArg) {
    MR::fn_80013880(pHost, &pHost->mFront, pArg);
    MR::sendMsgEnemyAttackToBindedSensor(pHost, pHost->getSensor("Message"));
}

void BellyDragonUtil::pushBindedByFront(BellyDragon* pHost, f32 farg0, f32 farg1) {
    TVec3f sp8 = pHost->mFront * farg0;
    MR::addVelocity(pHost, sp8);
    MR::attenuateVelocity(pHost, farg1);
    MR::sendMsgEnemyAttackToBindedSensor(pHost, pHost->getSensor("Message"));
}

void BellyDragonUtil::startBodyBindB(BellyDragon* pHost) {
    pHost->mBody->startBindB();
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
void BellyDragonUtil::calcGroundHillMtx(TPos3f* pOut, NameObj* pObj, const f32 (*mtx)[4], f32 farg0) {
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
    MR::calcMapHitAlongDirection(sp44, &sp38, sp2C, NULL, NULL, NULL, NULL, lbl_807D7834);
    MR::makeMtxUpSidePos(pOut, sp20, sp5C, sp38);
}

void BellyDragonUtil::calcGroundUpMtx(TPos3f* pOut, NameObj* pObj, const TVec3f* pPos) {
    TVec3f sp2C(*pPos);
    TVec3f sp20;
    MR::calcGravityVector(pObj, sp2C, &sp20, NULL, 0);
    TVec3f sp14 = -sp20;
    TVec3f sp8 = sp14 * lbl_807D7830;
    sp2C.add(sp8);
    MR::calcMapHitAlongDirection(sp2C, &sp2C, sp20, NULL, NULL, NULL, NULL, lbl_807D7834);
    MR::makeMtxUpNoSupportPos(pOut, sp14, sp2C);
}

void BellyDragonUtil::appearJawHill(BellyDragon* pHost) {
    pHost->mBody->appearJawHill();
}

void BellyDragonUtil::startHeadAttackHill(BellyDragon* pHost) {
    pHost->mBody->startHeadAttackHill();
}

void BellyDragonUtil::appearHeadHill(BellyDragon* pHost) {
    pHost->mBody->appearHeadHill();
}

void BellyDragonUtil::breakHeadHill(BellyDragon* pHost) {
    pHost->mBody->breakHeadHill();
}

void BellyDragonUtil::updatePodFireEffect(BellyDragon* pHost) {
    if (isAttackNerve(pHost)) {
        pHost->mPod->updateFireEffect(pHost);
    }
}

void BellyDragonUtil::killPodObjects(BellyDragon* pHost) {
    pHost->mPod->killAll();
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
void BellyDragonUtil::killWeakPoints(BellyDragon* pHost) {
    s32 count = BODY_COUNT(pHost->mBody);
    for (s32 i = 0; i < count; i++) {
        LiveActor* pPart = BODY_PART(pHost->mBody, i);
        if (!MR::isDead(pPart)) {
            pPart->kill();
        }
    }
}

void BellyDragonUtil::appearFireGround(BellyDragon* pHost) {
    pHost->mPod->appearFireGround(0, NULL);
}

void BellyDragonUtil::appearFireGroundShort(BellyDragon* pHost) {
    pHost->mPod->appearFireGround(1, NULL);
}

NO_INLINE void BellyDragonUtil::appearFireGroundAt(BellyDragon* pHost, const TVec3f* pVec) {
    pHost->mPod->appearFireGround(0, pVec);
}

void BellyDragonUtil::killFireGrounds(BellyDragon* pHost) {
    pHost->mPod->killFireGrounds();
}

void BellyDragonUtil::appearPodSparks(BellyDragon* pHost, s32 count) {
    pHost->mPod->appearSparks(count);
}

s32 BellyDragonUtil::getAttackableWeakPointNum(BellyDragon* pHost) {
    s32 count = BODY_COUNT(pHost->mBody);
    s32 alive = 0;
    for (s32 i = 0; i < count; i++) {
        if (BODY_PART(pHost->mBody, i)->isAttackable()) {
            alive += 1;
        }
    }
    return alive;
}

bool BellyDragonUtil::isSensorWeakPoint(HitSensor* pSensor) {
    return MR::isSensorType(pSensor, 0x39);
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
s32 BellyDragonUtil::isEnoughWeakPointsBroken(BellyDragon* pHost, s32 flag) {
    if (isAttackNerve(pHost)) {
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

s32 BellyDragonUtil::isNoAttackableWeakPoint(BellyDragon* pHost, s32 flag) {
    s32 n = BellyDragonUtil::getAttackableWeakPointNum(pHost);
    if (flag != 0) {
        n -= 1;
    }
    if (n <= 0) {
        return 1;
    }
    return 0;
}

void BellyDragonUtil::setWeakPointsWaitC(BellyDragon* pHost) {
    s32 count = BODY_COUNT(pHost->mBody);
    for (s32 i = 0; i < count; i++) {
        BODY_PART(pHost->mBody, i)->setNerveWaitC();
    }
}

void BellyDragonUtil::startPodFlame(BellyDragon* pHost) {
    pHost->mPod->startFlame();
}

void BellyDragonUtil::stopPodFlame(BellyDragon* pHost) {
    pHost->mPod->stopFlame();
}

void BellyDragonUtil::updatePodFlameMtx(BellyDragon* pHost) {
    pHost->mPod->updateFlameMtx();
}

void BellyDragonUtil::resetPodFlame(BellyDragon* pHost) {
    pHost->mPod->resetFlame();
}

HitSensor* BellyDragonUtil::getBodySensor(LiveActor* pActor) {
    return pActor->getSensor("Message");
}

void BellyDragonUtil::initPodAnimCamera(BellyDragon* pHost, const char* pName) {
    BellyDragonPod* pPod = pHost->mPod;
    MR::initAnimCamera(pHost, pPod->mCameraInfo, pName);
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
void BellyDragonUtil::setupDemoPose(BellyDragon* pHost, const char* pName) {
    BellyDragonPod* pPod = pHost->mPod;
    pHost->mBaseMtx.set(pPod->mHostMtx.mMtx);
    TVec3f sp8(pPod->mHostMtx.mMtx[0][3], pPod->mHostMtx.mMtx[1][3], pPod->mHostMtx.mMtx[2][3]);
    MR::setPosition(pHost, sp8);
    MR::zeroVelocity(pHost);
    pHost->mBody->freeze();
    if (MR::fn_80008F00(pHost, pName)) {
        MR::startAction(pHost, pName);
    }
    MR::calcAnimDirect(pHost);
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
s32 BellyDragonUtil::tryStartDemo(BellyDragon* pHost, const char* pName, bool) {
    if (!MR::tryStartDemoMarioPuppetable(pHost, pName)) {
        return 0;
    }
    MR::setPlayerPos("プレイヤーデモ位置（ベリードラゴン戦）");
    MR::startBckPlayer("BattleWait", NULL);
    BellyDragonUtil::setupDemoPose(pHost, pName);
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
    BellyDragonBodyPart* pBody = pHost->mBody->mHead;
    BellyDragonUtil::setBodyPartMtx(pBody, MR::getJointMtx(pPod->mDecoration, (s32)0));
    pHost->mBaseMtx.set(MR::getJointMtx(pPod->mDecoration, (s32)0));
    MR::setPosition(pHost, pBody->mPosition);
    ((TPos3f*)((u8*)pPod->mCameraTargetMtx + 0x14))->set(pPod->mHostMtx.mMtx);
    CameraTargetArg targetArg;
    MR::startAnimCameraTargetOther(pHost, pPod->mCameraInfo, pName, targetArg, 0, false, lbl_807D7828);
    return 1;
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
s32 BellyDragonUtil::updateDemo(BellyDragon* pHost, const char* pName, bool useFirst, const f32* pTime, f32 farg0) {
    pHost->mPod->mDecoration->movement();
    pHost->mPod->mDecoration->calcAnim();
    const f32 (*mtx)[4] = MR::getJointMtx(pHost->mPod->mDecoration, (s32)0);
    TVec3f sp44(mtx[0][2], mtx[1][2], mtx[2][2]);
    TVec3f sp38(mtx[0][3], mtx[1][3], mtx[2][3]);
    pHost->mFront.set(sp44);
    MR::setPosition(pHost, sp38);
    pHost->updateFrontVec(farg0);
    s32 found;
    if (useFirst) {
        found = MR::tryEndDemoAndAnimCamera(pHost, pHost->mPod->mCameraInfo, pName, 0);
    } else {
        found = MR::endDemoAndAnimCameraIfActionEnd(pHost, pHost->mPod->mCameraInfo, pName, 0);
    }
    if (found != 0) {
        if (MR::isDemoEndWaitCameraInterpolating()) {
            pHost->mBody->forceDeleteEffects();
        }
        char name[0x100];
        snprintf(name, 0x100, "%sSkip", pName);
        BellyDragonUtil::setupDemoPose(pHost, name);
        const f32 (*mtxB)[4] = MR::getJointMtx(pHost, "Head");
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
        BellyDragonUtil::setBodyPartMtx(pHost->mBody->mHead, mm.mMtx);
        BellyDragonUtil::syncToHead(pHost, NULL);
        MR::startAction(pHost, "FlyWaitNoInterpole");
        return 1;
    }
    return 0;
}

void BellyDragonPodActor::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "BellyDragonMeteor", false);
    initNerve(&NrvBellyDragonPodActor::BellyDragonPodActorNrvWait::sInstance, 0);
    makeActorDead();
}

void BellyDragonPodActor::appear() {
    LiveActor::appear();
    setNerve(&NrvBellyDragonPodActor::BellyDragonPodActorNrvWait::sInstance);
}

// NONMATCHING: regalloc (inlining, scheduling, stack slots or equivalent branch layout).
void BellyDragonPodActor::exeWait() {
    MR::isFirstStep(this);
    MR::addVelocityToGravity(this, lbl_807D7838);
    MR::attenuateVelocity(this, lbl_807D783C);
    if (MR::isBinded(this)) {
        BellyDragonUtil::appearFireGroundAt(mHost, &mPosition);
        MR::tryHitReaction(this, "地面衝突");
        MR::startActionSound(this, "Gnd", -1, -1, -1);
        kill();
        return;
    }
    if (MR::isGreaterStep(this, 0x5A)) {
        MR::startActionSound(this, "Fall", -1, -1, -1);
    }
    if (MR::isGreaterEqualStep(this, 0x384)) {
        MR::tryHitReactionBreak(this);
        kill();
    }
}

void BellyDragonPodActor::calcAndSetBaseMtx() {
    mBaseMtx.mMtx[0][3] = mPosition.x;
    mBaseMtx.mMtx[1][3] = mPosition.y;
    mBaseMtx.mMtx[2][3] = mPosition.z;
    MR::setBaseTRMtx(this, mBaseMtx);
}

void BellyDragonPodActor::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    MR::sendMsgEnemyAttackFire(pReceiver, pSender);
}
