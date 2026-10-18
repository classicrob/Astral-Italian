#pragma auto_inline off
#include "Game/MapObj/WanwanLiftUpStep.hpp"
#include "Game/Util.hpp"
namespace {
const char* sStartEffects[] = {"SwitchStartA", "SwitchStartB"};
const char* sEndEffects[] = {"SwitchEndA", "SwitchEndB"};
}
WanwanLiftUpStep::WanwanLiftUpStep(const char* name) : RailMoveObj(name), mEffectType(0) {}
void WanwanLiftUpStep::init(const JMapInfoIter& iter) { RailMoveObj::init(iter); MR::getJMapInfoArg0NoInit(iter, &mEffectType); }
void WanwanLiftUpStep::initCaseUseSwitchB(const MapObjActorInitInfo&) {
    MR::listenStageSwitchOnOffB(this, MR::Functor(this, &WanwanLiftUpStep::listenSwitchOnB), MR::Functor(this, &WanwanLiftUpStep::listenSwitchOffB));
    setNerveWait();
}
void WanwanLiftUpStep::listenSwitchOnB() { startMapPartsFunctions(); MR::emitEffect(this, sStartEffects[mEffectType]); }
void WanwanLiftUpStep::listenSwitchOffB() { endMapPartsFunctions(); MR::emitEffect(this, sEndEffects[mEffectType]); }
WanwanLiftUpStep::~WanwanLiftUpStep() {}
