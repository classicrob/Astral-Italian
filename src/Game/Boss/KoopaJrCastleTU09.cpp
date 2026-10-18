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

namespace NrvKoopaJrCastleStateOpening { KoopaJrCastleStateOpeningNrvA KoopaJrCastleStateOpeningNrvA::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateOpening { KoopaJrCastleStateOpeningNrvB KoopaJrCastleStateOpeningNrvB::sInstance ATTRIBUTE_WEAK; }
namespace NrvKoopaJrCastleStateOpening { KoopaJrCastleStateOpeningNrvC KoopaJrCastleStateOpeningNrvC::sInstance ATTRIBUTE_WEAK; }

KoopaJrCastleStateOpening::KoopaJrCastleStateOpening(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("Phase[\x83I\x81[\x83v\x83j\x83\x93\x83O]", pHost) {}

KoopaJrCastleStateOpening::~KoopaJrCastleStateOpening() {}
