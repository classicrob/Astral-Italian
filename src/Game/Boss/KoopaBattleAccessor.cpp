#pragma auto_inline off
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

KoopaBattleAccessor::KoopaBattleAccessor()
    : NameObj("クッパ戦アクセス"), mKoopaBattle(nullptr), mDemoChamber(nullptr), mBattlePlanet(nullptr), mFinalStarPlanet(nullptr),
      mReceiverSensors(nullptr), mReceiverSensorNum(0), mReceiverSensorMax(64), mAttackRocks(nullptr), mAttackRockNum(0),
      mAttackRockMax(8), mQuatA(0.0f, 0.0f, 0.0f, 1.0f), mPosA(0.0f, 0.0f, 0.0f), mQuatB(0.0f, 0.0f, 0.0f, 1.0f),
      mPosB(0.0f, 0.0f, 0.0f) {
    mReceiverSensors = new HitSensor*[mReceiverSensorMax];
    mAttackRocks = new AttackRockFinal*[mAttackRockMax];
}

void KoopaBattleAccessor::registerKoopaBattle(KoopaBattle* p) {
    mKoopaBattle = p;
}

void KoopaBattleAccessor::registerKoopaDemoChamber(KoopaDemoChamber* p) {
    mDemoChamber = p;
}

void KoopaBattleAccessor::registerKoopaBattlePlanet(KoopaBattlePlanet* p) {
    mBattlePlanet = p;
}

void KoopaBattleAccessor::registerKoopaFinalStarPlanet(KoopaFinalStarPlanet* p) {
    mFinalStarPlanet = p;
}

void KoopaBattleAccessor::setPoseA(const LiveActor* pActor) {
    MR::makeQuatFromRotate(&mQuatA, pActor);
    mPosA.set< f32 >(pActor->mPosition);
}

void KoopaBattleAccessor::getPoseA(TQuat4f* pQuat, TVec3f* pPos) const {
    pQuat->set< f32 >(mQuatA);
    pPos->set< f32 >(mPosA);
}

void KoopaBattleAccessor::setPoseB(const LiveActor* pActor) {
    MR::makeQuatFromRotate(&mQuatB, pActor);
    mPosB.set< f32 >(pActor->mPosition);
}

void KoopaBattleAccessor::getPoseB(TQuat4f* pQuat, TVec3f* pPos) const {
    pQuat->set< f32 >(mQuatB);
    pPos->set< f32 >(mPosB);
}

void KoopaBattleAccessor::addReceiverSensor(HitSensor* pSensor) {
    mReceiverSensors[mReceiverSensorNum] = pSensor;
    mReceiverSensorNum++;
}

HitSensor* KoopaBattleAccessor::getReceiverSensor(s32 idx) const {
    if (idx >= 0 && idx < mReceiverSensorNum) {
        return mReceiverSensors[idx];
    }
    return nullptr;
}

void KoopaBattleAccessor::addAttackRockFinal(AttackRockFinal* pRock) {
    mAttackRocks[mAttackRockNum] = pRock;
    mAttackRockNum++;
}

AttackRockFinal* KoopaBattleAccessor::getAttackRockFinal(s32 idx) const {
    if (idx >= 0 && idx < mAttackRockNum) {
        return mAttackRocks[idx];
    }
    return nullptr;
}

namespace MR {
    void createKoopaBattleAccessor() {
        MR::createSceneObj(SceneObj_KoopaBattleAccessor);
    }

    KoopaBattleAccessor* getKoopaBattleAccessor() {
        if (!MR::isExistSceneObj(SceneObj_KoopaBattleAccessor)) {
            return nullptr;
        }
        return MR::getSceneObj< KoopaBattleAccessor >(SceneObj_KoopaBattleAccessor);
    }
};  // namespace MR
