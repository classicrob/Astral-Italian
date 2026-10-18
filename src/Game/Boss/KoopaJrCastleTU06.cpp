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
    _98.x = 0.0f;
    _98.y = 0.0f;
    _98.z = 0.0f;
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

KoopaJrCastleFireBar::~KoopaJrCastleFireBar() {}
