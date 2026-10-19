#include "Game/System/GrandGalaxyMapStateWorldWarp.hpp"
#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvGrandGalaxyMapStateWorldWarp {
    NEW_NERVE(GrandGalaxyMapStateWorldWarpWarp, GrandGalaxyMapStateWorldWarp, Warp);
}

GrandGalaxyMapStateWorldWarp::GrandGalaxyMapStateWorldWarp(GrandGalaxyMap* pHost)
    : ActorStateBase<GrandGalaxyMap>("State[ワールドワープ]", pHost), mWarpParam0(-1), mWarpParam1(-1), mFovy(0.0f) {
}

void GrandGalaxyMapStateWorldWarp::init() {
    initNerve(&NrvGrandGalaxyMapStateWorldWarp::GrandGalaxyMapStateWorldWarpWarp::sInstance, 0);
    mIsDead = true;
}

void GrandGalaxyMapStateWorldWarp::appear() {
    mIsDead = false;
    setNerve(&NrvGrandGalaxyMapStateWorldWarp::GrandGalaxyMapStateWorldWarpWarp::sInstance);
    MR::startGalaxyWipeForward();
    mFovy = MR::getFovy();
}

// NONMATCHING: the truthful u8 stage-request parameter adds a clrlwi for the
// stored world ID. Target passes the full word; valid world IDs fit in one byte.
void GrandGalaxyMapStateWorldWarp::exeWarp() {
    if (MR::isFirstStep(this) && mWarpParam0 != GameDataFunction::getCurrentWorldId()) {
        MR::stopStageBGM(30);
        MR::stopSubBGM(0);
    }

    f32 max = mFovy;
    s32 frames = 30;
    f32 fovy = MR::getEaseInValue((f32)getNerveStep() / (f32)frames, mFovy, 0.01f, 1.0f);
    MR::setFovy(MR::clamp(fovy, 0.01f, max));

    if (MR::isGreaterEqualStep(this, 30)) {
        if (mWarpParam0 == GameDataFunction::getCurrentWorldId()) {
            kill();
        } else {
            GameSequenceFunction::setWorldMapCurrentNode(mWarpParam1);
            GameSequenceFunction::requestChangeStageWorldMapEnterType4(mWarpParam0);
        }
    }
}

GrandGalaxyMapStateWorldWarp::~GrandGalaxyMapStateWorldWarp() {
}
