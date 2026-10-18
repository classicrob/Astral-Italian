#pragma auto_inline off
#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

static const char* sAppearActionName = "MiniGreenComet";

MiniGreenComet::MiniGreenComet() : LiveActor("グリーン") {
}

void MiniGreenComet::init(const JMapInfoIter& rIter) {
    s32 result = MR::initActor(this, rIter, "MiniGreenComet", false);
    if (result == 0) {
        MR::connectToSceneWorldMapMiniObj(this);
        initEffectKeeper(8, nullptr, false);
    }

    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    if (GameSequenceFunction::isGreenStarEventOnly()) {
        initSound(4, nullptr, nullptr, TVec3f(0.0f));
    }
    makeActorDead();
}

void MiniGreenComet::appear() {
    MR::startAction(this, sAppearActionName);
    LiveActor::appear();
}

void MiniGreenComet::control() {
    if (MR::isActionEnd(this)) {
        kill();
    }
}

MiniGreenComet::~MiniGreenComet() {
}

MiniGreenCometShower::MiniGreenCometShower() : LiveActor("降り注ぐ") {
}

void MiniGreenCometShower::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    initEffectKeeper(8, "MiniGreenCometShower", false);
    if (GameSequenceFunction::isGreenStarEventOnly()) {
        initSound(4, "MiniGreenCometShower", nullptr, TVec3f(0.0f));
    }
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorDead();
}

void MiniGreenCometShower::appear() {
    LiveActor::appear();
    MR::emitEffect(this, "Shower");
    MR::startActionSound(this, "Shower", -1, -1, -1);
}

MiniGreenCometShower::~MiniGreenCometShower() {
}
