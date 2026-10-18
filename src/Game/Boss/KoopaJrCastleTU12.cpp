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

namespace NrvKoopaJrCastleStateBattle1 { KoopaJrCastleStateBattle1NrvA KoopaJrCastleStateBattle1NrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle1 { KoopaJrCastleStateBattle1NrvB KoopaJrCastleStateBattle1NrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateBattle1 { KoopaJrCastleStateBattle1NrvC KoopaJrCastleStateBattle1NrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateBattle1::KoopaJrCastleStateBattle1(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82P\x92i\x8aK]", pHost) {}

KoopaJrCastleStateBattle1::~KoopaJrCastleStateBattle1() {}
