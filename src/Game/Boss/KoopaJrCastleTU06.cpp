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

namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvA KoopaJrCastleFireBarNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvB KoopaJrCastleFireBarNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvC KoopaJrCastleFireBarNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleFireBar::KoopaJrCastleFireBar(const char* pName) : LiveActor(pName) {
    mBody = nullptr;
    mMtxs = nullptr;
    mChildren = nullptr;
    mChildCount = 0;
    _A4 = 0.0f;
    _A8 = 0.0f;
    _AC = 1.0f;
    _B0 = 0.0f;
    _B4 = -0.5f;
    _B8 = 3;
    _BC = 1.0f;
    _C0 = 650.0f;
    _C4 = 0.0f;
    _C8 = 3000.0f;
    _CC = true;
}

void KoopaJrCastleFireBar::makeActorAppeared() {
    for (s32 i = 0; i < mChildCount; i++) {
        mChildren[i]->makeActorAppeared();
        MR::emitEffect(mChildren[i], "Move");
    }
    LiveActor::makeActorAppeared();
    setNerve(&NrvKoopaJrCastleFireBar::KoopaJrCastleFireBarNrvA::sInstance);
    _C4 = 0.0f;
    _B0 = 0.0f;
    _BC = 1.0f;
    _CC = true;
}

void KoopaJrCastleFireBar::makeActorDead() {
    for (s32 i = 0; i < mChildCount; i++) {
        mChildren[i]->makeActorDead();
    }
    LiveActor::makeActorDead();
}

// NONMATCHING: clamp branch layout: target uses cror/fcmpu for the >=/== compares, ours uses bge/fcmpo
void KoopaJrCastleFireBar::exe800D2B50() {
    f32 temp = _C4 + 25.0f;
    f32 v = 0.0f;
    f32 max = _C8;
    _C4 = temp;
    if (temp >= v) {
        if (temp > max) {
            v = max;
        } else {
            v = temp;
        }
    }
    _C4 = v;
    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);
    if (_C4 == _C8) {
        setNerve(&NrvKoopaJrCastleFireBar::KoopaJrCastleFireBarNrvB::sInstance);
    }
}

void KoopaJrCastleFireBar::exe800D2BF0() {
    MR::isFirstStep(this);
    _BC = MR::calcNerveEaseInRate(this, 60);
    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);
}

void KoopaJrCastleFireBar::fn_800D2C50() {
    _C4 = _C8;
}

void KoopaJrCastleFireBar::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackFire(pReceiver, pSender);
    }
}

KoopaJrCastleFireBar::~KoopaJrCastleFireBar() {}
