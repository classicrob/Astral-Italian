#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/MapObj/StarPieceScatter.hpp"
#include "Game/System/MiniRoutePoint.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/Effect/PlayerRocketSmokeMoveParticleCallBack.hpp"
#include "Game/Effect/PlayerRocketSmokeWaitParticleCallBack.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"


// ---- MiniWorldMapMark ----

MiniWorldMapMark::MiniWorldMapMark(LiveActor* pOwner, const char* pName) : LiveActor("ミニ跡地"), mOwner(pOwner), mName(pName) {}

void MiniWorldMapMark::init(const JMapInfoIter&) {
    if (!MR::initActor(this, mName, false)) {
        MR::connectToSceneWorldMapMiniObj(this);
    }
    MR::setClippingFarMax(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    MR::copyTransRotateScale(mOwner, this);
    makeActorDead();
}

MiniWorldMapMark::~MiniWorldMapMark() {}
