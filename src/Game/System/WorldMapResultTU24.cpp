#include "Game/System/GameSequenceFunction.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/WorldMapResultStateWorldWarp.hpp"

namespace MR {
    void onDestroySceneKeepAllSound();
    void closeSystemWipeWhiteFade(s32);
    f32 calcNerveEaseInOutRate(const NerveExecutor*, s32);
}


WorldMapResultStateWorldWarp::WorldMapResultStateWorldWarp(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ワールドワープ]", pHost) {
    mInt10 = -1;
    mInt14 = -1;
    mInt18 = -1;
    mPosA.x = 0.0f;
    mPosA.y = 0.0f;
    mPosA.z = 0.0f;
    mPosB.x = 0.0f;
    mPosB.y = 0.0f;
    mPosB.z = 0.0f;
    mDir.x = 0.0f;
    mDir.y = 0.0f;
    mDir.z = 0.0f;
}

void WorldMapResultStateWorldWarp::init() {
    initNerve(GET_NERVE(WorldMapResultStateWorldWarp, WorldMapResultStateWorldWarpMove), 0);
    mIsDead = true;
}

void WorldMapResultStateWorldWarp::appear() {
    mHost->startDemo();
    mIsDead = false;
    setNerve(GET_NERVE(WorldMapResultStateWorldWarp, WorldMapResultStateWorldWarpMove));
}

WorldMapResultStateWorldWarp::~WorldMapResultStateWorldWarp() {
}

void WorldMapResultStateWorldWarp::exeMove() {
    LiveActor* pActor = mHost->getCamera()->mRocket;
    if (MR::isFirstStep(this)) {
        MR::stopStageBGM(30);
        MR::startActionSound(pActor, "SyWMapBtwWorld", -1, -1, -1);
        MR::onDestroySceneKeepAllSound();
        mPosA.set(pActor->mPosition);
        mPosB.set(mHost->getWorld()->getNode(mInt18)->mPos);

        TVec3f diff(mPosB);
        diff -= mPosA;
        mDir.set(diff);
        MR::normalizeOrZero(&mDir);

        TVec3f scaled = mDir * 9000.0f;
        mPosB.set(mPosA + scaled);
        mHost->startRocketMoveSound();
    }

    if (MR::isStep(this, 15)) {
        MR::closeSystemWipeWhiteFade(15);
    }

    f32 rate = MR::calcNerveEaseInOutRate(this, 30);
    f32 inv = 1.0f - rate;

    TVec3f from = mPosA * inv;
    TVec3f to = mPosB * rate;
    TVec3f pos = to + from;
    mHost->setRocketPos(pos, mDir);

    if (MR::isGreaterEqualStep(this, 30)) {
        mHost->disappearWorld();
        GameSequenceFunction::setWorldMapCurrentNode(mInt14);
        GameSequenceFunction::requestChangeStageWorldMapMoveToWorld(mInt10);
    }
}

namespace NrvWorldMapResultStateWorldWarp {
    WMR_NERVE_DEF(WorldMapResultStateWorldWarpMove, WorldMapResultStateWorldWarp, Move);
};  