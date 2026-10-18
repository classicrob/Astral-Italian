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

namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvA KoopaJrCastleStateBattle2NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvB KoopaJrCastleStateBattle2NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvC KoopaJrCastleStateBattle2NrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvD KoopaJrCastleStateBattle2NrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvE KoopaJrCastleStateBattle2NrvE::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle2 { KoopaJrCastleStateBattle2NrvF KoopaJrCastleStateBattle2NrvF::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle2::KoopaJrCastleStateBattle2(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82Q\x92i\x8aK]", pHost) {}

KoopaJrCastleStateBattle2::~KoopaJrCastleStateBattle2() {}
