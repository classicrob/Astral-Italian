#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/Util.hpp"

MiniWorldMapMark::MiniWorldMapMark(LiveActor* pOwner, const char* pName) : LiveActor("ミニ跡地"), mOwner(pOwner), mName(pName) {}

MiniWorldMapMark::~MiniWorldMapMark() {}

void MiniWorldMapMark::init(const JMapInfoIter&) {
    if (!MR::initActor(this, mName, false)) {
        MR::connectToSceneWorldMapMiniObj(this);
    }
    MR::setClippingFarMax(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    MR::copyTransRotateScale(mOwner, this);
    makeActorDead();
}

extern "C" bool fn_804E5A10(const char* pName) {
    return MR::isEqualString(pName, "StarCheckPointGrand");
}
