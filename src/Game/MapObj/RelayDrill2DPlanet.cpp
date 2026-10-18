#include "Game/MapObj/RelayDrill2DPlanet.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
#include "Game/Util.hpp"
RelayDrill2DPlanet::RelayDrill2DPlanet(const char* name) : MapObjActor(name) {}
void RelayDrill2DPlanet::init(const JMapInfoIter& iter) { MapObjActor::init(iter); MapObjActorInitInfo info; MapObjActorUtil::setupInitInfoPlanet(&info); initialize(iter, info); MR::invalidateCollisionParts(this); }
void RelayDrill2DPlanet::initCaseUseSwitchA(const MapObjActorInitInfo&) { MR::listenStageSwitchOnA(this, MR::Functor_Inline(this, &RelayDrill2DPlanet::validateCollision)); }
void RelayDrill2DPlanet::validateCollision() { MR::validateCollisionParts(this); }
RelayDrill2DPlanet::~RelayDrill2DPlanet() {}
