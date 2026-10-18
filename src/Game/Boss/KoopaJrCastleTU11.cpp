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

namespace NrvKoopaJrCastleRingBeam { KoopaJrCastleRingBeamNrvPre KoopaJrCastleRingBeamNrvPre::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleRingBeam { KoopaJrCastleRingBeamNrvA KoopaJrCastleRingBeamNrvA::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleRingBeam::KoopaJrCastleRingBeam(const char* pName) : LiveActor(pName) {
    _90 = nullptr;
    _94 = 0.0f;
}

KoopaJrCastleRingBeam::~KoopaJrCastleRingBeam() {}
