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

namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvA KoopaJrCastleStateBattle3NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvB KoopaJrCastleStateBattle3NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvC KoopaJrCastleStateBattle3NrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvD KoopaJrCastleStateBattle3NrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvE KoopaJrCastleStateBattle3NrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvF KoopaJrCastleStateBattle3NrvF::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvG KoopaJrCastleStateBattle3NrvG::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvH KoopaJrCastleStateBattle3NrvH::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvI KoopaJrCastleStateBattle3NrvI::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvJ KoopaJrCastleStateBattle3NrvJ::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvK KoopaJrCastleStateBattle3NrvK::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvL KoopaJrCastleStateBattle3NrvL::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvM KoopaJrCastleStateBattle3NrvM::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle3 { KoopaJrCastleStateBattle3NrvN KoopaJrCastleStateBattle3NrvN::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle3::KoopaJrCastleStateBattle3(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82R\x92i\x8aK]", pHost) {
    _pad[0] = 0;
}

KoopaJrCastleStateBattle3::~KoopaJrCastleStateBattle3() {}
