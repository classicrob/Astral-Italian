#include "Game/MapObj/MorphItemRock.hpp"
#include "Game/Ride/RollingRock.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
MorphItemRock::MorphItemRock(const char*) : MorphItemObjNeo("岩オブジェ") { mRock = new RollingRock("岩マリオ"); }
void MorphItemRock::init(const JMapInfoIter& iter) { mRock->init(iter); MorphItemObjNeo::init(iter); }
void MorphItemRock::emitAppearEffects() {
    MR::emitEffect(this, "AppearRock");
    MR::startActionSound(this, "OjMorphItemAppear");
}
void MorphItemRock::appearTransformation() { mRock->makeActorAppeared(); }
void MorphItemRock::killTransformation() {
    HitSensor* receiver = mRock->getSensor("Body");
    mRock->receiveOtherMsg(0x95, getSensor("Body"), receiver);
    mRock->makeActorDead();
}
int MorphItemRock::getPowerUp() { return 8; }
