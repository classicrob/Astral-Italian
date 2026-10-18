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

namespace NrvKoopaJrCastleWindUp { KoopaJrCastleWindUpNrvA KoopaJrCastleWindUpNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWindUp { KoopaJrCastleWindUpNrvB KoopaJrCastleWindUpNrvB::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleWindUp::KoopaJrCastleWindUp(const char* pName) : LiveActor(pName), mModel(nullptr), mLength(0.0f) {
    PSMTXIdentity(mMtx);
}

KoopaJrCastleWindUp::~KoopaJrCastleWindUp() {}

void KoopaJrCastleWindUp::makeActorAppeared() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffect[i]->makeActorAppeared();
    }
    LiveActor::makeActorAppeared();
}

void KoopaJrCastleWindUp::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvA::sInstance);
}

void KoopaJrCastleWindUp::makeActorDead() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffect[i]->makeActorDead();
    }
    LiveActor::makeActorDead();
}

bool KoopaJrCastleWindUp::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xF4) {
        setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvB::sInstance);
        return true;
    }
    return false;
}
