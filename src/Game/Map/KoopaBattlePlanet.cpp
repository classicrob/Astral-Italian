#include "Game/Map/KoopaBattlePlanet.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/MapUtil.hpp"

KoopaBattlePlanet::KoopaBattlePlanet(const char* pName) : PlanetMap(pName, nullptr) {}

void KoopaBattlePlanet::init(const JMapInfoIter& rIter) {
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->registerKoopaBattlePlanet(this);
    mChildGroup = MR::initChildObjGroup(this, "\x8E\x71\x8B\x9F\x8A\xC7\x97\x9D", rIter);  // "子供管理"
    PlanetMap::init(rIter);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);
    makeActorAppeared();
}

void KoopaBattlePlanet::initAfterPlacement() {
    NameObj::initAfterPlacement();
    makeActorDead();
}

void KoopaBattlePlanet::makeActorAppeared() {
    PlanetMap::makeActorAppeared();
    if (mChildGroup != nullptr) {
        mChildGroup->appearAll();
    }
}

void KoopaBattlePlanet::makeActorDead() {
    PlanetMap::makeActorDead();
    if (mChildGroup != nullptr) {
        mChildGroup->makeAllDead();
    }
}

KoopaBattlePlanet::~KoopaBattlePlanet() {}
