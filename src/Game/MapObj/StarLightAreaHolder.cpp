#pragma auto_inline off
#include "Game/MapObj/StarLightAreaHolder.hpp"
#include "Game/MapObj/ClipArea.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
StarLightAreaHolder::StarLightAreaHolder(const char* name) : DeriveActorGroup<ClipArea>(name,64), mReceivers(), mReceiverCount(0) {
    mReceivers.init(256);
}
// NONMATCHING: target retains an impossible null check on the array member address.
StarLightAreaHolder::~StarLightAreaHolder() {}
void StarLightAreaHolder::init(const JMapInfoIter&) { MR::connectToSceneAreaObj(this); }
void StarLightAreaHolder::movement() {
    const s32 count = mReceiverCount;
    for (s32 i=0; i<count; i++) {
        HitSensor* sensor = mReceivers[i];
        if (isInStarLight(*MR::getSensorPos(sensor))) MR::sendArbitraryMsg(0xCD,sensor,MR::getMessageSensor());
        else MR::sendArbitraryMsg(0xCE,sensor,MR::getMessageSensor());
    }
}
bool StarLightAreaHolder::isInStarLight(const TVec3f& position) {
    const s32 count = getObjNum();
    for (s32 i=0; i<count; i++) {
        if (!MR::isDead(getMember(i)) && getMember(i)->isInArea(position)) return true;
    }
    return false;
}
bool StarLightAreaHolder::tryRegisterReceiver(HitSensor* sensor) {
    if (!MR::isInAreaObj("ClipDrawArea",*MR::getSensorPos(sensor))) return false;
    mReceivers[mReceiverCount++] = sensor;
    return true;
}
namespace MR {
    void registerStarLightArea(ClipArea* actor) {
        if (!isExistSceneObj(SceneObj_StarLightAreaHolder)) createSceneObj(SceneObj_StarLightAreaHolder);
        getSceneObj<StarLightAreaHolder*>(SceneObj_StarLightAreaHolder)->registerActor(actor);
    }
    bool tryRegisterStarLightReceiver(HitSensor* sensor) {
        if (!isExistSceneObj(SceneObj_StarLightAreaHolder)) createSceneObj(SceneObj_StarLightAreaHolder);
        return getSceneObj<StarLightAreaHolder*>(SceneObj_StarLightAreaHolder)->tryRegisterReceiver(sensor);
    }
}
template MR::AssignableArray<HitSensor*>::~AssignableArray();
