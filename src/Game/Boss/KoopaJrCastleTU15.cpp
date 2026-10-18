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

namespace NrvKoopaJrCastleStateBattle4 { KoopaJrCastleStateBattle4NrvA KoopaJrCastleStateBattle4NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle4 { KoopaJrCastleStateBattle4NrvB KoopaJrCastleStateBattle4NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle4 { KoopaJrCastleStateBattle4NrvC KoopaJrCastleStateBattle4NrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle4::KoopaJrCastleStateBattle4(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x83_\x83\x81\x81[\x83W]", pHost) {}

KoopaJrCastleStateBattle4::~KoopaJrCastleStateBattle4() {}
