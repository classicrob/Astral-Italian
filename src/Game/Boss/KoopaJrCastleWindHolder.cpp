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

namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvA KoopaJrCastleWindNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvB KoopaJrCastleWindNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvC KoopaJrCastleWindNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvD KoopaJrCastleWindNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleWind { KoopaJrCastleWindNrvE KoopaJrCastleWindNrvE::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleWind::KoopaJrCastleWind(KoopaJrCastle* pHost, const char* pName) : LiveActor(pName) {
    _pad[0] = 0;
}

KoopaJrCastleWind::~KoopaJrCastleWind() {}

KoopaJrCastleWindHolder::KoopaJrCastleWindHolder() : LiveActorGroup("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x95\x97\x83z\x83\x8b\x83_\x81[", 0x20) {}

KoopaJrCastleWindHolder::~KoopaJrCastleWindHolder() {}
