#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/System/WorldMapResultLocal.hpp"

WorldMapResultCommand::WorldMapResultCommand() : NameObj("ワールドマップ（リザルト指揮）") {
    mWorldNum = 7;
    mWorlds = nullptr;
    mController = nullptr;
    mDraw = nullptr;
    mRocket = nullptr;
    mActor = nullptr;
}

void WorldMapResultCommand::init(const JMapInfoIter&) {
    mWorlds = new WorldMapResultWorld*[mWorldNum];
    for (s32 i = 0; i < mWorldNum; i++) {
        s32 world = i + 1;
        if (isWorldEnabled(world)) {
            mWorlds[i] = new WorldMapResultWorld(world);
            mWorlds[i]->initWorld(0);
            mWorlds[i]->setNum(9);
        } else {
            mWorlds[i] = nullptr;
        }
    }
    mDraw = new WorldMapResultDraw();
    mDraw->initWithoutIter();
    mController = new WorldMapResultController(this);
    mController->init();
    mRocket = new LiveActor("顔惑星");
    MR::initActor(mRocket, "MiniPlayerRocket", "ResultMap", false);
    mRocket->initWithoutIter();
    mRocket->makeActorDead();
}

WorldMapResultCommand::~WorldMapResultCommand() {
}

