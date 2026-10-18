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

namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvA KoopaJrCastleStateBattleNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvB KoopaJrCastleStateBattleNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvC KoopaJrCastleStateBattleNrvC::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvD KoopaJrCastleStateBattleNrvD::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle { KoopaJrCastleStateBattleNrvE KoopaJrCastleStateBattleNrvE::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle::KoopaJrCastleStateBattle(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("Phase[\x90\xed\x93\xac]", pHost) {
    mPhaseState[0] = nullptr;
    mPhaseState[1] = nullptr;
    mPhaseState[2] = nullptr;
    mPhaseState[3] = nullptr;
    mPhase = 3;
}

KoopaJrCastleStateBattle::~KoopaJrCastleStateBattle() {}
