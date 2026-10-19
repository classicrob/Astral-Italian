#pragma auto_inline off
#include "Game/Boss/KoopaJrCastleNerves.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

namespace NrvKoopaJrCastleElectric { KoopaJrCastleElectricNrvA KoopaJrCastleElectricNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleElectric { KoopaJrCastleElectricNrvB KoopaJrCastleElectricNrvB::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleCapsule::KoopaJrCastleCapsule(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleCapsule", 0, 0x13, false) {}

KoopaJrCastleCapsule::~KoopaJrCastleCapsule() {}

bool KoopaJrCastleCapsule::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mHost->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool KoopaJrCastleCapsule::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mHost->receiveOtherMsg(msg, pSender, pReceiver);
}

void KoopaJrCastleCapsule::init(const JMapInfoIter& rIter) {
    MR::startBrk(this, "Wait");
    MR::startBva(this, "Normal");
    makeActorAppeared();
}

KoopaJrCastleElectric::KoopaJrCastleElectric(const char* pName) : LiveActor(pName) {}

KoopaJrCastleElectric::~KoopaJrCastleElectric() {}

void KoopaJrCastleElectric::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    initNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvB::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrCastleElectric::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvA::sInstance);
    KoopaJrCastleFunction::getCastle();
    KoopaJrCastleFunction::getCastle();
    KoopaJrCastleFunction::validateBodyAndHeadSensors();
}

void KoopaJrCastleElectric::makeActorDead() {
    KoopaJrCastleFunction::getCastle();
    LiveActor* pHead = KoopaJrCastleFunction::getCastle()->mHead;
    MR::deleteEffect(pHead, "LeakSign");
    MR::deleteEffect(pHead, "Leak");
    LiveActor::makeActorDead();
}

void KoopaJrCastleElectric::exeDischargeSign() {
    LiveActor* head = KoopaJrCastleFunction::getCastle()->mHead;
    if (!MR::isEffectValid(head, "LeakSign")) MR::emitEffect(head, "LeakSign");
    MR::startActionSound(head, "EmLvElecbazPreElecLeak");
    if (MR::isGreaterStep(this, 60)) {
        MR::deleteEffect(head, "LeakSign");
        setNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvB::sInstance);
    }
}

void KoopaJrCastleElectric::exeDischarge() {
    LiveActor* head = KoopaJrCastleFunction::getCastle()->mHead;
    if (!MR::isEffectValid(head, "Leak")) MR::emitEffect(head, "Leak");
    MR::startActionSound(head, "EmLvElecbazElecLeak");
    if (MR::isGreaterStep(this, 330)) {
        MR::deleteEffect(head, "Leak");
        kill();
    }
}

bool KoopaJrCastleElectric::isElectricDischarging() {
    if (!MR::isDead(this) && isNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvB::sInstance)) {
        return true;
    }
    return false;
}
