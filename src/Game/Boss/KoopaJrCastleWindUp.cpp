#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

namespace NrvKoopaJrCastleWindUp { KoopaJrCastleWindUpNrvA KoopaJrCastleWindUpNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWindUp { KoopaJrCastleWindUpNrvB KoopaJrCastleWindUpNrvB::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleWindUp::KoopaJrCastleWindUp(const char* pName) : KoopaJrCastleWindBase(pName), mModel(nullptr), mLength(0.0f) {
    PSMTXIdentity(mMtx);
}

// NONMATCHING: the effect loop is emitted with a loop-entry guard (cmpwi/ble) instead of the target's jump-to-condition form; the rest is identical.
void KoopaJrCastleWindUp::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "Fan", false);
    MR::validateClipping(this);
    initNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvA::sInstance, 0);
    MR::initDefaultPos(this, rIter);
    TVec3f front(0.0f);
    MR::calcFrontVec(&front, this);
    mLength = 1200.0f;
    MR::getJMapInfoArg0NoInit(rIter, &mLength);
    PartsModel* pModel = new PartsModel(this, "\x95\x97\x83\x82\x83\x66\x83\x8b", "FanWind", NULL, 9, false);
    mModel = pModel;
    pModel->initFixedPosition(getBaseMtx(), TVec3f(0.0f, mLength - 2200.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    pModel->initWithoutIter();
    MR::registerDemoSimpleCastAll(pModel);
    mEffectNum = 2;
    f32 step = mLength / (mEffectNum - 1);
    mEffect = new FluffWindEffect*[mEffectNum];
    TVec3f pos(mPosition);
    for (s32 i = 0; i < mEffectNum; i++) {
        FluffWindEffect* pEffect = new FluffWindEffect("\x82\xed\x82\xbd\x82\xb0\x83\x47\x83\x74\x83\x46\x83\x4e\x83\x67");
        pEffect->initEffectInfo(pos, -mGravity, front, "KoopaJrCastleWindUp", -1.0f);
        pEffect->initWithoutIter();
        mEffect[i] = pEffect;
        pos += -mGravity * step;
    }
    makeActorAppeared();
    KoopaJrCastleWindFunction::createWindHolder();
    KoopaJrCastleWindFunction::registerWind(this);
}

void KoopaJrCastleWindUp::makeActorAppeared() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffect[i]->makeActorAppeared();
    }
    LiveActor::makeActorAppeared();
}

void KoopaJrCastleWindUp::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvA::sInstance);
}

void KoopaJrCastleWindUp::makeActorDead() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffect[i]->makeActorDead();
    }
    LiveActor::makeActorDead();
}

void KoopaJrCastleWindUp::exeBlow() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mEffectNum; i++) {
            mEffect[i]->makeActorAppeared();
        }
        mModel->makeActorAppeared();
    }
    MR::startActionSound(this, "Move", -1, -1, -1);
}

void KoopaJrCastleWindUp::exeStop() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < mEffectNum; i++) {
            mEffect[i]->makeActorDead();
        }
        mModel->makeActorDead();
    }
    if (MR::isGreaterStep(this, 10)) {
        setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvA::sInstance);
    }
}

bool KoopaJrCastleWindUp::calcWindInfo(const TVec3f* pPos, TVec3f* pDir, f32* pPower) {
    if (isNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvB::sInstance)) {
        return false;
    }
    TVec3f toTarget(0.0f);
    MR::calcVecToTargetHorizon(&toTarget, this, pPos, NULL);
    f32 dist = PSVECMag(&toTarget);
    if (0.0f < dist && dist < 500.0f && MR::calcDistanceVertical(this, *pPos) < mLength) {
        TVec3f dir = -mGravity * 0.5f;
        *pDir = dir;
        *pPower = dist;
        return true;
    }
    return false;
}

bool KoopaJrCastleWindUp::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xF4) {
        setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvB::sInstance);
        return true;
    }
    return false;
}

KoopaJrCastleWindUp::~KoopaJrCastleWindUp() {}
