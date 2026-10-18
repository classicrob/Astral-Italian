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
#include "Game/Util/FixedPosition.hpp"

namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvA KoopaJrCastleFireBarNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvB KoopaJrCastleFireBarNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleFireBar { KoopaJrCastleFireBarNrvC KoopaJrCastleFireBarNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleFireBar::KoopaJrCastleFireBar(const char* pName) : LiveActor(pName) {
    mFixedPos = nullptr;
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

void KoopaJrCastleFireBar::setCurrentToMax() {
    _C4 = _C8;
}

// fn_800D2AB0: fixed-position helper bound to the host; a name selects the named variant.
void KoopaJrCastleFireBar::setup(LiveActor* pHost, const TVec3f& rPos, const TVec3f& rOffset, const char* pName) {
    if (pName != nullptr) {
        mFixedPos = new FixedPosition(pHost, pName, rPos, rOffset);
    } else {
        mFixedPos = new FixedPosition(pHost, rPos, rOffset);
    }
}

// fn_800D2C60: request movement on every child part.
void KoopaJrCastleFireBar::requestMovementOnChildren() {
    for (s32 i = 0; i < mChildCount; i++) {
        MR::requestMovementOn(mChildren[i]);
    }
}

// fn_800D2CC0: shadow every child except one per group of _B8 (group size mChildCount / _B8).
void KoopaJrCastleFireBar::updateChildShadows() {
    MR::invalidateShadowAll(this);
    s32 lastInGroup;
    s32 groupSize = mChildCount / _B8;
    lastInGroup = groupSize - 1;
    for (s32 i = 0; i < mChildCount; i++) {
        if (lastInGroup != i % groupSize) {
            MR::validateShadowAll(mChildren[i]);
        } else {
            MR::invalidateShadowAll(mChildren[i]);
        }
    }
}

void KoopaJrCastleFireBar::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackFire(pReceiver, pSender);
    }
}

KoopaJrCastleFireBar::~KoopaJrCastleFireBar() {}
