#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include <algorithm>
#include "Game/MapObj/StarPieceGroup.hpp"
#include "Game/MapObj/StarPiece.hpp"
#include "Game/Enemy/HomingKiller.hpp"
#include "Game/Util.hpp"
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/Nerve.hpp"
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#else
#include <functional.hpp>
#endif

template < typename It, typename Fn >
static inline void forEachCallInline(It it, It end, Fn fn) {
    It last = end;
    for (It first = it; first != last; ++first) {
        fn(*first);
    }
}

template < typename It, typename Fn >
static Fn forEachFn(It it, It end, Fn fn) {
    for (; it != end; ++it) {
        fn(*it);
    }
    return fn;
}

HomingKiller* KoopaJrRobot::allocKiller(LiveActor* pOwner) {
    return mSeq->mPool->allocate(pOwner);
}

bool KoopaJrRobot::releaseKiller(LiveActor* pOwner, HomingKiller* pKiller) {
    return mSeq->mPool->release(pOwner, pKiller);
}

bool KoopaJrRobot::isKillerShotEnd(HitSensor* pSensor) {
    return mSeq->mPool->isShotEnd(pSensor);
}

void KoopaJrRobot::attackShootableKillers() {
    mSeq->attackShootableKillers();
}

bool KoopaJrRobot::isArmSensor(HitSensor* pSensor) {
    const char* names[6] = {"ElbowL", "UpperArmL", "ForeArmL", "ElbowR", "UpperArmR", "ForeArmR"};
    for (u32 i = 0; i < 6; i++) {
        if (MR::isEqualSensor(pSensor, this, names[i])) {
            return true;
        }
    }
    return false;
}

bool KoopaJrRobot::isWeakPointSensor(HitSensor* pSensor) {
    for (s32 i = 0; i < 4; i++) {
        if (MR::isEqualSensor(pSensor, this, KoopaJrRobotPart::getSensorName(i))) {
            return true;
        }
    }
    return false;
}

bool KoopaJrRobotFunction::isPlayerInRushAir() {
    bool ret = false;
    if (MR::isPlayerInRush() && !MR::isOnGroundPlayer()) {
        ret = true;
    }
    return ret;
}

bool KoopaJrRobot::isPlayerNearBack(f32 angle) {
    bool inAir = false;
    if (MR::isPlayerInRush() && !MR::isOnGroundPlayer()) {
        inAir = true;
    }
    if (inAir) {
        return false;
    }
    TVec3f player(*MR::getPlayerCenterPos());
    TVec3f dir(player);
    dir.sub(mPosition);
    MR::vecKillElement(dir, mGravity, &dir);
    MR::normalizeOrZero(&dir);
    TVec3f back;
    back.negate(mZAxis);
    return MR::diffAngleAbs(back, dir) * 57.29578f < angle;
}

bool KoopaJrRobot::isAllBodyPartsBroken() {
    bool ret = false;
    if (mParts[0]->isBroken() && mParts[1]->isBroken() && mParts[2]->isBroken()) {
        ret = true;
    }
    return ret;
}

LiveActor* KoopaJrRobot::getKillerOwner(HomingKiller* pKiller) {
    return mSeq->mPool->getOwner(pKiller);
}

void KoopaJrRobotFunction::attachBodyToAllRoot(KoopaJrRobot* pHost) {
    KoopaJrRobotPod* pBody = pHost->mBody;
    pBody->setMtx(MR::getJointMtx(pHost, "AllRoot"));
}

void KoopaJrRobotFunction::attachBodyToHead(KoopaJrRobot* pHost) {
    KoopaJrRobotPod* pBody = pHost->mBody;
    pBody->setMtx(MR::getJointMtx(pHost, "Head"));
}

void KoopaJrRobotFunction::showStarPieces(KoopaJrRobot* pHost) {
    StarPieceGroup* pGroup = pHost->mStarPieceGroup;
    for (s32 i = 0; i < pGroup->mNumPieces; i++) {
        LiveActor* pPiece = pGroup->mPieces[i];
        MR::showModelAndEffects(pPiece);
        NameObjFunction::requestMovementOn(pPiece);
    }
}

void KoopaJrRobotFunction::hideStarPieces(KoopaJrRobot* pHost) {
    StarPieceGroup* pGroup = pHost->mStarPieceGroup;
    for (s32 i = 0; i < pGroup->mNumPieces; i++) {
        LiveActor* pPiece = pGroup->mPieces[i];
        MR::hideModelAndEffects(pPiece);
        NameObjFunction::requestMovementOff(pPiece);
    }
}

LiveActor* KoopaJrRobotFunction::createHand(KoopaJrRobot* pHost, const char* pName) {
    CollisionObj* pObj = new CollisionObj(pHost, "クッパJrロボットコリジョンオブジェ", pName,
                                         pHost->getSensor(pName), MR::getJointMtx(pHost, pName), -1);
    pObj->initWithoutIter();
    return pObj;
}

KoopaJrRobotPart* KoopaJrRobotFunction::createPart(KoopaJrRobot* pHost, s32 id) {
    return KoopaJrRobotPart::create(pHost, id);
}

struct KoopaJrRobotBeamInfo {
    const char* mName;
    const char* mJoint;
    Vec mOffset;
    f32 mSpeed;
    s32 mLife;
    f32 mA;
    f32 mB;
};

static const KoopaJrRobotBeamInfo sBeamInfo[3] = {
    {"\x83r\x81[\x83\x80L", "HandL", {750.0f, 0.0f, -700.0f}, 20.0f, 130, 600.0f, 3500.0f},
    {"\x83r\x81[\x83\x80R", "HandR", {750.0f, 0.0f, 700.0f}, 20.0f, 130, 600.0f, 3500.0f},
    {"\x83r\x81[\x83\x80""BODY", "AllRoot", {0.0f, 400.0f, 0.0f}, 20.0f, 180, 2200.0f, 5500.0f},
};

// NONMATCHING: two loads (lwz life / lfs speed) scheduled in swapped order
LiveActor* KoopaJrRobotFunction::createCollision(KoopaJrRobot* pHost, const JMapInfoIter& rIter, s32 index) {
    const KoopaJrRobotBeamInfo* pInfo = &sBeamInfo[index];
    KoopaJrRobotBeam* pBeam = new KoopaJrRobotBeam(pInfo->mName, pHost, MR::getJointMtx(pHost, pInfo->mJoint), TVec3f(pInfo->mOffset), pInfo->mLife, pInfo->mSpeed,
                                                    pInfo->mA, pInfo->mB);
    pBeam->init(rIter);
    return pBeam;
}

KoopaJrRobotBinderActor* KoopaJrRobotFunction::createFollower(LiveActor* pHost, s32 index) {
    const char* names[5] = {"ForeArmL", "ForeArmR", "ElbowL", "ElbowR", "HeadCover"};
    KoopaJrRobotBinderActor* pActor = new KoopaJrRobotBinderActor(pHost->getSensor(names[index]));
    pActor->initWithoutIter();
    return pActor;
}

KoopaJrRobotPod* KoopaJrRobotFunction::createBody(KoopaJrRobot* pHost, MtxPtr pMtx) {
    KoopaJrRobotPod* pPod = new KoopaJrRobotPod(pHost, pMtx);
    pPod->initWithoutIter();
    return pPod;
}

PartsModel* KoopaJrRobotFunction::createHeadModel(KoopaJrRobotPod* pPod) {
    PartsModel* pModel = MR::createPartsModelNpc(pPod, "\x83N\x83""b\x83pJr", "KoopaJr", MR::getJointMtx(pPod, "KoopaJrPos"));
    MR::registerDemoSimpleCastAll(pModel);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(pModel, "Shadow");
    pModel->kill();
    return pModel;
}

void KoopaJrRobotFunction::hideAll(KoopaJrRobot* pHost) {
    MR::hideModelAndOnCalcAnim(pHost);
    MR::hideModelAndOnCalcAnim(pHost->mParts[0]->mModel);
    MR::hideModelAndOnCalcAnim(pHost->mParts[1]->mModel);
    MR::hideModelAndOnCalcAnim(pHost->mParts[2]->mModel);
    LiveActor** pChildren = pHost->mChildren.mArray.mArr;
    forEachFn(pChildren, pChildren + pHost->mChildren.mCount, MR::hideModelAndOnCalcAnim);
}

void KoopaJrRobotFunction::showAll(KoopaJrRobot* pHost) {
    MR::showModel(pHost);
    MR::showModel(pHost->mParts[0]->mModel);
    MR::showModel(pHost->mParts[1]->mModel);
    MR::showModel(pHost->mParts[2]->mModel);
    LiveActor** pChildren = pHost->mChildren.mArray.mArr;
    forEachFn(pChildren, pChildren + pHost->mChildren.mCount, MR::showModel);
}

void KoopaJrRobotFunction::offDrawEffect(KoopaJrRobot* pHost) {
    MR::offDrawEffect(pHost->mParts[0]->mModel);
    MR::offDrawEffect(pHost->mParts[1]->mModel);
    MR::offDrawEffect(pHost->mParts[2]->mModel);
}

void KoopaJrRobotFunction::onDrawEffect(KoopaJrRobot* pHost) {
    MR::onDrawEffect(pHost->mParts[0]->mModel);
    MR::onDrawEffect(pHost->mParts[1]->mModel);
    MR::onDrawEffect(pHost->mParts[2]->mModel);
}
