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

namespace NrvKoopaJrCastleDominate { KoopaJrCastleDominateNrvA KoopaJrCastleDominateNrvA::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleDominate::KoopaJrCastleDominate(const char* pName) : LiveActor(pName) {
    _90 = nullptr;
    _94 = 0.0f;
    _98 = NAN;
    _9C = 0.0f;
    _A0 = 0.0f;
    _A4 = 0.0f;
}

KoopaJrCastleDominate::~KoopaJrCastleDominate() {}
