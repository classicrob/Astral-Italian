#include "Game/MapObj/ConstructionLight.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
namespace NrvConstructionLight {
NEW_NERVE(ConstructionLightNrvNormal, ConstructionLight, Normal);
NEW_NERVE(ConstructionLightNrvBreak, ConstructionLight, Break);
}
ConstructionLight::ConstructionLight(const char* name) : LiveActor(name), mDropsCoin(false) {}
void ConstructionLight::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    MR::getJMapInfoArg0NoInit(iter, &mDropsCoin);
    if (mDropsCoin) MR::declareCoin(this, 1);
    else MR::declareStarPiece(this, 3);
    initNerve(&NrvConstructionLight::ConstructionLightNrvNormal::sInstance);
    makeActorAppeared();
}
void ConstructionLight::appear() { LiveActor::appear(); MR::emitEffect(this, "Appear"); MR::startActionSound(this, "Appear"); }
void ConstructionLight::exeNormal() {
    if (MR::isFirstStep(this)) { MR::startAction(this, "Normal"); MR::setBvaFrameAndStop(this, 0.0f); }
    MR::attachSupportTicoToTarget(this);
}
void ConstructionLight::exeBreak() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Break");
        MR::tryHitReaction(this, "壊れリアクション");
        MR::setBvaFrameAndStop(this, 0.0f);
        MR::invalidateCollisionParts(this);
        if (mDropsCoin) { MR::appearCoinPop(this, mPosition, 1); return; }
        MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false);
    }
}
bool ConstructionLight::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor* receiver) {
    if (isNerve(&NrvConstructionLight::ConstructionLightNrvBreak::sInstance)) return false;
    if (MR::isEqualSensor(receiver, this, "Body")) return false;
    if (MR::isMsgStarPieceAttack(msg) || MR::isMsgPlayerSpinAttackOrSupportTico(msg) || MR::isMsgDrillAttack(msg) || MR::isMsgRockAttack(msg)) {
        setNerve(&NrvConstructionLight::ConstructionLightNrvBreak::sInstance);
        return true;
    }
    return false;
}
ConstructionLight::~ConstructionLight() {}
