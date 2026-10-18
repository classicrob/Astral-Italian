#include "Game/LiveActor/CollisionObj.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

CollisionObj::CollisionObj(LiveActor* pHost, const char* pName, const char* pArcName, bool noSensor, MtxPtr pBaseMtx, int drawType)
    : LiveActor(pName), mBaseMtx(nullptr) {
    if (drawType < 0) {
        drawType = 0x1d;
    }

    MR::connectToScene(this, drawType, -1, -1, -1);

    HitSensor* pSensor = nullptr;
    if (!noSensor) {
        initHitSensor(1);
        MR::addBodyMessageSensorMapObj(this);
        pSensor = getSensor(nullptr);
    }

    mBaseMtx = pBaseMtx;
    if (pBaseMtx == nullptr) {
        mBaseMtx = pHost->getBaseMtx();
    }

    MR::invalidateClipping(this);
    MR::initCollisionPartsFromResourceHolder(this, pArcName, pSensor, MR::getResourceHolder(pHost), pBaseMtx);
}

void CollisionObj::init(const JMapInfoIter& rIter) {
    // vtable slot 0x30 tail call (SMG2)
    LiveActor::init(rIter);
}

void CollisionObj::movement() {
    LiveActor::movement();
    // vtable slot 0x1C (SMG2)
    calcAndSetBaseMtx();
}

MtxPtr CollisionObj::getBaseMtx() const {
    return mBaseMtx;
}
