#include "Game/System/GrandGalaxyMap.hpp"
#include "Game/System/WorldMapResultLocal.hpp"
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/WorldMapOwner.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Effect/ParticleCalcExecutor.hpp"
#include "Game/System/WorldMapResultStateGalaxyMap.hpp"
#include "Game/Util/ScreenUtil.hpp"

static ParticleCalcExecutor* getGalaxyMapCalc() {
#ifdef GALAXY_NATIVE
    return MR::getEffectSystem()->mCalcExec;
#else
    return *reinterpret_cast< ParticleCalcExecutor** >(reinterpret_cast< u8* >(MR::getEffectSystem()) + 0x20);
#endif
}

WorldMapResultStateGalaxyMap::WorldMapResultStateGalaxyMap(WorldMapOwner* pHost) : ActorStateBase< WorldMapOwner >("State[ギャラクシーマップ]", pHost) {
    mInt10 = -1;
    mInt14 = -1;
}

void WorldMapResultStateGalaxyMap::init() {
    initNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart), 0);
    WorldMapResultWorld* pController = static_cast< GrandGalaxyMap* >(mHost->mMainLayout)->getWorld();
    mInt10 = mHost->getCamera()->mCurrent;
    mInt14 = GrandGalaxyMap::findWorldNode(pController, mHost->getWorldId());
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::appear() {
    mIsDead = false;
    mInt10 = mHost->getCamera()->mCurrent;
    mHost->getMainLayout()->warpRocketToNode(mInt14);
    MR::tryForceDeleteEffectAll(mHost->getRocket());
    mHost->getMainLayout()->activate();
    mHost->finishWorld();
    setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapStart));
}

void WorldMapResultStateGalaxyMap::kill() {
    mIsDead = true;
}

void WorldMapResultStateGalaxyMap::control() {
    mHost->getMainLayout()->update();
}

WorldMapResultStateGalaxyMap::~WorldMapResultStateGalaxyMap() {
}

void WorldMapResultStateGalaxyMap::exeStart() {
    MR::setStageBGMStateBit(0x100);
    if (mHost->getMainLayout()->isWorldWarp() != 0) {
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapWait));
    }
}

void WorldMapResultStateGalaxyMap::exeWait() {
    if (mHost->getMainLayout()->isWorldWarpEnd() != 0) {
        mHost->getMainLayout()->deactivate();
        mHost->prepareWorld();
        mInt14 = mHost->getCamera()->mCurrent;
        mHost->warpRocketToNode(mInt10);
        setNerve(GET_NERVE(WorldMapResultStateGalaxyMap, WorldMapResultStateGalaxyMapEnd));
    }
}

void WorldMapResultStateGalaxyMap::exeEnd() {
    for (s32 i = 0; i < 15; i++) {
        getGalaxyMapCalc()->movementCheckUpdate();
        getGalaxyMapCalc()->movementNormal();
    }

    if (MR::isGreaterEqualStep(this, 6) && MR::tryEndModelWipe()) {
        kill();
    }
}

namespace NrvWorldMapResultStateGalaxyMap {
    WorldMapResultStateGalaxyMapStart WorldMapResultStateGalaxyMapStart::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateGalaxyMapWait WorldMapResultStateGalaxyMapWait::sInstance ATTRIBUTE_WEAK;
    WorldMapResultStateGalaxyMapEnd WorldMapResultStateGalaxyMapEnd::sInstance ATTRIBUTE_WEAK;

    void WorldMapResultStateGalaxyMapEnd::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateGalaxyMap*>(pSpine->mExecutor)->exeEnd();
    }

    void WorldMapResultStateGalaxyMapWait::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateGalaxyMap*>(pSpine->mExecutor)->exeWait();
    }

    void WorldMapResultStateGalaxyMapStart::execute(Spine* pSpine) const {
        reinterpret_cast<WorldMapResultStateGalaxyMap*>(pSpine->mExecutor)->exeStart();
    }
};
